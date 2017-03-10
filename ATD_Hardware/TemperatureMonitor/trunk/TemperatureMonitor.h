#ifndef __TEMPERATURE_MONITOR__
#define __TEMPERATURE_MONITOR__

#include "HardWareTypes.h"      
#include "HardWareDevice.h" 
#include "signals.h"
#include "gci_ui_module.h"   
#include "gci_utils.h" 

#define TEMPERATURE_MONITOR_SUCCESS 0
#define TEMPERATURE_MONITOR_ERROR -1

#define TEMPERATURE_MONITOR_VTABLE_PTR(ob, member) ((ob)->lpVtbl.member)
#define TEMPERATURE_MONITOR_VTABLE(ob, member) (*((ob)->lpVtbl.member))

#define CHECK_TEMPERATURE_MONITOR_VTABLE_PTR(ob, member) if(TEMPERATURE_MONITOR_VTABLE_PTR(ob, member) == NULL) { \
    send_temperature_monitor_error_text(ob, "member not implemented"); \
    return TEMPERATURE_MONITOR_ERROR; \
}  

#define CALL_TEMPERATURE_MONITOR_VTABLE_PTR(ob, member) if( TEMPERATURE_MONITOR_VTABLE(ob, member)(ob) == TEMPERATURE_MONITOR_ERROR ) { \
	send_temperature_monitor_error_text(ob, "member failed");  \
	return TEMPERATURE_MONITOR_ERROR; \
}

typedef enum {TEMP_ERROR_INFO, TEMP_ERROR_CRITICAL} TEMP_ERROR;

typedef struct
{
	TEMP_ERROR type;
	char error[200];
	
} TempError;

typedef struct _TemperatureMonitor TemperatureMonitor;

typedef struct _TemperatureMonitorVtbl
{
	int (*hw_initialise) (TemperatureMonitor* tm); 
	int (*initialise) (TemperatureMonitor* tm);    
	int (*destroy) (TemperatureMonitor* tm);
	int (*read_temperatures) (TemperatureMonitor* tm, double *value1, double *value2, double *value3, double *value4);
	int (*get_status_flags) (TemperatureMonitor* tm, int *thermostat_error, int *heater_over_temp, int *enclosure_stat);
	int (*set_errors) (TemperatureMonitor* tm);

} TemperatureMonitorVtbl;


struct _TemperatureMonitor
{
  HardwareDevice parent; 
  
  TemperatureMonitorVtbl lpVtbl;
 
  int		 _panel_id;
  int		 _setup_panel_id;
  int     	 _timer;
  int		 _initialised; 
  int		 _hw_initialised;

  double	 _timer_interval;

  int		 _error_box;

  ListType	 _error_list;
};


void temperature_monitor_constructor(TemperatureMonitor* tm, const char *name, const char *description, const char *data_dir);

int send_temperature_monitor_error_text (TemperatureMonitor* tm, char fmt[], ...);

int temperature_monitor_hardware_initialise(TemperatureMonitor* tm); 
int temperature_monitor_initialise(TemperatureMonitor* tm);    
int temperature_monitor_is_initialised(TemperatureMonitor* tm);

void temperature_monitor_set_error_handler(TemperatureMonitor* tm, UI_MODULE_ERROR_HANDLER handler, void *callback_data);

int temperature_monitor_add_settings_panel (TemperatureMonitor* tm, int panel_id);
int temperature_monitor_destroy(TemperatureMonitor* tm);
int temperature_monitor_read_temperatures (TemperatureMonitor* tm, double *value1, double *value2, double *value3, double *value4);
int temperature_monitor_get_status_flags (TemperatureMonitor* tm, int *thermostat_error,
											int *heater_over_temp, int *enclosure_stat);

int temperature_monitor_clear_errors (TemperatureMonitor* tm);
int temperature_monitor_add_error (TemperatureMonitor* tm, int log, TEMP_ERROR type, char *error_str);

void temperature_monitor_disable_timer(TemperatureMonitor* tm);
void temperature_monitor_enable_timer(TemperatureMonitor* tm);

int temperature_monitor_check_for_error (TemperatureMonitor* tm, char *string);

// Signals
typedef void (*TEMPERATURE_MONITOR_EVENT_HANDLER) (TemperatureMonitor* tm, void *data); 

int temperature_monitor_signal_enclosure_state_handler_connect (TemperatureMonitor *tm, TEMPERATURE_MONITOR_EVENT_HANDLER handler, void *callback_data);

#endif

 
