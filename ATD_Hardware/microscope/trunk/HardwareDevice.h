#ifndef __HARDWARE_DEVICE__
#define __HARDWARE_DEVICE__

#include "HardwareTypes.h" 
#include "gci_ui_module.h"  

// Hardware parent

#define HARDWARE_DEVICE_CAST(obj) ((HardwareDevice *) (obj))   

#define HARDWARE_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define HARDWARE_VTABLE(ob, member) (*((ob)->vtable.member))

#define HARDWARE_SUCCESS 0
#define HARDWARE_ERROR -1

#define CHECK_HARDWARE_VTABLE_PTR(ob, member) if(HARDWARE_VTABLE_PTR(ob, member) == NULL) { \
    hardware_device_send_error_text(ob, "member not implemented"); \
    return HARDWARE_ERROR; \
}  

#define CALL_HARDWARE_VTABLE_PTR(ob, member) if(HARDWARE_VTABLE(ob, member)(ob) == HARDWARE_ERROR ) { \
	return HARDWARE_ERROR; \
}

#define HARDWARE_DEVICE_ITEM(item) (*((HardwareDevice **) (ListGetPtrToItem (*hardware_device_list, item))))

typedef struct
{
	int (*hardware_initialise) (HardwareDevice* device);  
	int (*hardware_save_state_to_file) (HardwareDevice* device, const char* filepath, const char *mode);
	int (*hardware_load_state_from_file) (HardwareDevice* device, const char* filepath);
	int (*hardware_getinfo) (HardwareDevice* device, char* info); 
	int (*hardware_get_current_value_text) (HardwareDevice* device, char* info);

} HardwareDeviceVtbl;


struct _HardwareDevice
{ 
  UIModule parent;    
  
  int _hardware_list_id;     // the location of the device in the hardware device list
  int _hardware_initialised; // has the device been initialised?

  BYTE _manual;              // indicates whether a device is under manual control, otherwise controlled by software

  int number_of_consecutive_errors;

  HardwareDeviceVtbl vtable;
};

int hardware_device_hardware_constructor (HardwareDevice* device, const char* name);
int hardware_device_hardware_initialise (HardwareDevice* device);
int hardware_device_hardware_is_initialised (HardwareDevice* device);
int hardware_device_destroy (HardwareDevice* device);

int hardware_device_number_of_devices(void);
int hardware_device_send_error_text (HardwareDevice* device, char fmt[], ...);
int hardware_load_state_from_file (HardwareDevice* device, const char* filepath);
int hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode);
int hardware_device_getinfo (HardwareDevice* device, char* info);
int hardware_device_set_as_manual(HardwareDevice* device);
int hardware_device_is_manual(HardwareDevice* device);
int hardware_device_get_current_value_text(HardwareDevice* device, char* info);
int hardware_device_get_number_of_consecutive_errors(HardwareDevice* device);
void hardware_device_increment_consecutive_errors(HardwareDevice* device);
void hardware_device_reset_consecutive_errors(HardwareDevice* device);

ListType* hardware_device_get_device_list();
HardwareDevice* hardware_device_at_index(int index);

void hardware_device_print_all_devices();

// Signal Marshallers
int HARDWARE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);
int HARDWARE_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);
int HARDWARE_PTR_VOID_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

#endif
