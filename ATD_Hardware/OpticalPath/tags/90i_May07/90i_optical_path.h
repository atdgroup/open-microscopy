#ifndef __90i_OPTICAL_PATH__
#define __90i_OPTICAL_PATH__ 

#include "optical_path.h"

typedef struct
{
	OpticalPathManager parent;
	
	CAObjHandle hOpticalPath; 
	
} OpticalPath90i;

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Optical path control.
////////////////////////////////////////////////////////////////////////////

OpticalPathManager* Nikon90i_optical_path_manager_new(CAObjHandle hNikon90i, const char *filepath);

#endif
