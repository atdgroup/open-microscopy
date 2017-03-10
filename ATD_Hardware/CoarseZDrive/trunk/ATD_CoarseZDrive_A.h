#ifndef __ATD_COARSE_ZDRIVE_A__
#define __ATD_COARSE_ZDRIVE_A__

#include "HardWareTypes.h"     
#include "HardWareDevice.h" 
#include "FTDI_Utils.h"

#include "signals.h"
#include "gci_ui_module.h"   
#include "gci_utils.h" 

#define COARSE_Z_DRIVE_SUCCESS 0
#define COARSE_Z_DRIVE_ERROR -1

#define COARSE_Z_DRIVE_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define COARSE_Z_DRIVE_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_COARSE_Z_DRIVE_VTABLE_PTR(ob, member) if(COARSE_Z_DRIVE_VTABLE_PTR(ob, member) == NULL) { \
    hardware_device_send_error_text(HARDWARE_DEVICE_CAST(ob), "member not implemented"); \
    return COARSE_Z_DRIVE_ERROR; \
}  

#define CALL_COARSE_Z_DRIVE_VTABLE_PTR(ob, member) if( COARSE_Z_DRIVE_VTABLE(ob, member)(ob) == COARSE_Z_DRIVE_ERROR ) { \
	hardware_device_send_error_text(HARDWARE_DEVICE_CAST(ob), "member failed");  \
	return COARSE_Z_DRIVE_ERROR; \
}

typedef struct
{
	int (*initialise) (CoarseZDrive* zd);    
	int (*destroy) (CoarseZDrive* zd);

} CoarseZDriveVtbl;

struct _CoarseZDrive
{
	HardwareDevice parent; 
  
	CoarseZDriveVtbl vtable;
	
	FTDIController* controller;

	int	 	 _com_port;
	int		 _i2c_chip_address;
	int 	 _i2c_bus;
	int 	 _i2c_chip_type;
  	int 	 _lock;
	int		 _panel_id;
	int		 _setup_panel_id;
	int      _timer;
	int		 _initialised;
	int		 _driving_lock;
	int		 _drive_direction;
	int		 _driving;
	int		 _full_range_um;
	double   _timer_interval;
	double	 _calibration;
	double	 _timeout;
	char	 _settings_file[500];
};

CoarseZDrive* atd_coarse_zdrive_a_new(const char *name, const char *description,
									  UI_MODULE_ERROR_HANDLER handler, const char *data_dir, const char *data_file);

int coarse_z_drive_destroy(CoarseZDrive* zd);
int coarse_z_drive_initialise(CoarseZDrive* zd);    
int coarse_z_drive_hardware_initialise(CoarseZDrive* zd); 
int coarse_z_drive_is_initialised(CoarseZDrive* zd);
int coarse_z_drive_hardware_is_initialised(CoarseZDrive* zd); 
void coarse_zdrive_disable_timer(CoarseZDrive* zd);
void coarse_zdrive_enable_timer(CoarseZDrive* zd);
int coarse_zdrive_move_to_top(CoarseZDrive* zd);
int coarse_zdrive_move_to_bottom(CoarseZDrive* zd);
int coarse_zdrive_wait_for_stop_moving(CoarseZDrive* zd);

#endif
 
