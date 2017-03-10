#include "90i_optical_zoom.h"
#include "single_range_hardware_device_ui.h"

#include <cviauto.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

#include "profile.h"

int Nikon90i_optical_zoom_destroy (SingleRangeHardwareDevice* single_range_hardware_device)
{
	OpticalZoom90i *optical_zoom_90i = (OpticalZoom90i *) single_range_hardware_device; 
	
	CA_DiscardObjHandle(optical_zoom_90i->hOpticalZoom);  
	
	free(optical_zoom_90i);
	
	return HARDWARE_SUCCESS;
}

static HRESULT OpticalZoomCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	OpticalZoom90i *optical_zoom_90i = (OpticalZoom90i *) caCallbackData;   
	
  	single_range_hardware_device_on_change(optical_zoom_90i);

	return 0;
}

static int Nikon90i_set_optical_zoom (SingleRangeHardwareDevice* single_range_hardware_device, double val)
{
	ERRORINFO errInfo;
	int err;
    
	OpticalZoom90i *optical_zoom_90i = (OpticalZoom90i *) single_range_hardware_device;    
	
	//Set field stop, (val = 1.0 to 12.0).

	if (optical_zoom_90i == NULL)
	  return HARDWARE_ERROR;
	
	if (optical_zoom_90i->_mounted != 1)
	  return HARDWARE_ERROR;

	err = ISCOPELib_IOpticalZoomSet_Zoom (optical_zoom_90i->hOpticalZoom, &errInfo, val);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(optical_zoom_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(optical_zoom_90i), errInfo.description); 

		return HARDWARE_ERROR;
	}
	
	return HARDWARE_SUCCESS;
}

static int Nikon90i_get_optical_zoom (SingleRangeHardwareDevice* single_range_hardware_device, double *val)
{
	ERRORINFO errInfo;
	int err;
    
	OpticalZoom90i *optical_zoom_90i = (OpticalZoom90i *) single_range_hardware_device;    
	
	//Get field stop, (val = 1.0 to 12.0).

	if (optical_zoom_90i == NULL)
	  return HARDWARE_ERROR;
	
	if (optical_zoom_90i->_mounted != 1)
	  return HARDWARE_ERROR;

	err = ISCOPELib_IOpticalZoomGet_Zoom (optical_zoom_90i->hOpticalZoom, &errInfo, val);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(optical_zoom_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(optical_zoom_90i), errInfo.description); 

		return HARDWARE_ERROR;
	}
	
	return HARDWARE_SUCCESS;
}

static int Nikon90i_set_field_range(SingleRangeHardwareDevice* single_range_hardware_device, double min, double max, double increment)
{

	return HARDWARE_SUCCESS;
}

static int Nikon90i_optical_zoom_initialise (SingleRangeHardwareDevice* single_range_hardware_device)
{
	return single_range_hardware_device_set_range(single_range_hardware_device, 0.8, 2.0, 0.1);
}

SingleRangeHardwareDevice* Nikon90i_optical_zoom_new(CAObjHandle hNikon90i,  const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType;

    SingleRangeHardwareDevice* single_range_hardware_device = single_range_hardware_device_contructor(name, description, sizeof(OpticalZoom90i));
	OpticalZoom90i *optical_zoom_90i =  (OpticalZoom90i *) single_range_hardware_device;

	ui_module_set_data_dir( UIMODULE_CAST(optical_zoom_90i), data_dir);
    ui_module_set_error_handler(UIMODULE_CAST(optical_zoom_90i), error_handler, data); 
	
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, destroy) = Nikon90i_optical_zoom_destroy; 
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range_value) = Nikon90i_set_optical_zoom;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, get_range_value) = Nikon90i_get_optical_zoom;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range) = Nikon90i_set_field_range;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, hardware_initialise) = Nikon90i_optical_zoom_initialise;

	if(err = ISCOPELib_INikon90iGetOpticalZoom (hNikon90i, &errInfo, &(optical_zoom_90i->hOpticalZoom)))
		goto Error;
		
	if(err = ISCOPELib_IOpticalZoomGet_IsMounted (optical_zoom_90i->hOpticalZoom, &errInfo, &mounted))
		goto Error;
	
	optical_zoom_90i->_mounted = mounted;

	single_range_hardware_device_set_range(single_range_hardware_device, 0.8, 2.0, 0.1);   //found impirically
	
	#ifndef ENABLE_OPTICAL_ZOOM_POLLING
	
	if(err = ISCOPELib_IOpticalZoomGetZoom (optical_zoom_90i->hOpticalZoom, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, OpticalZoomCallback, optical_zoom_90i, 1, NULL))
		goto Error;
	
	#endif
	
	single_range_hardware_device_on_change(optical_zoom_90i);   //display initial value
		
	return optical_zoom_90i;
	
	Error:
	
	mounted = 0; 
	
	CA_GetAutomationErrorString (err, error_str, 200);  
	ui_module_send_error (UIMODULE_CAST(optical_zoom_90i), "Epi Field stop error", "%s", error_str);
	
	return NULL;
}
