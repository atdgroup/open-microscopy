#ifndef __90I_OPTICAL_PATH__
#define __90I_OPTICAL_PATH__ 

#include "OpticalPath.h"

typedef struct
{
	OpticalPathManager parent;
	
	CAObjHandle hOpticalPath; 
	
} OpticalPath90i;

OpticalPathManager* Nikon90i_optical_path_manager_new(CAObjHandle hNikon90i, const char *name, const char *description, const char* data_dir, const char *file);
int Nikon90i_optical_path_enable_Nikon_callback (OpticalPathManager* optical_path_manager);
int Nikon90i_optical_path_disable_Nikon_callback (OpticalPathManager* optical_path_manager);

#endif
