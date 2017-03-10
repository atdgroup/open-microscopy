#ifndef __ATD_LASERPOWER_MONITOR_LOGP__
#define __ATD_LASERPOWER_MONITOR_LOGP__ 

#include "LaserPowerMonitor.h"
#include "FTDI_Utils.h"

typedef enum {LASER_POWER_MODE_14BIT = 0x54,
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

	double _calibration;   // from config.ini, to make all units read the same, get from S:\ATD_Projects
	double _wavelength_factor;  // correct for the wavelength range, use 1 for no correction
	char   _wavelength_message[64];
	double _detector_aperture;   // from config.ini, this is needed to correct for obj aperture
	double _aperture_factor;  // correct for this objective back aperture, use 1 for no corection
	char   _aperture_message[64];

	double level;  // The current displayed value in mA

} ATD_LaserPowerMonitor_LOGP;

LaserPowerMonitor* atd_laserpowermonitor_LOGP_new(const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data);

int atd_laserpowermonitor_LOGP_set_gain(ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP, LaserPowerverMonitorGainValue gain);


#endif
