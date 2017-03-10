#ifndef __ATD_SHUTTER_A_SHUTTER__
#define __ATD_SHUTTER_A_SHUTTER__

#include "shutter.h"
#include "FTDI_Utils.h"

typedef struct
{
	Shutter parent;

	FTDIController* controller;

	int		 _inhibited;
	int	 	 _com_port;
  	int 	 _lock;
	int		 _fast_byte;
	double	 _open_time;

	#ifdef HEAVY_I2C_TESTING
	int shutter_status;
	#endif
	
} ATD_SHUTTER_A;

Shutter* atd_shutter_a_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler);

int atd_shutter_a_shutter_set_automatic_control(Shutter* shutter, int manual);

#endif
