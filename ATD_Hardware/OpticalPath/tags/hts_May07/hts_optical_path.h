#ifndef __HTS_OPTICAL_PATH__
#define __HTS_OPTICAL_PATH__ 

#include "optical_path.h"
#include "minipak mirror stepper.h"

typedef struct
{
	OpticalPathManager parent;
	
	MirrorStepper* mirror_stepper; 
	
} OpticalPathHTS;

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI HTS Microscope system. 
//Optical path control.
////////////////////////////////////////////////////////////////////////////

OpticalPathManager* hts_optical_path_manager_new(const char *filepath);

#endif
