#ifndef __ATD_SCANNER_A_SCANNER__
#define __ATD_SCANNER_A_SCANNER__

#include "scanner.h"
#include "FTDI_Utils.h"

typedef struct
{
	Scanner parent;
	
	FTDIController*		_controller;

	int	 	 _i2c_chip_type; 
	int	 	 _com_port;
    int	 	 _i2c_bus;
    int	 	 _i2c_chip_address;
	int		 _dwell;
	double	 _current_clock_frequency;
	
} ATD_Scanner_A;

Scanner* atd_scanner_a_new(char *name, char *description,  const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data);

#endif
