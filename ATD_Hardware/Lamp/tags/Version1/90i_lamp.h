#ifndef __90i_LAMP__
#define __90i_LAMP__ 

#include "lamp.h"

typedef struct
{
	Lamp parent;
	
	CAObjHandle hLamp; 
	
} Lamp90i;

Lamp* Nikon90i_lamp_new(CAObjHandle hNikon90i);

#endif
