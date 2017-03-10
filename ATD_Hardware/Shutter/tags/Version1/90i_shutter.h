#ifndef __90i_SHUTTER__
#define __90i_SHUTTER__ 

#include "shutter.h"

typedef struct
{
	Shutter parent;
	
	CAObjHandle hShutter; 
	
} Shutter90i;

Shutter* Nikon90i_shutter_new(CAObjHandle hNikon90i);

#endif
