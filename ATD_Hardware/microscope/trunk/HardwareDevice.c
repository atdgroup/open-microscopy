#include "HardwareDevice.h"

static ListType	hardware_device_list = NULL;

int HARDWARE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (HardwareDevice*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (HardwareDevice *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int HARDWARE_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (HardwareDevice*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (HardwareDevice *) args[0].void_ptr_data,  (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int HARDWARE_PTR_VOID_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (HardwareDevice*, void *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (HardwareDevice *) args[0].void_ptr_data, (void *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int hardware_device_send_error_text (HardwareDevice* device, char fmt[], ...)
{
	int ret=0;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(device), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(device), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(device), "Hardware Error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}

int hardware_device_hardware_constructor (HardwareDevice* device, const char* name)
{
	#ifdef INTERACTION_REPORT
	printf ("Creating hardware device %s.\n", name);
	#endif

	if(hardware_device_list == NULL)
		hardware_device_list = ListCreate (sizeof(HardwareDevice *)); 

	memset(device, 0, sizeof(HardwareDevice));

	ui_module_constructor(UIMODULE_CAST(device), name);    

	HARDWARE_VTABLE_PTR(device, hardware_initialise) = NULL; 
	HARDWARE_VTABLE_PTR(device, hardware_save_state_to_file) = NULL;
	HARDWARE_VTABLE_PTR(device, hardware_load_state_from_file) = NULL;
	HARDWARE_VTABLE_PTR(device, hardware_getinfo) = NULL; 
	HARDWARE_VTABLE_PTR(device, hardware_get_current_value_text) = NULL; 

	device->_manual = 0;
	device->_hardware_initialised = 0;
	device->number_of_consecutive_errors = 0;
	
	ListInsertItem (hardware_device_list, &device, END_OF_LIST); 

	device->_hardware_list_id = ListNumItems (hardware_device_list);

	return HARDWARE_SUCCESS;
}

int hardware_device_number_of_devices(void)
{
	return ListNumItems (hardware_device_list);
}

HardwareDevice* hardware_device_at_index(int index)
{
	HardwareDevice **device_pt_ptr = ListGetPtrToItem (hardware_device_list, index);

	return (HardwareDevice*) *device_pt_ptr;
}

int hardware_device_destroy (HardwareDevice* device)
{
	int number_of_devices;

	number_of_devices = ListNumItems (hardware_device_list);

	if(number_of_devices > 0) {
		ListRemoveItem (hardware_device_list, NULL, device->_hardware_list_id);
		number_of_devices--;
	}

	ui_module_destroy(UIMODULE_CAST(device));

	if(number_of_devices <= 0)
		ListDispose (hardware_device_list); 

	return HARDWARE_SUCCESS;
}

int hardware_device_set_as_manual(HardwareDevice* device)
{
	device->_manual = 1;
	
	return HARDWARE_SUCCESS;
}

int hardware_device_is_manual(HardwareDevice* device)
{
	return device->_manual;
}

ListType* hardware_device_get_device_list()
{
	return &hardware_device_list;
}

void hardware_device_print_all_devices()
{
	int i;
	HardwareDevice *device = NULL; 
	
	int number_of_devices = ListNumItems (hardware_device_list);

	for(i=1; i <= number_of_devices; i++) { 
	
		device = *((HardwareDevice **) (ListGetPtrToItem (hardware_device_list, i)));

		printf("Hardware device: %s, %s\n", UIMODULE_GET_NAME(device), UIMODULE_GET_DESCRIPTION(device));
	} 
}

int hardware_device_hardware_initialise (HardwareDevice* device)
{
	if(HARDWARE_VTABLE_PTR(device, hardware_initialise) == NULL)
		return HARDWARE_ERROR;
		
	if( HARDWARE_VTABLE(device, hardware_initialise)(device) == HARDWARE_ERROR ) {
		return HARDWARE_ERROR;
	}
		
	device->_hardware_initialised = 1;
	hardware_device_reset_consecutive_errors(device);

  	return HARDWARE_SUCCESS;
}

int hardware_device_hardware_is_initialised (HardwareDevice* device)
{
	return device->_hardware_initialised;
}

int hardware_device_getinfo (HardwareDevice* device, char* info)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_HARDWARE_VTABLE_PTR(device, hardware_getinfo) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( HARDWARE_VTABLE(device, hardware_getinfo)(device, info) == HARDWARE_ERROR ) {
			status = hardware_device_send_error_text(device, "hardware_getinfo failed");
		
			if(status == UIMODULE_ERROR_IGNORE) { 
				return HARDWARE_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
  	return HARDWARE_SUCCESS;
}

int hardware_device_get_current_value_text (HardwareDevice* device, char* info)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_HARDWARE_VTABLE_PTR(device, hardware_get_current_value_text) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( HARDWARE_VTABLE(device, hardware_get_current_value_text)(device, info) == HARDWARE_ERROR ) {
			status = hardware_device_send_error_text(device, "hardware_get_current_value_text failed");
		
			if(status == UIMODULE_ERROR_IGNORE) { 
				return HARDWARE_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
  	return HARDWARE_SUCCESS;
}

int hardware_device_get_number_of_consecutive_errors(HardwareDevice* device)
{
	return device->number_of_consecutive_errors;
}

void hardware_device_increment_consecutive_errors(HardwareDevice* device)
{
	device->number_of_consecutive_errors++;
}

void hardware_device_reset_consecutive_errors(HardwareDevice* device)
{
	device->number_of_consecutive_errors = 0;
}

int hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	CHECK_HARDWARE_VTABLE_PTR(device, hardware_save_state_to_file) 

	if( HARDWARE_VTABLE(device, hardware_save_state_to_file)(device, filepath, mode) == HARDWARE_ERROR ) {
		return HARDWARE_ERROR; 
	}
	
	return HARDWARE_SUCCESS;
}

int hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	CHECK_HARDWARE_VTABLE_PTR(device, hardware_load_state_from_file) 

	if( HARDWARE_VTABLE(device, hardware_load_state_from_file)(device, filepath) == HARDWARE_ERROR ) {
		return HARDWARE_ERROR; 
	}
	
	return HARDWARE_SUCCESS;
}