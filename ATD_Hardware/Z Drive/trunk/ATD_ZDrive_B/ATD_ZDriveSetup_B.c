#include <userint.h>

#include "ATD_ZDrive_B.h"
#include "ATD_ZDriveAutoFocus_B.h"

int CVICALLBACK OnAtdBZdriveSetup_Close (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{

			ATD_ZDRIVE_B *atd_zdrive_b_zd = (ATD_ZDRIVE_B *) callbackData;  
		
			ui_module_hide_panel(UIMODULE_CAST(atd_zdrive_b_zd), atd_zdrive_b_zd->_setup_panel_id);   
			
			break;
		}
	}

	return 0;
}

int CVICALLBACK OnAtdBZdriveSetup_ModeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_ZDRIVE_B *atd_zdrive_b_zd = (ATD_ZDRIVE_B *) callbackData;  
			int mode;

			GetCtrlVal(panel, control, &mode);
			
			// Here we slect from AF, OPT_FOCUS or OFF	
			atd_zdrive_b_set_output_mode(atd_zdrive_b_zd, mode);

			if(mode != ATD_ZDRIVE_B_OUTPUT_MODE_DAC ) {
				SetCtrlAttribute(panel, SETUP_PNL_DAC_VAL, ATTR_DIMMED, 1);			
			}
			else {
				SetCtrlAttribute(panel, SETUP_PNL_DAC_VAL, ATTR_DIMMED, 0);	
			}

			break;
		}
	}

	return 0;
}

int CVICALLBACK OnAtdBZdriveSetup_ConversionControlChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_ZDRIVE_B *atd_zdrive_b_zd = (ATD_ZDRIVE_B *) callbackData;  
			int conversion_val, sampling_rate;

			GetCtrlVal(panel, SETUP_PNL_CONVERSION_CONTROL, &conversion_val);
			GetCtrlVal(panel, SETUP_PNL_SAMPLINGRATE, &sampling_rate);

			// Check parameters
			if(conversion_val != ATD_ZDRIVE_B_CONVERSION_MODE_CONTINOUS && conversion_val != ATD_ZDRIVE_B_CONVERSION_MODE_SINGLE) {
				MessagePopup("ZDrive Error", "Invalid parameter for conversion control");
				return -1;
			}

			if(sampling_rate < ATD_ZDRIVE_B_SAMPLING_VAL_60 || sampling_rate > ATD_ZDRIVE_B_SAMPLING_VAL_15) {
				MessagePopup("ZDrive Error", "Invalid parameter for sampling rate");
				return -1;
			}

			atd_zdrive_b_initialise(atd_zdrive_b_zd, conversion_val, sampling_rate);

			break;
		}
	}

	return 0;
}

int CVICALLBACK OnAtdBZdriveSetup_OnDac1Selected (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_ZDRIVE_B *atd_zdrive_b_zd = (ATD_ZDRIVE_B *) callbackData;  
			
			atd_zdrive_b_set_dac1_use(atd_zdrive_b_zd);

			break;
		}
	}

	return 0;
}

int CVICALLBACK OnAtdBZdriveSetup_OnDac2Selected (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_ZDRIVE_B *atd_zdrive_b_zd = (ATD_ZDRIVE_B *) callbackData;  
			
			atd_zdrive_b_set_dac2_use(atd_zdrive_b_zd);

			break;
		}
	}

	return 0;
}

int CVICALLBACK OnAtdBZdriveSetup_OnDacValChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_ZDRIVE_B *atd_zdrive_b_zd = (ATD_ZDRIVE_B *) callbackData;  
			int val;

			GetCtrlVal(panel, control, &val);

			if(atd_zdrive_b_zd->_stage_removed) {
		
				// If the stage gets removed, we can not happily proceed with objective control by use of
				// the objective dac. The hardware switches the objective to be controlled by the focus dac (crazy hardware) !

				if(atd_zdrive_b_set_dac(atd_zdrive_b_zd, FOCUS_DAC, val, 1) == Z_DRIVE_ERROR) {

					MessagePopup("ZDrive Error", "Falied to set dac val");
					return -1;
				}
			}
			else {

				if(atd_zdrive_b_set_dac(atd_zdrive_b_zd, atd_zdrive_b_zd->_current_dac, val, 1) == Z_DRIVE_ERROR) {

					MessagePopup("ZDrive Error", "Falied to set dac val");
					return -1;
				}
			}

			break;
		}
	}

	return 0;
}

int CVICALLBACK OnAtdBZdriveSetup_OnDacSettingsChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_ZDRIVE_B *atd_zdrive_b_zd = (ATD_ZDRIVE_B *) callbackData;  

			Dac *dac = &(atd_zdrive_b_zd->_dacs[atd_zdrive_b_zd->_current_dac]);
			
			GetCtrlVal(panel, SETUP_PNL_SET_POS_SCALE, &(dac->_scale_factor));
			GetCtrlVal(panel, SETUP_PNL_SET_POS_SCALE, &(dac->_offset));
			GetCtrlVal(panel, SETUP_PNL_ACT_POS_SCALE, &(dac->_adc_scale_factor));
			GetCtrlVal(panel, SETUP_PNL_ACT_POS_OFFSET, &(dac->_adc_offset));

			break;
		}
	}

	return 0;
}