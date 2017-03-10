#include <cviauto.h>

#include "90i_lamp.h"
#include "lamp_ui.h"

#include "iscope90i.h"
#include "mipparam90i.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Brightness control.
////////////////////////////////////////////////////////////////////////////


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
		err = CA_DisplayErrorInfo (lamp_90i->hLamp, NULL, err, &errInfo);
		return LAMP_ERROR;
	}
	
	lamp_on_change(lamp); //update uir
	
  	return LAMP_SUCCESS;
}

static int Nikon90i_lamp_on(Lamp* lamp)
{
	ERRORINFO errInfo;
	int err;

	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	err = ISCOPELib_ILampOn (lamp_90i->hLamp, &errInfo);

	if (err) {
		err = CA_DisplayErrorInfo (lamp_90i->hLamp, NULL, err, &errInfo);
		return LAMP_ERROR;
	}
	
	lamp_on_change(lamp); //update uir
	
  	return LAMP_SUCCESS;
}

static int Nikon90i_get_lamp_status (Lamp* lamp, int *status)
{
	enum ISCOPELibEnum_EnumStatus lampon;
	ERRORINFO errInfo;
	int err;
    
	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	//Read current lamp status, (1 = on).

	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	err = ISCOPELib_ILampGet_IsOn (lamp_90i->hLamp, &errInfo, &lampon);
	
	if (err) {
		err = CA_DisplayErrorInfo (lamp_90i->hLamp, NULL, err, &errInfo);
		return LAMP_ERROR;
	}
	
	*status = lampon;
	
	return LAMP_SUCCESS;
}

static int Nikon90i_set_lamp_intensity (Lamp* lamp, double intensity)
{
	ERRORINFO errInfo;
	int err;
    
	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	//Set lamp intensity, (voltage = 1.0 to 12.0).

	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	err = ISCOPELib_ILampSet_ControlVoltage (lamp_90i->hLamp, &errInfo, intensity);
	
	if (err) {
		err = CA_DisplayErrorInfo (lamp_90i->hLamp, NULL, err, &errInfo);
		return LAMP_ERROR;
	}
	
	return LAMP_SUCCESS;
}

static int Nikon90i_get_lamp_intensity (Lamp* lamp, double *intensity)
{
	ERRORINFO errInfo;
	int err;
    
	Lamp90i *lamp_90i = (Lamp90i *) lamp; 
	
	//Get lamp intensity, (voltage = 1.0 to 12.0).

	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	err = ISCOPELib_ILampGet_MeasuredVoltage (lamp_90i->hLamp, &errInfo, intensity);
	
	if (err) {
		err = CA_DisplayErrorInfo (lamp_90i->hLamp, NULL, err, &errInfo);
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
	
  	PROFILE_START ("LampCallback") ;
	lamp_on_change(lamp);
	PROFILE_STOP ("LampCallback") ;
	//PROFILE_PRINT () ;
	
	return 0;
}

Lamp* Nikon90i_lamp_new(CAObjHandle hNikon90i)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType; 
	
	Lamp* lamp = lamp_new("90i lamp", "Brightness Control", sizeof(Lamp90i));
	
	Lamp90i *lamp_90i = (Lamp90i *) lamp;
	
	LAMP_VTABLE_PTR(lamp, destroy) = Nikon90i_lamp_destroy; 
	LAMP_VTABLE_PTR(lamp, lamp_off) = Nikon90i_lamp_off; 
	LAMP_VTABLE_PTR(lamp, lamp_on) = Nikon90i_lamp_on; 
	LAMP_VTABLE_PTR(lamp, lamp_status) = Nikon90i_get_lamp_status;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity) = Nikon90i_set_lamp_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_get_intensity) = Nikon90i_get_lamp_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range) = Nikon90i_set_intensity_range;

	if(err = ISCOPELib_INikon90iGetLamp (hNikon90i, &errInfo, &(lamp_90i->hLamp)))
		goto Error;

	if(err = ISCOPELib_ILampGet_IsMounted (lamp_90i->hLamp, &errInfo, &mounted))
		goto Error;

	lamp->_mounted = mounted;
	SetCtrlAttribute (lamp->_main_ui_panel, LAMP_PNL_ONOFF, ATTR_DIMMED, !mounted);
	SetCtrlAttribute (lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_DIMMED, !mounted);

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
	
	lamp_on_change(lamp); //update uir
	
	return lamp;
	
	Error:
	
	mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_lamp_error_text (lamp, "%s", error_str);     
	
	return NULL;
}
