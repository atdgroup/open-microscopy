#ifndef __MICROSCOPY_OBJECTIVE_MANAGER__ 
#define __MICROSCOPY_OBJECTIVE_MANAGER__

#include "objectives.h" 

typedef struct
{
	ObjectiveManager parent;
	
	CAObjHandle hNosepiece; 
	
} ObjectiveManager90i;

ObjectiveManager* Nikon90i_objective_manager_new(CAObjHandle hNikon90i, const char *name, const char *description, const char *data_dir, const char *filepath);

#endif
