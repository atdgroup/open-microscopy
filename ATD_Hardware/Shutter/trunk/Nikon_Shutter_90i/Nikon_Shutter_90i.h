#ifndef __90i_SHUTTER__
#define __90i_SHUTTER__ 

#include "shutter.h"

typedef struct
{
	Shutter parent;
	
	CAObjHandle hShutter; 
	
} Shutter90i;

Shutter* Nikon90i_shutter_new(CAObjHandle hNikon90i,  const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data);

#endif
