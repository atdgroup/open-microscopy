#ifndef __SINGLE_RANGE_HW_DEVICE__
#define __SINGLE_RANGE_HW_DEVICE__

#include "HardwareTypes.h"
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"   

#define SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define SINGLE_RANGE_HW_DEVICE_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(ob, member) if(SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(ob, member) == NULL) { \
    ui_module_send_error (UIMODULE_CAST(ob), "Error", "member not implemented"); \
	return HARDWARE_ERROR; \
}  

#define CALL_SINGLE_RANGE_HW_DEVICE_VTABLE_PTR(ob, member) if( SINGLE_RANGE_HW_DEVICE_VTABLE(ob, member)(ob) == HARDWARE_ERROR ) { \
	ui_module_send_error (UIMODULE_CAST(ob), "Error", "member failed"); \
	return HARDWARE_ERROR; \
}

#define SINGLE_RANGE_HW_DEVICE_CAST(obj) ((SingleRangeHardwareDevice *) (obj)) 

typedef struct
{
    int (*destroy) (SingleRangeHardwareDevice* single_range_hardware_device);
	int (*hardware_initialise) (SingleRangeHardwareDevice* single_range_hardware_device);
	int (*set_range_value) (SingleRangeHardwareDevice* single_range_hardware_device, double val);
	int (*get_range_value) (SingleRangeHardwareDevice* single_range_hardware_device, double *val);
	int (*set_range) (SingleRangeHardwareDevice *single_range_hardware_device, double min, double max, double increment);

} SingleRangeHardwareDeviceVtbl;


struct _SingleRangeHardwareDevice
{
  HardwareDevice parent; 

  SingleRangeHardwareDeviceVtbl vtable;
  
  int        _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int		 _initialised;
  double	 _min;
  double	 _max;
  double	 _increment;
};

SingleRangeHardwareDevice* single_range_hardware_device_contructor(const char *name, const char *description, size_t obj_size);

int  single_range_hardware_device_initialise (SingleRangeHardwareDevice* single_range_hardware_device);
int  single_range_hardware_device_hardware_initialise (SingleRangeHardwareDevice* single_range_hardware_device);
int  single_range_hardware_device_is_hardware_initialised (SingleRangeHardwareDevice* single_range_hardware_device);
int  single_range_hardware_device_destroy(SingleRangeHardwareDevice* single_range_hardware_device);
int  single_range_hardware_device_set(SingleRangeHardwareDevice* single_range_hardware_device, double val);
int  single_range_hardware_device_get(SingleRangeHardwareDevice* single_range_hardware_device, double *val);
int  single_range_hardware_device_set_range(SingleRangeHardwareDevice *single_range_hardware_device, double min, double max, double increment);
int  single_range_hardware_device_get_range(SingleRangeHardwareDevice *single_range_hardware_device, double *min, double *max);
int  single_range_hardware_device_set_readonly(SingleRangeHardwareDevice *single_range_hardware_device);
int  single_range_hardware_device_set_units(SingleRangeHardwareDevice *single_range_hardware_device, const char *units);

void single_range_hardware_device_on_change(SingleRangeHardwareDevice* single_range_hardware_device);
void single_range_hardware_device_disable_timer(SingleRangeHardwareDevice* single_range_hardware_device);
void single_range_hardware_device_enable_timer(SingleRangeHardwareDevice* single_range_hardware_device);

// Signals
typedef void (*SINGLE_RANGE_HW_DEVICE_EVENT_HANDLER) (SingleRangeHardwareDevice* single_range_hardware_device, void *data); 
typedef void (*SINGLE_RANGE_HW_DEVICE_CHANGE_EVENT_HANDLER) (SingleRangeHardwareDevice* single_range_hardware_device, double val, void *data); 

int single_range_hardware_device_signal_hide_handler_connect (SingleRangeHardwareDevice* single_range_hardware_device,
  SINGLE_RANGE_HW_DEVICE_EVENT_HANDLER handler, void *callback_data);

int single_range_hardware_device_changed_handler(SingleRangeHardwareDevice* single_range_hardware_device,
  SINGLE_RANGE_HW_DEVICE_CHANGE_EVENT_HANDLER handler, void *data );

int CVICALLBACK OnSingleRangeHardwareDevice (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnSingleRangeHardwareDeviceClose (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
