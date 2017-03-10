#include <ansi_c.h>
#include <userint.h>
#include "gci_utils.h"
#include "gci_ui_logger.h"

#include <windows.h> 
#include <utility.h>
#include <commctrl.h>

#include "ThreadDebug.h"

// There is only one logger, let all ui modules talk to it, and so need the ref count of modules using it
// Each method has a *ext_logger input because initally more than one logger was to be allowed
// this has ramained for backward compatability, but is ignored. It should be the case that ext_logger=logger
static Logger *logger = NULL;
static int ref_count = 0;

// Function that creates the list view window.
#define ID_LISTVIEW 1001
#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT  16
#define MAX_ITEMLEN 1000

#define LOGGED_MSG (WM_USER+2)      

//#define FILE_LOG_NAME "Log_%d.txt"

typedef struct 
{
	char msg[1000];
	LOGGER_PRIORITY priority;
	
} MessageInfo;

static MessageInfo infos[10];   
static int current_info = 0;

static int logger_get_lock(Logger *ext_logger)   // There is only one logger really, ext_logger = logger
{
	return GciCmtGetLock(logger->lock);
}

static int logger_release_lock(Logger *ext_logger)   // There is only one logger really, ext_logger = logger
{
	return GciCmtReleaseLock(logger->lock);
}

HWND CreateListView (HWND hWndParent)                                     
{
	HWND list;      // Handle to the list view window.
	RECT rcl;       // Rectangle for setting the size of the window.
	HICON hIcon;        // Handle to an icon.
	HIMAGELIST hSmall; // Handles to image lists for large and small     
	LV_COLUMN lvC;      // List view column structure.

   	HINSTANCE hInst = GetModuleHandle(NULL);

	// Ensure that the common window DLL is loaded.
	InitCommonControls();

	// Get the size and position of the parent window.
	GetClientRect(hWndParent, &rcl);

	// Create the list view window.
	list = CreateWindowEx(0L,
						  WC_LISTVIEW,                // List view class.
      					  "",                         // No default text.
      					  WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT, // Window styles.
      					  10, 40,                       // Default x and y position.
      					  rcl.right - rcl.left - 20, rcl.bottom - rcl.top - 50, // Width and height of window.
      				 	  hWndParent,                 // Parent window.
      					  (HMENU) ID_LISTVIEW,        // Window ID.
						  hInst,                      // Current instance.
						  NULL );                     // No application-defined data.

	if (list == NULL )
		return NULL;
														  
	// Initialize the list view window
	// First, initialize the image lists we will need
	// to create an image list for the small icons.
	hSmall = ImageList_Create ( BITMAP_WIDTH, BITMAP_HEIGHT, ILC_COLOR16|ILC_MASK, 3, 0 );

	// Load the icons and add them to the image lists.
	hIcon = LoadIcon ( NULL, IDI_INFORMATION);

    if ((ImageList_AddIcon(hSmall, hIcon) == -1))
		return NULL;
	  
	hIcon = LoadIcon ( NULL, IDI_WARNING);

    if ((ImageList_AddIcon(hSmall, hIcon) == -1))
       return NULL;
	
	hIcon = LoadIcon ( NULL, IDI_ERROR);

    if ((ImageList_AddIcon(hSmall, hIcon) == -1))
       return NULL;

   	// Make sure that all of the icons are in the lists.
	if (ImageList_GetImageCount(hSmall) < 3)
    	return FALSE;


	// Associate lists with list view window.
	ListView_SetImageList(list, hSmall, LVSIL_SMALL);

	// Now initialize the columns we will need.
	// Initialize the LV_COLUMN structure.
	// The mask specifies that the .fmt, .ex, width, and .subitem members 
	// of the structure are valid.
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // Left-align the column.
	lvC.cx = 500;            // Width of the column, in pixels.
	lvC.pszText = "Message Log";   // The text for the column.
	lvC.iSubItem = 0;

	if (ListView_InsertColumn(list, 0, &lvC) == -1)
		return NULL;
	
   	return list;
}



static int add_list_item(HWND list, char *text, LOGGER_PRIORITY priority)
{
	LV_ITEM lvI;        // List view item structure.
   
	// Fill in the LV_ITEM structure for each of the items to add to the list.
	// The mask specifies that the .pszText, .iImage, .lParam and .state
	// members of the LV_ITEM structure are valid.

	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;      
	lvI.stateMask = 0;  

	lvI.iItem = ListView_GetItemCount(list);
	lvI.iSubItem = 0;

	// The parent window is responsible for storing the text. The list view
	// window will send an LVN_GETDISPINFO when it needs the text to display.
	lvI.pszText = text;
	lvI.cchTextMax = MAX_ITEMLEN;
	lvI.iImage = priority;
	lvI.lParam = 0; 

	if (ListView_InsertItem(list, &lvI) == -1)
   		return -1;
   
	ListView_EnsureVisible(list, lvI.iItem, 1); 

	if(priority == LOGGER_ERROR)
		logger->number_of_errors++;

   return 0;
}




static LRESULT CALLBACK LoggerWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
//	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	switch(message) {
			
		case WM_EXITSIZEMOVE:
    	{
			ui_module_name_read_or_write_registry_settings("Logger", logger->panel_id, 1);

			return 0;
		}

    	case WM_SIZE:
    	{
			int width = LOWORD(lParam);
    		int height = HIWORD(lParam);
			
			SetWindowPos(logger->list, NULL, 10, 40, width - 20, height - 50, SWP_NOZORDER | SWP_NOACTIVATE);
			
			break;
    	}

		case WM_SYSCOMMAND:
    	{
    		switch (wParam)
    		{
    			/* Dont pass this to lab windows WndProc as it is ignored */
    			case SC_CLOSE:
   
    				HidePanel(logger->panel_id);   
						 
					return 0;
    		}

 			break;
    	}
		
		case LOGGED_MSG:
    	{
			MessageInfo *info = (MessageInfo *) wParam;
			char errors_str[200] = "";
			char buffer[500] = "", date[100] = "";

			if(logger->logging_enabled) {     
				
				if(logger->fp == NULL) {

					sprintf(buffer, "%s\\Log_%d.txt", logger->save_dir, logger->file_id);
					logger->fp = fopen(buffer, "w");

					if(logger->fp == NULL) {
						add_list_item(logger->list, "Failed to open log file", LOGGER_ERROR);      
					}
				}

				add_list_item(logger->list, info->msg, info->priority);      
	
				//Bring to the front
				SetPanelAttribute (logger->panel_id, ATTR_FLOATING, VAL_FLOAT_APP_ACTIVE);
				SetPanelAttribute (logger->panel_id, ATTR_FLOATING, VAL_FLOAT_NEVER);
	
				sprintf(errors_str, "Number of errors: %d", logger->number_of_errors);
				SetCtrlVal(logger->panel_id, logger->number_of_errors_label, errors_str);

				if(logger->fp != NULL) {

					if(info->priority == LOGGER_INFORMATIONAL)
						fprintf(logger->fp, "%s\n", info->msg);
					if(info->priority == LOGGER_WARNING)
						fprintf(logger->fp, "Warning: %s\n", info->msg);
					if(info->priority == LOGGER_ERROR)
						fprintf(logger->fp, "Error: %s\n", info->msg);
					
					// Make sure we write now.
					fflush(logger->fp);
				}
			}
		
			return 0;  
    	}
		
      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) logger->panel_original_proc_fun_ptr,
							logger->window_handle_hwnd, message, wParam, lParam);
}


static int CVICALLBACK OnEnableToggle (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(panel, control, &logger->logging_enabled);
	
			break;
	}
	
	return 0;
}

void delete_file_if_old (char pathname[], int months_old)
{
	int file_month, file_day, file_year, file_age;
	int sys_month, sys_day, sys_year;

	GetFileDate(pathname, &file_month, &file_day, &file_year);
	GetSystemDate(&sys_month, &sys_day, &sys_year);

	file_age = sys_month - file_month + (sys_year - file_year)*12;

	if (file_age > months_old)
		DeleteFile(pathname);
}

int delete_old_files (char basepath[], char file_spec[], int months_old)
{
	int last_file_id = 1, id, err = 0;
	char filename[512], pathname[512];

	if ((err = GetFirstFile(file_spec, 1, 0, 0, 0, 0, 0, filename)) < -1) {
		return -1; // General I0 error
	}

	if(err == -1) { // No files of that name
		return 1;
	}

	// We have found at least one file so we increment the id.
	sscanf(filename, "Log_%d.txt", &id);
	last_file_id = id + 1;

	sprintf(pathname, "%s\\%s", basepath, filename); 
	delete_file_if_old (pathname, months_old);

	while (GetNextFile(filename)==0)
	{
		sscanf(filename, "Log_%d.txt", &id);

		if (id >= last_file_id)
			last_file_id = id + 1;

		sprintf(pathname, "%s\\%s", basepath, filename); 
		delete_file_if_old (pathname, months_old);
	}

	return last_file_id; // Success
}

Logger* logger_new(const char* save_dir)
{
	int window_handle;
	
	ref_count++;

	if(logger != NULL)
		return logger;
	
	logger = (Logger*) malloc (sizeof(Logger));
	
	if(save_dir != NULL)
		strcpy(logger->save_dir, save_dir);
	
	logger->fp = NULL;
	logger->logging_enabled = 1;
	logger->number_of_errors = 0;

	GciCmtNewLock("Logger", 0, &logger->lock);
	
	logger->panel_id =  NewPanel (0, "Log", 100, 100, 200, 400);

	ui_module_name_read_or_write_registry_settings("Logger", logger->panel_id, 0);

	logger->enable_toggle_id = NewCtrl(logger->panel_id, CTRL_SQUARE_TEXT_BUTTON_LS, "", 5, 10);
	SetCtrlAttribute(logger->panel_id , logger->enable_toggle_id, ATTR_LABEL_FONT, "Arial");
	SetCtrlAttribute(logger->panel_id , logger->enable_toggle_id, ATTR_LABEL_POINT_SIZE, 13);
	SetCtrlAttribute(logger->panel_id , logger->enable_toggle_id, ATTR_ON_COLOR, MakeColor(30, 87, 174));
	SetCtrlAttribute(logger->panel_id , logger->enable_toggle_id, ATTR_OFF_COLOR, MakeColor(30, 87, 174));  
	InstallCtrlCallback(logger->panel_id , logger->enable_toggle_id, OnEnableToggle, NULL);
	SetCtrlAttribute(logger->panel_id, logger->enable_toggle_id, ATTR_OFF_TEXT, "Disabled");
	SetCtrlAttribute(logger->panel_id, logger->enable_toggle_id, ATTR_ON_TEXT, "Enabled");
	SetCtrlVal(logger->panel_id, logger->enable_toggle_id, 1);
	SetCtrlAttribute(logger->panel_id, logger->enable_toggle_id, ATTR_TEXT_COLOR, VAL_WHITE);  
	
	logger->number_of_errors_label = NewCtrl(logger->panel_id, CTRL_TEXT_MSG, "", 9, 100);
	SetCtrlAttribute(logger->panel_id , logger->number_of_errors_label, ATTR_LABEL_FONT, "Arial");
	SetCtrlAttribute(logger->panel_id , logger->number_of_errors_label, ATTR_LABEL_POINT_SIZE, 11);

	GetPanelAttribute (logger->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	
	logger->window_handle_hwnd = (HWND) window_handle;
	
	/* Store the original windows procedure function pointer */
	logger->panel_original_proc_fun_ptr = GetWindowLongPtr (logger->window_handle_hwnd, GWL_WNDPROC);
 
	/* Store the window structure with the window for use in WndProc */
	SetWindowLongPtr (logger->window_handle_hwnd, GWLP_USERDATA, (LONG_PTR) logger);
	
	 /* Set the new Wnd Proc to be called */	
	SetWindowLongPtr (logger->window_handle_hwnd, GWL_WNDPROC, (LONG_PTR) LoggerWndProc);

	logger->list = CreateListView (logger->window_handle_hwnd);   
	
	return logger;
}

void logger_get_log_filepath(Logger *ext_logger, char *path)
{
	sprintf(path, "%s\\Log_%d.txt", logger->save_dir, logger->file_id);
}

void logger_destroy(Logger *ext_logger)   // There is only one logger really, ext_logger = logger
{
	if(logger == NULL)
		return;
	
	ref_count--;      
	
	if(ref_count == 0) {
		
		if(logger->panel_original_proc_fun_ptr != 0)
			SetWindowLongPtr (logger->window_handle_hwnd, GWL_WNDPROC, logger->panel_original_proc_fun_ptr);

		DiscardPanel(logger->panel_id); 
		logger->panel_id = -1;
		
		if(logger->fp != NULL)
			fclose(logger->fp);

		CmtDiscardLock(logger->lock);
	
		printf("Logger Destroy\n");

		free(logger);
		logger = NULL;
		ref_count = 0;
	}
}


void logger_set_data_dir(Logger *ext_logger, const char *path)
{
	static int logger_set_data_dir_called = 0;
	int err = 0;
	char buffer[500] = "";
	
	if(logger_set_data_dir_called)
		return;

	if(path != NULL)
		strcpy(logger->save_dir, path);	

	sprintf(buffer, "%s\\Log_*.txt", logger->save_dir);
	
	logger->file_id = delete_old_files (logger->save_dir, buffer, 3);

	if(logger->file_id < 0)
		GCI_ErrorPopup("Error", "IO error writing log file");

	sprintf(buffer, "%s\\Log_%d.txt", logger->save_dir, logger->file_id);
	logger_log(logger, LOGGER_INFORMATIONAL, "Log file created: %s", buffer);

	logger_set_data_dir_called = 1;
}

void logger_log_ap(Logger *ext_logger, LOGGER_PRIORITY priority, char *fmt, va_list ap)    
{
	char buffer[MAX_ITEMLEN], text[MAX_ITEMLEN];
	MessageInfo *info_ptr;

	if(logger == NULL || logger->panel_id <= 0)
		return;
	
	logger_get_lock(logger);

	vsprintf(buffer, fmt, ap);

	strcpy(text, TimeStr());
	strcat(text, " ");
	strcat(text, buffer);
	
	info_ptr = &infos[current_info];
	strcpy(info_ptr->msg, text);
	info_ptr->priority = priority;
	
	if(info_ptr->priority == LOGGER_ERROR)
		info_ptr->priority = info_ptr->priority;

	if(current_info++ >= 9)
		current_info = 0;

	PostMessage(logger->window_handle_hwnd, LOGGED_MSG, (WPARAM) info_ptr, 0);
	
	logger_release_lock(logger);
}

void logger_log(Logger *ext_logger, LOGGER_PRIORITY priority, char *fmt, ...)    
{
	va_list ap;
	
	logger_get_lock(logger);

	va_start(ap, fmt);

	logger_log_ap(logger, priority, fmt, ap);   

	va_end(ap);    

	logger_release_lock(logger);
}

void logger_show(Logger *ext_logger)   // There is only one logger really, ext_logger = logger    
{
	int count = 0;   
	
	if(logger == NULL)
		return;
	  
	//Bring to the front
	SetPanelAttribute (logger->panel_id, ATTR_FLOATING, VAL_FLOAT_APP_ACTIVE);
	SetPanelAttribute (logger->panel_id, ATTR_FLOATING, VAL_FLOAT_NEVER);
	
	DisplayPanel(logger->panel_id);
}

void logger_hide(Logger *ext_logger)   // There is only one logger really, ext_logger = logger    
{
	if(logger == NULL)
		return;

	HidePanel(logger->panel_id);
}
