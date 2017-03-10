#ifndef __90i_LAMP__
#define __90i_LAMP__ 

#include "lamp.h"

typedef struct
{
	Lamp parent;
	
	enum ISCOPELibEnum_EnumStatus _mounted;	 
	CAObjHandle hLamp; 
	
} Lamp90i;

Lamp* Nikon90i_lamp_new(CAObjHandle hNikon90i, char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

#endif
