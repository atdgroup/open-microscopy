#ifndef __PRIOR_Z_DRIVE__
#define __PRIOR_Z_DRIVE__ 

#include "ZDrive.h"
#include "LStep\LStepXY.h"

typedef struct
{
	Z_Drive parent;
	
	int		 _panel_id;
	int	 	 _com_port;
	int		 _lock;
	double 	 _steps_per_micron;
	double 	 _range;
	LStepXYStage *lstep_stage;

} Z_DriveLStep;

Z_Drive* marzhauser_zdrive_lstep_new(LStepXYStage *lstep_stage, const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

int lstep_focus_set_indicators(Z_Drive* zd, double focus_microns);

#endif
