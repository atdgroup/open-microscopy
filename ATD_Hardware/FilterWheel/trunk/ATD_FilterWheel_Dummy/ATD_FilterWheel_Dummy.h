#ifndef __MICROSCOPY_FLUO_CUBES__
#define __MICROSCOPY_FLUO_CUBES__ 

#include "CubeSlider.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Fluorescent cube control for manual microscope.
////////////////////////////////////////////////////////////////////////////

typedef struct
{
	FluoCubeManager parent;

	int stored_position;
	
} FluoCubeManagerManual;


FluoCubeManager* Manual_fluo_cube_manager_new(const char *name, const char *description, const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data);

#endif
