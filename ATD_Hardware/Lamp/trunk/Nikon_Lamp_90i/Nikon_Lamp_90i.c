#include "Nikon_Lamp_90i.h"
#include "90i_lamp_ui.h"

#include <cviauto.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "ISCOPELib.h"
#include "MIPPARAMLib.h"


int Nikon90i_lamp_destroy (Lamp* lamp)
{
	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	CA_DiscardObjHandle(lamp_90i->hLamp); 
	
	return LAMP_SUCCESS;
}


static int Nikon90i_lamp_off(Lamp* lamp)
{
	ERRORINFO errInfo;
	int err;

	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	err = ISCOPELib_ILampOff (lamp_90i->hLamp, &errInfo);

	if (err) {

		logger_log(UIMODULE_LOGGER(lamp_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(lamp_90i), errInfo.description);  

		return LAMP_ERROR;
	}
	
	SetCtrlVal(lamp->_main_ui_panel, LAMP_PNL_ONOFF, 0);
	SetCtrlVal(lamp->_main_ui_panel, LAMP_PNL_LED, 0);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
	
  	return LAMP_SUCCESS;
}

static int Nikon90i_lamp_on(Lamp* lamp)
{
	ERRORINFO errInfo;
	int err;
	double intensity;

	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	if (lamp_90i == NULL)
	  return LAMP_ERROR;
	  
	if (lamp_90i->_mounted != 1)
	  return LAMP_ERROR;

	err = ISCOPELib_ILampOn (lamp_90i->hLamp, &errInfo);

	if (err) {

		logger_log(UIMODULE_LOGGER(lamp_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(lamp_90i), errInfo.description); 

		return LAMP_ERROR;
	}
	
	// now set the voltage to the ui value
	GetCtrlVal(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, &intensity);
	Nikon90i_set_lamp_intensity(lamp, intensity);

	SetCtrlVal(lamp->_main_ui_panel, LAMP_PNL_ONOFF, 1);
	SetCtrlVal(lamp->_main_ui_panel, LAMP_PNL_LED, 1);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
	
  	return LAMP_SUCCESS;
}

static int Nikon90i_get_lamp_status (Lamp* lamp, int *status)
{
	enum ISCOPELibEnum_EnumStatus lampon;
	ERRORINFO errInfo;
	int err;
    
	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	//Read current lamp status, (1 = on).

	if (lamp_90i == NULL)
	  return LAMP_ERROR;
	  
	if (lamp_90i->_mounted != 1)
	  return LAMP_ERROR;

	err = ISCOPELib_ILampGet_IsOn (lamp_90i->hLamp, &errInfo, &lampon);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(lamp_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(lamp_90i), errInfo.description); 

		return LAMP_ERROR;
	}
	
	if(lampon)
		*status = LAMP_ON;
	else
		*status = LAMP_OFF;    
	
	return LAMP_SUCCESS;
}

static int Nikon90i_set_lamp_intensity (Lamp* lamp, double intensity)
{
	ERRORINFO errInfo;
	int err, status;
    
	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	//Set lamp intensity, (voltage = 1.0 to 12.0).

	if (lamp_90i == NULL)
	  return LAMP_ERROR;
	  
	if (lamp_90i->_mounted != 1)
	  return LAMP_ERROR;

	if(intensity < 1.0)
		intensity = 1.0;

	if(intensity > 12.0)
		intensity = 12.0;

	Nikon90i_get_lamp_status (lamp, &status);

	if(status == 1) {  // can only really set the lamp voltage if the lamp is on
		err = ISCOPELib_ILampSet_ControlVoltage (lamp_90i->hLamp, &errInfo, intensity);
		
		if (err) {

			logger_log(UIMODULE_LOGGER(lamp_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
				UIMODULE_GET_DESCRIPTION(lamp_90i), errInfo.description); 

			return LAMP_ERROR;
		}
	}

	SetCtrlVal(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, intensity);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
	
	return LAMP_SUCCESS;
}

static int Nikon90i_get_lamp_intensity (Lamp* lamp, double *intensity)
{
	ERRORINFO errInfo;
	int err;
    
	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	//Get lamp intensity, (voltage = 1.0 to 12.0).

	if (lamp_90i == NULL)
	  return LAMP_ERROR;
	  
	if (lamp_90i->_mounted != 1)
	  return LAMP_ERROR;

	err = ISCOPELib_ILampGet_MeasuredVoltage (lamp_90i->hLamp, &errInfo, intensity);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(lamp_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(lamp_90i), errInfo.description); 

		return LAMP_ERROR;
	}
	
	return LAMP_SUCCESS;
}

static int Nikon90i_set_intensity_range(Lamp *lamp, double min, double max, double increment)
{
	return LAMP_SUCCESS;
}

static HRESULT LampCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	Lamp *lamp = (Lamp *) caCallbackData;   
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);

	return 0;
}

static int CVICALLBACK On90iLampStateChanged (int panel, int control, int event,
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

int CVICALLBACK On90iLampQuit (int panel, int control, int event,
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

int CVICALLBACK On90iLampIntensityChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Lamp *lamp = (Lamp *) callbackData;
			double intensity;
	
			GetCtrlVal (lamp->_main_ui_panel, LAMP_PNL_INTENSITY, &intensity);

			lamp_set_intensity(lamp, intensity);
			
			break;		
		}
	}
	
	return 0;
}


static int Nikon90i_lamp_hardware_initialise(Lamp* lamp)
{
	double intensity;
	int status;

	Nikon90i_get_lamp_status (lamp, &status);

	Nikon90i_get_lamp_intensity (lamp, &intensity);
	Nikon90i_set_lamp_intensity (lamp, intensity);

    return LAMP_SUCCESS;
}

static int Nikon90i_lamp_initialise(Lamp* lamp)
{
	double intensity;
	int status;

	lamp_set_main_panel (lamp, ui_module_add_panel(UIMODULE_CAST(lamp), "90i_lamp_ui.uir", LAMP_PNL, 1));

	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_CLOSE, On90iLampQuit, lamp) < 0)
		return LAMP_ERROR;

	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_ONOFF, On90iLampStateChanged, lamp) < 0)
		return LAMP_ERROR;

	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LAMP_PNL_INTENSITY, On90iLampIntensityChanged, lamp) < 0)
		return LAMP_ERROR;

    return LAMP_SUCCESS;
}


// Saves to arbitary file - for changing microscope modes.
int Nikon90i_lamp_load_settings (Lamp* lamp, const char *filepath)
{
	dictionary* d = NULL;
	int file_size, power;
	char *data_filepath = NULL, buffer[100];
	Lamp90i *lamp_90i = (Lamp90i *) lamp;     
	double intensity;
	
	if(!FileExists(filepath, &file_size))
		return LAMP_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
		intensity = dictionary_getdouble(d,
		  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(lamp), "Intensity"), 0.0);
  		
  		Nikon90i_set_lamp_intensity (lamp, intensity);

		power = dictionary_getdouble(d,
		  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(lamp), "Power"), 0);

		if (power == LAMP_ON)
			Nikon90i_lamp_on(lamp);
		else
			Nikon90i_lamp_off(lamp);
  		
        dictionary_del(d);
	}

	return LAMP_SUCCESS;	      
}


int Nikon90i_lamp_save_settings (Lamp* lamp, const char *filepath, const char *flags)
{
	FILE *fd;
	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
    dictionary *d = dictionary_new(20);
	double intensity;
	int power;
	
	Nikon90i_get_lamp_intensity (lamp, &intensity);
	Nikon90i_get_lamp_status(lamp, &power);

	fd = fopen(filepath, flags);
	
	dictionary_set(d, UIMODULE_GET_NAME(lamp), NULL);

	dictionary_setint(d, "Power", power);
	dictionary_setdouble(d, "Intensity", intensity);
 
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return LAMP_SUCCESS;	      
}

Lamp* Nikon90i_lamp_new(CAObjHandle hNikon90i, char *name, char *description,
  UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType; 
	
	Lamp90i* lamp_90i = (Lamp90i*) malloc(sizeof(Lamp90i));
	Lamp *lamp = (Lamp*) lamp_90i;
	
	lamp_constructor(lamp, name, description, handler, data_dir);

	LAMP_VTABLE_PTR(lamp, destroy) = Nikon90i_lamp_destroy; 
	LAMP_VTABLE_PTR(lamp, init) = Nikon90i_lamp_initialise; 
	LAMP_VTABLE_PTR(lamp, hardware_init) = Nikon90i_lamp_hardware_initialise; 
	LAMP_VTABLE_PTR(lamp, lamp_off) = Nikon90i_lamp_off; 
	LAMP_VTABLE_PTR(lamp, lamp_on) = Nikon90i_lamp_on; 
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity) = Nikon90i_set_lamp_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_get_intensity) = Nikon90i_get_lamp_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range) = Nikon90i_set_intensity_range;
    LAMP_VTABLE_PTR(lamp, save_settings) = Nikon90i_lamp_save_settings;   
	LAMP_VTABLE_PTR(lamp, load_settings) = Nikon90i_lamp_load_settings; 
	LAMP_VTABLE_PTR(lamp, lamp_off_on_status) = Nikon90i_get_lamp_status;

	if(err = ISCOPELib_INikon90iGetLamp (hNikon90i, &errInfo, &(lamp_90i->hLamp)))
		goto Error;

	if(err = ISCOPELib_ILampGet_IsMounted (lamp_90i->hLamp, &errInfo, &mounted))
		goto Error;

	lamp_90i->_mounted = mounted;

	lamp_set_intensity_range(lamp, 1.0, 12.0, 1.0);

	#ifndef ENABLE_LAMP_STATUS_POLLING
	
	if(err = ISCOPELib_ILampGetMeasuredVoltage (lamp_90i->hLamp, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, LampCallback, lamp, 1, NULL))
		goto Error;
	
	#endif
	
	return lamp;
	
	Error:
	
	mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200); 
	ui_module_send_error (UIMODULE_CAST(lamp), "Lamp error", "%s", error_str);
	
	return NULL;
}
