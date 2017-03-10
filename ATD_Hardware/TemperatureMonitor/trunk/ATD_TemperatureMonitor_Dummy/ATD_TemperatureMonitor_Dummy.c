#include "ATD_TemperatureMonitor_Dummy.h"
#include "TemperatureMonitorUI.h" 
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include "asynctmr.h"
#include <rs232.h>
#include <utility.h>

static int atd_temperature_monitor_read_temperatures (TemperatureMonitor* tm, double *value1, double *value2, double *value3, double *value4)
{
	*value1 = 100.0 * ((double) rand() / RAND_MAX);
	*value2 = 100.0 * ((double) rand() / RAND_MAX);
	*value3 = 100.0 * ((double) rand() / RAND_MAX);
	*value4 = 100.0 * ((double) rand() / RAND_MAX);

	return TEMPERATURE_MONITOR_SUCCESS;  
}

static int atd_temperature_set_errors (TemperatureMonitor* tm)
{
	temperature_monitor_clear_errors (tm);
	
	temperature_monitor_add_error (tm, 0, TEMP_ERROR_CRITICAL, "Heater: Over temp.  ");
	temperature_monitor_add_error (tm, 0, TEMP_ERROR_INFO, "Enclosure Open          ");

	return TEMPERATURE_MONITOR_SUCCESS;  
}


static int atd_temperature_monitor_hardware_init(TemperatureMonitor *tm)
{
	return TEMPERATURE_MONITOR_SUCCESS;  
}


static int atd_temperature_monitor_init(TemperatureMonitor *tm)
{
	return TEMPERATURE_MONITOR_SUCCESS;   
}


static int atd_temperature_monitor_destroy (TemperatureMonitor *tm)
{			
	return TEMPERATURE_MONITOR_SUCCESS;  
}

TemperatureMonitor* atd_temperature_monitor_dummy_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	TemperatureMonitor* tm = (TemperatureMonitor*) malloc(sizeof(ATD_TEMPERATURE_MONITOR_DUMMY));  
	ATD_TEMPERATURE_MONITOR_DUMMY* atd_temp_dummy = (ATD_TEMPERATURE_MONITOR_DUMMY *) tm;    

	memset(atd_temp_dummy, 0, sizeof(ATD_TEMPERATURE_MONITOR_DUMMY));  
	
	temperature_monitor_constructor(tm, name, description, data_dir);

	TEMPERATURE_MONITOR_VTABLE_PTR(tm, hw_initialise) = atd_temperature_monitor_hardware_init; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, initialise) = atd_temperature_monitor_init; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, destroy) = atd_temperature_monitor_destroy; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, read_temperatures) = atd_temperature_monitor_read_temperatures; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, set_errors) = atd_temperature_set_errors;

	return tm;
}
