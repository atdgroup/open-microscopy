#ifndef __ATD_CUBESLIDER_A__
#define __ATD_CUBESLIDER_A__

#include "CubeSlider.h"
#include "FTDI_Utils.h"

typedef struct
{
	FluoCubeManager parent;
	
	FTDIController*		_controller;

	int	 	 _com_port;
	int	 	 _i2c_chip_type;  
  	int		 _i2c_bus;
  	int		 _i2c_chip_address;
	
	char	 _device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH];

} ATD_CUBESLIDER_A;


FluoCubeManager* atd_cubeslider_a_new(const char *name, const char *description, const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data);

#endif
