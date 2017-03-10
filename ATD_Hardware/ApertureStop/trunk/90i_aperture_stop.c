#include "90i_aperture_stop.h"
#include "single_range_hardware_device_ui.h"

#include <cviauto.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

int Nikon90i_aperture_stop_destroy (SingleRangeHardwareDevice* single_range_hardware_device)
{
	ApertureStop90i* aperture_stop90i = (ApertureStop90i*) single_range_hardware_device;      
	
	CA_DiscardObjHandle(aperture_stop90i->hApertureStop); 
	
	free(aperture_stop90i);
	
	return HARDWARE_SUCCESS;
}


static int Nikon90i_set_aperture_stop (SingleRangeHardwareDevice* single_range_hardware_device, double val)
{
	ERRORINFO errInfo;
	int err;
    
	ApertureStop90i* aperture_stop90i = (ApertureStop90i*) single_range_hardware_device;      
	
	//Set aperture stop, (val = 1.0 to 12.0).

	if (aperture_stop90i == NULL)
		return HARDWARE_ERROR;
	
	if (aperture_stop90i->_mounted != 1)
		return HARDWARE_ERROR;

	err = ISCOPELib_IApertureStopSet_ApertureStop (aperture_stop90i->hApertureStop, &errInfo, val);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(aperture_stop90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(aperture_stop90i), errInfo.description);  

		return HARDWARE_ERROR;
	}
	
	return HARDWARE_SUCCESS;
}

static int Nikon90i_get_aperture_stop (SingleRangeHardwareDevice* single_range_hardware_device, double *val)
{
	ERRORINFO errInfo;
	int err;
    
	ApertureStop90i* aperture_stop90i = (ApertureStop90i*) single_range_hardware_device;      
	
	//Get aperture stop, (val = 1.0 to 12.0).

	if (aperture_stop90i == NULL)
		return HARDWARE_ERROR;
	
	if (aperture_stop90i->_mounted != 1)
		return HARDWARE_ERROR;

	err = ISCOPELib_IApertureStopGet_ApertureStop (aperture_stop90i->hApertureStop, &errInfo, val);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(aperture_stop90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(aperture_stop90i), errInfo.description);  

		return HARDWARE_ERROR;
	}
	
	return HARDWARE_SUCCESS;
}

static int Nikon90i_set_aperture_range(SingleRangeHardwareDevice* single_range_hardware_device, double min, double max, double increment)
{
	return HARDWARE_SUCCESS;
}

static HRESULT ApertureCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	ApertureStop90i* aperture_stop90i = (ApertureStop90i *) caCallbackData;   
	
    single_range_hardware_device_on_change(SINGLE_RANGE_HW_DEVICE_CAST(aperture_stop90i));
    
	return 0;
}

static int Nikon90i_aperture_initialise (SingleRangeHardwareDevice* single_range_hardware_device)
{
	single_range_hardware_device_set_range(single_range_hardware_device, 2.0, 30.5, 0.1);

	return HARDWARE_SUCCESS;
}

SingleRangeHardwareDevice* Nikon90i_aperture_stop_new(CAObjHandle hNikon90i,  const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType;
	
    SingleRangeHardwareDevice* single_range_hardware_device = single_range_hardware_device_contructor(name, description, sizeof(ApertureStop90i));
	ApertureStop90i *aperture_stop90i =  (ApertureStop90i *) single_range_hardware_device;

	ui_module_set_data_dir( UIMODULE_CAST(aperture_stop90i), data_dir);
    ui_module_set_error_handler(UIMODULE_CAST(aperture_stop90i), error_handler, data); 
	
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, destroy) = Nikon90i_aperture_stop_destroy; 
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range_value) = Nikon90i_set_aperture_stop;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, get_range_value) = Nikon90i_get_aperture_stop;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range) = Nikon90i_set_aperture_range;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, hardware_initialise) = Nikon90i_aperture_initialise;

	if(err = ISCOPELib_INikon90iGetApertureStop (hNikon90i, &errInfo, &(aperture_stop90i->hApertureStop)))
		goto Error;

	if(err = ISCOPELib_IApertureStopGet_IsMounted (aperture_stop90i->hApertureStop, &errInfo, &mounted))
		goto Error;
	
	aperture_stop90i->_mounted = mounted;

	single_range_hardware_device_set_range(single_range_hardware_device, 2.0, 30.0, 1.0);   //Found impirically

	#ifndef ENABLE_APERTURE_STATUS_POLLING 
	
	if(err = ISCOPELib_IApertureStopGetApertureStop (aperture_stop90i->hApertureStop, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, ApertureCallback, aperture_stop90i, 1, NULL))
		goto Error;
	
	single_range_hardware_device_on_change(SINGLE_RANGE_HW_DEVICE_CAST(aperture_stop90i));
	
	#endif
	
	return aperture_stop90i;
	
	Error:
	
	aperture_stop90i->_mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200);
	ui_module_send_error (UIMODULE_CAST(aperture_stop90i), "Aperature stop error", "%s", error_str);
	
	return NULL;
}
