#ifndef __MICROSCOPY_FLUO_CUBES__
#define __MICROSCOPY_FLUO_CUBES__ 

#include "fluorescent_cubes.h"

typedef struct
{
	FluoCubeManager parent;
	
	CAObjHandle hCube; 
	
} FluoCubeManager90i;


FluoCubeManager* Nikon90i_fluo_cube_manager_new(CAObjHandle hNikon90i, const char *filepath);

#endif
