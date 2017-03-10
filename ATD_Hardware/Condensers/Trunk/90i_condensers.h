#ifndef __90i_CONDENSERS__
#define __90i_CONDENSERS__ 

#include "condensers.h"

typedef struct
{
	CondenserManager parent;
	
	CAObjHandle hCondenser; 
	
} CondenserManager90i;

CondenserManager* Nikon90i_condenser_manager_new(CAObjHandle hNikon90i, const char *name, const char *description, const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data);

#endif
