#include <cviauto.h>

#include "ATD_Lamp_Dummy.h"
#include "ATD_Lamp_Dummy_UI.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Brightness control for manual microscope.
////////////////////////////////////////////////////////////////////////////

static int CVICALLBACK OnDummyLampStateChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            int mode;
   
			GetCtrlVal(panel, control, &mode);
			
			if(mode == 1)
				lamp_on(lamp);	
			else
				lamp_off(lamp);	

            break;
		}
	}
    
	return 0;
}

int CVICALLBACK OnDummyLampQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
			lamp_hide_main_ui(lamp);
            
            break;
		}
	}
    
	return 0;
}

static int manual_lamp_destroy (Lamp* lamp)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	return LAMP_SUCCESS;
}

static int manual_hardware_init (Lamp* lamp)
{
	return LAMP_SUCCESS;
}

static int manual_lamp_init (Lamp* lamp)
{
	lamp_set_main_panel (lamp, ui_module_add_panel(UIMODULE_CAST(lamp), "ATD_Lamp_Dummy_UI.uir", LAMP_PNL, 1));

	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_CLOSE, OnDummyLampQuit, lamp) < 0)
		return LAMP_ERROR;

	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_ONOFF, OnDummyLampStateChanged, lamp) < 0)
		return LAMP_ERROR;

	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_INTENSITY, OnLampIntensity, lamp) < 0)
		return LAMP_ERROR;

	return LAMP_SUCCESS;
}

static int manual_lamp_off(Lamp* lamp)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	if (lamp == NULL) return LAMP_ERROR;

	lamp_manual->_status = 0;

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_LED, 0);
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_ONOFF, 0); 

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
	
  	return LAMP_SUCCESS;
}

static int manual_lamp_on(Lamp* lamp)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	if (lamp == NULL) return LAMP_ERROR;

	lamp_manual->_status = 1;

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_LED, 1);
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_ONOFF, 1); 

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
	
  	return LAMP_SUCCESS;
}

static int Manual_get_lamp_status (Lamp* lamp, LampStatus *status)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	//Read current lamp status, (1 = on).

	if (lamp == NULL) return LAMP_ERROR;
//	if (lamp->_mounted != 1) return LAMP_ERROR;

	*status = lamp_manual->_status;
	
	return LAMP_SUCCESS;
}

static int Manual_set_lamp_intensity (Lamp* lamp, double intensity)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	//Set lamp intensity, (voltage = 1.0 to 12.0).

	if (lamp == NULL) return LAMP_ERROR;
//	if (lamp->_mounted != 1) return LAMP_ERROR;

	lamp_manual->_intensity = intensity;
	
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_INTENSITY, intensity); 

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
	
	return LAMP_SUCCESS;
}

static int Manual_get_lamp_intensity (Lamp* lamp, double *intensity)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	//Get lamp intensity, (voltage = 1.0 to 12.0).

	if (lamp == NULL) return LAMP_ERROR;
//	if (lamp->_mounted != 1) return LAMP_ERROR;

	*intensity = lamp_manual->_intensity;
	
	return LAMP_SUCCESS;
}

static int Manual_set_intensity_range(Lamp *lamp, double min, double max, double increment)
{
	return LAMP_SUCCESS;
}


static int Manual_save_settings (Lamp *lamp, const char *filepath, const char *flags)
{
	return LAMP_SUCCESS;     	
}

static int Manual_load_settings (Lamp *lamp, const char *filepath)
{
	return LAMP_SUCCESS;     		
}
	
Lamp* manual_lamp_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	LampManual* lamp_manual = (LampManual*) malloc(sizeof(LampManual));
	Lamp *lamp = (Lamp*) lamp_manual;
	
	memset(lamp_manual, 0, sizeof(LampManual));
	
	lamp_constructor(lamp, name, description, handler, data_dir);
	
	lamp_manual->_status = 1;
	lamp_manual->_intensity = 1.0;
	
	LAMP_VTABLE_PTR(lamp, init) = manual_lamp_init; 
	LAMP_VTABLE_PTR(lamp, hardware_init) = manual_hardware_init; 
	LAMP_VTABLE_PTR(lamp, destroy) = manual_lamp_destroy; 
	LAMP_VTABLE_PTR(lamp, lamp_off) = manual_lamp_off; 
	LAMP_VTABLE_PTR(lamp, lamp_on) = manual_lamp_on; 
	LAMP_VTABLE_PTR(lamp, lamp_off_on_status) = Manual_get_lamp_status;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity) = Manual_set_lamp_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_get_intensity) = Manual_get_lamp_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range) = Manual_set_intensity_range;
	LAMP_VTABLE_PTR(lamp, load_settings) = Manual_load_settings;
	LAMP_VTABLE_PTR(lamp, save_settings) = Manual_save_settings; 

	lamp_set_intensity_range(lamp, 1.0, 12.0, 1.0);

	return lamp;
}
