#include <userint.h>
#include "password.h"

#include "ATD_StepperMotor_A.h"
#include "ATD_StepperMotorUI_A.h"

////////////////////////////////////////////////////////////////////////////
//RJL/RGN April 2007
//GCI HTP Microscope system. 
//Mirror stepper control.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK cb_mirror_stepper_cal (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;

			 GCI_ShowPasswordProtectedPanel(mirror_stepper->_calib_ui_panel, mirror_stepper->_main_ui_panel);  
			}break;
		}
	return 0;
}

int CVICALLBACK cb_mirror_stepper_init (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;

			mirror_stepper_initialise(mirror_stepper, 1);
			}break;
		}
	return 0;
}

int CVICALLBACK cb_mirror_stepper_save (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;

			mirror_stepper_save_data(mirror_stepper);

			break;
		}
	}

	return 0;
}


int CVICALLBACK cb_mirror_stepper_quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;

			mirror_stepper_hide_main_ui(mirror_stepper);
			}break;
		}
	return 0;
}

int CVICALLBACK cb_mirror_stepper_setpos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			int mirrorpos;
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;

			GetCtrlVal(mirror_stepper->_main_ui_panel, MS_PANEL_MIRRORPOS ,&mirrorpos);
			mirror_stepper_set_pos(mirror_stepper, mirrorpos);
			}break;
		}
	return 0;
}

int CVICALLBACK cb_mirror_stepper_posoffset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;
			int offset;
	
			GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET, &offset); 
			mirror_stepper_set_offset(mirror_stepper, 0, offset);

			}break;
		}
	return 0;
}

int CVICALLBACK cb_mirror_stepper_posoffset1 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;
			int offset;
	
			GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_1, &offset); 
			mirror_stepper_set_offset(mirror_stepper, 1, offset);
			}break;
		}
	return 0;
}


int CVICALLBACK cb_mirror_stepper_posoffset2 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;
			int offset;
	
			GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_2, &offset); 
			mirror_stepper_set_offset(mirror_stepper, 2, offset);
			}break;
		}
	return 0;
}

int CVICALLBACK cb_mirror_stepper_posoffset3 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;
			int offset;
	
			GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_3, &offset); 
			mirror_stepper_set_offset(mirror_stepper, 3, offset);
			}break;
		}
	return 0;
}

int CVICALLBACK cb_mirror_stepper_posoffset4 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;
			int offset;
	
			GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_4, &offset); 

			mirror_stepper_set_offset(mirror_stepper, 4, offset);
			}break;
		}
	return 0;
}


int CVICALLBACK cb_mirror_stepper_set_default (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;

			SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_1 ,0);
			SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_2 ,0); 
			SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_3 ,0); 
			SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_4 ,0);
			SetCtrlVal(panel, control ,1);
			
			if (control == MS_CALPNL_DEFAULT_1)
				mirror_stepper->_default_pos = 1;
			else if (control == MS_CALPNL_DEFAULT_2)
				mirror_stepper->_default_pos = 2;
			else if (control == MS_CALPNL_DEFAULT_3)
				mirror_stepper->_default_pos = 3;
			else if (control == MS_CALPNL_DEFAULT_4)
				mirror_stepper->_default_pos = 4;
				
			}break;
		}
	return 0;
}

int CVICALLBACK cb_mirror_stepper_close_cal (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			MirrorStepper *mirror_stepper = (MirrorStepper *) callbackData;

			mirror_stepper_hide_calib_ui(mirror_stepper);
			}break;
		}
	return 0;
}

