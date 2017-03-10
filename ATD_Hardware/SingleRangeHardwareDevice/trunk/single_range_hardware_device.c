#include "single_range_hardware_device.h"
#include "single_range_hardware_device_ui.h"

#include "gci_utils.h"
#include "GL_CVIRegistry.h"
#include "toolbox.h"
#include "dictionary.h"

#include "asynctmr.h"

#include <utility.h>
#include <ansi_c.h> 

static void OnSingleRangeHardwareDeviceChanged(SingleRangeHardwareDevice* single_range_hardware_device);

static int SINGLE_RANGE_HW_DEVICE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (SingleRangeHardwareDevice*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (SingleRangeHardwareDevice *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int SINGLE_RANGE_HW_DEVICE_PTR_DOUBLE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (SingleRangeHardwareDevice*, double, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (SingleRangeHardwareDevice *) args[0].void_ptr_data, (double) args[1].double_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int CVICALLBACK OnSingleRangeHardwareDeviceTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	SingleRangeHardwareDevice *single_range_hardware_device = (SingleRangeHardwareDevice *) callbackData;

    switch (event)
    {
        case EVENT_TIMER_TICK:
        
			single_range_hardware_device_on_change(single_range_hardware_device);
            
            break;
    }
    
    return 0;
}

int single_range_hardware_device_hardware_initialise (SingleRangeHardwareDevice* single_range_hardware_device)
{
	CHECK_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, hardware_initialise)  	
	CALL_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, hardware_initialise) 

	single_range_hardware_device->_initialised = 1;

	return HARDWARE_SUCCESS;
}

int single_range_hardware_device_initialise (SingleRangeHardwareDevice* single_range_hardware_device)
{
	single_range_hardware_device->_timer = -1;
	single_range_hardware_device->_min = 0.0;
  	single_range_hardware_device->_max = 0.0;
  	single_range_hardware_device->_increment = 0.0;
	
    single_range_hardware_device->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(single_range_hardware_device), "single_range_hardware_device_ui.uir", SR_HD_PNL, 1);
 
  	if ( InstallCtrlCallback (single_range_hardware_device->_main_ui_panel, SR_HD_PNL_RANGE_VAL, OnSingleRangeHardwareDevice, single_range_hardware_device) < 0)
		return HARDWARE_ERROR;
		
	if ( InstallCtrlCallback (single_range_hardware_device->_main_ui_panel, SR_HD_PNL_CLOSE, OnSingleRangeHardwareDeviceClose, single_range_hardware_device) < 0)
		return HARDWARE_ERROR;	
		
	SetPanelAttribute (single_range_hardware_device->_main_ui_panel, ATTR_TITLE, UIMODULE_GET_DESCRIPTION(single_range_hardware_device));
	
	#ifdef ENABLE_SR_HD_PNL_RANGE_VAL_STATUS_POLLING
	single_range_hardware_device->_timer = NewAsyncTimer (2.0, -1, 1, OnSingleRangeHardwareDeviceTimerTick, single_range_hardware_device);
	SetAsyncTimerAttribute (single_range_hardware_device->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif

	single_range_hardware_device_on_change(single_range_hardware_device);

  	return HARDWARE_SUCCESS;
}

int single_range_hardware_device_is_hardware_initialised (SingleRangeHardwareDevice* single_range_hardware_device)
{
	return single_range_hardware_device->_initialised;
}

int single_range_hardware_device_signal_hide_handler_connect (SingleRangeHardwareDevice* single_range_hardware_device, SINGLE_RANGE_HW_DEVICE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(single_range_hardware_device), "Hide", handler, callback_data) == SIGNAL_ERROR) {

		ui_module_send_error (UIMODULE_CAST(single_range_hardware_device), UIMODULE_GET_DESCRIPTION(single_range_hardware_device), "Can not connect signal handler for Hide signal"); 
		
		return HARDWARE_ERROR;
	}

	return HARDWARE_SUCCESS;
}


int single_range_hardware_device_changed_handler(SingleRangeHardwareDevice* single_range_hardware_device, SINGLE_RANGE_HW_DEVICE_CHANGE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(single_range_hardware_device), "SingleRangeHardwareDeviceChanged", handler, data) == SIGNAL_ERROR) {

		ui_module_send_error (UIMODULE_CAST(single_range_hardware_device), UIMODULE_GET_DESCRIPTION(single_range_hardware_device), "Can not connect signal handler for Change signal"); 
		
		return HARDWARE_ERROR;
	}

	return HARDWARE_SUCCESS;
}

void single_range_hardware_device_on_change(SingleRangeHardwareDevice* single_range_hardware_device)
{
	double val, previous_val;  
	
	CmtGetLock (single_range_hardware_device->_lock);
       		
	if (single_range_hardware_device_get(single_range_hardware_device, &val) == HARDWARE_SUCCESS) {
	
		GetCtrlVal (single_range_hardware_device->_main_ui_panel, SR_HD_PNL_RANGE_VAL, &previous_val);
	
		if (FP_Compare (val, previous_val) != 0) {  // It's changed
			SetCtrlVal (single_range_hardware_device->_main_ui_panel, SR_HD_PNL_RANGE_VAL, val);
			
			//Pass on the event to higher modules		
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(single_range_hardware_device), "SingleRangeHardwareDeviceChanged",
				GCI_VOID_POINTER, single_range_hardware_device, GCI_DOUBLE, val);
		}
	}
			
	CmtReleaseLock(single_range_hardware_device->_lock);
}

static int single_range_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	double val;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	if (single_range_hardware_device_get(SINGLE_RANGE_HW_DEVICE_CAST(device), &val) == HARDWARE_SUCCESS) {
	
		fp = fopen(filepath, mode);
	
		if(fp == NULL) {
			return HARDWARE_ERROR;
		}
		
		dictionary_set(d, UIMODULE_GET_NAME(device), NULL);
		dictionary_setint(d, "val", val);      

		iniparser_save(d, fp); 
	
		fclose(fp);
	}
		
	dictionary_del(d);

	return HARDWARE_SUCCESS;
}

static int single_range_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	dictionary* d = NULL;
	double val;
	int file_size;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return HARDWARE_ERROR;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {

		val = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(device), "val"), -999999.0); 

		single_range_hardware_device_set(SINGLE_RANGE_HW_DEVICE_CAST(device), val);
	}

    dictionary_del(d);

	return HARDWARE_SUCCESS;
}

SingleRangeHardwareDevice* single_range_hardware_device_contructor(const char *name, const char *description, size_t obj_size)
{
	SingleRangeHardwareDevice* single_range_hardware_device = (SingleRangeHardwareDevice*) malloc(obj_size);
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(single_range_hardware_device), name); 
	ui_module_set_description(UIMODULE_CAST(single_range_hardware_device), description);       
	
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range_value) = NULL;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, get_range_value) = NULL;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range) = NULL;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, destroy) = NULL;
	SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, hardware_initialise) = NULL;

	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(single_range_hardware_device), hardware_save_state_to_file) = single_range_hardware_save_state_to_file;
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(single_range_hardware_device), hardware_load_state_from_file) = single_range_hardware_load_state_from_file;
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(single_range_hardware_device), "Hide", SINGLE_RANGE_HW_DEVICE_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(single_range_hardware_device), "SingleRangeHardwareDeviceChanged", SINGLE_RANGE_HW_DEVICE_PTR_DOUBLE_MARSHALLER);

	CmtNewLock (NULL, 0, &(single_range_hardware_device->_lock) );  
	
	return single_range_hardware_device;
}

int single_range_hardware_device_destroy(SingleRangeHardwareDevice* single_range_hardware_device)
{
  	CmtDiscardLock (single_range_hardware_device->_lock);
  		
    single_range_hardware_device_disable_timer(single_range_hardware_device);
	
	CHECK_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, destroy) 
  	
	CALL_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, destroy) 

	ui_module_destroy(UIMODULE_CAST(single_range_hardware_device));

	single_range_hardware_device->_main_ui_panel = -1;
  	
  	CmtDiscardLock (single_range_hardware_device->_lock);

  	return HARDWARE_SUCCESS;
}


int single_range_hardware_device_set_range(SingleRangeHardwareDevice *single_range_hardware_device, double min, double max, double increment)
{
	CHECK_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range)

	if(SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range)(single_range_hardware_device, min, max, increment) == HARDWARE_ERROR ) {
		ui_module_send_error (UIMODULE_CAST(single_range_hardware_device), "Hardware error", "single_range_hardware_device_set_range failed");
		return HARDWARE_ERROR;
	}

	single_range_hardware_device->_min = min;
  	single_range_hardware_device->_max = max;
  	single_range_hardware_device->_increment = increment;
	
	SetCtrlAttribute(single_range_hardware_device->_main_ui_panel, SR_HD_PNL_RANGE_VAL, ATTR_MIN_VALUE, min);
	SetCtrlAttribute(single_range_hardware_device->_main_ui_panel, SR_HD_PNL_RANGE_VAL, ATTR_MAX_VALUE, max);
	SetCtrlAttribute(single_range_hardware_device->_main_ui_panel, SR_HD_PNL_RANGE_VAL, ATTR_INCR_VALUE, increment);
	
	SetCtrlAttribute(single_range_hardware_device->_main_ui_panel, SR_HD_PNL_RANGE_VAL, ATTR_DIMMED, 0); 
	
	return HARDWARE_SUCCESS;
}

int single_range_hardware_device_get_range(SingleRangeHardwareDevice *single_range_hardware_device, double *min, double *max)
{
	if (FP_Compare(single_range_hardware_device->_max, 0.0) == 0) return HARDWARE_ERROR;
	
	*min = single_range_hardware_device->_min;
	*max = single_range_hardware_device->_max;
	return HARDWARE_SUCCESS;
}

int single_range_hardware_device_set(SingleRangeHardwareDevice* single_range_hardware_device, double val)
{
  	CHECK_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range_value) 

	if(SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, set_range_value)(single_range_hardware_device, val) == HARDWARE_ERROR ) {
		ui_module_send_error (UIMODULE_CAST(single_range_hardware_device), "Hardware error", "single_range_hardware_device_set failed");
		return HARDWARE_ERROR;
	}

	single_range_hardware_device_on_change(single_range_hardware_device);

  	return HARDWARE_SUCCESS;
}

int single_range_hardware_device_get(SingleRangeHardwareDevice* single_range_hardware_device, double *val)
{
  	CHECK_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, get_range_value) 

	if(SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(single_range_hardware_device, get_range_value)(single_range_hardware_device, val) == HARDWARE_ERROR ) {
		ui_module_send_error (UIMODULE_CAST(single_range_hardware_device), "Hardware error", "single_range_hardware_device_get failed");
		return HARDWARE_ERROR;
	}

  	return HARDWARE_SUCCESS;
}

int single_range_hardware_device_set_readonly(SingleRangeHardwareDevice *single_range_hardware_device)
{
    SetCtrlAttribute(single_range_hardware_device->_main_ui_panel, SR_HD_PNL_RANGE_VAL, ATTR_CTRL_MODE, VAL_INDICATOR); 
}

int single_range_hardware_device_set_units(SingleRangeHardwareDevice *single_range_hardware_device, const char *units)
{
    SetCtrlVal(single_range_hardware_device->_main_ui_panel, SR_HD_PNL_UNITS, units); 
}

void single_range_hardware_device_disable_timer(SingleRangeHardwareDevice* single_range_hardware_device)
{
	#ifdef ENABLE_SR_HD_PNL_RANGE_VAL_STATUS_POLLING
	SetAsyncTimerAttribute (single_range_hardware_device->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void single_range_hardware_device_enable_timer(SingleRangeHardwareDevice* single_range_hardware_device)
{
	#ifdef ENABLE_SR_HD_PNL_RANGE_VAL_STATUS_POLLING        
	SetAsyncTimerAttribute (single_range_hardware_device->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif  
}