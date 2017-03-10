#ifndef __90i_OPTICAL_ZOOM__
#define __90i_OPTICAL_ZOOM__ 

#include "single_range_hardware_device.h"

typedef struct
{
	SingleRangeHardwareDevice parent; 
	
	enum ISCOPELibEnum_EnumStatus _mounted;
	
	CAObjHandle hOpticalZoom; 
	
} OpticalZoom90i;

SingleRangeHardwareDevice* Nikon90i_optical_zoom_new(CAObjHandle hNikon90i,  const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data);

#endif
