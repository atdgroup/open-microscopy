#ifndef __OPTICAL_CALIBRATION_DEVICE__
#define __OPTICAL_CALIBRATION_DEVICE__

#include "HardwareTypes.h" 
#include "HardwareDevice.h" 

#include "icsviewer_window.h"
#include "icsviewer_signals.h"

// Hardware parent

#define OPTICAL_CALIBRATION_DEVICE_CAST(obj) ((OpticalCalibrationDevice *) (obj))   

#define OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define OPTICAL_CALIBRATION_DEVICE_VTABLE(ob, member) (*((ob)->vtable.member))

#define OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE (UIMODULE_CAST(optical_calibration_device_get_default_device()))
#define OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE_NAME ((OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE)->_name)

#define OPTICAL_CALIBRATION_DEVICE_SUCCESS 0
#define OPTICAL_CALIBRATION_DEVICE_ERROR -1

#define CHECK_OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(ob, member) if(OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(ob, member) == NULL) { \
	hardware_device_send_error_text((HardwareDevice *)ob, "member not implemented"); \
    return OPTICAL_CALIBRATION_DEVICE_ERROR; \
}  

#define CALL_OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(ob, member) if(OPTICAL_CALIBRATION_DEVICE_VTABLE(ob, member)(ob) == OPTICAL_CALIBRATION_DEVICE_ERROR ) { \
	return OPTICAL_CALIBRATION_DEVICE_ERROR; \
}

#define OPTICAL_CALIBRATION_DEVICE_ITEM(item) (*((OpticalCalibrationDevice **) (ListGetPtrToItem (*hardware_device_list, item))))

typedef struct
{
	int (*setup_device_for_calibration) (OpticalCalibrationDevice* device, PROFILE_HANDLER handler, void *data);  

	// Crap name. This is call when another device is being setup so this device
	// can perform actions like removing it,s display / profile handler etc
	int (*deinitilise_device_for_calibration) (OpticalCalibrationDevice* device); 

	IcsViewerWindow* (*get_calibration_window) (OpticalCalibrationDevice* device);  

} OpticalCalibrationDeviceVtbl;


struct _OpticalCalibrationDevice
{ 
  HardwareDevice parent;    
  
  int _optical_calibration_device_list_id;     // the location of the device in the hardware device list

  OpticalCalibrationDeviceVtbl vtable;
};

int optical_calibration_device_constructor (OpticalCalibrationDevice* device, const char* name);
int optical_calibration_device_initialise (OpticalCalibrationDevice* device);
int optical_calibration_device_destroy (OpticalCalibrationDevice* device);

int optical_calibration_device_set_device_as_default (OpticalCalibrationDevice* device);
OpticalCalibrationDevice* optical_calibration_device_get_default_device (void);

int optical_calibration_device_setup_device_for_calibration (OpticalCalibrationDevice* device, PROFILE_HANDLER handler, void *data);
int optical_calibration_device_deinitilise_device_for_calibration (OpticalCalibrationDevice* device);
IcsViewerWindow* optical_calibration_device_get_calibration_window (OpticalCalibrationDevice* device);  

int optical_calibration_device_number_of_devices(void);
ListType* optical_calibration_device_get_device_list();
OpticalCalibrationDevice* optical_calibration_device_at_index(int index);

// Signal handlers
typedef void (*OPTICAL_CALIBRATION_DEVICE_PTR_HANDLER) (OpticalCalibrationDevice* device, void *data); 
typedef void (*OPTICAL_CALIBRATION_DEVICE_PTR_INT_HANDLER) (OpticalCalibrationDevice* device, void *data); 
typedef void (*OPTICAL_CALIBRATION_DEVICE_PTR_VOID_PTR_HANDLER) (OpticalCalibrationDevice* device, void *data); 

int optical_calibration_device_on_setup_handler(OpticalCalibrationDevice* device, OPTICAL_CALIBRATION_DEVICE_PTR_HANDLER handler, void *data);
int optical_calibration_device_set_as_default_handler(OpticalCalibrationDevice* device, OPTICAL_CALIBRATION_DEVICE_PTR_HANDLER handler, void *data);
int optical_calibration_device_pre_set_as_default_handler(OpticalCalibrationDevice* device, OPTICAL_CALIBRATION_DEVICE_PTR_HANDLER handler, void *data);

#endif
