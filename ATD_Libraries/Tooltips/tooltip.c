#include "gci_utils.h"

#include <windows.h>
#include <ansi_c.h>
#include <userint.h>

#include <commctrl.h>

static HINSTANCE gToolTipHInstance;
//static HWND gHandle;
//static HWND gToolTip;

// EnumChildProc - registers control windows with a tooltip control by
//     using the TTM_ADDTOOL message to pass the address of a 
//     TOOLINFO structure. 
// Returns TRUE if successful, or FALSE otherwise.
//
// hwndCtrl - handle of a control window 
// lParam - application-defined value (not used) 


BOOL EnumChildProc(HWND hwndCtrl, LPARAM lParam) 
{ 
    TOOLINFO ti; 
    char szClass[64]; 
    HWND toolTipHwnd = (HWND) lParam;
	HWND ownerHwnd = GetWindow(toolTipHwnd, GW_OWNER);
	
    // Skip static controls. 
    GetClassName(hwndCtrl, szClass, sizeof(szClass)); 
	
    if (lstrcmpi(szClass, "STATIC"))
	{ 
        ti.cbSize = sizeof(TOOLINFO); 
        ti.uFlags = TTF_IDISHWND; 
        ti.hwnd = ownerHwnd; 
        ti.uId = (UINT) hwndCtrl; 
        ti.hinst = 0; 
        ti.lpszText = LPSTR_TEXTCALLBACK; 
		
        SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, 
            (LPARAM) (LPTOOLINFO) &ti); 
    } 
	
    return TRUE;
} 




HWND ToolTip_Create (int panel)
{
	HWND handle;
	HWND toolTipHwnd;
	int panel_handle;
	int dword = WS_POPUP | TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP;
	
	InitCommonControls();
	
	GetPanelAttribute (panel, ATTR_SYSTEM_WINDOW_HANDLE, &panel_handle); 
	handle = (HWND) panel_handle;    
	
	toolTipHwnd = CreateWindowEx(0, 
                              "tooltips_class32",
                              NULL, 
                              dword,
                              CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                              handle, 
                              NULL, gToolTipHInstance, NULL
							 );
	
	 if(toolTipHwnd == NULL)
		 GCI_MessagePopup("Error", "Can't create tooltip window");
	 
	 SetWindowPos(toolTipHwnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	 
	 
	 // Enumerate the child windows to register them with the tooltip
    // control. 
	EnumChildWindows(handle, (WNDENUMPROC) EnumChildProc, (LPARAM) toolTipHwnd);
	
    return toolTipHwnd;
}


void ToolTip_AddTool(HWND tooltip, RECT rect, char *text)
{
	TOOLINFO ti;     
	//HWND handle;
	int id = 0;     // offset to string identifiers 
	
	HWND ownerHwnd = GetWindow(tooltip, GW_OWNER);   
	
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = ownerHwnd;
	ti.hinst = gToolTipHInstance;
	ti.uId = (UINT) id; 
	ti.lpszText = text;
	ti.rect.left = rect.left; 
    ti.rect.top = rect.top; 
    ti.rect.right = rect.right; 
    ti.rect.bottom = rect.bottom; 

	//toolinfo.rect
	if(!SendMessage(tooltip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti))
		GCI_MessagePopup("Error", "Error");
}


void ToolTip_AddToolTipForCtrl(HWND tooltip, int panel, int ctrl, char *text) 
{
	RECT rect;
	int width, height;
	
	GetCtrlAttribute(panel, ctrl, ATTR_LEFT, &(rect.left));
	GetCtrlAttribute(panel, ctrl, ATTR_TOP, &(rect.top));
	GetCtrlAttribute(panel, ctrl, ATTR_WIDTH, &width); 
	GetCtrlAttribute(panel, ctrl, ATTR_HEIGHT, &height); 
	
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
	
	ToolTip_AddTool(tooltip, rect, text);  
}
