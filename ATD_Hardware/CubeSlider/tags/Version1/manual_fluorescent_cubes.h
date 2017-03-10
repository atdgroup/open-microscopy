#ifndef __MICROSCOPY_FLUO_CUBES__
#define __MICROSCOPY_FLUO_CUBES__ 

#include "fluorescent_cubes.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Fluorescent cube control for manual microscope.
////////////////////////////////////////////////////////////////////////////

typedef struct
{
	FluoCubeManager parent;

} FluoCubeManagerManual;


FluoCubeManager* Manual_fluo_cube_manager_new(const char *filepath);

#endif
