#ifndef __MICROSCOPY_CONDENSERS__
#define __MICROSCOPY_CONDENSERS__ 

#include "condensers.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Condenser control for manual microscope.
////////////////////////////////////////////////////////////////////////////

typedef struct
{
	CondenserManager parent;
	
	int _current_position;
	
} CondenserManagerManual;

CondenserManager* Manual_condenser_manager_new(const char *name, const char *description, const char* data_dir, const char *filepath);

#endif
