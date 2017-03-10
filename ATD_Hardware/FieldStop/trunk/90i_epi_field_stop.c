#include "90i_epi_field_stop.h"
#include "single_range_hardware_device_ui.h"

#include <cviauto.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

int Nikon90i_epi_single_range_hardware_device_destroy (SingleRangeHardwareDevice* single_range_hardware_device)
{
	EpiFieldStop90i *epi_field_stop =  (EpiFieldStop90i *) single_range_hardware_device;
	
	CA_DiscardObjHandle(epi_field_stop->hEpiFieldStop);
	
	free(epi_field_stop);
	
	return HARDWARE_SUCCESS;
}


static HRESULT SingleRangeHardwareDeviceCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	EpiFieldStop90i *epi_fieldStop =  (EpiFieldStop90i *) caCallbackData;   
	
	single_range_hardware_device_on_change(epi_fieldStop);

	return 0;
}

static int Nikon90i_set_epi_single_range_hardware_device (SingleRangeHardwareDevice* single_range_hardware_device, double val)
{
	ERRORINFO errInfo;
	int err;
    
	EpiFieldStop90i *epi_field_stop =  (EpiFieldStop90i *) single_range_hardware_device; 
	
	//Set field stop, (val = 1.0 to 12.0).

	if (single_range_hardware_device == NULL)
		return HARDWARE_ERROR;
	
	if (epi_field_stop->_mounted != 1)
		return HARDWARE_ERROR;

	err = ISCOPELib_IEpiFieldStopSet_FieldStop (epi_field_stop->hEpiFieldStop, &errInfo, val);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(epi_field_stop), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(epi_field_stop), errInfo.description);  

		return HARDWARE_ERROR;
	}
	
	return HARDWARE_SUCCESS;
}


static int Nikon90i_get_epi_single_range_hardware_device (SingleRangeHardwareDevice* single_range_hardware_device, double *val)
{
	ERRORINFO errInfo;
	int err;
    
	EpiFieldStop90i *epi_field_stop =  (EpiFieldStop90i *) single_range_hardware_device; 
	
	//Get field stop, (val = 1.0 to 12.0).

	if (epi_field_stop == NULL)
		return HARDWARE_ERROR;

	if (epi_field_stop->_mounted != 1)
		return HARDWARE_ERROR;

	err = ISCOPELib_IEpiFieldStopGet_FieldStop (epi_field_stop->hEpiFieldStop, &errInfo, val);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(epi_field_stop), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(epi_field_stop), errInfo.description);  

		return HARDWARE_ERROR;
	}
	
	return HARDWARE_SUCCESS;
}

static int Nikon90i_set_epi_field_range(SingleRangeHardwareDevice* single_range_hardware_device, double min, double max, double increment)
{
	return HARDWARE_SUCCESS;
}

static int Nikon90i_epi_field_stop_initialise (SingleRangeHardwareDevice* single_range_hardware_device)
{
	single_range_hardware_device_set_range(single_range_hardware_device, 2.0, 8.9, 0.1);

	return HARDWARE_SUCCESS;
}

SingleRangeHardwareDevice* Nikon90i_epi_field_stop_new(CAObjHandle hNikon90i,  const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType; 
	
	SingleRangeHardwareDevice* single_range_hardware_device = single_range_hardware_device_contructor(name, description, sizeof(EpiFieldStop90i));
	EpiFieldStop90i *epi_field_stop =  (EpiFieldStop90i *) single_range_hardware_device;

	ui_module_set_data_dir( UIMODULE_CAST(epi_field_stop), data_dir);
    ui_module_set_error_handler(UIMODULE_CAST(epi_field_stop), error_handler, data); 
	
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, destroy) = Nikon90i_epi_single_range_hardware_device_destroy; 
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range_value) = Nikon90i_set_epi_single_range_hardware_device;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, get_range_value) = Nikon90i_get_epi_single_range_hardware_device;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range) = Nikon90i_set_epi_field_range;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, hardware_initialise) = Nikon90i_epi_field_stop_initialise;

	if(err = ISCOPELib_INikon90iGetEpiFieldStop (hNikon90i, &errInfo, &(epi_field_stop->hEpiFieldStop)))
		goto Error;

	if(err = ISCOPELib_IEpiFieldStopGet_IsMounted (epi_field_stop->hEpiFieldStop, &errInfo, &mounted))
		goto Error;
	
	epi_field_stop->_mounted = mounted;

	single_range_hardware_device_set_range(single_range_hardware_device, 2.0, 8.9, 1.0);   //found impirically
	single_range_hardware_device_set(single_range_hardware_device, 6.0);   //found impirically  
	
	// Check that Polling is disabled
	#ifndef ENABLE_EPI_FIELD_STOP_STATUS_POLLING

	if(err = ISCOPELib_IEpiFieldStopGetFieldStop (epi_field_stop->hEpiFieldStop, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, SingleRangeHardwareDeviceCallback, epi_field_stop, 1, NULL))
		goto Error;
	
   	single_range_hardware_device_on_change(epi_field_stop);   //display initial value

	#endif
	
	return epi_field_stop;
	
	Error:
	
	mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200);
	ui_module_send_error (UIMODULE_CAST(epi_field_stop), "Epi Field stop error", "%s", error_str);
	
	return NULL;
}
