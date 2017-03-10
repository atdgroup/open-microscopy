#ifndef __DUMMY_STAGE__
#define __DUMMY_STAGE__

#include "stage\stage.h"

XYStage* stage_dummy_new(const char* name, const char* desription, UI_MODULE_ERROR_HANDLER error_handler, void *data, const char *data_dir);

#endif
