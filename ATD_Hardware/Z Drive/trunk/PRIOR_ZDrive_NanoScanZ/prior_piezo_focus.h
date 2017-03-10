#ifndef __PRIOR_Z_DRIVE__
#define __PRIOR_Z_DRIVE__ 

#include "ZDrive.h"

typedef struct
{
	Z_Drive parent;
	
	int		 _panel_id;
	int	 	 _com_port;
	int		 _lock;
	double 	 _steps_per_micron;
	double 	 _range;
	
} Z_DrivePrior;

Z_Drive* prior_zdrive_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

#endif
