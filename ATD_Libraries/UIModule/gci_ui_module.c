#include <ansi_c.h>
#include "gci_ui_module.h"
#include "toolbox.h"
#include "gci_utils.h"

#ifdef FORTIFY
#include "fortify.h"
#endif

#include <utility.h>
#include "GL_CVIRegistry.h"
#include "string_utils.h"
#include "iniparser.h"

#include "gci_ui_logger.h"

#define REGKEY_HKCU 0x80000001 /* HKEY_CURRENT_USER   */

static unsigned short ui_module_id_count = 0;

static LONG_PTR original_cvi_proc_fun_ptr = 0;

static ListType	ui_module_list = NULL;

static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);    
	
	GCI_MessagePopup(title, error_string); 
	
	return UIMODULE_ERROR_NONE;
}


int ui_module_send_valist_error(UIModule *module, const char *title, const char *fmt, va_list ap)
{
	char message[500];

	if(module == NULL || module->_error_handler == NULL)
		return UIMODULE_ERROR_NONE;

	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);  
	
	vsprintf(message, fmt, ap);

	if(module->_error_handler != NULL)
		return module->_error_handler(module, title, message, module->callback_data);
	
	return UIMODULE_ERROR_NONE;
}

int ui_module_send_error(UIModule *module, const char *title, const char *fmt, ...)
{
	int ret;
	va_list ap;
	
	va_start(ap, fmt);

	ret = ui_module_send_valist_error(module, title, fmt, ap); 
		
	va_end(ap);
	
	return ret;
}

int ui_module_dialog_feedback(UIModule *module, const char *title, const char *fmt, ...)
{
	va_list ap;
	int id, label_id, conform_to_sys=0, screen_height, screen_width;
	const int width = 400, height = 100;
	char message[500];

	// For centering panel on desktop
	GetScreenSize (&screen_height, &screen_width);  
	
	// Create a panel
	id = NewPanel(0, title, (screen_height / 2) - height / 2,
							(screen_width / 2) - width / 2,
							height, width);

	SetPanelAttribute(id, ATTR_MOVABLE, 1);
	SetPanelAttribute(id, ATTR_SIZABLE, 0);
	SetPanelAttribute(id, ATTR_CLOSE_ITEM_VISIBLE, 0); 
	SetPanelAttribute(id, ATTR_CAN_MINIMIZE, 0);
	SetPanelAttribute(id, ATTR_CAN_MAXIMIZE, 0);
	
	if(module->_panel_ids[0])
		GetPanelAttribute(module->_panel_ids[0], ATTR_CONFORM_TO_SYSTEM, &conform_to_sys);
	
	if(conform_to_sys)
		SetPanelAttribute(id, ATTR_CONFORM_TO_SYSTEM, 1);
	
	SetPanelAttribute(id, ATTR_TITLE, title);    
	
	label_id = NewCtrl (id, CTRL_TEXT_MSG, "", (height / 2) - 10, 10);
	SetCtrlAttribute(id, label_id, ATTR_WIDTH, width - 20);
	SetCtrlAttribute(id, label_id, ATTR_TEXT_BOLD , 1);      
	SetCtrlAttribute(id, label_id, ATTR_TEXT_FONT , VAL_MESSAGE_BOX_FONT);  
	SetCtrlAttribute(id, label_id, ATTR_TEXT_JUSTIFY, VAL_CENTER_JUSTIFIED);   	

	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);   
	
	va_start(ap, fmt);

	vsprintf(message, fmt, ap);

	va_end(ap);

	SetCtrlVal(id, label_id, message);
	
	GCI_MovePanelToDefaultMonitorForDialogs(id);

	DisplayPanel(id);
	
	return id;
}

void ui_module_hide_logger(UIModule *module)
{
	logger_hide(module->_logger);
}

void ui_module_set_logger(UIModule *module, Logger *log)
{
	module->_logger = log;	
}

Logger* ui_module_get_logger(UIModule *module)
{
	return module->_logger;
}


void ui_module_set_error_handler(UIModule *module, UI_MODULE_ERROR_HANDLER handler, void *callback_data)
{
	module->_error_handler = handler;
	module->callback_data = callback_data;
}


UI_MODULE_ERROR_HANDLER ui_module_get_error_handler(UIModule *module)
{
	return module->_error_handler;	
}

static RECT GetWindowSizeWithoutBorder(HWND hwnd)
{
	RECT windowRect, clientRECT, rect;
	int window_width, window_height, client_width, client_height;
	int horz_border, title_bar_height;

	GetWindowRect(hwnd, &windowRect);
	GetClientRect(hwnd, &clientRECT);

	window_width = windowRect.right - windowRect.left;
	window_height = windowRect.bottom - windowRect.top;

	client_width = clientRECT.right - clientRECT.left;
	client_height = clientRECT.bottom - clientRECT.top;

	horz_border = (window_width - client_width) / 2;
	title_bar_height = (window_height - client_height - horz_border);

	rect.left = windowRect.left + horz_border;
	rect.top =  windowRect.top + title_bar_height;

	rect.right = rect.left + client_width;
	rect.bottom = rect.top + client_height;

	return rect;
}

static void PositionAttachedWindow(UIModule *module, AttacheePanelInfo panel)
{
	int panel_width, panel_height, handle, offset_x, offset_y;
	RECT rect;
	HWND hwnd;
	
	GetPanelAttribute(UIMODULE_MAIN_PANEL_ID(module), ATTR_SYSTEM_WINDOW_HANDLE, &handle);
	hwnd = (HWND) handle;

	rect = GetWindowSizeWithoutBorder(hwnd);
	panel_width = rect.right - rect.left;
	panel_height = rect.bottom - rect.top;

	offset_x = panel._offset_x;
	offset_y = panel._offset_y;

	if(panel._relative == UI_MODULE_REL_TOP_LEFT) {

		offset_x += rect.left;
		offset_y += rect.top;
	}
	else if(panel._relative == UI_MODULE_REL_TOP_RIGHT) {

		offset_x += rect.right;
		offset_y += rect.top;
	}
	else if(panel._relative == UI_MODULE_REL_BOTTOM_LEFT) {

		offset_x += rect.left;
		offset_y += rect.bottom;
	}
	else if(panel._relative == UI_MODULE_REL_CENTRE) {

		POINT parent_center;
		int attachee_width, attachee_height;
		parent_center.x = (rect.right - rect.left) / 2;
		parent_center.y = (rect.bottom - rect.top) / 2;

		GetPanelAttribute(panel._attachee_panel_id, ATTR_WIDTH, &attachee_width);
		GetPanelAttribute(panel._attachee_panel_id, ATTR_HEIGHT, &attachee_height);

		offset_x += rect.left;
		offset_y += rect.top;
		offset_x += (parent_center.x - (attachee_width / 2));
		offset_y += (parent_center.y - (attachee_height / 2));
	}

	SetPanelAttribute(panel._attachee_panel_id, ATTR_LEFT, offset_x);
	SetPanelAttribute(panel._attachee_panel_id, ATTR_TOP, offset_y);

	// Always bring to top with the image window
	GetPanelAttribute(panel._attachee_panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &handle);
	BringWindowToTop((HWND) handle);
		
	ui_module_panel_read_or_write_registry_settings(module, panel._attachee_panel_id, 1);        
}

static void PositionAttachedWindows(UIModule *module)
{
	int i=0;

	for (i=0; i < module->_number_of_attachee_panels; i++) {

		PositionAttachedWindow(module, module->_attachee_panels[i]);
	}
}

static void MinimiseAttachedWindows(UIModule *module)
{
	int i=0;

	for (i=0; i < module->_number_of_attachee_panels; i++) {

		//SetPanelAttribute(module->_attachee_panels[i]._attachee_panel_id, ATTR_WINDOW_ZOOM, VAL_MINIMIZE);
		
		// We don't minimise the attached panel will just hide it then we don't get a button
		// hovering above the startbar
		SetPanelAttribute(module->_attachee_panels[i]._attachee_panel_id, ATTR_VISIBLE, 0);
		
	}
}

static void RestoreAttachedWindows(UIModule *module)
{
	int i=0;

	for (i=0; i < module->_number_of_attachee_panels; i++) {

		// We don't minimise the attached panel will just hide it then we don't get a button
		// hovering above the startbar
		SetPanelAttribute(module->_attachee_panels[i]._attachee_panel_id, ATTR_VISIBLE, 1);
	}
}

static LRESULT CALLBACK UIModuleWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	UIModule *module = (UIModule *) data;
	
	switch(message) {
			
		case WM_ENTERSIZEMOVE:
		{
			break;
		}

		case WM_EXITSIZEMOVE:
    	{
			PositionAttachedWindows(module);

			ui_module_read_or_write_registry_settings(module, 1);    

			return 0;
		}

		case WM_MOUSEACTIVATE:
		{
			if(!(module->_prevent_mouse_activation))
				break;

			return MA_NOACTIVATE;
		}

		case WM_MOVING:
		{
			PositionAttachedWindows(module);

			break;
		}

		case WM_SIZING:
		{
			PositionAttachedWindows(module);

			break;
		}

		case WM_SYSCOMMAND:
    	{
    		switch (wParam)
    		{
				case SC_MINIMIZE:
				{
					MinimiseAttachedWindows(module);

					break;
				}

				case SC_MAXIMIZE:
				{
			
					break;
				}

				case SC_RESTORE:
				{
					RestoreAttachedWindows(module);
	
					break;
				}
    		}

 			break;
    	}

      	default:

        	break;
   	}

	if(original_cvi_proc_fun_ptr == 0)
	{
		logger_log(module->_logger, LOGGER_WARNING, "Something has gone wrong original_cvi_proc_fun_ptr == 0");
		return 0;
	}

	return CallWindowProc ((WNDPROC) original_cvi_proc_fun_ptr,
		hwnd, message, wParam, lParam);
}

int ui_module_add_panel_as_child(UIModule *module, const char *filename, int panel_id, int parent_panel_id)
{	
	int id;
	char full_path[GCI_MAX_PATHNAME_LEN];
	
	if(find_resource(filename, full_path) < 0) {
		ui_module_send_error(module, "Uir load error", "Can not find file %s", filename);
		return -1;  
	}
	
	/* module->_child_panel_ids[module->_number_of_child_panels++] = */ id = LoadPanel(parent_panel_id, full_path, panel_id);   

	return id;
}

void ui_module_set_main_panel_title (UIModule *module)
{
	char title[512];
	sprintf(title, "%s", module->_description);
	SetPanelAttribute(module->_main_panel_id, ATTR_TITLE, title);
}

static void ui_module_setup_wnd_proc(UIModule *module, int panel_id)
{
	int window_handle;

	GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   

//	module->window_handle_hwnd = (HWND) window_handle;

	/* Store the window structure with the window for use in WndProc */
	SetWindowLongPtr ((HWND)window_handle, GWLP_USERDATA, (LONG_PTR) module);
	
	 /* Set the new Wnd Proc to be called */	
	SetWindowLongPtr ((HWND)window_handle, GWL_WNDPROC, (LONG_PTR) UIModuleWndProc);
}

void ui_module_force_send_sizing_message(UIModule *module)
{
	int width, height;

	int window_handle;
	
	GetPanelAttribute (UIMODULE_MAIN_PANEL_ID(module), ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   

	GetPanelAttribute(UIMODULE_MAIN_PANEL_ID(module), ATTR_WIDTH, &width);
	GetPanelAttribute(UIMODULE_MAIN_PANEL_ID(module), ATTR_HEIGHT, &height);

	SendMessage((HWND) window_handle, WM_SIZING, 0, MAKELPARAM(width, height)); 
}

int ui_module_add_panel(UIModule *module, const char *filename, int panel_id, int is_main)
{	
	int id;
	char full_path[GCI_MAX_PATHNAME_LEN];
	
	if(find_resource(filename, full_path) < 0) {
		ui_module_send_error(module, "Uir load error", "Can not find file %s", filename);
		return -1;  
	}
	
	id = module->_panel_ids[module->_number_of_panels++] = LoadPanel(0, full_path, panel_id);   
	
	ui_module_setup_wnd_proc(module, id);

	if(is_main)
		module->_main_panel_id = id;
	
	return id;
}

static int UI_MODULE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (UIModule*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ((UIModule *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int UI_MODULE_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (UIModule*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ((UIModule *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static LONG_PTR get_wnd_proc(int panel_id)
{
	int window_handle;
	
	GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   

	return GetWindowLongPtr ((HWND) window_handle, GWL_WNDPROC);
}

void ui_module_prevent_mouse_activation(UIModule *module)
{
	module->_prevent_mouse_activation = 1;
}

void ui_module_enable_mouse_activation(UIModule *module)
{
	module->_prevent_mouse_activation = 0;
}

// Takes strings (panel names) ending in NULL
void ui_module_constructor(UIModule *module, const char *name)
{	
	#ifdef INTERACTION_REPORT
	printf ("Creating UI module %s.\n", name);
	#endif

	if(ui_module_list == NULL)
	  ui_module_list = ListCreate (sizeof(UIModule *)); 
		
	memset(module, 0, sizeof(UIModule));

	module->_id = ui_module_id_count++;

	module->_number_of_panels = 0;
	module->_main_panel_id = 0;     
	module->_prevent_mouse_activation = 0;

	ui_module_set_name(module, name);       
	memset(module->_description, 0, 1);
	
	// Initiliase the Signals
	GCI_SignalSystem_Create(&(module->_signal_table), 50, UIMODULE_GET_NAME(module)); 
	
	// Create a Hide main panel event
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(module), "HideOrCloseMain", UI_MODULE_PTR_MARSHALLER);     
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(module), "ShowMain", UI_MODULE_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(module), "ShowPanel", UI_MODULE_PTR_INT_MARSHALLER);

	// Set Default Error Handler
	module->_error_handler = default_error_handler;
	module->_logger = logger_new("C:");

	if(original_cvi_proc_fun_ptr == 0)
		original_cvi_proc_fun_ptr = get_wnd_proc(NewPanel(0, "", 0,0,10,10));

	#ifdef FORTIFY
	printf("Creating %d: %s\n", module->_id, module->_name);
	#endif
	
	ListInsertItem (ui_module_list, &module, END_OF_LIST); 
	
	module->_module_list_id = ListNumItems (ui_module_list);

}

LONG_PTR ui_module_get_original_wndproc_ptr(UIModule *module)
{
	return original_cvi_proc_fun_ptr;
}

int ui_module_main_panel_hide_or_close_handler_connect (UIModule *module, UI_MODULE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(module), "HideOrCloseMain", handler, callback_data) == SIGNAL_ERROR) {
		ui_module_send_error(module, "UIModule Error", "Can not attach signal HideOrClose");  
		return UI_MODULE_ERROR;
	}

	return UI_MODULE_SUCCESS;	
	
}


int ui_module_main_panel_show_mainpanel_handler_connect (UIModule *module, UI_MODULE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(module), "ShowMain", handler, callback_data) == SIGNAL_ERROR) {
		ui_module_send_error(module, "UIModule Error", "Can not attach signal ShowMain");  
		return UI_MODULE_ERROR;
	}

	return UI_MODULE_SUCCESS;		
}

int ui_module_panel_show_handler_connect (UIModule *module, UI_MODULE_EVENT_INT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(module), "ShowPanel", handler, callback_data) == SIGNAL_ERROR) {
		ui_module_send_error(module, "UIModule Error", "Can not attach signal ShowPanel");  
		return UI_MODULE_ERROR;
	}

	return UI_MODULE_SUCCESS;		
}

LONG_PTR ui_module_set_window_proc(UIModule *module, int panel_id, LONG_PTR proc)
{
	int window_handle;
	LONG_PTR old_ptr;

	GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	
//	module->window_handle_hwnd = (HWND) window_handle;
	
	/* Store the original windows procedure function pointer */
	old_ptr = get_wnd_proc(panel_id);

	/* Store the window structure with the window for use in WndProc */
	SetWindowLongPtr ((HWND)window_handle, GWLP_USERDATA, (LONG_PTR) module);
	
	 /* Set the new Wnd Proc to be called */	
	SetWindowLongPtr ((HWND)window_handle, GWL_WNDPROC, proc);

	return old_ptr;
}


// Are any of the modules panels visible
int ui_module_panel_are_any_panels_visible(UIModule *module)
{
	int i, visible = 0;
	
	for(i=0; i < module->_number_of_panels; i++) {
		
		if(module->_panel_ids[i] == 0)
			continue;
		
		GetPanelAttribute(module->_panel_ids[i], ATTR_VISIBLE, &visible);
	
		if(visible)
			return 1;
	}
	
	return 0;
}

// Are any of the modules panels visible
int ui_module_panel_is_visible(UIModule *module, int panel_id)
{
	int visible = 0;
	
	GetPanelAttribute(panel_id, ATTR_VISIBLE, &visible);
	
	if(visible)
		return 1;
	
	return 0;
}

int ui_module_main_panel_is_visible(UIModule *module)
{
	return ui_module_panel_is_visible(module, module->_main_panel_id);
}

void ui_module_display_panel_without_activation(UIModule *module, int panel_id)
{
	int window_handle, visible = 0;
	
	GetPanelAttribute(panel_id, ATTR_VISIBLE, &visible);
	
	if(visible)
		return;
	
	ui_module_panel_read_or_write_registry_settings(module, panel_id, 0);        
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowPanel", GCI_VOID_POINTER, module, GCI_INT, panel_id, GCI_END_OF_LIST);

	GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	
	ShowWindow( (HWND) window_handle, SW_SHOWNA);
}

void ui_module_display_panel(UIModule *module, int panel_id)
{
	int visible = 0;
	
	GetPanelAttribute(panel_id, ATTR_VISIBLE, &visible);
	
	if(visible)
		return;
	
	ui_module_panel_read_or_write_registry_settings(module, panel_id, 0);        
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowPanel", GCI_VOID_POINTER, module, GCI_INT, panel_id);

	PositionAttachedWindows(module);  // in case this is an attached window

	DisplayPanel(panel_id);	
}


void ui_module_display_main_panel_without_activation(UIModule *module)
{
	int window_handle;

	ui_module_panel_read_or_write_registry_settings(module, module->_main_panel_id, 0);      

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowMain", GCI_VOID_POINTER, module);
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowPanel", GCI_VOID_POINTER, module, GCI_INT,  module->_main_panel_id);

	GetPanelAttribute (module->_main_panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	
	ShowWindow( (HWND) window_handle, SW_SHOWNA);
}

void ui_module_display_main_panel(UIModule *module)
{
	ui_module_panel_read_or_write_registry_settings(module, module->_main_panel_id, 0);      

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowMain", GCI_VOID_POINTER, module);
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowPanel", GCI_VOID_POINTER, module, GCI_INT,  module->_main_panel_id);

	DisplayPanel(module->_main_panel_id);

	PositionAttachedWindows(module);
}

void ui_module_display_main_panel_without_registry(UIModule *module)
{
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowMain", GCI_VOID_POINTER, module);
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowPanel", GCI_VOID_POINTER, module, GCI_INT,  module->_main_panel_id);

	DisplayPanel(module->_main_panel_id); 
}

void ui_module_display_all_panels(UIModule *module)
{
	int i;
	
	for(i=0; i < module->_number_of_panels; i++) {

		ui_module_display_panel(module, module->_panel_ids[i]); 
	}	
}


void ui_module_hide_main_panel(UIModule *module)
{
	ui_module_hide_panel(module, module->_main_panel_id);          	
}


void ui_module_hide_panel(UIModule *module, int panel_id)
{
	int visible = 0;
	
	GetPanelAttribute(panel_id, ATTR_VISIBLE, &visible);
	
	if(!visible)
		return;
	
	HidePanel(panel_id);      
	
	ui_module_panel_read_or_write_registry_settings(module, panel_id, 1);   

	if(module->_main_panel_id == panel_id) {
		
		// Signal that the main panel has been closed or hidden
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "HideOrCloseMain", GCI_VOID_POINTER, module);
	}
}


void ui_module_hide_all_panels(UIModule *module)
{
	int i;
	
	for(i=0; i < module->_number_of_panels; i++) {

		ui_module_hide_panel(module, module->_panel_ids[i]); 
	}
	
	// Signal that the main panel has been closed or hidden
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "HideOrCloseMain", GCI_VOID_POINTER, module);
}


void ui_module_save_settings(UIModule *module, const char *filepath, const char *flags)
{
	FILE *fd;
	int i, count, left, top, width, height, visible, remote_session;
	char section[100] = "", panel_define[100] = "", buffer[500] = "";
    dictionary *d = dictionary_new(20);
	
	remote_session = GetSystemMetrics(SM_REMOTESESSION);

	if(remote_session)
		return;

	fd = fopen(filepath, flags);
	
	sprintf(section, "%s UI", UIMODULE_GET_NAME(module));
	dictionary_set(d, section, NULL);

	count = 1;
	
	for(i=0; i < module->_number_of_panels; i++) {

		visible = ui_module_panel_is_visible(module, module->_panel_ids[i]); 
		
		GetPanelAttribute(module->_panel_ids[i], ATTR_LEFT, &left);
		GetPanelAttribute(module->_panel_ids[i], ATTR_TOP, &top);      
		GetPanelAttribute(module->_panel_ids[i], ATTR_WIDTH, &width);      
		GetPanelAttribute(module->_panel_ids[i], ATTR_HEIGHT, &height);      
		
		sprintf(buffer, "%d,%d,%d,%d,%d", left, top, width, height, visible);  
		
		sprintf(section, "Panel%d", count++);  
		dictionary_set(d, section, buffer);      
	}
	
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
}


void ui_module_restore_settings(UIModule *module, const char *filepath)
{
	dictionary* d = NULL;
	int i, err, file_size, count = 1, left, top, width, height, visible, remote_session;    
	char *buffer = NULL, key[100] = "";
	
	if(module == NULL)
		return;
	
	remote_session = GetSystemMetrics(SM_REMOTESESSION);

	if(remote_session)
		return;

	if(!FileExists(filepath, &file_size))
		return;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
		for(i=1; i <= module->_number_of_panels; i++)
		{
			sprintf(key, "%s UI:Panel%d", UIMODULE_GET_NAME(module), i);
		
			buffer = dictionary_get(d, key, ""); 
		
			if(strcmp(buffer, "") == 0)
				continue;
			
			sscanf(buffer, "%d,%d,%d,%d,%d", &left, &top, &width, &height, &visible); 
		
			err = SetPanelAttribute(module->_panel_ids[i-1], ATTR_LEFT, left);
			
			if(err < 0) 
				logger_log(module->_logger, LOGGER_WARNING, "Error calling SetPanelAttribute");
	
			SetPanelAttribute(module->_panel_ids[i-1], ATTR_TOP, top);     

			//SetPanelAttribute(module->_panel_ids[i-1], ATTR_WIDTH, width);      
			//SetPanelAttribute(module->_panel_ids[i-1], ATTR_HEIGHT, height);      
	
			if(visible) {
				// ui_module_display_panel(module, module->_panel_ids[i-1]);  Don't do this as it call registry settings back

				if( module->_panel_ids[i-1] == module->_main_panel_id)
					GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowMain", GCI_VOID_POINTER, module);

				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "ShowPanel", GCI_VOID_POINTER, module, GCI_INT, module->_panel_ids[i-1]);
				DisplayPanel(module->_panel_ids[i-1]);
			}
			else {

				if( module->_panel_ids[i-1] == module->_main_panel_id)
					GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "HideOrCloseMain", GCI_VOID_POINTER, module);

				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "HidePanel", GCI_VOID_POINTER, module, GCI_INT, module->_panel_ids[i-1]);
				HidePanel(module->_panel_ids[i-1]); 
			}
		}
	
        dictionary_del(d);
	}
}

void ui_module_restore_cvi_wnd_proc(int panel_id)
{
	int window_handle, parent = 0;

	GetPanelAttribute (panel_id, ATTR_PANEL_PARENT, &parent);     
	
	// We havevn't setup wnd proc redirection for child panels.
	if(parent)
		return;
	
	GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	
	/* Set the original Wnd Proc to be called */
	if(original_cvi_proc_fun_ptr != 0)
		SetWindowLongPtr ((HWND) window_handle, GWL_WNDPROC, original_cvi_proc_fun_ptr);
}

void ui_module_destroy_panel(UIModule *module, int panel_id)
{
	int i;
	
	if(panel_id <= 0)
		return;
	
	// If the main panel is destroyed send signal
	if(module->_main_panel_id == panel_id) {
		// Signal that the main panel has been closed or hidden
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(module), "HideOrCloseMain", GCI_VOID_POINTER, module);
	}
	
	ui_module_restore_cvi_wnd_proc(panel_id);

	DiscardPanel(panel_id); 
	
	// Search panel refs to reset it.
	for(i=0; i < module->_number_of_panels; i++) {

		if(panel_id == module->_panel_ids[i]) {
			module->_panel_ids[i] = 0;
			module->_number_of_panels--; 
		}
	}
}

ListType* ui_module_get_ui_module_list()
{
	return &ui_module_list;
}

int ui_module_number_of_modules(void)
{
	return ListNumItems (ui_module_list);
}

UIModule* ui_module_get_module_at_index(int index)
{
	UIModule **module_pt_ptr = ListGetPtrToItem (ui_module_list, index);

	return (UIModule*) *module_pt_ptr;
}

void ui_module_destroy(UIModule *module)
{
	int i, number_of_modules;
	
	ui_module_read_or_write_registry_settings(module, 1);     
	
	for(i=0; i < UI_MOD_MAX_PANELS; i++) {

		if(module->_panel_ids[i] != 0) {

			ui_module_restore_cvi_wnd_proc(module->_panel_ids[i]);

			DiscardPanel(module->_panel_ids[i]);
			module->_panel_ids[i] = 0;
			module->_number_of_panels--;
		}
	}

	logger_destroy(module->_logger);   

	number_of_modules = ListNumItems (ui_module_list);

	GCI_SignalSystem_Destroy(UIMODULE_SIGNAL_TABLE(module));
	
	if(number_of_modules > 0) {
		ListRemoveItem (ui_module_list, NULL, module->_module_list_id);
		number_of_modules--;
	}

	if(number_of_modules <= 0) {
		ListDispose (ui_module_list); 
		ui_module_list = NULL;
	}
		
	#ifdef FORTIFY
	printf("Destroying %d: %s\n", module->_id, module->_name);
	#endif
}

int ui_module_set_name(UIModule *module, const char* name)
{
    strncpy(module->_name, name, UIMODULE_NAME_LEN - 1);
  	
  	return UI_MODULE_SUCCESS;
}

int ui_module_get_name(UIModule *module, char* name)
{
    strncpy(name, module->_name, UIMODULE_NAME_LEN - 1);
  	
  	return UI_MODULE_SUCCESS; 
}

int ui_module_set_description(UIModule *module, const char* description)
{
    strncpy(module->_description, description, UIMODULE_NAME_LEN - 1);
  	
  	return UI_MODULE_SUCCESS;
}

int ui_module_get_description(UIModule *module, char* description)
{
    strncpy(description, module->_description, UIMODULE_NAME_LEN - 1);
  	
  	return UI_MODULE_SUCCESS; 
}



int ui_module_panel_saved_visibility_status (UIModule *module, int panel_id)
{
	char buffer[500], panel_define[100];    
	int vis = 0, error = 0;

	// Get the panel define as a module may have more than one panel
	GetPanelAttribute(panel_id, ATTR_CONSTANT_NAME, panel_define);
	
	sprintf(buffer, "software\\GCI\\Microscope\\PanelsDetails\\%s\\%s\\", module->_name, panel_define);
	
	if((error = RegReadLong (REGKEY_HKCU, buffer, "visible", &vis)) < 0)
		return 0;

	if(vis)
		return 1;
	
	return 0;
}


void ui_module_name_get_registry_store_for_name(const char *name, int panel_id, char *path)
{
	char panel_define[100] = "";

	path[0] = '0';

	if(name == NULL || panel_id <= 0)
		return;

	// load or save panel positions
	
	// Get the panel define as a module may have more than one panel
	GetPanelAttribute(panel_id, ATTR_CONSTANT_NAME, panel_define);
	
	sprintf(path, "software\\GCI\\Microscope\\PanelsDetails\\%s\\%s\\", name, panel_define);
}

void ui_module_name_read_or_write_registry_settings(const char *name, int panel_id, int write)
{
	char buffer[500], remote_session = 0;
	int maximised=0;

	if(name == NULL || panel_id <= 0)
		return;

	remote_session = GetSystemMetrics(SM_REMOTESESSION);

	if(remote_session)
		return;

	// load or save panel positions
	// Get the panel define as a module may have more than one panel
	ui_module_name_get_registry_store_for_name(name, panel_id, buffer);

	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {

		RegistrySavePanelVisibility (REGKEY_HKCU, buffer, panel_id);     
	}

	// Check if the window is maximised.
	// If it is dont try to set size and position
	GetPanelAttribute(panel_id, ATTR_WINDOW_ZOOM, &maximised);

	if(maximised == VAL_MAXIMIZE)
		return;

	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "top", panel_id, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "left", panel_id, ATTR_LEFT);
}

void ui_module_panel_read_or_write_registry_settings(UIModule *module, int panel_id, int write)
{
	if(module == NULL)
		return;

	ui_module_name_read_or_write_registry_settings(module->_name, panel_id, write);
}

// Writes settings for all panels
void ui_module_read_or_write_registry_settings(UIModule *module, int write)
{
	int i, remote_session = 0;

	if(module == NULL)
		return;

	remote_session = GetSystemMetrics(SM_REMOTESESSION);

	if(remote_session)
		return;

	for(i=0; i < module->_number_of_panels; i++) {
		
		ui_module_panel_read_or_write_registry_settings(module, module->_panel_ids[i], write);
	}
}

static int in_array(int *array, int array_size, int val)
{
	int i;
	
	for(i=0; i < array_size; i++)
		if(array[i] == val)
			return 1;
		
	return 0;
}

int ui_module_disable_all_panels_and_controls(UIModule *module, int disable)
{
	int i;
	
	for(i=0; i < module->_number_of_panels; i++) {
		
		if(module->_panel_ids[i] == 0)
			continue;
		
		SetPanelAttribute (module->_panel_ids[i], ATTR_DIMMED, disable);
	}
	
	return UI_MODULE_SUCCESS;       
}

int ui_module_disable_all_panel_controls(UIModule *module, int dimmed)
{
	int i;
	
	for(i=0; i < module->_number_of_panels; i++) {
		
		if(module->_panel_ids[i] == 0)
			continue;
		
		ui_module_set_panel_dimmed_attribute(module->_panel_ids[i], dimmed, 0);
	}
	
	return UI_MODULE_SUCCESS;            
}


int ui_module_set_panel_dimmed_attribute(int panel_id, int dimmed, int number_of_exceptions, ...)
{
	int i, exceptions[50], number_of_controls = 0;
	
	va_list ap;       
	va_start(ap, number_of_exceptions);

	for(i=0; i < number_of_exceptions; i++)
		exceptions[i] = va_arg(ap, int); 
	
	GetPanelAttribute(panel_id, ATTR_NUM_CTRLS, &number_of_controls);

	for (i=2; i <= number_of_controls + 1; i++) {
		 
		if(number_of_exceptions > 0 && in_array(exceptions, number_of_exceptions, i))
			continue;
		
		SetCtrlAttribute(panel_id, i, ATTR_DIMMED, dimmed);
	}
	
	va_end(ap); 
	
	return UI_MODULE_SUCCESS;       
}

int ui_module_disable_panel(int panel_id, int number_of_exceptions, ...)
{
	int i, exceptions[50], number_of_controls = 0;
	
	va_list ap;       
	va_start(ap, number_of_exceptions);

	for(i=0; i < number_of_exceptions; i++)
		exceptions[i] = va_arg(ap, int); 
	
	GetPanelAttribute(panel_id, ATTR_NUM_CTRLS, &number_of_controls);

	for (i=2; i <= number_of_controls + 1; i++) {
		 
		if(number_of_exceptions > 0 && in_array(exceptions, number_of_exceptions, i))
			continue;
		
		SetCtrlAttribute(panel_id, i, ATTR_DIMMED, 1);
	}
	
	va_end(ap); 
	
	return UI_MODULE_SUCCESS;       
}

int ui_module_enable_panel(int panel_id, int number_of_exceptions, ...)
{
	int i, exceptions[50], number_of_controls = 0;
	
	va_list ap;       
	va_start(ap, number_of_exceptions);

	for(i=0; i < number_of_exceptions; i++)
		exceptions[i] = va_arg(ap, int); 
	
	GetPanelAttribute(panel_id, ATTR_NUM_CTRLS, &number_of_controls);

	for (i=2; i <= number_of_controls + 1; i++) {
		 
		if(number_of_exceptions > 0 && in_array(exceptions, number_of_exceptions, i))
			continue;
		
		SetCtrlAttribute(panel_id, i, ATTR_DIMMED, 0);
	}
	
	va_end(ap); 
	
	return UI_MODULE_SUCCESS;       
}

int ui_module_anchor_control_to_panel_edge(int panel_id, int ctrl_id, int left, int right, int top, int bottom)
{
	int panel_width, panel_height;
	int real_right, real_bottom;
	
	GetPanelAttribute(panel_id, ATTR_WIDTH, &panel_width);       
	GetPanelAttribute(panel_id, ATTR_HEIGHT, &panel_height);   
	
	real_right = panel_width - right;
	real_bottom = panel_height - bottom;
	
	SetCtrlAttribute(panel_id, ctrl_id, ATTR_LEFT, left);  
	SetCtrlAttribute(panel_id, ctrl_id, ATTR_WIDTH, real_right - left); 
	
	SetCtrlAttribute(panel_id, ctrl_id, ATTR_TOP, top);  
	SetCtrlAttribute(panel_id, ctrl_id, ATTR_HEIGHT, real_bottom - top); 
	
	return UI_MODULE_SUCCESS;  
}

int ui_module_move_control_pixels_from_right(int panel_id, int ctrl_id, int pixels)
{
	int panel_width, ctrl_width;
	
	GetPanelAttribute(panel_id, ATTR_WIDTH, &panel_width);       
	GetCtrlAttribute(panel_id, ctrl_id, ATTR_WIDTH, &ctrl_width);
	
	SetCtrlAttribute(panel_id, ctrl_id, ATTR_LEFT, panel_width - ctrl_width - pixels); 
	
	return UI_MODULE_SUCCESS;  
}


int ui_module_move_control_pixels_from_bottom(int panel_id, int ctrl_id, int pixels)
{
	int panel_height, ctrl_height;
	
	GetPanelAttribute(panel_id, ATTR_HEIGHT, &panel_height);       
	GetCtrlAttribute(panel_id, ctrl_id, ATTR_HEIGHT, &ctrl_height);
	
	SetCtrlAttribute(panel_id, ctrl_id, ATTR_TOP, panel_height - ctrl_height - pixels); 
	
	return UI_MODULE_SUCCESS;  
}


int ui_module_set_data_dir(UIModule *module, const char* path)
{
	char log_path[GCI_MAX_PATHNAME_LEN] = "";

	strncpy(module->_data_dir, path, GCI_MAX_PATHNAME_LEN - 1);
	
	sprintf(log_path, "%s\\Logs", path);

	// Check if Logs dir is created
	if(!FileExists(log_path, NULL))
		MakeDir(log_path);

	logger_set_data_dir(module->_logger, log_path);	
	
	return UI_MODULE_SUCCESS; 
}

int ui_module_get_data_dir(UIModule *module, char* path)
{
	strncpy(path, module->_data_dir, GCI_MAX_PATHNAME_LEN - 1);
	
	return UI_MODULE_SUCCESS; 
}

void ui_module_clear_attached_panel(UIModule *module, int panel_id)
{	// Written but UNTESTED!
	int i=0, found=-1;

	// Search for and restore panel settings
	for (i=0; i < module->_number_of_attachee_panels; i++) {

		AttacheePanelInfo panel = module->_attachee_panels[i];

		if (panel._attachee_panel_id == panel_id) {
			SetPanelAttribute(panel._attachee_panel_id, ATTR_FRAME_STYLE, panel._old_style);
			SetPanelAttribute(panel._attachee_panel_id, ATTR_TITLEBAR_VISIBLE, panel._old_titlebar_visible);
			SetPanelAttribute(panel._attachee_panel_id, ATTR_SIZABLE, panel._old_sizeable);
		
			found = i;

			break;
		}
	}

	if (found>=0) {

		module->_number_of_attachee_panels --;

		// move list down
		for (; i < module->_number_of_attachee_panels; i++) {
			
			module->_attachee_panels[i]._panel_id             = module->_attachee_panels[i+1]._panel_id;
			module->_attachee_panels[i]._attachee_panel_id    = module->_attachee_panels[i+1]._attachee_panel_id;
			module->_attachee_panels[i]._offset_x             = module->_attachee_panels[i+1]._offset_x;
			module->_attachee_panels[i]._offset_y             = module->_attachee_panels[i+1]._offset_y;
			module->_attachee_panels[i]._old_style            = module->_attachee_panels[i+1]._old_style;
			module->_attachee_panels[i]._old_titlebar_visible = module->_attachee_panels[i+1]._old_titlebar_visible;
			module->_attachee_panels[i]._old_sizeable         = module->_attachee_panels[i+1]._old_sizeable;
			module->_attachee_panels[i]._relative             = module->_attachee_panels[i+1]._relative;

		}
	}

}

void ui_module_clear_attached_panels(UIModule *module)
{
	int i=0;

	// Restore panel settings
	for (i=0; i < module->_number_of_attachee_panels; i++) {

		AttacheePanelInfo panel = module->_attachee_panels[i];

		SetPanelAttribute(panel._attachee_panel_id, ATTR_FRAME_STYLE, panel._old_style);
		SetPanelAttribute(panel._attachee_panel_id, ATTR_TITLEBAR_VISIBLE, panel._old_titlebar_visible);
		SetPanelAttribute(panel._attachee_panel_id, ATTR_SIZABLE, panel._old_sizeable);
	}

	memset(module->_attachee_panels, 0, sizeof(AttacheePanelInfo) * UI_MOD_MAX_ATTACHEE_PANELS);

	module->_number_of_attachee_panels = 0;
}

void ui_module_attach_panel_to_panel_advanced(UIModule *module, int parent_panel, int attchee_panel, UIModuleRelativePoint relative,
													  int offset_x, int offset_y)
{
	AttacheePanelInfo panel;

	panel._panel_id = parent_panel;
	panel._attachee_panel_id = attchee_panel;
	panel._offset_x = offset_x;
	panel._offset_y = offset_y;
	panel._relative = relative;

	GetPanelAttribute(panel._attachee_panel_id, ATTR_FRAME_STYLE, &(panel._old_style));
	GetPanelAttribute(panel._attachee_panel_id, ATTR_TITLEBAR_VISIBLE, &(panel._old_titlebar_visible));
	GetPanelAttribute(panel._attachee_panel_id, ATTR_SIZABLE, &(panel._old_sizeable));

	SetPanelAttribute(panel._attachee_panel_id, ATTR_FRAME_STYLE, VAL_OUTLINED_FRAME);
	
	if(panel._old_titlebar_visible)
		SetPanelAttribute(panel._attachee_panel_id, ATTR_TITLEBAR_VISIBLE, 0);

	SetPanelAttribute(panel._attachee_panel_id, ATTR_SIZABLE, 0);

	module->_attachee_panels[module->_number_of_attachee_panels++] = panel;

//	ui_module_display_panel(module, panel._attachee_panel_id);

	PositionAttachedWindow(module, panel);
}

void ui_module_attach_panel_to_panel(UIModule *module,int attchee_panel, UIModuleRelativePoint relative,
													  int offset_x, int offset_y)
{
	ui_module_attach_panel_to_panel_advanced(module, UIMODULE_MAIN_PANEL_ID(module), attchee_panel, relative, offset_x, offset_y);
}