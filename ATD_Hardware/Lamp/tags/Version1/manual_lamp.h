#ifndef __MICROSCOPY_LAMP__
#define __MICROSCOPY_LAMP__ 

#include "lamp.h"

typedef struct
{
	Lamp parent;

	int 	_status;	//1 = on
	double 	_intensity;
	
} LampManual;

Lamp* Manual_lamp_new(void);

#endif
