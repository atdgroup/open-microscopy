#ifndef __DUMMY_LASER__
#define __DUMMY_LASER__ 

#include "laser.h"

typedef int (*GET_DAC_VALUE_FUNC_PTR) (Laser* laser, int *value); 

typedef struct _DummyLaser
{
	Laser    parent;

    int does_nothing;   // We need one member to compile

} DummyLaser;

Laser* om_laser_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

#endif
