#include "icsviewer_private.h"
#include "icsviewer_uir.h"
#include "icsviewer_plugin.h"
#include "icsviewer_tools.h"
#include "ImageViewer.h"
#include "string_utils.h"
#include "ImageViewer_Drawing.h" 
#include "FreeImageAlgorithms_Utilities.h" 

#include "windows.h"

extern int windowsRef;

static 
LRESULT IcsViewerWindow_OnSizingEvent(IcsViewerWindow *window, WPARAM wParam, LPARAM lParam)
{
	int width, height;
    RECT *rect = (RECT*)lParam;
    
    /* Don't resize below minimum allowed size */
    width = rect->right - rect->left;
    height = rect->bottom - rect->top;
    	
    if(width < PANEL_MIN_WIDTH) {
    
    	switch(wParam) {
    		
    		case WMSZ_RIGHT:
    		case WMSZ_BOTTOMRIGHT:
    		case WMSZ_TOPRIGHT:
    			rect->right = rect->left + PANEL_MIN_WIDTH;
    			break;
    			
    		case WMSZ_LEFT:
    		case WMSZ_BOTTOMLEFT:
    		case WMSZ_TOPLEFT:
    			rect->left = rect->right - PANEL_MIN_WIDTH;
    			break;
    	}
    }
    		
    if(height < PANEL_MIN_HEIGHT) {
    
    	switch(wParam) {
    		
    		case WMSZ_TOP:
    		case WMSZ_TOPLEFT:
    		case WMSZ_TOPRIGHT:
    
    			rect->top = rect->bottom - PANEL_MIN_HEIGHT;
    			break;
    			
    		case WMSZ_BOTTOM:
    		case WMSZ_BOTTOMRIGHT:
    		case WMSZ_BOTTOMLEFT:
    	
    			rect->bottom = rect->top + PANEL_MIN_HEIGHT;
    			break;
    	}
    }

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "WindowResizing",
		GCI_VOID_POINTER, window,
		GCI_INT, rect->left,
		GCI_INT, rect->top,
		GCI_INT, rect->right,
		GCI_INT, rect->bottom); 
    		
	return 1;
}


static LRESULT GCI_Window_OnSize(IcsViewerWindow *window, int width, int height)
{
	static int old_width = -1, old_height = 1;
	int checked, left, right, panel_width;
	const int STATUSBAR_HEIGHT = 15;
	
	//if(width == old_width && height == old_height)
	//	return 0;

	GetMenuBarAttribute (window->panel_menu->menubar_id, 
		window->panel_menu->show_toolbar_menu_item_id, ATTR_CHECKED, &checked);  
	
	left = checked ? VIEWER_LEFT_WITH_TOOLBAR : VIEWER_LEFT_WITHOUT_TOOLBAR;
	
	GetPanelAttribute(window->panel_id, ATTR_WIDTH, &panel_width);
	
	right = window->panel_palettebar_status ?  (width - VIEWER_DIST_FROM_RIGHT_WITH_PALETTEBAR) : (width - VIEWER_DIST_FROM_RIGHT_WITHOUT_PALETTEBAR);

	SetWindowPos(window->canvas_window, NULL, left, VIEWER_TOP,
		right - left,
		height - VIEWER_TOP - IMAGE_VIEW_BORDER - STATUSBAR_HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE);
	
	SetWindowPos(window->hWndStatus, NULL, left, height - IMAGE_VIEW_BORDER,
		right - left,
		STATUSBAR_HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE);
	
	old_width = width;
	old_height = height;

	return 0;
}	


LRESULT CALLBACK GCI_WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	IcsViewerWindow *window = (IcsViewerWindow*) data;
	
	char last_filename[GCI_MAX_PATHNAME_LEN];
	int realStringSize;
	
	if(message == RESTORE_MSG) {
	
        ics_viewer_registry_read_string(window, "LastArgvArg", last_filename, &realStringSize);
  
		GCI_ImagingWindow_LoadImageFile(window, last_filename);   
		
		SetForegroundWindow(hwnd);
		
		if (IsIconic(hwnd))
        	ShowWindow(hwnd, SW_RESTORE);
		
		return 0;
	}
	
	switch(message) {

		case ICSVIEWER_IMAGE_LOAD:
		{
			// Don't attempt to load images from other threads when panel in being moved.
			if(window->is_moving)
				return 1;

			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "ImagePreLoaded", GCI_VOID_POINTER, window, GCI_VOID_POINTER, (FIBITMAP*) lParam);

			GCI_ImagingWindow_LoadImage(window, (FIBITMAP*) lParam);
			
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "ImagePostLoaded", GCI_VOID_POINTER, window);

			return 1;
		}

		case ICSVIEWER_UPDATE_TITLEBAR:
		{
			SetPanelAttribute(window->panel_id, ATTR_TITLE, window->panel_tmp_title);

			return 1;
		}
		
    	case WM_SIZING:
    	{
    		return IcsViewerWindow_OnSizingEvent(window, wParam, lParam);
    	}

		case WM_MOVING:
    	{
			RECT *rect = (RECT*)lParam;

			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "WindowResizing",
				GCI_VOID_POINTER, window,
				GCI_INT, rect->left,
				GCI_INT, rect->top,
				GCI_INT, rect->right,
				GCI_INT, rect->bottom); 

    		break;
    	}

    	case WM_SIZE:
    	{
    		EventData data1 = NewEventData();
			EventData data2 = NewEventData();
    		
			data1.width = LOWORD(lParam);
    		data2.height = HIWORD(lParam);

			if(data1.width == 0 || data2.height == 0)
  				return 0;

			GCI_Window_OnSize(window, data1.width, data2.height);
			
			SEND_EVENT(window, on_resize, data1, data2, "on_resize")  

			break;
    	}
  	
		case WM_SHOWWINDOW:
		{
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "WindowShowOrHide", GCI_VOID_POINTER, window, GCI_INT, wParam); 

			return 0;
		}

		case WM_ENTERSIZEMOVE:
		{
			window->is_moving = 1;
		
			return 0;
		}
		
    	case WM_EXITSIZEMOVE:
    	{
    		int width, height;
    		
    		EventData data1 = NewEventData();
			EventData data2 = NewEventData();
    	
			window->is_moving = 0;      
			
    		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "ResizedorMoved", GCI_VOID_POINTER, window); 
    		
    		GetPanelAttribute(window->panel_id, ATTR_WIDTH, &width);
    		GetPanelAttribute(window->panel_id, ATTR_HEIGHT, &height);
    		
    		data1.width = width;
    		data2.height = height;
    		
    		SEND_EVENT(window, on_exit_resize, data1, data2, "on_exit_resize") 
    		
    		return 0;
    	}
   	
    	case WM_SYSCOMMAND:
    	{
    		switch (wParam)
    		{
    			/* Dont pass this to lab windows WndProc as it is ignored */
    			case SC_CLOSE:
   
    				GCI_ImagingWindow_Close(window);    
						 
					return 0;

				case SC_MINIMIZE:
				{
					GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "WindowMinimised", GCI_VOID_POINTER, window); 
    		
					break;
				}

				case SC_MAXIMIZE:
				{
					GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "WindowMaximised", GCI_VOID_POINTER, window); 
    		
					break;
				}

				case SC_RESTORE:
				{
					GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "WindowRestored", GCI_VOID_POINTER, window); 
    		
					break;
				}
    		}

 			break;
    	}
    	
    	case WM_CLOSE:
    	{
    		return 0;
    	
    	}
    	
		case WM_DESTROY:
    	{
        	Window_Destroy(window);  
        
        	return 0;
      	}

      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) window->panel_original_proc_fun_ptr,
							window->panel_window_handle, message, wParam, lParam);
}
   
 

LRESULT CALLBACK GCI_WindowCanvasProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	IcsViewerWindow *window = (IcsViewerWindow*) data;
	
	switch(message) {
		
		case WM_ERASEBKGND:
		{
			return 1;
		}
		
		
		case IMAGEVIEWER_BUFFER_PAINTED:
		{
		    SEND_EVENT(window, on_buffer_paint, NewEventData(), NewEventData(), "on_buffer_paint")
		    
			break;
		}
			
		case WM_PAINT:
		{
		    SEND_EVENT(window, on_paint, NewEventData(), NewEventData(), "on_paint")
	
		  	break;
		};
		
		
		case WM_LBUTTONDOWN:
		{
			EventData data1, data2;
			POINT image_point;
	
			data1 = NewEventData();
			data1.button = LEFT_MOUSE_BUTTON;
			
			data2 = NewEventData();
			data2.point.x = LOWORD(lParam);
			data2.point.y = HIWORD(lParam);
			
			SEND_EVENT(window, on_mouse_down, data1, data2, "on_mouse_down")  
			
			if(GCI_Signal_IsConnected(UIMODULE_SIGNAL_TABLE(window), "MouseDown")) {

				ImageViewer_TranslateWindowPointToImagePoint(window->canvas_window, data2.point, &image_point);
			
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "MouseDown", GCI_VOID_POINTER,
					window, GCI_POINT, image_point, GCI_POINT, data2.point);
			}

			break;
		}
	
		
		case WM_LBUTTONUP:
		{
			EventData data1, data2;
			POINT image_point;

			data1 = NewEventData();
			data1.button = LEFT_MOUSE_BUTTON;
			
			data2 = NewEventData();
			data2.point.x = LOWORD(lParam);
			data2.point.y = HIWORD(lParam);
		
			SEND_EVENT(window, on_mouse_up, data1, data2, "on_mouse_update")  
			
			if(GCI_Signal_IsConnected(UIMODULE_SIGNAL_TABLE(window), "MouseUp")) {

				ImageViewer_TranslateWindowPointToImagePoint(window->canvas_window, data2.point, &image_point);
			
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "MouseUp", GCI_VOID_POINTER,
					window, GCI_POINT, image_point, GCI_POINT, data2.point);
			}

			break;
		}
		
		case WM_MOUSEMOVE:
		{
			char buffer0[200], buffer1[200];
			EventData data1, data2;
			POINT image_point;
			static int old_x, old_y;
			int width, height;
			
			int xPos = LOWORD(lParam); 
			int yPos = HIWORD(lParam);

			if(window->panel_dib == NULL)
				return TRUE;

			if(old_x == xPos && old_y == yPos)     
				break;
			
			old_x = xPos;
			old_y = yPos;
			
			data1 = NewEventData();
			data1.button = LEFT_MOUSE_BUTTON;
			
			data2 = NewEventData();
	
			data2.point = ImageViewer_GetClosetPosOnZoomedImageFromCursorPos(window->canvas_window);  

			SEND_EVENT(window, on_mouse_move, data1, data2, "on_mouse_move")  
			
			ImageViewer_TranslateWindowPointToImagePoint(hwnd, data2.point, &image_point);        

			width = FreeImage_GetWidth(window->panel_dib);
			height = FreeImage_GetHeight(window->panel_dib);  		
			
			if(FIA_IsGreyScale(window->panel_dib)) 
			{
				double intensity;
				int y = FreeImage_GetHeight(window->panel_dib) - image_point.y - 1; // FreeImage's are vertically flipped
				FREE_IMAGE_TYPE type;
				
				FIA_GetPixelValue(window->panel_dib, image_point.x, y, &intensity);
				
				type = FreeImage_GetImageType(window->panel_dib);
				
				sprintf(buffer0, "%d, %d", image_point.x, image_point.y);
				
				if (type==FIT_FLOAT || type == FIT_DOUBLE) 
				{
					sprintf(buffer1, "%.3f", intensity);
					strcat(buffer1, " (float)");
				}
				else  
				{
					sprintf(buffer1, "%.0f", intensity);
					strcat(buffer1, " (int)");
				}
				
				SetStatusbarText(window, 0, buffer0); 
				SetStatusbarText(window, 1, buffer1);

			}
			else 
			{
				RGBQUAD value;
				int y = FreeImage_GetHeight(window->panel_dib) - image_point.y; // FreeImage's are vertically flipped
				FreeImage_GetPixelColor(window->panel_dib, image_point.x, y, &value);
				sprintf(buffer0, "%d, %d", image_point.x, image_point.y);
				sprintf(buffer1, "R: %d  G: %d  B: %d", value.rgbRed, value.rgbGreen, value.rgbBlue);
				
				SetStatusbarText(window, 0, buffer0); 
				SetStatusbarText(window, 1, buffer1);
			}		 
			
			break;
		}
		
        // Provide a hand cursor when the mouse moves over us
		case WM_SETCURSOR:
		{
			HCURSOR	mouse_cursor = 0;
	
			if(LOWORD(lParam) != HTCLIENT) {
			
				return FALSE;   
			}
			else {
	
				if(window->cursor_type == CURSOR_CROSS)
					mouse_cursor = LoadCursor(NULL, IDC_CROSS);
				else if (window->cursor_type == CURSOR_NORMAL)
					mouse_cursor = LoadCursor(NULL, IDC_ARROW);
				else if (window->cursor_type == CURSOR_SIZE_ALL)
					mouse_cursor = LoadCursor(NULL, IDC_SIZEALL);
				else if (window->cursor_type == CURSOR_NESW)
					mouse_cursor = LoadCursor(NULL, IDC_SIZENESW);
				else if (window->cursor_type == CURSOR_NWSE)
					mouse_cursor = LoadCursor(NULL, IDC_SIZENWSE);
				else if (window->cursor_type == CURSOR_WE)
					mouse_cursor = LoadCursor(NULL, IDC_SIZEWE);
				else if (window->cursor_type == CURSOR_NS)
					mouse_cursor = LoadCursor(NULL, IDC_SIZENS);
				else
					break;
			
				SetCursor(mouse_cursor); 
					
				return TRUE;
			}
        }
        
        case IMAGEVIEWER_ZOOM_CHANGED:
        {
        	EventData data1 = NewEventData();       
           	data1.zoom_factor = ImageViewer_GetZoomFactor(window->canvas_window);
            
			SEND_EVENT(window, on_zoom_changed, data1, NewEventData(), "on_zoom_changed")  
        	
        	return 0;
        }
        
        default:
		
        	break;
   	}
   	
	return CallWindowProc ((WNDPROC) window->canvas_original_proc_fun_ptr, hwnd, message, wParam, lParam);
}
