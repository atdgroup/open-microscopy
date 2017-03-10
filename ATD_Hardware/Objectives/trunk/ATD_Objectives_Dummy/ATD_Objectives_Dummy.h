#ifndef __DUMMY_OBJECTIVE_MANAGER__ 
#define __DUMMY_OBJECTIVE_MANAGER__

#include "objectives.h" 

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Nosepiece control for manual microscope.
////////////////////////////////////////////////////////////////////////////

ObjectiveManager* Dummy_objective_manager_new(const char *name, const char *description, const char *data_dir, const char *filepath);

#endif
