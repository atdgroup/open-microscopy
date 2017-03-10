#ifndef __MANUAL_Z_DRIVE__
#define __MANUAL_Z_DRIVE__ 

#include "ZDrive.h"

typedef struct
{
	Z_Drive parent;
	
	int		 _panel_id;
	double 	 _focus_microns;
	
} Z_DriveManual;

Z_Drive* manual_zdrive_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

#endif
