#include <ansi_c.h>
#include <userint.h>
#include "gci_utils.h"
#include "status.h"

Feedback* g_fb = NULL;

static void resize_controls(void)
{
	ui_module_anchor_control_to_panel_edge(g_fb->panel_id, g_fb->textbox_id, 10, 10, 10, 50);

	// Move Hide Button 
	ui_module_move_control_pixels_from_right(g_fb->panel_id, g_fb->hide_button_id, 10);     
	ui_module_move_control_pixels_from_bottom(g_fb->panel_id, g_fb->hide_button_id, 10); 
}

static LRESULT CALLBACK StatusWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	//LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	switch(message) {
			
    	case WM_EXITSIZEMOVE:
    	{
			resize_controls();
			
			break;
    	}

      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) g_fb->old_wndproc,
							hwnd, message, wParam, lParam);
}

static int CVICALLBACK OnHideFeedback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			ui_module_hide_panel(UIMODULE_CAST(g_fb), g_fb->panel_id);  
			
			break;
	}
	
	return 0;
}

void feedback_new(void)
{
	if(g_fb != NULL)
		return;
	
	g_fb = (Feedback*) malloc (sizeof(Feedback));
	     
	g_fb->panel_id =  NewPanel (0, "Feedback", 100, 100, 200, 400);

	g_fb->textbox_id = NewCtrl(g_fb->panel_id, CTRL_TEXT_BOX, "", 10, 10);
	g_fb->hide_button_id = NewCtrl(g_fb->panel_id, CTRL_SQUARE_COMMAND_BUTTON, "Hide", 80, 150);    
	
	//SetPanelAttribute(g_fb->panel_id, ATTR_CONFORM_TO_SYSTEM, 1);
	SetPanelAttribute(g_fb->panel_id, ATTR_BACKCOLOR, MICROSCOPE_GRAY);

	// Set text box to indicator
	SetCtrlAttribute(g_fb->panel_id, g_fb->textbox_id, ATTR_NO_EDIT_TEXT, 1);	
	SetCtrlAttribute (g_fb->panel_id, g_fb->textbox_id, ATTR_SCROLL_BARS, VAL_BOTH_SCROLL_BARS);
	
	SetCtrlAttribute(g_fb->panel_id, g_fb->hide_button_id, ATTR_CMD_BUTTON_COLOR, MICROSCOPE_BLUE);
	SetCtrlAttribute(g_fb->panel_id, g_fb->hide_button_id, ATTR_LABEL_COLOR, VAL_WHITE);   
	
	InstallCtrlCallback(g_fb->panel_id, g_fb->hide_button_id, OnHideFeedback, NULL);
	
	ui_module_constructor(UIMODULE_CAST(g_fb), "Feedback"); 

	resize_controls();

	g_fb->old_wndproc = ui_module_set_window_proc(UIMODULE_CAST(g_fb), g_fb->panel_id, (LONG_PTR) StatusWndProc);   
}

void feedback_destroy(void)
{
	if(g_fb == NULL)
		return;
	
	// Restore WndProc pointer
	ui_module_restore_cvi_wnd_proc(g_fb->panel_id); 

	ui_module_destroy(UIMODULE_CAST(g_fb)); 
	
	free(g_fb);
	
	g_fb = NULL;
}

void feedback_show(char *title, char *message)    
{
	if(g_fb == NULL)
		return;
	  
	SetCtrlVal(g_fb->panel_id, g_fb->textbox_id, message);
	
	//Bring to the front
	SetPanelAttribute (g_fb->panel_id, ATTR_FLOATING, VAL_FLOAT_APP_ACTIVE);
	SetPanelAttribute (g_fb->panel_id, ATTR_FLOATING, VAL_FLOAT_NEVER);
	
	ui_module_display_panel(UIMODULE_CAST(g_fb), g_fb->panel_id);   
}


