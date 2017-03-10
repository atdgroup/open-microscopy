#ifndef __MICROSCOPY_FLUO_CUBES__
#define __MICROSCOPY_FLUO_CUBES__ 

#include "CubeSlider.h"

typedef struct
{
	FluoCubeManager parent;
	
	CAObjHandle hCube; 
	
} FluoCubeManager90i;


FluoCubeManager* Nikon90i_fluo_cube_manager_new(CAObjHandle hNikon90i, const char *name, const char *description, const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data);

#endif
