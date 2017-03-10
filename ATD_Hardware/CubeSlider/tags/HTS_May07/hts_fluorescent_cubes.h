#ifndef __MICROSCOPY_FLUO_CUBES__
#define __MICROSCOPY_FLUO_CUBES__ 

#include "fluorescent_cubes.h"

typedef struct
{
	FluoCubeManager parent;
	
	
} FluoCubeManagerHts;


FluoCubeManager* HTS_fluo_cube_manager_new(const char *filepath);

#endif
