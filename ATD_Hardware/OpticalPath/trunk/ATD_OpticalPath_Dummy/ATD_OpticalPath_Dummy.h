#ifndef __DUMMY_OPTICAL_PATH__
#define __DUMMY_OPTICAL_PATH__ 

#include "OpticalPath.h"

typedef struct
{
	OpticalPathManager parent;
	
	int manual_position;
    
} ManualOpticalPath;

OpticalPathManager* manual_optical_path_new(const char *name, const char *description, const char* data_dir, const char *filepath);

#endif
