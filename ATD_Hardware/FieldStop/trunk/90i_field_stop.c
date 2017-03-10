#include "90i_field_stop.h"
#include "single_range_hardware_device_ui.h"

#include <cviauto.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

int Nikon90i_single_range_hardware_device_destroy (SingleRangeHardwareDevice* single_range_hardware_device)
{
	FieldStop90i *fieldStop90i =  (FieldStop90i *) single_range_hardware_device;  
	
	CA_DiscardObjHandle(fieldStop90i->hFieldStop);   
	
	free(fieldStop90i);
	
	return HARDWARE_SUCCESS;
}


static int Nikon90i_set_single_range_hardware_device (SingleRangeHardwareDevice* single_range_hardware_device, double val)
{
	ERRORINFO errInfo;
	int err;
    
	FieldStop90i *fieldStop90i =  (FieldStop90i *) single_range_hardware_device;
	
	//Set field stop, (val = 1.0 to 12.0).

	if (single_range_hardware_device == NULL)
	  return HARDWARE_ERROR;
	  
	if (fieldStop90i->_mounted != 1)
	  return HARDWARE_ERROR;

	err = ISCOPELib_IFieldStopSet_FieldStop (fieldStop90i->hFieldStop, &errInfo, val);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(fieldStop90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(fieldStop90i), errInfo.description);  

		return HARDWARE_ERROR;
	}
	
	return HARDWARE_SUCCESS;
}

static int Nikon90i_get_single_range_hardware_device (SingleRangeHardwareDevice* single_range_hardware_device, double *val)
{
	ERRORINFO errInfo;
	int err;
    
	FieldStop90i *fieldStop90i =  (FieldStop90i *) single_range_hardware_device; 
	
	// Get field stop, (val = 1.0 to 12.0).
	if (single_range_hardware_device == NULL)
	  return HARDWARE_ERROR;
	  
	if (fieldStop90i->_mounted != 1)
	  return HARDWARE_ERROR;

	err = ISCOPELib_IFieldStopGet_FieldStop (fieldStop90i->hFieldStop, &errInfo, val);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(fieldStop90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(fieldStop90i), errInfo.description);  

		return HARDWARE_ERROR;
	}
	
	return HARDWARE_SUCCESS;
}

static int Nikon90i_set_field_range(SingleRangeHardwareDevice* single_range_hardware_device, double min, double max, double increment)
{
	return HARDWARE_SUCCESS;
}


static HRESULT SingleRangeHardwareDeviceCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	SingleRangeHardwareDevice* single_range_hardware_device =  (SingleRangeHardwareDevice*) caCallbackData;   
	
	single_range_hardware_device_on_change(single_range_hardware_device);
	
	return 0;
}

static int Nikon90i_field_stop_initialise (SingleRangeHardwareDevice* single_range_hardware_device)
{
	single_range_hardware_device_set_range(single_range_hardware_device, 0.8, 30.6, 0.1);

	return HARDWARE_SUCCESS;
}

SingleRangeHardwareDevice* Nikon90i_field_stop_new(CAObjHandle hNikon90i,  const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType; 
	
	SingleRangeHardwareDevice* single_range_hardware_device = single_range_hardware_device_contructor(name, description, sizeof(FieldStop90i));
	FieldStop90i *fieldStop90i =  (FieldStop90i *) single_range_hardware_device;

	ui_module_set_data_dir( UIMODULE_CAST(single_range_hardware_device), data_dir);
    ui_module_set_error_handler(UIMODULE_CAST(single_range_hardware_device), error_handler, data); 
	
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, destroy) = Nikon90i_single_range_hardware_device_destroy; 
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range_value) = Nikon90i_set_single_range_hardware_device;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, get_range_value) = Nikon90i_get_single_range_hardware_device;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range) = Nikon90i_set_field_range;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, hardware_initialise) = Nikon90i_field_stop_initialise;

	if(err = ISCOPELib_INikon90iGetFieldStop (hNikon90i, &errInfo, &(fieldStop90i->hFieldStop)))
		goto Error;
		
	if(err = err = ISCOPELib_IFieldStopGet_IsMounted (fieldStop90i->hFieldStop, &errInfo, &mounted))
		goto Error;
	
	fieldStop90i->_mounted = mounted;

	single_range_hardware_device_set_range(single_range_hardware_device, 0.8, 30.6, 1.0);   //found impirically

	// Check that Polling is disabled
	#ifndef ENABLE_FIELD_STOP_STATUS_POLLING
	
	if(err = ISCOPELib_IFieldStopGetFieldStop (fieldStop90i->hFieldStop, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, SingleRangeHardwareDeviceCallback, single_range_hardware_device, 1, NULL))
		goto Error;
	
	#endif

	return single_range_hardware_device;
	
	Error:
	
	fieldStop90i->_mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200);
	ui_module_send_error (UIMODULE_CAST(single_range_hardware_device), "Field stop error", "%s", error_str);
	
	return NULL;
}














	//Sub AdviseAsync (ByVal hWnd As Long, ByVal uMsg As Long, ByVal wParam As Long, pLink As IMipParamEventSink)





	
	/* Store the window structure with the window for use in WndProc */
//	SetWindowLongPtr (window->panel_window_handle, GWLP_USERDATA, (LONG_PTR)window);
	
    /* Set the new Wnd Proc to be called */	
//	SetWindowLongPtr (window->panel_window_handle, GWL_WNDPROC, (LONG_PTR)GCI_WndProc);
