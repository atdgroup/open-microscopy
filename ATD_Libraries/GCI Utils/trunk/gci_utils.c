#ifndef _CVI_
#include <ShObjIdl.h>
#endif

#include <shlobj.h> 

#include <ole2.h>
#include <objbase.h>
#include <initguid.h>
#include <windows.h>
#include "string_utils.h"

#include "ThreadDebug.h"

// Config.h can contain useful defines for locations like the binary output directory
// source directory etc. For constants see MicroscopeSystems/Galileo/MainApplication/Config.h.in
#ifdef LOCATIONS_DEFINED   
#include "Config.h"
#endif

#ifdef BUILT_RESOURCE_TABLE
#include "ResourceFiles.h"
#endif

#include <rs232.h>  
#include "toolbox.h"
#include <utility.h>
#include <userint.h>
#include "gci_utils.h"
#include "time.h"

#include "iniparser.h"
#include "string_utils.h"
#include "multiple-monitors.h"

#ifndef _CVI_ 
#include <setupapi.h>
#include <regstr.h>
#include "strlib.h"
#else
#include <ansi_c.h>
#endif

typedef struct
{
	int portNo;
	const char deviceName[200];
	int baudRate;
	int parity;
	int dataBits;
	int stopBits;
	int iqSize;
	int oqSize;
	double timeout;

} ComPortThreadInfo;

static int comport_thread_info_lock = -1;
static ComPortThreadInfo comport_thread_info;

static int _debug_level = -1;

static int _panel_id_for_default_dialog_monitor = 0;

static int gci_thread_pool_id = -1;

static int opened_ports_ref_count[50] = {0,0,0,0,0,0,0,0,0,0,
										 0,0,0,0,0,0,0,0,0,0,
										 0,0,0,0,0,0,0,0,0,0,
										 0,0,0,0,0,0,0,0,0,0,
										 0,0,0,0,0,0,0,0,0,0
										};

int gci_get_debug_level()
{
    char config_fullpath[GCI_MAX_PATHNAME_LEN] = "";
	char level_str[100] = "";

	if(_debug_level == -1) {

		if(find_resource("config.ini", config_fullpath) < 0)
			return -1;      
        							
		if(GetPrivateProfileString("Debug", "Level", "0", level_str, 500, config_fullpath) <= 0)
			return -1;

		sscanf(level_str, "%d", &_debug_level);
	}

	return	_debug_level;
}


int gci_thread_pool() 
{
	// First call so we create the thread pool
	if(gci_thread_pool_id == -1) {

		int err = 0;

		if ((err = CmtNewThreadPool (20, &gci_thread_pool_id)) < 0) {

			return err;
		}
	}

	return gci_thread_pool_id;
}

static unsigned char* ConvertHIconToByteArray(HDC panel_hdc, HDC draw_hdc, HICON hIcon, int width, int height)
{
	unsigned char *data = NULL;   
	BITMAPINFO bi;
	HBITMAP old_hbmp;
	HBITMAP bmp = CreateCompatibleBitmap(panel_hdc, width, height);           	
	
	old_hbmp = (HBITMAP) SelectObject(draw_hdc, bmp);
	
	SetStretchBltMode(draw_hdc, COLORONCOLOR);       

	DrawIconEx(draw_hdc, 0, 0, hIcon, width, height, 0, NULL, DI_COMPAT | DI_NORMAL); 
	
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = 32;
	bi.bmiHeader.biHeight = 32;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	
	SelectObject(draw_hdc, old_hbmp);    
	
	data = (unsigned char*) malloc(128 * 32);
	
	GetDIBits(draw_hdc, bmp, 0, 32, data, &bi, DIB_RGB_COLORS);
	
	DeleteObject(bmp);          
	
	return data;
}


static unsigned char* ConvertHIconMaskToByteArray(HDC hdc, HICON hIcon, int width, int height)
{
	int i;
	unsigned char *data = NULL;   
	unsigned char *alpha_byte_data = NULL;   
	BITMAPINFO bi;
	HBITMAP old_hbmp;
	HBITMAP bmp = CreateCompatibleBitmap(hdc, width, height);           	
	
	old_hbmp = (HBITMAP) SelectObject(hdc, bmp);
	
	DrawIconEx(hdc, 0, 0, hIcon, width, height, 0, NULL, DI_COMPAT | DI_MASK); 
	
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = 32;
	bi.bmiHeader.biHeight = 32;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	
	SelectObject(hdc, old_hbmp);    
	
	data = (unsigned char*) malloc(4 * 32 * 32);
	alpha_byte_data = (unsigned char*) malloc(32 * 32);    
		
	memset(data, 0, 4 * 32 * 32);
	
	GetDIBits(hdc, bmp, 0, 32, data, &bi, DIB_RGB_COLORS);
	
	// CVI expects black doesn't draw . The mask data we have has black pixels 
	// at the place we wish to draw so we need to reverse the mask values.

	for(i=0; i < 1024; i++) {
		
		alpha_byte_data[i] = data[i * 4];
		
		if(alpha_byte_data[i] > 0)
			alpha_byte_data[i] = 0;
		else
			alpha_byte_data[i] = 255;
	}
	
	
	free(data);
	
	DeleteObject(bmp);

	return alpha_byte_data;
}


// This function draws a bitmap onto a canvas where the vertical axis is flipped.
// This is very ineffient please dont use this function else where.
// I can get away with it as I am only displays images 32x32.
static int CanvasDrawEntireBitmapVflip(int panelHandle, int controlID, int nBitmapID)
{
	int height, width, r1;

	GetCtrlAttribute (panelHandle, controlID, ATTR_HEIGHT, &height);
	GetCtrlAttribute (panelHandle, controlID, ATTR_WIDTH,&width);

	CanvasStartBatchDraw (panelHandle, controlID);

	for(r1=0; r1 < height; r1++) {
		
		CanvasDrawBitmap (panelHandle, controlID, nBitmapID,
			MakeRect (r1, 0, 1, width), MakeRect (height - r1 - 1, 0, 1, width));
	}

	CanvasEndBatchDraw (panelHandle, controlID);

	return 0;
}

static void DrawIconToCanvas(int panel_id, int canvas_id, char * icon)
{
	HDC panel_hdc, dc;
	HICON hIcon; 
	int cvi_bitmap;
	unsigned char *data = NULL;
	unsigned char *alpha_data = NULL;   
	
	int handle;

    GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &handle);

    panel_hdc = GetDC ((HWND) handle);

	dc = CreateCompatibleDC(panel_hdc);
	assert(dc != NULL);
	
	hIcon = LoadIcon ( NULL, icon);
	
	data = ConvertHIconToByteArray(panel_hdc, dc, hIcon, 32, 32);
	alpha_data = ConvertHIconMaskToByteArray(dc, hIcon, 32, 32);
	
	NewBitmapEx (128, 32, 32, 32, NULL, data, NULL, alpha_data, &cvi_bitmap); 
		
	CanvasDrawEntireBitmapVflip(panel_id, canvas_id, cvi_bitmap);  
	
	ReleaseDC((HWND) handle, dc);       
	
	DeleteDC(dc);

	// The icon is not longer needed
	DestroyIcon(hIcon);

	DiscardBitmap(cvi_bitmap);
	
	free(data);
	free(alpha_data);
}




static int CreateGciButton(int panel_id, char *text)
{
	int button_id = NewCtrl(panel_id, CTRL_SQUARE_COMMAND_BUTTON, text, 5, 10);
	
	SetCtrlAttribute(panel_id , button_id, ATTR_CMD_BUTTON_COLOR, MakeColor(30, 87, 174));
	SetCtrlAttribute(panel_id, button_id, ATTR_LABEL_COLOR, VAL_WHITE); 

	return button_id;
}

#ifndef _CVI_

int GetAllDevices(void)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i;

	// Create a HDEVINFO with all present devices.
	hDevInfo = SetupDiGetClassDevs(NULL,
		0, // Enumerator
		0,
		DIGCF_PRESENT | DIGCF_ALLCLASSES );

	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		// Insert error handling here.
		return 1;
	}

	// Enumerate through all devices in Set.

	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,
		&DeviceInfoData);i++)
	{
		DWORD DataT;
		LPTSTR buffer = NULL;
		DWORD buffersize = 0;

		//
		// Call function with null to begin with, 
		// then use the returned buffer size (doubled)
		// to Alloc the buffer. Keep calling until
		// success or an unknown failure.
		//
		//  Double the returned buffersize to correct
		//  for underlying legacy CM functions that 
		//  return an incorrect buffersize value on 
		//  DBCS/MBCS systems.
		// 
		while (!SetupDiGetDeviceRegistryProperty(
			hDevInfo,
			&DeviceInfoData,
			SPDRP_DEVICEDESC,
			&DataT,
			(PBYTE)buffer,
			buffersize,
			&buffersize))
		{
			if (GetLastError() == 
				ERROR_INSUFFICIENT_BUFFER)
			{
				// Change the buffer size.
				if (buffer) LocalFree(buffer);
				// Double the size to avoid problems on 
				// W2k MBCS systems per KB 888609. 
				buffer = (LPTSTR) LocalAlloc(LPTR,buffersize * 2);
			}
			else
			{
				// Insert error handling here.
				break;
			}
		}

		printf("Result:[%s]\n",buffer);

		if (buffer) LocalFree(buffer);
	}


	if ( GetLastError()!=NO_ERROR &&
		GetLastError()!=ERROR_NO_MORE_ITEMS )
	{
		// Insert error handling here.
		return 1;
	}

	//  Cleanup
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return 0;
}

int IsDeviceConnected(const char* name)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i;

	// Create a HDEVINFO with all present devices.
	hDevInfo = SetupDiGetClassDevs(NULL,
		0, // Enumerator
		0,
		DIGCF_PRESENT | DIGCF_ALLCLASSES );

	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		// Insert error handling here.
		return 1;
	}

	// Enumerate through all devices in Set.

	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,
		&DeviceInfoData);i++)
	{
		DWORD DataT;
		LPTSTR buffer = NULL;
		DWORD buffersize = 0;

		//
		// Call function with null to begin with, 
		// then use the returned buffer size (doubled)
		// to Alloc the buffer. Keep calling until
		// success or an unknown failure.
		//
		//  Double the returned buffersize to correct
		//  for underlying legacy CM functions that 
		//  return an incorrect buffersize value on 
		//  DBCS/MBCS systems.
		// 
		while (!SetupDiGetDeviceRegistryProperty(
			hDevInfo,
			&DeviceInfoData,
			SPDRP_DEVICEDESC,
			&DataT,
			(PBYTE)buffer,
			buffersize,
			&buffersize))
		{
			if (GetLastError() == 
				ERROR_INSUFFICIENT_BUFFER)
			{
				// Change the buffer size.
				if (buffer) LocalFree(buffer);
				// Double the size to avoid problems on 
				// W2k MBCS systems per KB 888609. 
				buffer = (LPTSTR) LocalAlloc(LPTR,buffersize * 2);
			}
			else
			{
				// Insert error handling here.
				break;
			}
		}

		if(strcmp(buffer, name) == 0)
			return 1;

		if (buffer) LocalFree(buffer);
	}


	if ( GetLastError()!=NO_ERROR &&
		GetLastError()!=ERROR_NO_MORE_ITEMS )
	{
		// Insert error handling here.
		return 1;
	}

	//  Cleanup
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return 0;
}

#endif

int GCIDialogNoButtons (int parent, char title[], char * icon, char fmt[], ...)
{
	va_list ap;
	char message[5000];
	int panel_width, panel_height, subpanel_width, subpanel_height, colour, subpanel_id, panel_id;
	int label_id, canvas_id, label_width, label_height;
	
	va_start(ap, fmt);

	vsprintf(message, fmt, ap);

	va_end(ap);

	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	
	panel_id = NewPanel(parent, title, 0, 0, 100, 200);
	subpanel_id = NewPanel(panel_id, "", 0, 0, 100, 200);   
	
	SetPanelAttribute(subpanel_id, ATTR_TITLEBAR_VISIBLE, 0);      
	SetPanelAttribute(subpanel_id, ATTR_SIZABLE, 0);
	SetPanelAttribute(subpanel_id, ATTR_SYSTEM_MENU_VISIBLE, 0);
	SetPanelAttribute(subpanel_id, ATTR_FRAME_STYLE, VAL_HIDDEN_FRAME); 
	SetPanelAttribute(subpanel_id, ATTR_FLOATING, VAL_FLOAT_ALWAYS);         
	
	label_id = NewCtrl(subpanel_id, CTRL_TEXT_MSG, message, 0, 0);
	canvas_id = NewCtrl(subpanel_id, CTRL_CANVAS, "", 0, 0);     
		
	SetCtrlAttribute(subpanel_id, canvas_id, ATTR_WIDTH, 32);
	SetCtrlAttribute(subpanel_id, canvas_id, ATTR_HEIGHT, 32); 
	
	GetPanelAttribute(subpanel_id, ATTR_BACKCOLOR, &colour);
	SetCtrlAttribute(subpanel_id, canvas_id, ATTR_PICT_BGCOLOR, colour);
	
	SetCtrlAttribute(subpanel_id, label_id, ATTR_SIZE_TO_TEXT, 1);
	
	GetCtrlAttribute(subpanel_id, label_id, ATTR_WIDTH, &label_width);
	GetCtrlAttribute(subpanel_id, label_id, ATTR_HEIGHT, &label_height);    
	
	subpanel_width = label_width + 60;
	subpanel_height = label_height + 35;
	
	SetPanelAttribute(subpanel_id, ATTR_WIDTH, subpanel_width);
	SetPanelAttribute(subpanel_id, ATTR_HEIGHT, subpanel_height);    
	
	// Position label on subpanel
	SetCtrlAttribute(subpanel_id, label_id, ATTR_LEFT, 42);    
	SetCtrlAttribute(subpanel_id, label_id, ATTR_TOP, 10);        

	// Position Canvas
	SetCtrlAttribute(subpanel_id, canvas_id, ATTR_TOP, 10);  
	
	SetPanelAttribute(panel_id, ATTR_WIDTH, subpanel_width + 40);
	SetPanelAttribute(panel_id, ATTR_HEIGHT, subpanel_height + 10);    
	
	GetPanelAttribute(panel_id, ATTR_WIDTH, &panel_width);
	GetPanelAttribute(panel_id, ATTR_HEIGHT, &panel_height);    
	SetPanelAttribute(subpanel_id, ATTR_TOP, (panel_height - subpanel_height) / 2);      
	
	// VAL_AUTO_CENTER For ATT_TOP Does not work as cvi thinks the titlebar is visible
	SetPanelAttribute(subpanel_id, ATTR_LEFT, VAL_AUTO_CENTER);           
	
	SetPanelAttribute(panel_id, ATTR_LEFT, VAL_AUTO_CENTER);           
	SetPanelAttribute(panel_id, ATTR_TOP, VAL_AUTO_CENTER);  
	
	SetPanelAttribute(panel_id, ATTR_SIZABLE, 0);
	SetPanelAttribute(panel_id, ATTR_SYSTEM_MENU_VISIBLE, 0);
	
	SetPanelAttribute(panel_id, ATTR_FLOATING, 1);
	
	DisplayPanel(subpanel_id); 
	DisplayPanel(panel_id);   
	
	DrawIconToCanvas(subpanel_id, canvas_id, icon);          
	
	return panel_id;    
}


static int GCIDialogAdvanced (int parent, const char title[], char * icon, int buttons, int yes_no, const char fmt[], va_list ap)
{
/*
	buttons:
		GCI_CANCEL_BUTTON
		GCI_OK_BUTTON

	icons:
		IDI_APPLICATION Default application icon.
		IDI_ASTERISK Asterisk icon. Same as IDI_INFORMATION.
		IDI_ERROR Hand-shaped icon.
		IDI_EXCLAMATION Exclamation point icon. Same as IDI_WARNING.
		IDI_HAND Hand-shaped icon. Same as IDI_ERROR.
		IDI_INFORMATION Asterisk icon.
		IDI_QUESTION Question mark icon.
		IDI_SHIELD Security Shield icon.
		IDI_WARNINGE xclamation point icon.
		IDI_WINLOGO Default application icon.

*/

	char message[5000];
	int panel_width, panel_height, subpanel_width, subpanel_height, colour, subpanel_id, panel_id;
	int label_id, canvas_id, label_width, label_height, tmp_pnl, button_pressed_id, pressed_button;
	int cancel_button, ok_button, ok_button_width = 0, ok_button_height = 0, cancel_button_width = 0, cancel_button_height;
	
	vsprintf(message, fmt, ap);

	panel_id = NewPanel(parent, title, 0, 0, 100, 200);
	subpanel_id = NewPanel(panel_id, "", 0, 0, 100, 200);   

	SetPanelAttribute(subpanel_id, ATTR_TITLEBAR_VISIBLE, 0);      
	SetPanelAttribute(subpanel_id, ATTR_SIZABLE, 0);
	SetPanelAttribute(subpanel_id, ATTR_SYSTEM_MENU_VISIBLE, 0);
	SetPanelAttribute(subpanel_id, ATTR_FRAME_STYLE, VAL_HIDDEN_FRAME);   
	SetPanelAttribute(subpanel_id, ATTR_FLOATING, VAL_FLOAT_ALWAYS); 
	
	label_id = NewCtrl(subpanel_id, CTRL_TEXT_MSG, message, 0, 0);
	canvas_id = NewCtrl(subpanel_id, CTRL_CANVAS, "", 0, 0);     
	
	if(icon == NULL)
		SetCtrlAttribute(subpanel_id, canvas_id, ATTR_VISIBLE, 0);

	if(buttons & GCI_OK_BUTTON) {

		if(yes_no)
			ok_button = CreateGciButton(subpanel_id, "Yes");   
		else
			ok_button = CreateGciButton(subpanel_id, "OK");   

		GetCtrlAttribute(subpanel_id, ok_button, ATTR_WIDTH, &ok_button_width);
		GetCtrlAttribute(subpanel_id, ok_button, ATTR_HEIGHT, &ok_button_height);    
	}
		
	if(buttons & GCI_CANCEL_BUTTON) {  

		if(yes_no)
			cancel_button = CreateGciButton(subpanel_id, "No");    
		else
			cancel_button = CreateGciButton(subpanel_id, "Cancel");    

		GetCtrlAttribute(subpanel_id, cancel_button, ATTR_WIDTH, &cancel_button_width);
		GetCtrlAttribute(subpanel_id, cancel_button, ATTR_HEIGHT, &cancel_button_height);    
	}
		
	SetCtrlAttribute(subpanel_id, canvas_id, ATTR_WIDTH, 32);
	SetCtrlAttribute(subpanel_id, canvas_id, ATTR_HEIGHT, 32); 
	
	GetPanelAttribute(subpanel_id, ATTR_BACKCOLOR, &colour);
	SetCtrlAttribute(subpanel_id, canvas_id, ATTR_PICT_BGCOLOR, colour);
	
	SetCtrlAttribute(subpanel_id, label_id, ATTR_SIZE_TO_TEXT, 1);
	
	GetCtrlAttribute(subpanel_id, label_id, ATTR_WIDTH, &label_width);
	GetCtrlAttribute(subpanel_id, label_id, ATTR_HEIGHT, &label_height);    
	
	subpanel_width = label_width + 60;
	subpanel_height = ok_button_height + label_height + 35;
	
	SetPanelAttribute(subpanel_id, ATTR_WIDTH, subpanel_width);
	SetPanelAttribute(subpanel_id, ATTR_HEIGHT, subpanel_height);    
	
	// Position label on subpanel
	if(icon == NULL)
		SetCtrlAttribute(subpanel_id, label_id, ATTR_LEFT, 10);    
	else
		SetCtrlAttribute(subpanel_id, label_id, ATTR_LEFT, 42);    
	
	SetCtrlAttribute(subpanel_id, label_id, ATTR_TOP, 10);        

	// Position Canvas
	SetCtrlAttribute(subpanel_id, canvas_id, ATTR_TOP, 10);  
	
	// Position ok/yes button on subpanel to the left of cancel/no
	if(buttons & GCI_OK_BUTTON) {   
		SetCtrlAttribute(subpanel_id, ok_button, ATTR_LEFT, subpanel_width - ok_button_width - cancel_button_width - 10);    
		SetCtrlAttribute(subpanel_id, ok_button, ATTR_TOP, subpanel_height - ok_button_height - 5);  
		SetCtrlAttribute(subpanel_id, ok_button, ATTR_SHORTCUT_KEY, VAL_ENTER_VKEY);  
	}
	
	if(buttons & GCI_CANCEL_BUTTON) {   
		// Position cancel/no button on subpanel
		SetCtrlAttribute(subpanel_id, cancel_button, ATTR_LEFT, subpanel_width - cancel_button_width - 5);    
		SetCtrlAttribute(subpanel_id, cancel_button, ATTR_TOP, subpanel_height - ok_button_height - 5);     
		SetCtrlAttribute(subpanel_id, ok_button, ATTR_SHORTCUT_KEY, VAL_ESC_VKEY);  
	}
	
	SetPanelAttribute(panel_id, ATTR_WIDTH, subpanel_width + 40);
	SetPanelAttribute(panel_id, ATTR_HEIGHT, subpanel_height + 10);    
	
	GetPanelAttribute(panel_id, ATTR_WIDTH, &panel_width);
	GetPanelAttribute(panel_id, ATTR_HEIGHT, &panel_height);    
	SetPanelAttribute(subpanel_id, ATTR_TOP, (panel_height - subpanel_height) / 2);      
	
	// VAL_AUTO_CENTER For ATT_TOP Does not work as cvi thinks the titlebar is visible
	SetPanelAttribute(subpanel_id, ATTR_LEFT, VAL_AUTO_CENTER);           
	
	SetPanelAttribute(panel_id, ATTR_LEFT, VAL_AUTO_CENTER);           
	SetPanelAttribute(panel_id, ATTR_TOP, VAL_AUTO_CENTER);  
	
	SetPanelAttribute(panel_id, ATTR_SIZABLE, 0);
	SetPanelAttribute(panel_id, ATTR_SYSTEM_MENU_VISIBLE, 0);
	
	SetPanelAttribute(panel_id, ATTR_FLOATING, 1);
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);

	DisplayPanel(subpanel_id); 

	if(icon != NULL)
		DrawIconToCanvas(subpanel_id, canvas_id, icon);          
	
	if(buttons == 0)
		return panel_id; 	
	
	GCI_MovePanelToDefaultMonitorForDialogs(panel_id);
	
	InstallPopup (panel_id);  

	while(1) {
		
		ProcessSystemEvents();

		GetUserEvent (0, &tmp_pnl, &button_pressed_id);
		
		if (tmp_pnl == subpanel_id)
			break;
	}

	if(ok_button == button_pressed_id)
		pressed_button = GCI_OK_BUTTON;
	else if(cancel_button == button_pressed_id)
		pressed_button = GCI_CANCEL_BUTTON;
	else
		pressed_button = 0;
	
	DiscardPanel(panel_id);
	
	return pressed_button;
}

int GCIDialog (int parent, char title[], char * icon, int buttons, char fmt[], ...)
{
	int ret = 0;
	va_list ap;

	va_start(ap, fmt);

	ret = GCIDialogAdvanced (parent, title, icon, buttons, 0, fmt, ap);

	va_end(ap);

	return ret;
}

// This function sets the panel_id whose monitor that popup dialogs will use 
void GCI_SetPanelsMonitorAsDefaultForDialogs(int panel_id)
{
	_panel_id_for_default_dialog_monitor = panel_id;
}

void GCI_MovePanelToDefaultMonitorForDialogs(int panel_id)
{
	if(_panel_id_for_default_dialog_monitor > 0) {
		CenterWindowOnOtherWindowsMonitor(_panel_id_for_default_dialog_monitor, panel_id);
	}
}

int GCI_ConfirmPopup (char *title, char * icon, char fmt[], ...)
{
	int button_pressed = 0;
	va_list ap;

	va_start(ap, fmt);

	button_pressed = GCIDialogAdvanced (0, title, icon, GCI_OK_BUTTON | GCI_CANCEL_BUTTON, 1, fmt, ap);

	if(button_pressed == GCI_OK_BUTTON)
		return 1;

	return 0;
}


int GCI_MessagePopup (const char title[], const char fmt[], ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);

	ret = GCIDialogAdvanced (0, title, NULL, GCI_OK_BUTTON, 0, fmt, ap);

	va_end(ap);

	return ret;   
}

int GCI_ErrorPopup (const char title[], const char fmt[], ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);

	ret = GCIDialogAdvanced (0, title, IDI_ERROR, GCI_OK_BUTTON, 0, fmt, ap);

	va_end(ap);

	return ret;   
}
	
void GetFilenameFromPath(const char *filepath, char *fileName)
{
	char driveName[MAX_DRIVENAME_LEN];
	char dirName[MAX_DIRNAME_LEN];

	SplitPath (filepath, driveName, dirName, fileName);
}

void GetDirectoryForFile(const char *filepath, char *directory)
{
	char driveName[MAX_DRIVENAME_LEN];
	char dirName[MAX_DIRNAME_LEN];
	char fileName[MAX_FILENAME_LEN];

	SplitPath (filepath, driveName, dirName, fileName);
	
	if(directory != NULL) {
		strcpy(directory, driveName);
		strcat(directory, dirName);	
	}
}


/*
void GetGCIProjectyDirectory(char *directory)
{
	char drive_name[10];
	char directory_name[300];
	char filename[300];
	char project_dir[300]; 
	char real_project_dir[300];
	
	GetProjectDir(project_dir);  
	
	SplitPath (project_dir, drive_name, directory_name, filename);

	memset(real_project_dir, 0, 300);
	
	// If the project directory is a "bin" directory we are not
	// running from a distribution kit.
	// We must go up one directory.
	if(strcmp(filename, "bin") == 0) {
		strcat(real_project_dir, drive_name); 
		strcat(real_project_dir, directory_name);
	}
	else {
		sprintf(real_project_dir, "%s\\", project_dir); 
	}
		
	strcpy(directory, real_project_dir); 
		
	return;
}
*/

/*

int FindAndLoadUIR(int parentPanelHandle, char *filename, int panelResourceID)
{
	char project_dir[300];
	char uir_path[400];
	
	GetGCIProjectyDirectory(project_dir);  
	
	if (!str_get_path_for_file(project_dir, filename, uir_path)) {
		
		return -1;	
	}
	
	return LoadPanel (parentPanelHandle, uir_path, panelResourceID);
}
*/

int LoadIconIntoButton(int PanelID, int ButtonID, char *dir, char *icon_name)
{
	int  file_size;
	char icon_path[GCI_MAX_PATHNAME_LEN];
	
	strcpy(icon_path, dir);
	strcat(icon_path, icon_name);
	
	if(!FileExists (icon_path, &file_size)) {
	
		printf("%s not found!\n", icon_path);
		return -1;
	}
	
	SetCtrlAttribute(PanelID, ButtonID, ATTR_IMAGE_FILE, icon_path);
	
	return 0;
}	


int DisplayImageFileAtDirectory(int PanelID, int controlID, const char *dir, char *filename)
{
	int  file_size;
	char icon_path[GCI_MAX_PATHNAME_LEN];
	
	strcpy(icon_path, dir);
	strcat(icon_path, filename);
	
	if(!FileExists (icon_path, &file_size)) {
	
		printf("%s not found!", icon_path);
		return -1;
	}
	
	DisplayImageFile (PanelID, controlID, icon_path);
	
	return 0;
}

/*
int FindPathForFile(const char *filename, char* filepath)
{
	char project_dir[300];
	
	GetGCIProjectyDirectory(project_dir);
	
	if (!str_get_path_for_file(project_dir, filename, filepath)) {
		
		return -1;	
	}
	
	return 0;
}
*/

static int DoesFileExist(const char *directory, const char *name, char *fullpath)
{
	int fileSize = 0;  
	char tmp[500] = "";  
	
	memset(fullpath, 0, 1);
	
	MakePathname (directory, name, tmp);   
		
	if(FileExists(tmp, &fileSize)) {
		strcpy(fullpath, tmp);
		return 1;
	}
		
	return 0;
}

	
int search_system_path_for_file(const char *filename, char *filepath)
{
    char *dir;
   
    // Allocate  buffer
    char system_path[5000] = "";
   
    filepath[0] = '0';   
   
    // Get temp dir
    if(!GetEnvironmentVariable("Path", system_path, 5000))
        GCI_ErrorPopup("Error", "Can not retrieve the path variable");

    dir = strtok (system_path, ";"); 

    while(dir != NULL) {
   
        // Search directory for file
        if(DoesFileExist(dir, filename, filepath))
            return 0;
       
        dir = strtok (NULL, ";"); 
    }
   
    return -1;
}


int find_resource_path_relative_to_exe(const char *filename, char *fullpath)
{
	/* Get the name of the directory that contains myfile. */
	int file_size = 0;
	char projectDir[GCI_MAX_PATHNAME_LEN];

	// the cvi docs are completly wrong this seems to always return the exe directory,
	// which is what we want.
	if (GetProjectDir (projectDir) < 0)
		return -1;

	MakePathname (projectDir, filename, fullpath);

	// Does the file exist ?
	if(FileExists(fullpath, &file_size) <= 0) {	
		// Clear return path
		fullpath[0] = 0;
		return -1;
	}
	
	return 0;
}

static int find_resource_relative_to_path(const char *path, const char *filename, char *fullpath)
{
	/* Get the name of the directory that contains myfile. */
	int file_size = 0;

	fullpath[0] = 0;

	MakePathname (path, filename, fullpath);

	// Does the file exist ?
	if(FileExists(fullpath, &file_size) <= 0) {
		// Clear return path
		fullpath[0] = 0;
		return -1;
	}
	
	return 0;
}

// Find a resource from certain paths.
// Order of search is as follows.
// Search the binary directory.
// Search directory defined by RUNTIME_OUTPUT_DIRECTORY this is defined by the cmake build
// Search the system path. Useful for when dll's are using resources and ar in the path.
static int find_resource_from_paths(const char *name, char *fullpath)
{
	// Clear passed in buffer.
	memset(fullpath, 0, 1);
	
	#ifdef LOCATIONS_DEFINED

		if(find_resource_relative_to_path(RUNTIME_OUTPUT_DIRECTORY, name, fullpath) == 0)
			return 0;

		if(find_resource_path_relative_to_exe(name, fullpath) == 0)
			return 0;

	#else

	// the cvi docs are completly wrong this seems to always return the exe directory,
	// which is what we want.
	if(find_resource_path_relative_to_exe(name, fullpath) == 0)
		return 0;

	#endif

	// Try the system path
	if(search_system_path_for_file(name, fullpath) == 0)
		return 0;
	
	return -1;
}


int find_resource_from_ini_file(const char* name, char *fullpath)
{
	char relative_path[1000];
	char path[2000];
	LPSTR *tmp = NULL;
	int file_size = 0;  
	
	// Clear passed in buffer.
	memset(fullpath, 0, 1);
	
	#ifdef LOCATIONS_DEFINED

		if(find_resource_relative_to_path(RUNTIME_OUTPUT_DIRECTORY, "config.ini", path) < 0) {
			if(find_resource_from_paths("config.ini", path) < 0) {
				return -1;
			}
		}
	#else

	if(find_resource_from_paths("config.ini", path) < 0)
		return -1;

	#endif

	if(GetPrivateProfileString("Resources", name, NULL, relative_path, 5000, path) <= 0)
		return -1;

	if(GetFullPathName(relative_path, 5000, fullpath, tmp) <= 0)
		return -1;

	// Does the file exist ?
	if(FileExists(fullpath, &file_size) <= 0)
		return -1;
	
	return 0;
}

#ifdef BUILT_RESOURCE_TABLE
static char* get_filename_from_fullpath_in_resource_table(const char *fullpath)
{
	char *tmp = (char*) fullpath;

	tmp += strlen(fullpath);

	while(*tmp != '\\')
		tmp--;

	return tmp + 1;
}

static int get_uir_path_form_resource_table(const char *name, char *fullpath)
{
	int i=0;
	char name_without_path[500];
	char lower_name[500] = "";
	char lower_table_name[500] = "";

	fullpath[0] = '\0';

	// Strip any path info from name incase something like buttons/new.ico is passed in
	get_filename_from_fullpath(name, name_without_path);

	for(i=0;;i++) {
		char *path = resource_files[i];
		char *table_name = NULL;

		if(path == NULL)
			break;

		table_name = get_filename_from_fullpath_in_resource_table(path);

		strtolwr(name_without_path, lower_name);
		strtolwr(table_name, lower_table_name);

		if(strcmp(lower_name, lower_table_name) == 0) {
			strcpy(fullpath, path);
			return 0;
		}
	}

	return -1;
}
#endif

// Find a resource.
// Order of search is as follows.
// Search binary directory.
// Search the system path. Useful for when dll's are using resources and ar in the path.
// Get the path from an ini file next to the binary.
int find_resource(const char *name, char *fullpath)
{
	if(find_resource_from_paths(name, fullpath) == 0)
		return 0;
	
	#ifdef BUILT_RESOURCE_TABLE
	if(get_uir_path_form_resource_table(name, fullpath) == 0)
		return 0;
	#endif

	// Try the ini file
	if(find_resource_from_ini_file(name, fullpath) == 0)
		return 0;

	return -1;
}

int get_device_int_param_from_ini_file(const char* device_name, char *param, int *value)
{
	char value_str[100] = "";
	
	if(get_device_string_param_from_ini_file(device_name, param, value_str) < 0)
		return -1;

	sscanf(value_str, "%i", value);
	
	return 0;
}

int get_device_double_param_from_ini_file(const char* device_name, char *param, double *value)
{
	char value_str[100];
	float fval;
	
	if(get_device_string_param_from_ini_file(device_name, param, value_str) < 0)
		return -1;

	sscanf(value_str, "%f", &fval);
	
	*value = (double)fval;
	
	return 0;
}

int section_from_ini_file_exists(const char* device_name)
{
	char path[2000];
	char value[2000];

	if(find_resource_from_paths("config.ini", path) < 0)
		return -1;

	if(GetPrivateProfileString(device_name, NULL, NULL, value, 500, path) <= 0)
		return 0;

	return 1;
}

int get_device_string_param_from_ini_file(const char* device_name, char *param, char *value)
{
	char path[2000], *p;
	char t[2000];

	value[0] = 0;

	if(find_resource_from_paths("config.ini", path) < 0)
		return -1;

	if(GetPrivateProfileString(device_name, param, NULL, value, 500, path) <= 0)
		return -1;

	p = strtok (value, ";");
	strcrop_r(p, t);
	strcpy(value, t);

	return 0;
}

int get_device_param_from_ini_file(const char* device_name, char *param, unsigned int *value)
{   // Legacy
	return get_device_int_param_from_ini_file(device_name, param, value);
}


// GetLinkInfo() fills the filename and path buffer
// with relevant information.
// hWnd         - calling application's window handle.
//
// lpszLinkName - name of the link file passed into the function.
//
// lpszPath     - the buffer that receives the file's path name.
//
// lpszDescription - the buffer that receives the file's
// description.
static HRESULT
GetLinkInfo( HWND    hWnd,
             LPCTSTR lpszLinkName,
             LPSTR   lpszPath,
             LPSTR   lpszDescription)
{

	HRESULT hres;
    IShellLink *pShLink;
    WIN32_FIND_DATA wfd;

    // Initialize the return parameters to null strings.
    *lpszPath = '\0';
    *lpszDescription = '\0';

	// Call CoCreateInstance to obtain the IShellLink
   	// Interface pointer. This call fails if
   	// CoInitialize is not called, so it is assumed that
   	// CoInitialize has been called.
    hres = CoCreateInstance( &CLSID_ShellLink,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             &IID_IShellLink,
                             (LPVOID *)&pShLink );

    if (SUCCEEDED(hres))
    {
		IPersistFile *ppf;

   		// The IShellLink Interface supports the IPersistFile
   		// interface. Get an interface pointer to it.
        hres = pShLink->lpVtbl->QueryInterface(pShLink,
                                            &IID_IPersistFile,
                                            (LPVOID *)&ppf );
        if (SUCCEEDED(hres))
        {
        	WORD wsz[MAX_PATH];

   			// Convert the given link name string to a wide character string.
            MultiByteToWideChar( CP_ACP, 0,
                                 lpszLinkName,
                                 -1, wsz, MAX_PATH );
   			
   			// Load the file.
            hres = ppf->lpVtbl->Load(ppf, wsz, STGM_READ );
            if (SUCCEEDED(hres))
            {
   				// Resolve the link by calling the Resolve() interface function.
   				// This enables us to find the file the link points to even if
   				// it has been moved or renamed.
               	hres = pShLink->lpVtbl->Resolve(pShLink,  hWnd,
                                               SLR_ANY_MATCH | SLR_NO_UI);
               	if (SUCCEEDED(hres))
               	{
   					// Get the path of the file the link points to.
                	hres = pShLink->lpVtbl->GetPath( pShLink, lpszPath,
                                               MAX_PATH,
                                               &wfd,
                                               SLGP_SHORTPATH );

   					// Only get the description if we successfully got the path
   					// (We can't return immediately because we need to release ppf &
   					//  pShLink.)
                  	if(SUCCEEDED(hres))
                  	{
   						// Get the description of the link.
                    	hres = pShLink->lpVtbl->GetDescription(pShLink,
                                                         lpszDescription,
                                                         MAX_PATH );
                  	}
               	}
            }
            
            ppf->lpVtbl->Release(ppf);
         }
         
         pShLink->lpVtbl->Release(pShLink);
      }
      
	return hres;
}


/* This popup fileselector handles the case when a client selects a shortcut or lnk file.
   It uses the COM interface ShellLink to get the location the links points to. 
   It then creates a new file popup for this location.
   The old FilePopup fails on shortcuts. 
*/
int LessLameFileSelectPopup (int panel_id, char defaultDirectory[], char defaultFileSpec[], char fileTypeList[],
							 char title[], int buttonLabel, int restrictDirectory,
							 int restrictExtension, int allowCancel, int allowMakeDirectory, char pathName[])
{
	char ext[500], lnk_path[GCI_MAX_PATHNAME_LEN], lnk_description[500];
	int window_handle, ret;
	
	if((ret = FileSelectPopup (defaultDirectory, defaultFileSpec, fileTypeList,
					 title, buttonLabel, restrictDirectory,
					 restrictExtension,  allowCancel, allowMakeDirectory, pathName)) <= 0) {
					 
		return ret;
	}
	
	get_file_extension(pathName, ext); 
	
	if(strcmp(ext, ".lnk") == 0) {
		
		GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);
		
		if( FAILED(GetLinkInfo( (HWND) window_handle, pathName, lnk_path, lnk_description)) )
			return -1;
		
		if(FileSelectPopup (lnk_path, defaultFileSpec, fileTypeList,
					 title, buttonLabel, restrictDirectory,
					 restrictExtension,  allowCancel, allowMakeDirectory, pathName) < 0)
		{
			return -1;
		}
	}
	
	return 1;
}


void LogError(const char *device, const char *message)
{
	char path[GCI_MAX_PATHNAME_LEN];
	FILE *fp;
	static int lock=0;
	
	//Log faults/recoveries to a text file in the data folder
	
	GetPrivateDataFolder("Microscope Data", path);
	strcat(path, "\\Error.log");

	if (lock == 0) {
		GciCmtNewLock ("GciUtilsLogError", 0, &lock);
		fp = fopen (path, "w");	   //create log file
		fclose(fp);
	}
	
	GciCmtGetLock (lock);
		
	fp = fopen (path, "a");	   //append to log file
		
	if (fp == NULL) return;
	
	fprintf(fp, "%s\t%s\n", device, message);
	fclose(fp);
	
	GciCmtReleaseLock (lock);
}


int GetPrivateDataFolder(char *folder, char *path)
{
	//char *command;
	
	//Return the full path of the folder where application data is stored.
	//e.g. "c:\\program files\\microfocus data\\"
	
	if (GetProjectDir(path) != 0)
		return -1;
	
	strcat(path, "\\");
	strcat(path, folder);
	
	//If it doesn't exist create it.
	if (!FileExists(path, NULL)) {
		if (MakeDir (path) != 0)
			return -1;		//Disk full?
	}
	
	//Note: Permissions are now set at installation time by running CreateDataDir.exe
	//		However, this is quite useful code, so its been left here for reference.
	
	//Set file permissions so that any user can update the files in the folder.
	//NB only works for NTFS drives and only the creator or an Administrator will be able to do it.
	//command = (char *)calloc(strlen(path) + 50, sizeof(char));
	//sprintf(command, "cacls \"%s\" /t /c /e /g \"BUILTIN\\Users\":F", path);
	//GCI_MessagePopup("", command);
	//system (command); 
	//free(command);
	
	strcat(path, "\\");
	
	return 0;
}


COLORREF CviColourToColorRef(int colour)
{
	unsigned char red, green, blue;
		
	red = colour >> 16;
	green = colour >> 8;
	blue = colour;
	
	return RGB(red, green, blue);
}


int ColorRefToCviColour(COLORREF colour)
{
	return MakeColor(GetRValue(colour), GetGValue(colour), GetBValue(colour));
}

int RGBToHSV( unsigned char red, unsigned char green, unsigned char blue,
							double *hue, double *satuation, double *value)
{
	unsigned char max = 0, min = 0; //, rgb[3];

	*hue = 0;
	*satuation = 0;
	*value = 0;

	max = red;
	min = red;
	
	if(green > max)
		max = green;
	
	if(blue > max)
		max = blue;
	
	if(green < min)
		min = green;
	
	if(blue < min)
		min = blue;
	
	if(max != min) {
	
		if(red == max)
			*hue = ((double) (green - blue) / (max - min)) * 60.0;
		
		if(green == max)
			*hue = (2 + (double) (blue - red) / (max - min)) * 60.0;
		
		if(blue == max)
			*hue = (4 + (double) (red - green) / (max - min)) * 60.0;
	}
		
	*value = (double) max / 255.0;
	
	if(max != 0)
		*satuation = ((double) (max - min) / max);

	return 0;
}


// Red, Green and Blue are between 0 and 255
// Hue varies between 0 and 360
// Satuation between 0 and 1
// Value between 0 and 1
int HSVToRGB( double hue, double saturation, double value,
							unsigned char *red, unsigned char *green, unsigned char *blue)
{
	unsigned int h,p,q,t,v;
	double temp_hue, f;
	
	//if(saturation == 0.0)
	//	return 0;	// why this test? seems to work as expected
	
	// The if statement is here as the % operator requires integers
	// and thus looses some accuracy.
	if(hue < 0 || hue > 360) {
	
		temp_hue = floor(hue + 0.5);
		temp_hue = (int) temp_hue % 360;
	}
	else
		temp_hue = hue;
	
	*red = (unsigned char) floor(value + 0.5);
	*green = *red;
	*blue = *red;
	
	h = (unsigned int) (temp_hue / 60);  
	f = (double) (temp_hue / 60) - h;
	p = (unsigned int) floor( (value * (1 - saturation) * 255) + 0.5);
	q = (unsigned int) floor( (value * (1 - f * saturation) * 255) + 0.5);
	t = (unsigned int) floor( (value * (1 - (1 - f) * saturation) * 255) + 0.5);
	
	v = (unsigned int) (value * 255.0);	
	
	switch (h) {

		case 6:
		case 0:
		
			*red = v;
			*green = t;
			*blue = p;
			
			break;
		
		case 1:
		
			*red = q;
			*green = v;
			*blue = p;
			
			break;
		
		case 2:
		
			*red = p;
			*green = v;
			*blue = t;
			
			break;
		
		case 3: 
		
			*red = p;
			*green = q;
			*blue = v;
			
			break;
		
		case 4:
		
			*red = t;
			*green = p;
			*blue = v;
			
			break;
			
		case 5:
		
			*red = v;
			*green = p;
			*blue = q;
	}
	
	return 0;
} 


COLORREF GetComplementaryColour(COLORREF colour)
{
	double hue, satuation, value;
	unsigned char red, green, blue;
	
	RGBToHSV(GetRValue(colour), GetGValue(colour), GetBValue(colour),
		&hue, &satuation, &value);
	
	// Add 180 degrees to hue and find where on the colour wheel that is
	hue = ((int) (hue + 180.0)) % 360;
	
	HSVToRGB(hue, satuation, value, &red, &green, &blue);
	
	return RGB(red, green, blue);    
}


int GetComplementaryCviColour(int colour)
{
	COLORREF ref = GetComplementaryColour(CviColourToColorRef(colour));
	
	return ColorRefToCviColour(ref);  
}


void SetCtrlTextColourToComplementary(int panel, int ctrl)
{
	int colour, comp_colour;
	
	// Get Ctrl background colour.
	GetCtrlAttribute(panel, ctrl, ATTR_TEXT_BGCOLOR, &colour); 
		
	comp_colour = GetComplementaryCviColour(colour);       
	
	SetCtrlAttribute(panel, ctrl, ATTR_TEXT_COLOR, comp_colour);    
}
   

void SetCellTextColourToComplementary(int panel, int ctrl, Point cell)
{
	int colour, comp_colour;
	
	// Get Ctrl background colour.
	GetTableCellAttribute(panel, ctrl, cell, ATTR_TEXT_BGCOLOR, &colour); 
		
	comp_colour = GetComplementaryCviColour(colour);       
	
	SetTableCellAttribute(panel, ctrl, cell, ATTR_TEXT_COLOR, comp_colour);    	
}

void get_time_string(char *time_str)
{
	time_t rawtime;
	time ( &rawtime );

	time_str[0] = 0;

	strncpy(time_str, ctime (&rawtime), 199);
	time_str[strlen(time_str) - 1] = 0;	// Get rid of \n
}

void seconds_to_friendly_time(double secs, char *time)
{
	int days, hours, minutes, seconds, len;
	char hours_str[10] = "", min_str[10] = "", sec_str[10] = "", days_str[10] = "";
	// Clear the string
	time[0] = 0;
	
	days = (int) (secs / 86400);
	secs -= (int) (days * 86400);
	hours = (int) (secs / 3600);
	secs -= (int) (hours * 3600);
	minutes = (int) (secs / 60);
	seconds = (int) (secs - minutes * 60);	
	
	sprintf(days_str, "%dd ", days);
	sprintf(hours_str, "%dh ", hours);
	sprintf(min_str, "%dm ", minutes);
	sprintf(sec_str, "%ds ", seconds);

	if(days != 0)
		strncat(time, days_str, 5);

	if(hours != 0)
		strncat(time, hours_str, 5);

	if(minutes != 0)
		strncat(time, min_str, 5);

	if(seconds != 0)
		strncat(time, sec_str, 5);

	if(strcmp(time,"") == 0)
		strcpy(time, "0s");

	// Remove end space
	len = strlen(time);

	if(len > 1)
		time[len - 1] = 0;
}

int get_temp_directory(char *path)
{
	memset(path, 0, 1);
	
	// Get temp dir
	if(!GetEnvironmentVariable("Temp", path, 500)) {
		
		GCI_ErrorPopup("Error", "No temporary directory exists!");
	}
	
	return 0;
}

void ShowStandaloneDeviceController(int *index, int *stress, ...)
{  
	int panel, device_list, stress_button_id, button_id;
	char *string_val = NULL;

	va_list ap;
	va_start(ap, stress);     

	panel = NewPanel(0, "Device Selection", 100, 100, 80, 290);

	device_list = NewCtrl(panel, CTRL_RING_LS, "Devices", 25, 20);

	SetPanelAttribute(panel, ATTR_SIZABLE, 0);
	SetCtrlAttribute(panel, device_list, ATTR_WIDTH, 200);

	do {
		string_val = va_arg(ap, char*);
		InsertListItem(panel, device_list, -1, string_val);

	} while(string_val != NULL);

	stress_button_id = NewCtrl(panel, CTRL_CHECK_BOX, "Stress Test", 50, 20);
	if (*stress==0) SetCtrlAttribute(panel, stress_button_id, ATTR_DIMMED, 1);

	button_id = NewCtrl(panel, CTRL_SQUARE_COMMAND_BUTTON_LS, "OK", 25, 240);

	va_end(ap);  

	DisplayPanel(panel);

	while (1) {
		
		int ctrl, pnl;

		ProcessSystemEvents();
			
		GetUserEvent (0, &pnl, &ctrl);
			
		if (pnl != panel)
			continue;

		if (ctrl == button_id) {
				
			GetCtrlIndex(panel, device_list, index);
			GetCtrlVal(panel, stress_button_id, stress);

			DiscardPanel(panel);

			break;
		}
	}

	DiscardPanel(panel);
}

int ShowStandaloneIniDevicesController(const char * inipath, char *type_filter, int *stress, DeviceInfo *info)
{  
	int i, panel, number_of_devices, device_list, stress_button_id, button_id, close_button_id;
	char *section_value = NULL, *device_type;
	char buffer[500] = "";
	dictionary *d = NULL;

	memset(info, 0, sizeof(DeviceInfo));

	d = iniparser_load(inipath);

	if(d == NULL)
		return -1;

	number_of_devices = iniparser_getnsec(d);

	if(number_of_devices < 1)
		return -1;

	panel = NewPanel(0, "Device Selection", 100, 100, 80, 390);

	device_list = NewCtrl(panel, CTRL_RING_LS, "Devices", 25, 20);
	
	SetPanelAttribute(panel, ATTR_SIZABLE, 0);
	SetCtrlAttribute(panel, device_list, ATTR_WIDTH, 280);
	SetCtrlAttribute(panel, device_list, ATTR_DATA_TYPE, VAL_INTEGER);

	for(i=0; i < number_of_devices; i++) {

		section_value = iniparser_getsecname(d, i);
		device_type = iniparser_getstr_from_section(d, section_value, "DeviceType");

		if(section_value == NULL || device_type == NULL)
			continue;

		// If device type string is not found in the passed in filter 
		// do not display the device.
		if(strstr(type_filter, device_type) == NULL)
			continue;

		sprintf(buffer, "%s (%s)", device_type, section_value);
		InsertListItem(panel, device_list, -1, buffer, i);
	}

	stress_button_id = NewCtrl(panel, CTRL_CHECK_BOX, "Stress Test", 50, 20);
	
	if (*stress==0)
		SetCtrlAttribute(panel, stress_button_id, ATTR_DIMMED, 1);

	button_id = NewCtrl(panel, CTRL_SQUARE_COMMAND_BUTTON_LS, "OK", 25, 330);

	SetCtrlAttribute(panel, button_id, ATTR_CMD_BUTTON_COLOR, MakeColor(30, 87, 174));
	SetCtrlAttribute(panel, button_id, ATTR_LABEL_COLOR, VAL_WHITE);  

	close_button_id = NewCtrl(panel, CTRL_SQUARE_COMMAND_BUTTON_LS, "OK", 0, 0);
	SetCtrlAttribute(panel, close_button_id, ATTR_VISIBLE, 0);

	SetPanelAttribute(panel, ATTR_CLOSE_CTRL, close_button_id);

	DisplayPanel(panel);

	while (1) {
		
		int ctrl, pnl;

		ProcessSystemEvents();
			
		GetUserEvent (0, &pnl, &ctrl);
			
		if (pnl != panel)
			continue;

		if (ctrl == button_id) {
				
			GetCtrlVal(panel, device_list, &i);
			GetCtrlVal(panel, stress_button_id, stress);

			section_value = iniparser_getsecname(d, i);
			device_type = iniparser_getstr_from_section(d, section_value, "DeviceType");

			if(section_value == NULL || device_type == NULL)
				return -1;

			strncpy(info->name, section_value, 500);
			strncpy(info->type, device_type, 500);	

			DiscardPanel(panel);

			break;
		}

		if(ctrl == close_button_id) {
			
			DiscardPanel(panel);
			return -1;
		}
	}

	iniparser_freedict(d);
	DiscardPanel(panel);

	return 0;
}

static int CVICALLBACK ThreadedOpenComPortWithTimeOut(void *callback)
{
	ComPortThreadInfo *info = (ComPortThreadInfo *) callback;

    return OpenComConfig (info->portNo, info->deviceName, info->baudRate, info->parity,
		info->dataBits, info->stopBits, info->iqSize, info->oqSize);
}


static int OpenComPortWithTimeOut(int portNo, const char deviceName[], int baudRate, int parity,
                  int dataBits, int stopBits, int iqSize,
                  int oqSize, double timeout)
{
	int ret, thread_id, timed_out = 1, thread_status;
	double start_time, elapsed_time;

	if(comport_thread_info_lock == -1) {
		GciCmtNewLock("GciUtils-OpenComPortWithTimeOut", 0, &comport_thread_info_lock);
	}

	GciCmtGetLock(comport_thread_info_lock);

	comport_thread_info.portNo = portNo;	
	strncpy(comport_thread_info.deviceName, deviceName, 199);
	comport_thread_info.baudRate = baudRate;
	comport_thread_info.parity = parity;
	comport_thread_info.dataBits = dataBits;
	comport_thread_info.stopBits = stopBits;
	comport_thread_info.iqSize = iqSize;
	comport_thread_info.oqSize = oqSize;
	comport_thread_info.timeout = timeout;

	start_time = Timer();

	CmtScheduleThreadPoolFunction (gci_thread_pool(), ThreadedOpenComPortWithTimeOut, &comport_thread_info, &thread_id);

	elapsed_time = Timer() - start_time;

	while(elapsed_time < timeout) {
	
		if(CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_EXECUTION_STATUS, &thread_status) == 0) {

			if(thread_status > 2) { // Finish executing thread
				timed_out = 0;		
				break;
			}
		}

		Delay(0.005);
		elapsed_time = Timer() - start_time;
		ProcessSystemEvents();	
	}
	
	if(timed_out) {

		CmtTerminateThreadPoolThread (gci_thread_pool(), thread_id, -1);

		GciCmtReleaseLock(comport_thread_info_lock);

		return -200;
	}

	// Get the return value GetOutQLen
	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_RETURN_VALUE, &ret);
	
	CmtReleaseThreadPoolFunctionID (gci_thread_pool(), thread_id);
  
	GciCmtReleaseLock(comport_thread_info_lock);

    return ret;
}

static int CVICALLBACK ThreadedCloseComPortWithTimeOut(void *callback)
{
	ComPortThreadInfo *info = (ComPortThreadInfo *) callback;

	return close_comport(info->portNo);
}

int CloseComPortWithTimeOut(int portNo, double timeout)
{
	int ret, thread_id, timed_out = 1, thread_status;
	double start_time, elapsed_time;

	if(comport_thread_info_lock == -1) {
		GciCmtNewLock("GciUtils-CloseComPortWithTimeOut", 0, &comport_thread_info_lock);
	}

	GciCmtGetLock(comport_thread_info_lock);

	comport_thread_info.portNo = portNo;	
	
	start_time = Timer();

	CmtScheduleThreadPoolFunction (gci_thread_pool(), ThreadedCloseComPortWithTimeOut, &comport_thread_info, &thread_id);

	elapsed_time = Timer() - start_time;

	while(elapsed_time < timeout) {
	
		if(CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_EXECUTION_STATUS, &thread_status) == 0) {

			if(thread_status > 2) { // Finish executing thread
				timed_out = 0;		
				break;
			}
		}

		Delay(0.005);
		elapsed_time = Timer() - start_time;
		ProcessSystemEvents();	
	}
	
	if(timed_out) {

		CmtTerminateThreadPoolThread (gci_thread_pool(), thread_id, -1);

		GciCmtReleaseLock(comport_thread_info_lock);

		return -200;
	}

	// Get the return value GetOutQLen
	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_RETURN_VALUE, &ret);
	
	CmtReleaseThreadPoolFunctionID (gci_thread_pool(), thread_id);
  
	GciCmtReleaseLock(comport_thread_info_lock);

    return ret;
}

static int OpenComPort(int port, int baudrate)
{
	int err = 0;
	char port_string[10];
	
   	sprintf(port_string, "COM%d", port);

    if ((err = OpenComConfig (port, port_string, baudrate, 0, 8, 1, 164, -1 /*164 */)) < 0) {
        return -1;
    }
    
	SetComTime (port, 3.0);  // timeout=3sec
        
	FlushInQ(port);
        
    return 0;
}

int initialise_comport(int port, int baudrate)
{
	int ans, err;   
	
	if (port < 1 || port > 20) {
    	GCI_ErrorPopup("USB Error", "Invalid com port specified");
    	return -1;
    }

	if (opened_ports_ref_count[port] > 0) {
		opened_ports_ref_count[port]++;
		return 0;	 // Already initialised
	}

	CloseCom(port);

	while ((err = OpenComPort(port, baudrate)) < 0) {
		
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);

		ans = GCI_ConfirmPopup ("Failed to open comport", IDI_ERROR,
			"Failed to open comport %d\nRS232 message: %s\nDo you want to try again?", port, GetRS232ErrorString(err));   

		if (ans == 0) { 
			// Com port may have opened and it is the device that is faling to respond.
			// Just in case we close the comport here.
			CloseCom(port);
			return -1;
		}
	}
	
	FlushInQ (port);
	FlushOutQ (port); 
	
	opened_ports_ref_count[port]++;

	return 0;
}

int close_comport(int port)
{
	printf("Going to close Com Port %d\n", port);	

	if (port < 1 || port > 10) {
    	return -1;
    }

	opened_ports_ref_count[port]--;

	if (opened_ports_ref_count[port] <= 0) {		// Ref count of zero so we close the port
		printf("Closing Com Port %d\n", port);	
		opened_ports_ref_count[port]  = 0;          // just make sure this does not go below 0
		return CloseCom(port);	
	}

	return 0;
}

void get_date_as_valid_filepath_str(char *filename)
{
	time_t tim=time(NULL);
    struct tm *now=localtime(&tim);

    sprintf(filename, "%d_%02d_%02d_%02d_%02d", now->tm_year+1900, now->tm_mon+1, now->tm_mday,
		now->tm_hour, now->tm_min);
}


Rect SetRectRelativeToPoint(Rect *rect, Point pt)
{
	return SetRectRelativeToXY(rect, pt.x, pt.y);
}

Rect SetRectRelativeToXY(Rect *rect, int x, int y)
{
	Rect r;

	r.width = rect->width;
	r.height = rect->height;
	r.left = rect->left - x;
	r.top = rect->top - y;

	return r;
}

int GCI_Find_LoadPanel(int parent, const char* filename, int id)
{
	#ifdef _CVI_
	
		return LoadPanel (parent, filename, id);

	#else
    char full_path[500] = "";
	
    if(find_resource(filename, full_path) < 0) {
		GCI_ErrorPopup("Error", "UIR load error - Can not find file %s", filename);
		return -1;  
	}
	else {
	
	  return LoadPanel  (parent, full_path, id);
    }
	
	#endif
}

const char* GCI_GetUIRPath(const char* filename, char *full_path)
{  
	full_path[0] = '\0';
	
	#ifdef _CVI_
	
		return filename;

	#else
	
    if(find_resource(filename, full_path) < 0) {
		GCI_ErrorPopup("Error", "UIR load error - Can not find file %s", filename);
		return NULL;  
	}
	else {
	
	  return full_path;
	}
	
	#endif
}

void gettail_into_array(FILE *fp, unsigned int linenum, unsigned int line_length, char *buffer)
{
	int             cc;
	unsigned int    indx;
    unsigned char outstr[15];
    unsigned long int currline = 0L;
	char *line_buffer = NULL;
	int bytes_read;
	long int * tail = NULL;

      tail = (long int *)malloc(sizeof(*tail) * linenum);
	
      if (!tail)
      {
            fputs("Insufficient memory.", stderr);
            exit(1);
      }
      tail[0] = ftell(fp);
      indx = 0;

      for (cc = getc(fp); cc != EOF; cc = getc(fp))
      {
            if (cc == '\r')
            {
                  ++currline;
                  cc = getc(fp);
                  if (cc != '\n')
                        ungetc(cc, fp);
                  ++indx;
                  indx %= linenum;
                  tail[indx] = ftell(fp);
            }
            else
            {
                  if (cc == '\n')
                  {
                        ++currline;
                        cc = getc(fp);
                        if (cc != '\r')
                              ungetc(cc, fp);
                        ++indx;
                        indx %= linenum;
                        tail[indx] = ftell(fp);
                  }
            }
      }
      fputs("\" ", stderr);
      ltoa(currline, (char *)outstr, 10);
      fputs((char *)outstr, stderr);
      fputs(" lines", stderr);
      if (currline >= linenum - 1)
      {
            indx++;
            indx %= linenum;
      }
      else  indx = 0;

      if (fseek(fp, tail[indx], 0) == -1)
      {
            fputs("\nFile seek error.", stderr);
            exit(1);
      }
      free(tail);

	  line_buffer = (char *) malloc(sizeof(char) * line_length);

	  while ((bytes_read = fread (line_buffer, 1, line_length, fp)) > 0) {
			strcpy(buffer, line_buffer);
			buffer += bytes_read;
	  }

	  *buffer = '\0';


	  free(line_buffer);

	 // fread(
	//	while (fgets(json_buffer, 999, fp)!=NULL)
      //  mg_printf(conn, json_buffer);
}

void display_panel_without_activation(int panel_id)
{
	int window_handle;
	
	GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	
	ShowWindow( (HWND) window_handle, SW_SHOWNA);
}

//********************************* GCI_formatFloatToString **********************************************************************

int GCI_formatFloatToString (float n, char *s)
{
	if      (n==0.0)  sprintf(s, "0");
	else if (fabs(n)<0.010) sprintf(s, "%.3e",   n);
	else if (fabs(n)<1.000) sprintf(s, "%.4f", n);
	else if (fabs(n)<10.00) sprintf(s, "%.3f", n);
	else if (fabs(n)<100.0) sprintf(s, "%.2f", n);
	else if (fabs(n)<1000.0)sprintf(s, "%.1f", n);
	else			        sprintf(s, "%.3e", n);
	
	return (0);
}