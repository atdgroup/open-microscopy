#ifndef __MICROSCOPY_SHUTTER__
#define __MICROSCOPY_SHUTTER__ 

#include "shutter.h"

typedef struct
{
	Shutter parent;
	
	int opened;
	int inhibited;
	int computer_controlled;
	double open_time;
	
} ShutterManual;

Shutter* manual_shutter_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler);

#endif
