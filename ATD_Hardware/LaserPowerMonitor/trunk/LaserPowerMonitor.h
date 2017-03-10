#ifndef __LASERPOWER_MONITOR__
#define __LASERPOWER_MONITOR__ 

#include "HardwareDevice.h"

#define LASER_PWR_MONITOR_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define LASER_PWR_MONITOR_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_LASER_PWR_MONITOR_VTABLE_PTR(ob, member) if(LASER_PWR_MONITOR_VTABLE_PTR(ob, member) == NULL) { \
    hardware_device_send_error_text(HARDWARE_DEVICE_CAST(ob), "member not implemented"); \
    return HARDWARE_ERROR; \
}  

#define CALL_LASER_PWR_MONITOR_VTABLE_PTR(ob, member) if(LASER_PWR_MONITOR_VTABLE(ob, member)(ob) == LASER_PWR_MONITOR_ERROR ) { \
	hardware_device_send_error_text(HARDWARE_DEVICE_CAST(ob), "member failed");  \
	return HARDWARE_ERROR; \
}

typedef struct
{
	int	(*initialise) (LaserPowerMonitor* );
	int (*destroy) (LaserPowerMonitor* );
	int (*get_laser_power) (LaserPowerMonitor* , double *power);
	int (*get_gain) (LaserPowerMonitor* , int *gain);
	int (*set_gain) (LaserPowerMonitor* , int gain);
	int (*get_minimum_shutter_time) (LaserPowerMonitor* , double *val);
	void (*display_val) (LaserPowerMonitor* , double reading); 
	void (*set_wavelength_range) (LaserPowerMonitor* , double min_nm, double max_nm); 
	void (*set_aperture) (LaserPowerMonitor* , double aperture); 
} LaserPowerMonitorVtbl;

typedef struct _LaserPowerMonitor
{
	HardwareDevice parent; 

	LaserPowerMonitorVtbl vtable;

	int		 _main_panel_id;
	int		 _settings_panel_id;
	int		 _timer;
	double   _timer_interval;
};

void laserpowermonitor_constructor(LaserPowerMonitor* device, const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data);

int laserpowermonitor_destroy (LaserPowerMonitor *laserpower_mon);
int laserpowermonitor_initialise (LaserPowerMonitor *laserpower_mon);

int laserpowermonitor_display_val(LaserPowerMonitor* laserpower_mon, double reading);
int laserpowermonitor_get (LaserPowerMonitor* laserpower_mon, double *val);
int laserpowermonitor_get_and_display (LaserPowerMonitor* laserpower_mon);
int laserpowermonitor_get_gain (LaserPowerMonitor* laserpower_mon, int *val);
int laserpowermonitor_set_gain (LaserPowerMonitor* laserpower_mon, int val);
int laserpowermonitor_get_minimum_allowed_shutter_time (LaserPowerMonitor* laserpower_mon, double *val);

void laserpowermonitor_disable_timer(LaserPowerMonitor* laserpower_mon);
void laserpowermonitor_enable_timer(LaserPowerMonitor* laserpower_mon);
void laserpowermonitor_set_timer_interval(LaserPowerMonitor* laserpower_mon, double val);

void laserpowermonitor_set_wavelength_range (LaserPowerMonitor* laserpower_mon, double min_nm, double max_nm);
void laserpowermonitor_set_aperture (LaserPowerMonitor* laserpower_mon, double aperture);

#endif
