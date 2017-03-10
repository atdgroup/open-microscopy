#include "LaserPowerMonitor.h"
//#include "ATD_LaserPowerMonitor_UI.h"
#include "ATD_UsbInterface_A.h"

#include "asynctmr.h"

#include <cviauto.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "profile.h"
#include "math.h"



void laserpowermonitor_set_wavelength_range (LaserPowerMonitor* laserpower_mon, double min_nm, double max_nm)
{
	if(LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, set_wavelength_range) != NULL) {   // Optional virtual fn
		(laserpower_mon->vtable.set_wavelength_range)(laserpower_mon, min_nm, max_nm);
	}
}

void laserpowermonitor_set_aperture (LaserPowerMonitor* laserpower_mon, double aperture)
{
	if(LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, set_aperture) != NULL) {   // Optional virtual fn
		(laserpower_mon->vtable.set_aperture)(laserpower_mon, aperture);
	}
}

int laserpowermonitor_display_val(LaserPowerMonitor* laserpower_mon, double reading)
{
	CHECK_LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, display_val) 

	(laserpower_mon->vtable.display_val)(laserpower_mon, reading);

	return HARDWARE_SUCCESS;	
}

int CVICALLBACK OnLaserPowerMonitorTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {	
			static int count = 1;
			int refresh_interval = 1;
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor *) callbackData;
			double reading;

			static double start_time = 0;

			if(laserpowermonitor_get (laserpower_mon, &reading) == HARDWARE_ERROR)
				return -1;

			if((count % refresh_interval) == 0) {
				laserpowermonitor_display_val(laserpower_mon, reading);
			}

			count++;

			//printf("OnLaserPowerMonitorTimerTick took %f\n", Timer() - start_time);
			start_time = Timer();
            break;
		}
    }
    
    return 0;
}

void laserpowermonitor_disable_timer(LaserPowerMonitor* laserpower_mon)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute(laserpower_mon->_panel_id, laserpower_mon->_timer, ATTR_ENABLED, 0);
	#else
	SetAsyncTimerAttribute (laserpower_mon->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void laserpowermonitor_enable_timer(LaserPowerMonitor* laserpower_mon)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute(laserpower_mon->_panel_id, laserpower_mon->_timer, ATTR_ENABLED, 1);
	#else
	SetAsyncTimerAttribute (laserpower_mon->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}

void laserpowermonitor_set_timer_interval(LaserPowerMonitor* laserpower_mon, double val)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute(laserpower_mon->_panel_id, laserpower_mon->_timer, ATTR_INTERVAL, val);
	#else
	SetAsyncTimerAttribute (laserpower_mon->_timer, ASYNC_ATTR_INTERVAL, val);
	#endif
	
}


void laserpowermonitor_constructor(LaserPowerMonitor* device, const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	memset(device, 0, sizeof(LaserPowerMonitor));

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(device), name);
	ui_module_set_description(UIMODULE_CAST(device), description);

	ui_module_set_data_dir( UIMODULE_CAST(device), data_dir);
    ui_module_set_error_handler(UIMODULE_CAST(device), error_handler, data); 
}

static void OnLaserPowerMonitorPanelsClosedOrHidden (UIModule *module, void *data)
{
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) data; 

	laserpowermonitor_disable_timer(laserpower_mon);
}

static void OnLaserPowerMonitorDisplayed (UIModule *module, void *data)
{
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) data; 

	laserpowermonitor_enable_timer(laserpower_mon);
}

int laserpowermonitor_initialise (LaserPowerMonitor* laserpower_mon)
{
	CHECK_LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, initialise) 
		
	if( (laserpower_mon->vtable.initialise)(laserpower_mon) == HARDWARE_ERROR ) {
		hardware_device_send_error_text(HARDWARE_DEVICE_CAST(laserpower_mon), "atd_laserpowermonitor_initialise failed");
		
		return HARDWARE_ERROR; 
	} 

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(laserpower_mon), OnLaserPowerMonitorPanelsClosedOrHidden, laserpower_mon);
	//ui_module_main_panel_show_mainpanel_handler_connect (UIMODULE_CAST(laserpower_mon), OnLaserPowerMonitorDisplayed, laserpower_mon);

	#ifdef SINGLE_THREADED_POLLING
	
	laserpower_mon->_timer = NewCtrl(laserpower_mon->_panel_id, CTRL_TIMER, "", 0, 0);
		
	if ( InstallCtrlCallback (laserpower_mon->_panel_id, laserpower_mon->_timer, OnLaserPowerMonitorTimerTick, laserpower_mon) < 0)
		return OPTICAL_LIFT_ERROR; 
		
	SetCtrlAttribute(laserpower_mon->_panel_id, laserpower_mon->_timer, ATTR_INTERVAL, laserpower_mon->_timer_interval);  
	SetCtrlAttribute(laserpower_mon->_panel_id, laserpower_mon->_timer, ATTR_ENABLED, 0);
	
	#else
	
	laserpower_mon->_timer = NewAsyncTimer (laserpower_mon->_timer_interval, -1, 0, OnLaserPowerMonitorTimerTick, laserpower_mon);
	//SetAsyncTimerAttribute (laserpower_mon->_timer, ASYNC_ATTR_INTERVAL, 1.0);

	if(laserpower_mon->_timer <= 0)
		return HARDWARE_ERROR;

	SetAsyncTimerName(laserpower_mon->_timer, "LaserPowerMonitor");
	SetAsyncTimerAttribute (laserpower_mon->_timer, ASYNC_ATTR_ENABLED,  0);
	
	#endif

	laserpowermonitor_disable_timer(laserpower_mon);

	return HARDWARE_SUCCESS;	
}


int laserpowermonitor_destroy (LaserPowerMonitor *laserpower_mon)
{
	CHECK_LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, destroy) 

	if( (laserpower_mon->vtable.destroy)(laserpower_mon) == HARDWARE_ERROR ) {
		hardware_device_send_error_text((HardwareDevice *) laserpower_mon, "laserpowermonitor_destroy failed");
		
		return HARDWARE_ERROR; 
	} 

	return HARDWARE_SUCCESS; 
}

int laserpowermonitor_get (LaserPowerMonitor* laserpower_mon, double *val)
{
	CHECK_LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_laser_power) 

	if( (laserpower_mon->vtable.get_laser_power)(laserpower_mon, val) == HARDWARE_ERROR ) {
		hardware_device_send_error_text((HardwareDevice *) laserpower_mon, "laserpowermonitor_get failed");
		
		return HARDWARE_ERROR; 
	} 

	return HARDWARE_SUCCESS; 
}

int laserpowermonitor_get_and_display (LaserPowerMonitor* laserpower_mon)
{
	int err;
	double val;

	if((err = laserpowermonitor_get (laserpower_mon, &val)) != HARDWARE_ERROR) {
		laserpowermonitor_display_val(laserpower_mon, val);
	}

	return err;
}

static int laserpowermonitor_get_gain (LaserPowerMonitor* laserpower_mon, int *val)
{
	CHECK_LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_gain) 

	if( (laserpower_mon->vtable.get_gain)(laserpower_mon, val) == HARDWARE_ERROR ) {
		hardware_device_send_error_text((HardwareDevice *) laserpower_mon, "laserpowermonitor_get_gain failed");
		
		return HARDWARE_ERROR; 
	} 

	return HARDWARE_SUCCESS; 
}

int laserpowermonitor_get_minimum_allowed_shutter_time (LaserPowerMonitor* laserpower_mon, double *val)
{
	CHECK_LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_minimum_shutter_time) 

	if( (laserpower_mon->vtable.get_minimum_shutter_time)(laserpower_mon, val) == HARDWARE_ERROR ) {
		hardware_device_send_error_text((HardwareDevice *) laserpower_mon, "laserpowermonitor_get_minimum_allowed_shutter_time failed");
		
		return HARDWARE_ERROR; 
	} 

	return HARDWARE_SUCCESS; 
}