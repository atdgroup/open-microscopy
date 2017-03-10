#include "OpticalCalibrationDevice.h"

static ListType optical_calibration_device_list = NULL;
static OpticalCalibrationDevice* default_device = NULL;

static int OPTICAL_CALIBRATION_DEVICE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (OpticalCalibrationDevice*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (OpticalCalibrationDevice *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int optical_calibration_device_constructor (OpticalCalibrationDevice* device, const char* name)
{
	if(optical_calibration_device_list == NULL)
		optical_calibration_device_list = ListCreate (sizeof(OpticalCalibrationDevice *)); 

	memset(device, 0, sizeof(OpticalCalibrationDevice));

	hardware_device_hardware_constructor (HARDWARE_DEVICE_CAST(device), name);    

	OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(device, setup_device_for_calibration) = NULL; 
		
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDevicePreSetup", OPTICAL_CALIBRATION_DEVICE_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDevicePreSetAsDefault", OPTICAL_CALIBRATION_DEVICE_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDeviceSetAsDefault", OPTICAL_CALIBRATION_DEVICE_PTR_MARSHALLER);

	ListInsertItem (optical_calibration_device_list, &device, END_OF_LIST); 

	device->_optical_calibration_device_list_id = ListNumItems (optical_calibration_device_list);
	
	default_device = device;

	return OPTICAL_CALIBRATION_DEVICE_SUCCESS;
}

int optical_calibration_device_set_device_as_default (OpticalCalibrationDevice* device)
{
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDevicePreSetAsDefault", GCI_VOID_POINTER, device);

	default_device = device;

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDeviceSetAsDefault", GCI_VOID_POINTER, device);

	return OPTICAL_CALIBRATION_DEVICE_SUCCESS;
}

OpticalCalibrationDevice* optical_calibration_device_get_default_device (void)
{
	return default_device;
}

int optical_calibration_device_on_setup_handler(OpticalCalibrationDevice* device, OPTICAL_CALIBRATION_DEVICE_PTR_HANDLER handler, void *data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDevicePreSetup", handler, data) == SIGNAL_ERROR) {
		hardware_device_send_error_text(HARDWARE_DEVICE_CAST(device), "Can not connect signal handler for OpticalCalibrationDevicePreSetup signal");
		return OPTICAL_CALIBRATION_DEVICE_ERROR;
	}

	return OPTICAL_CALIBRATION_DEVICE_SUCCESS;
}

int optical_calibration_device_pre_set_as_default_handler(OpticalCalibrationDevice* device, OPTICAL_CALIBRATION_DEVICE_PTR_HANDLER handler, void *data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDevicePreSetAsDefault", handler, data) == SIGNAL_ERROR) {
		hardware_device_send_error_text(HARDWARE_DEVICE_CAST(device), "Can not connect signal handler for OpticalCalibrationDevicePreSetAsDefault signal");
		return OPTICAL_CALIBRATION_DEVICE_ERROR;
	}

	return OPTICAL_CALIBRATION_DEVICE_SUCCESS;
}

int optical_calibration_device_set_as_default_handler(OpticalCalibrationDevice* device, OPTICAL_CALIBRATION_DEVICE_PTR_HANDLER handler, void *data)
{
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDeviceSetAsDefault", handler, data) == SIGNAL_ERROR) {
		hardware_device_send_error_text(HARDWARE_DEVICE_CAST(device), "Can not connect signal handler for OpticalCalibrationDeviceSetAsDefault signal");
		return OPTICAL_CALIBRATION_DEVICE_ERROR;
	}

	return OPTICAL_CALIBRATION_DEVICE_SUCCESS;
}

int optical_calibration_device_number_of_devices(void)
{
	return ListNumItems (optical_calibration_device_list);
}

OpticalCalibrationDevice* optical_calibration_device_at_index(int index)
{
	OpticalCalibrationDevice **device_pt_ptr = ListGetPtrToItem (optical_calibration_device_list, index);

	return (OpticalCalibrationDevice*) *device_pt_ptr;
}

int optical_calibration_device_destroy (OpticalCalibrationDevice* device)
{
	int number_of_devices;

	number_of_devices = ListNumItems (optical_calibration_device_list);

	if(number_of_devices > 0) {
		ListRemoveItem (optical_calibration_device_list, NULL, device->_optical_calibration_device_list_id);
		number_of_devices--;
	}

	hardware_device_destroy(HARDWARE_DEVICE_CAST(device));

	if(number_of_devices <= 0)
		ListDispose (optical_calibration_device_list); 
	
	return OPTICAL_CALIBRATION_DEVICE_SUCCESS;
}

ListType* optical_calibration_device_get_device_list()
{
	return &optical_calibration_device_list;
}

int optical_calibration_device_setup_device_for_calibration (OpticalCalibrationDevice* device, PROFILE_HANDLER handler, void *data)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(device, setup_device_for_calibration) 
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(device), "OpticalCalibrationDevicePreSetup", GCI_VOID_POINTER, data);

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( OPTICAL_CALIBRATION_DEVICE_VTABLE(device, setup_device_for_calibration)(device, handler, data) == OPTICAL_CALIBRATION_DEVICE_ERROR ) {
			status = hardware_device_send_error_text(HARDWARE_DEVICE_CAST(device), "setup_device_for_calibration failed");
		
			if(status == UIMODULE_ERROR_IGNORE) { 
				return OPTICAL_CALIBRATION_DEVICE_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
  	return OPTICAL_CALIBRATION_DEVICE_SUCCESS;
}

int optical_calibration_device_deinitilise_device_for_calibration (OpticalCalibrationDevice* device)
{
	CHECK_OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(device, deinitilise_device_for_calibration) 

	return OPTICAL_CALIBRATION_DEVICE_VTABLE(device, deinitilise_device_for_calibration)(device);
}

IcsViewerWindow* optical_calibration_device_get_calibration_window (OpticalCalibrationDevice* device)
{
	// Cannot use the check macro as returns an int.
	//CHECK_OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(device, get_calibration_window) 

	if(OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(device, get_calibration_window) == NULL) { 
		hardware_device_send_error_text((HardwareDevice *)device, "get_calibration_window not implemented"); 
		return NULL;
	}

	return OPTICAL_CALIBRATION_DEVICE_VTABLE(device, get_calibration_window)(device);
}