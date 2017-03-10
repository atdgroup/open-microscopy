#ifndef __ATD_CUBESLIDER_A__
#define __ATD_CUBESLIDER_A__

#include "CubeSlider.h"

typedef struct
{
	FluoCubeManager parent;
	
	int	 	 _com_port;
	int	 	 _i2c_chip_type;  
  	int		 _i2c_bus;
  	int		 _i2c_chip_address;
	
} ATD_CUBESLIDER_A;


FluoCubeManager* atd_cubeslider_a_new(const char *name, const char *description, const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data);

#endif
