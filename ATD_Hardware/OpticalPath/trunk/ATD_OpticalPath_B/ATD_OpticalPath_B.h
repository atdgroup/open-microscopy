#ifndef __ATD_OP_B_OPTICAL_PATH__
#define __ATD_OP_B_OPTICAL_PATH__ 

#include "OpticalPath.h"

typedef enum {OPB_PID_PARAM_PROPORTIONAL = 2, OPB_PID_PARAM_INTEGAL = 3, OPB_PID_PARAM_DERIVITIVE = 4} OPB_PID_PARAM_TYPE;

typedef struct
{
	OpticalPathManager parent;
	int _setup_panel_id;
    int	 	 _com_port;
	int		 _i2c_chip_address;
	int 	 _i2c_bus;
	int 	 _i2c_chip_type;
  	int 	 _lock;
	int		 _last_position;
    
} ATD_OPTICALPATH_B;

OpticalPathManager* atd_op_b_optical_path_new(const char *name, const char *description, const char* data_dir, const char *filepath);

#endif
