#ifndef __OFFLINEIMAGER_SHUTTER__
#define __OFFLINEIMAGER_SHUTTER__

#include "shutter.h"

typedef struct
{
	Shutter parent;
	
	int		 _i2c_chip_type;
	int	 	 _com_port;
	int	 	 _i2c_bus;
	int	 	 _i2c_chip_address;
	int		 _monitor_timer;

	int opened;
	int inhibited;
	int computer_controlled;
	int has_charge_status;
	int has_inhibit_line;
	double last_closed_time;
	double open_time;
	
} OfflineImagerShutter;

Shutter* offline_imager_shutter_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler);
Shutter* offline_imager_shutter2_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler);

#endif
