#ifndef __90i_Z_DRIVE__
#define __90i_Z_DRIVE__ 

#include "ZDrive.h"

typedef struct
{
	Z_Drive parent;

	CAObjHandle hZDrive90i; 
	
} ZDrive90i;

Z_Drive* Nikoni90_z_drive_new(CAObjHandle hNikon90i, const char *name, const char *description, 
								 UI_MODULE_ERROR_HANDLER handler, const char *data_dir);
			
int Nikoni90_fast_inaccurate_move (Z_Drive* zdrive, double val);

#endif
