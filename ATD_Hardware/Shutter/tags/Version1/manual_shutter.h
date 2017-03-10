#ifndef __MICROSCOPY_SHUTTER__
#define __MICROSCOPY_SHUTTER__ 

#include "shutter.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Optical shutter control for manual microscope.
////////////////////////////////////////////////////////////////////////////

typedef struct
{
	Shutter parent;
	
} ShutterManual;

Shutter* manual_shutter_new(void);

#endif
