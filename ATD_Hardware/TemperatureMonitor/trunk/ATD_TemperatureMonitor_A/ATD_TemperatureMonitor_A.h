#ifndef __ATD_TEMPERATURE_MONITOR_A__
#define __ATD_TEMPERATURE_MONITOR_A__

#include "TemperatureMonitor.h"

typedef struct
{
	TemperatureMonitor parent;
	
	int		 _setup_panel_id;
	int	 	 _com_port;
	int 	 _i2c_address;
	int 	 _i2c_bus;
	int 	 _i2c_type;

	int		 _last_thermostat_error;  
    int		 _last_heater_over_temp;
    int		 _last_enclosure_state;

	double   _temp_cal_scale0;
	double   _temp_cal_scale1;
	double   _temp_cal_scale2;
	double   _temp_cal_scale3;
	double   _temp_cal_offset0;  
	double   _temp_cal_offset1;  
	double   _temp_cal_offset2;  
	double   _temp_cal_offset3;

} ATD_TEMPERATURE_MONITOR_A;

TemperatureMonitor* atd_temperature_monitor_a_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

void temperature_monitor_set_setup_panel(TemperatureMonitor*tm, int panel_id);

#endif