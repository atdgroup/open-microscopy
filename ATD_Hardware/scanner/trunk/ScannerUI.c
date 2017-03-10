#include "Scanner.h" 
#include "ScannerUI.h"

#include <utility.h>
#include <userint.h>

////////////////////////////////////////////////////////////////////////////
//RJL/RGN April/May 2007
//GCI HTS Microscope system. 
//Scanner control callbacks.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK cbzoom (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 
			int zoom;

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_ZOOM, &zoom);

			scanner_set_zoom(scanner, zoom);	  
  
			break;
		}
	}

	return 0;
}

int CVICALLBACK cbspeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SPEED, &scanner->_speed);
			GetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_HYST_OFFSET, &scanner->_hyst_offset);
			
			scanner_set_speed(scanner, scanner->_speed, scanner->_hyst_offset);
       		
			break;
		}
	}

	return 0;
}

int CVICALLBACK cbresolution (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_RESOLUTION, &scanner->_resolution);
			scanner_set_resolution(scanner, scanner->_resolution);
   
			break;
		}
	}

	return 0;
}

int CVICALLBACK cbshift_reset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			scanner_reset_shifts(scanner);
       		
			break;
		}
	}

	return 0;
}

int CVICALLBACK cbquit_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			scanner_hide_main_ui(scanner);

			//scanner_save_settings_as_default(scanner);

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbstart_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_FRAME_NUM, &scanner->_frames);
			scanner_start_scan(scanner, scanner->_frames);
			
			break;
		}
	}

	return 0;
}

int CVICALLBACK cbx_shift (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_X_SHIFT, &scanner->_x_shift);  
			scanner_set_x_shift(scanner, scanner->_x_shift);
       		
			break;
		}
	}

	return 0;
}

int CVICALLBACK cby_shift (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_Y_SHIFT, &scanner->_y_shift);  
			scanner_set_y_shift(scanner, scanner->_y_shift);

			break;
		}
	}
	return 0;
}

int CVICALLBACK cbframe_num (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_FRAME_NUM, &scanner->_frames);  

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbhyst_offset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SPEED, &scanner->_speed);
			GetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_HYST_OFFSET, &scanner->_hyst_offset);
			scanner_set_speed(scanner, scanner->_speed, scanner->_hyst_offset);

			break;
		}
	}
	return 0;
}

int CVICALLBACK OnOffsetChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_X_OFFSET, &scanner->_x_offset);
			GetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_Y_OFFSET, &scanner->_y_offset);
		
			scanner_set_shifts_to_centre(scanner);

			break;
		}
	}

	return 0;
}


int CVICALLBACK cbrev_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_REV_SCAN, &scanner->_reverse_scan);
			scanner_reverse_scan(scanner, scanner->_reverse_scan);
			break;
		}	
	}

	return 0;
}

int CVICALLBACK cbscan_disable (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			//Send 0 to enable, 1 for standby
			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_DISABLE, &scanner->_scan_disable);
			scanner_disable_scanner(scanner, scanner->_scan_disable);

			break;
		}
	}
	return 0;
}

int CVICALLBACK cbline_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_LINE_SCAN, &scanner->_line_scan);
			SetCtrlAttribute(scanner->_main_ui_panel, SCAN_PNL_FRAME_NUM, ATTR_DIMMED, scanner->_line_scan);
			
			if (scanner->_line_scan) {
				scanner->_frames = 0;	   //Line scan so continuous scanning
				SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_FRAME_NUM, 0);
			}
			
			scanner_line_scan(scanner, scanner->_line_scan);
       		
			break;
		}
	}
	return 0;
}

int CVICALLBACK cbselect_clock (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			GetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_PIXCLK, &scanner->_pixel_clock);
			GetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_LINECLK, &scanner->_line_clock);
			GetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_FRAMECLK, &scanner->_frame_clock);
			scanner_select_clock(scanner, scanner->_pixel_clock, scanner->_line_clock, scanner->_frame_clock);

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbstop_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			scanner_stop_scan(scanner);
			break;
		}
	}
	return 0;
}

int CVICALLBACK OnScannerTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

        	scanner_read_error_signal(scanner, &scanner->_servo_error);
        
        	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_ERROR_IND, scanner->_servo_error);
			
			if (scanner->_servo_error && scanner->_scanning) {
				logger_log(UIMODULE_LOGGER(scanner), LOGGER_ERROR, "Scanner server error. Disabling scanning", UIMODULE_GET_DESCRIPTION(scanner));		
				scanner_stop_scan(scanner);
			}

			//Is duration of frame scans complete yet?
			if (scanner->_scanning && !scanner->_line_scan && (scanner->_frames>0)) {
				if ((Timer() - scanner->_start_time) >= scanner->_scan_time) 
					scanner->_scanning = 0;
			}
			
			SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_ON_IND, scanner->_scanning);
			SetCtrlAttribute (scanner->_main_ui_panel, SCAN_PNL_START_SCAN , ATTR_DIMMED, scanner->_scanning);
			SetCtrlAttribute (scanner->_main_ui_panel, SCAN_PNL_FRAME_NUM , ATTR_DIMMED, scanner->_scanning || scanner->_line_scan);
			SetCtrlAttribute (scanner->_main_ui_panel, SCAN_PNL_SPEED , ATTR_DIMMED, scanner->_scanning);

        	//scanner_on_change(scanner);
			break;
		}
	}

	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

	return 0;
}

int CVICALLBACK cbload_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 
			char path[GCI_MAX_PATHNAME_LEN] = "";

            if (FileSelectPopup (UIMODULE_GET_DATA_DIR(scanner), scanner->_global_settings_file, "*.ini", "Load", VAL_LOAD_BUTTON, 0, 1, 1, 0, path) <= 0)
				return 0;
	
			if (!FileExists(path, NULL))
                return 0;
                
			scanner_load_settings(scanner, path);
			
			break;  
		}
	}
	
	return 0;
}

int CVICALLBACK cbsave_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 
			char path[GCI_MAX_PATHNAME_LEN];
			int ret;

			ui_module_get_data_dir(UIMODULE_CAST(scanner), path);

			strcat(path, "\\");
			strcat(path, scanner->_global_settings_file);

			ret = scanner_save_settings(scanner, path, "w");

			if (SCANNER_SUCCESS==ret) {
				GCIDialog(0, "Configuration Saved", IDI_INFORMATION, GCI_OK_BUTTON, "Configuration saved successfully to:\n%s", path);
			}
			else {
				GCIDialog(0, "Configuration Save Error", IDI_EXCLAMATION, GCI_OK_BUTTON, "Configuration FAILED to save to:\n%s", path);
			}

			break; 
		}
	}
	
	return 0;
}

int CVICALLBACK cbdefine_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			ui_module_display_panel(UIMODULE_CAST(scanner), scanner->_cal_ui_panel);
			
			break;
		}
	}

	return 0;
}

int CVICALLBACK cbcalclose_scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Scanner *scanner = (Scanner*) callbackData; 

			ui_module_hide_panel(UIMODULE_CAST(scanner), scanner->_cal_ui_panel);
			
			break;
		}
	}

	return 0;
}

