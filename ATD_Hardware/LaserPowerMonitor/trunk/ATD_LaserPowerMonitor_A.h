#ifndef __ATD_LASERPOWER_MONITOR_A__
#define __ATD_LASERPOWER_MONITOR_A__ 

#include "LaserPowerMonitor.h"
#include "FTDI_Utils.h"

typedef enum {LASER_POWER_MODE_14BIT=0x54,
			  LASER_POWER_MODE_16BIT = 0x58,
			  LASER_POWER_MODE_16BIT_TRIGGERED = 0x88} LaserPowerverMonitorBitModeType;

typedef enum {LASER_POWER_GAIN_1V_V = 0,
			  LASER_POWER_GAIN_2V_V = 1,
			  LASER_POWER_GAIN_4V_V = 2,
			  LASER_POWER_GAIN_8V_V = 3 } LaserPowerverMonitorGainValue;

typedef enum {LASER_POWER_GAIN_1V_V_FACTOR = 1,
			  LASER_POWER_GAIN_2V_V_FACTOR = 2,
			  LASER_POWER_GAIN_4V_V_FACTOR = 4,
			  LASER_POWER_GAIN_8V_V_FACTOR = 8 } LaserPowerverMonitorGainFactor;

static BYTE gain_factors[] = {LASER_POWER_GAIN_1V_V_FACTOR,
							  LASER_POWER_GAIN_2V_V_FACTOR,
							  LASER_POWER_GAIN_4V_V_FACTOR,
							  LASER_POWER_GAIN_8V_V_FACTOR};

typedef struct
{
	LaserPowerMonitor parent; 

	FTDIController*		_controller;

	int	 	 _com_port;
	int		 _i2c_chip_address;
	int 	 _i2c_bus;
	int 	 _i2c_chip_type;
  	int 	 _lock;

} ATD_LaserPowerMonitor_A;

LaserPowerMonitor* atd_laserpowermonitor_a_new(const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data);

int atd_laserpowermonitor_a_set_gain(ATD_LaserPowerMonitor_A* atd_laserpower_mon_a, LaserPowerverMonitorGainValue gain);


#endif
