#include "HardWareTypes.h"

#include "spc.h"

#include "spc_ui.h"
#include "gci_utils.h"

#include <utility.h>
#include <userint.h>  

#ifdef BUILD_MODULE_LASER_POWER_MONITOR
#include "LaserPowerMonitor.h"
#endif

int spc_init_main_panel(Spc* spc)
{
	if ((spc->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(spc), "spc_ui.uir", SPC_MAIN, 1)) < 0)
		return SPC_ERROR;  
	
	if ( InstallPanelCallback (spc->_main_ui_panel, cbKeypress, spc) < 0) 
		return SPC_ERROR;

	if ( InstallCtrlCallback (spc->_graph_ui_panel, SPC_GRAPH_SAVE_PROMT, OnSpcSavePrompt, spc) < 0)
		return SPC_ERROR;

//    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_DISPLAY, cbSetDisplayTime, spc) < 0)
//		return SPC_ERROR;
  	
    //if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_TAC_SEL, cbUpdateMainPanelParams, spc) < 0)
	//	return SPC_ERROR;
  	
//    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_DISPLAY_TIME, cbSetDisplayTime, spc) < 0)
//		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_MC_TAC_OFFSET, cbSendMainPanelParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, cbSendMainPanelParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_REPEAT, cbSendMainPanelParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_REPEAT_TIME, cbSendMainPanelParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_AUTOSAVE, cbSendMainPanelParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_ACCUMULATE, cbSendMainPanelParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_FROM_TW, cbSetTimeWindow, spc) < 0)
		return SPC_ERROR;

	 if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_TO_TW, cbSetTimeWindow, spc) < 0)
		return SPC_ERROR;
  	
 //   if ( InstallCtrlCallback (spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, cbConfigGraph, spc) < 0)
//		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_SCOPE, cbSPCscope, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_ADVANCED, cbShowSysParams, spc) < 0)
		return SPC_ERROR;
  	
    //if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_SAVE_RESPONSE_SIGNAL, cbSaveAsResponse, spc) < 0)
	//	return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_STOP, cbStopSPC, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_START, cbStartSPC, spc) < 0)
		return SPC_ERROR;
  	
	if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_START_LIVE, OnStartSpcLive, spc) < 0)
		return SPC_ERROR;

    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_TYPE, cbAcqLimit, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_VAL, cbAcqLimit, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_ADC_RES, cbSendMainPanelADCres, spc) < 0)
		return SPC_ERROR;
  	
	if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_SCANNER_ZOOM, OnSpcScannerControlsChanged, spc) < 0)
		return SPC_ERROR;

	if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_SCANNER_SPEED, OnSpcScannerControlsChanged, spc) < 0)
		return SPC_ERROR;

	if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_SCANNER_RESOLUTION, OnSpcScannerControlsChanged, spc) < 0)
		return SPC_ERROR;

    //if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_IMTYPE, cbTRimtype, spc) < 0)
	//	return SPC_ERROR;
  	
	/*
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_R_DISP, cbMultiChanDisplay, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_G_DISP, cbMultiChanDisplay, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_B_DISP, cbMultiChanDisplay, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_RED_GAIN, cbMultiChanDisplay, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_GREEN_GAIN, cbMultiChanDisplay, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_BLUE_GAIN, cbMultiChanDisplay, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_SAVE_RGB, cbSaveRGB, spc) < 0)
		return SPC_ERROR;
  	*/

    if ( InstallCtrlCallback (spc->_main_ui_panel, SPC_MAIN_CLOSE, cbSpcClose, spc) < 0)
		return SPC_ERROR;
 
	
	return SPC_SUCCESS;
}


int CVICALLBACK cbKeypress (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;

		case EVENT_KEYPRESS:
		{
			switch (eventData1)
			{
				case 32: // space bar
				{					 
					Spc *spc = (Spc *) callbackData;

					if (spc->_acquire == 0)
						spc_start_scope(spc, 0);			
					else
						spc_stop_scope(spc);

					return 1;
				}
			}
		}
	
	}

	return 0;
}

/*
int CVICALLBACK cbSetDisplayTime (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			Spc *spc = (Spc *) callbackData;

			spc_set_display_time(spc);

			break;
		}
	}

	return 0;
}
*/

int CVICALLBACK cbUpdateMainPanelParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			Spc *spc = (Spc *) callbackData;

			spc_update_main_panel_parameters(spc);
			}break;
		}
	return 0;
}

int CVICALLBACK cbSendMainPanelParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{		
			Spc *spc = (Spc *) callbackData;

			spc_send_main_panel_parameters(spc); 

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbSetTimeWindow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			Spc *spc = (Spc *) callbackData;
			int min, max;

			GetCtrlVal(panel, SPC_MAIN_FROM_TW, &min);
			GetCtrlVal(panel, SPC_MAIN_TO_TW, &max);

			if(min >= max) {
				GCI_MessagePopup("Spc Error", "Time window minimum value must be less than the maximum");
				return -1;
			}

			spc->_time_window_min = min;
			spc->_time_window_max = max;

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbConfigGraph (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			
			Spc *spc = (Spc *) callbackData;

			//if (bh_mode(spc) == OSCILLOSCOPE) {
			//	GetCtrlVal(spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, &spc->_max_count);
			//	SetAxisScalingMode (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, 0, spc->_max_count);
			//	break;
			//}
			
			if(control == SPC_GRAPH_MAX_COUNT) {
				SetCtrlVal(spc->_graph_ui_panel, SPC_GRAPH_AUTOSCALE, 0);
				spc_set_scope_y_axis_scale(spc, NULL);
			}
			else {
				spc_set_scope_y_axis_scale(spc, spc->_buffer);
			}

			/*
			if (spc->_scan_pattern == SPC_XY_SCAN) {
				GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_IMTYPE, &spc->_image_type);
				if (spc->_image_type > 0)	//Tau image
					GetCtrlVal(spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, &spc->_max_scale_for_tau);
				else {
					GetCtrlVal(spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, &spc->_scan_max_count);
				} 
			}
			*/

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbSPCscope (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;

#ifdef BUILD_MODULE_LASER_POWER_MONITOR
			if(spc->_ms->_laser_power_monitor != NULL)
				laserpowermonitor_enable_timer(spc->_ms->_laser_power_monitor);
#endif			

			spc_start_scope(spc, 0);			//start

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbShowSysParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;

			ui_module_display_panel(UIMODULE_CAST(spc), spc->_params_ui_panel);

			break;
		}
	}

	return 0;
}

/*
int CVICALLBACK cbFLIM (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK cbSaveAsResponse (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{

			break;
		}
	}
	
	return 0;
}
*/

int CVICALLBACK cbStopSPC (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;

#ifdef BUILD_MODULE_LASER_POWER_MONITOR
			if(spc->_ms->_laser_power_monitor != NULL)
				laserpowermonitor_disable_timer(spc->_ms->_laser_power_monitor);
#endif			

			spc_stop(spc);

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbStartSPC (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;

			spc_start(spc);

			break;
		}
	}

	return 0;
}

// Convience function that sets display and repeat and continously acquires
int CVICALLBACK OnStartSpcLive (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;

#ifdef BUILD_MODULE_LASER_POWER_MONITOR
			if(spc->_ms->_laser_power_monitor != NULL)
				laserpowermonitor_enable_timer(spc->_ms->_laser_power_monitor);
#endif			

			spc_set_acq_limit_adv(spc, SPC_ACQ_LIMIT_TYPE_FRAMES, 0);
			spc_start_advanced(spc, SPC_ACQ_LIMIT_TYPE_FRAMES, 1, 1, 1.0, 1, 1.0, 0, 0, 1, NULL);

			break;
		}
	}

	return 0;
}


int CVICALLBACK OnSpcScannerControlsChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;
			int val;

			GetCtrlVal(spc->_main_ui_panel, control, &val);

			if(control == SPC_MAIN_SCANNER_RESOLUTION) {
				scanner_set_resolution(spc->_scanner, val);

				// We could have used the time window range here.
				// Ie a smaller time window we could have a bigger scan size but we
				// seem to count the photons in the max possible time window for the adc
				spc_adapt_parameters_for_memory(spc, 1);
			}
			else if(control == SPC_MAIN_SCANNER_SPEED) {
				int hyst_offset;
				scanner_get_hyst_offset(spc->_scanner, &hyst_offset);
				scanner_set_speed(spc->_scanner, val, hyst_offset);
			}
			else if(control == SPC_MAIN_SCANNER_ZOOM) {
				scanner_set_zoom(spc->_scanner, val);
			}

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbAcqLimit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;

			//What determines when we stop acquisition?
			spc_set_acq_limit(spc);

			break;
		}
	}

	return 0;
}


int CVICALLBACK cbSendMainPanelADCres (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			unsigned short adc_resolution;
			Spc *spc = (Spc *) callbackData;

			GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ADC_RES, &adc_resolution);
			
			bh_set_adc_res(spc, adc_resolution);
	//		spc_adapt_parameters_for_memory(spc, spc->_spc_data->scan_size_x, spc->_spc_data->scan_size_y, adc_resolution, 0);
			spc_adapt_parameters_for_memory(spc, 0);

			break;
		}
	}

	return 0;
}


int CVICALLBACK OnSpcSavePrompt (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;

			/*
			char filepath[GCI_MAX_PATHNAME_LEN] = "";
			char *default_extensions = "*.ics;";
			char directory[GCI_MAX_PATHNAME_LEN] = "";
			int old_zoom;

			old_zoom = scanner_get_zoom(spc->_scanner);

			spc->_ignore_scanner_signals = 1;
			scanner_set_zoom(spc->_scanner, PARK_SCAN);
			spc_acquire_prompt(spc);

			if (LessLameFileSelectPopup (spc->_main_ui_panel, directory, "*.ics",
				default_extensions, "Save Image As", VAL_OK_BUTTON, 0, 0, 1, 1, filepath) <= 0) {
				scanner_set_zoom(spc->_scanner, old_zoom);
				spc->_ignore_scanner_signals = 0;
				return -1;
			}
			*/

			spc_save_prompt_image(spc);
			//spc_save_3d_image_from_spc_data (spc, filepath, 1);

			//scanner_set_zoom(spc->_scanner, old_zoom);
			//spc->_ignore_scanner_signals = 0;

			break;
		}
	}

	return 0;
}

/*
int CVICALLBACK cbTRimtype (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			Spc *spc = (Spc *) callbackData;

			GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_IMTYPE, &spc->_image_type);
			if (spc->_image_type == 0) {	//intensity image
				SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_FLIM, ATTR_DIMMED, 0);
				SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_SAVE_RESPONSE_SIGNAL, ATTR_DIMMED, 0);
				GetCtrlVal (spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, &spc->_max_scale_for_tau);
				SetCtrlAttribute (spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, ATTR_LABEL_TEXT, "Max Count");
				//GCI_SetDisplayLUT(LUT_NORMAL);
			}
			else {
				//Use reverse rainbow LUT for FLIM display
				//GCI_SetDisplayLUT(LUT_REVERSE_RAINBOW);
				SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_FLIM, ATTR_DIMMED, 1);
				SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_SAVE_RESPONSE_SIGNAL, ATTR_DIMMED, 1);
				SetCtrlVal (spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, spc->_max_scale_for_tau);
				SetCtrlAttribute (spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, ATTR_LABEL_TEXT, "Max ps");
			}
			
			//GCI_FLIM_SetRequiredImType(spc->_image_type);
			//rebuildImage();
			}break;
		}
	return 0;
}
*/

int CVICALLBACK cbSpcClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;

			spc_hide_ui(spc);
			break;
		}
	}

	return 0;
}

int CVICALLBACK OnOscilloscopeClosed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData;   

			spc->_acquire = 0;		// Stop    
			
			ui_module_hide_panel(UIMODULE_CAST(spc), spc->_graph_ui_panel);
			
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCOPE, 0);     
			
			break;
		}
	}
	return 0;
}


int CVICALLBACK cbMultiChanDisplay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
	
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK cbSaveRGB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{

			}break;
	}
	
	return 0;
}

int CVICALLBACK cbRateTimer (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			int ret;
			short sync_state;
			Spc *spc = (Spc *) callbackData; 
	
			bh_read_rates(spc);
			
			ret = SPC_get_sync_state(spc->_active_module, &sync_state);
	   		spc_check_error(spc, ret);
			
			if (sync_state == 1) {	//OK
				SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_SYNC_LED, 1);
				SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SYNC_LED, 1);  
			}
			else {
				SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_SYNC_LED, 0);
				SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SYNC_LED, 0); 
			}
			
			break;
		}
	}
	return 0;
}

int CVICALLBACK cbSendSysParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			unsigned short mode = 0;
			Spc *spc = (Spc *) callbackData; 
	
			//Remember collection time
			GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OP_MODE, &mode);

			if (mode == BH_SCOPE_MODE) 	
				GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COLLECT_TIME, &spc->_oscilloscope_collect_time);
			else	//scan	
				GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COLLECT_TIME, &spc->_scan_collect_time);

			bh_set_operation_mode(spc, mode);	// send all params and configure memory
			
		   	//bh_on_change(spc);

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbQuitSysParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData; 

			ui_module_hide_panel(UIMODULE_CAST(spc), spc->_params_ui_panel);

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbSaveSysParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData; 
            int ret;
            
			ret = spc_save_default_params(spc);
            
			if (SPC_SUCCESS==ret) {
				GCIDialog(0, "Configuration Saved", IDI_INFORMATION, GCI_OK_BUTTON, "Configuration saved successfully.");
			}
			else {
				GCIDialog(0, "Configuration Save Error", IDI_EXCLAMATION, GCI_OK_BUTTON, "Configuration FAILED to save.");
			}

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbLoadSysParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData; 

			spc_load_settings_dialog(spc);

			break;
		}
	}

	return 0;
}

int CVICALLBACK cbSysParamOpMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			unsigned short mode = 0;

			Spc *spc = (Spc *) callbackData; 

			GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OP_MODE, &mode);

			bh_set_operation_mode(spc, mode);

			break;
		}
	}

	return 0;
}


int CVICALLBACK cbCheckScanSize (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Spc *spc = (Spc *) callbackData; 

			//adjust controls such that total memory (16Mb) is not exceeded

			if (control == SPC_PARAM_ADC_RES) {
				GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ADC_RES, &spc->_spc_data->adc_resolution);
				//bh_check_scan_size(spc, ADC_RES);
				
				//break;
			}
			else if (control == SPC_PARAM_SCAN_X) {
				GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_X, &spc->_spc_data->scan_size_x);
			//	bh_check_scan_size(spc, SCAN_X);
				//break;
			}
			else if (control == SPC_PARAM_SCAN_Y) {
				GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_Y, &spc->_spc_data->scan_size_y);
			//	bh_check_scan_size(spc, SCAN_Y);
				//break;
			}
			else  if (control == SPC_PARAM_ROUTING_X) {	
				GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ROUTING_X, &spc->_spc_data->scan_rout_x);
				//bh_check_scan_size(spc, ROUTING_X);
				//break;
			}
			else  if (control == SPC_PARAM_ROUTING_Y) {	
				GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ROUTING_Y, &spc->_spc_data->scan_rout_y);
				//bh_check_scan_size(spc, ROUTING_Y);
				//break;
			}

			// Updated Spc values according to scanner values.
			spc_scanner_value_update(spc->_scanner, -1, (void *) spc);
			spc_adapt_parameters_for_memory(spc, 1);

			break;
		}
	}

	return 0;
}
