#ifndef __ATD_TEMPERATURE_MONITOR_DUMMY__
#define __ATD_TEMPERATURE_MONITOR_DUMMY__

#include "TemperatureMonitor.h"

typedef struct
{
	TemperatureMonitor parent;
	
} ATD_TEMPERATURE_MONITOR_DUMMY;

TemperatureMonitor* atd_temperature_monitor_dummy_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

#endif