#ifndef __90i_EPI_FIELD_STOP__
#define __90i_EPI_FIELD_STOP__ 

#include "single_range_hardware_device.h"

typedef struct 
{
	SingleRangeHardwareDevice parent; 
	
	enum ISCOPELibEnum_EnumStatus _mounted;	 
	CAObjHandle hEpiFieldStop; 
	
} EpiFieldStop90i;

SingleRangeHardwareDevice* Nikon90i_epi_field_stop_new(CAObjHandle hNikon90i,  const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data);


#endif
