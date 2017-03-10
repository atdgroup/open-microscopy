#ifndef __MICROSCOPY_LAMP__
#define __MICROSCOPY_LAMP__ 

#include "lamp.h"

typedef struct
{
	Lamp parent;

	LampStatus 	_status;	//1 = on
	double 	_intensity;
	
} LampManual;

Lamp* manual_lamp_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

#endif
