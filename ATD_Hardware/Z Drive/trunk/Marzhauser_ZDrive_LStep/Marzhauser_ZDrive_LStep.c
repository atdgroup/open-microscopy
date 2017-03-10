#include "Marzhauser_ZDrive_LStep.h" 
#include "ZDriveUI.h" 
#include "gci_utils.h"
#include "LStep\LStep4.h"  
#include "LStep\LStepXY.h"

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include "asynctmr.h"
#include <rs232.h>
#include <utility.h>
#include "stage\stage.h"   // This z drive is combined with xy

static int lstep_focus_get_position(Z_Drive* zd, double *focus_microns)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;     
	int err;
	double x, y, z, dummy;
	
	//Read returned coordinate values (x, y and z).
	if((err = LS_GetPos(&x, &y, &z, &dummy)) > 0) {
		LogLStepError(err);
		return Z_DRIVE_ERROR;		
	}
	
	*focus_microns = STAGE_CORRECT_VAL_FOR_ORIENTATION((XYStage*) lstep_zd->lstep_stage, z, ZAXIS);
	
	return Z_DRIVE_SUCCESS;
}

static int lstep_focus_set_position(Z_Drive* zd, double focus_microns)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;     
	int was_on = 0;
	double x, y;  
		
	stage_get_xy_position((XYStage*) lstep_zd->lstep_stage, &x, &y);

	stage_goto_xyz_position ((XYStage*) lstep_zd->lstep_stage, x, y, focus_microns);
	
	return Z_DRIVE_SUCCESS;
}

int lstep_focus_set_indicators(Z_Drive* zd, double focus_microns)
{
	if(!z_drive_is_busy (zd)) {
		SetCtrlVal(zd->_panel_id, FOCUS_FOCUS, focus_microns); 	    
		SetCtrlVal(zd->_panel_id, FOCUS_FOCUS_2, focus_microns);
	}
	
	return Z_DRIVE_SUCCESS;
}

static int lstep_wait_for_stop_moving(Z_Drive* zd, double timeout)
{
	int err, timed_out;
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;    
		
	/*
	
	timeout *= 2000; //to convert from seconds to milliseconds and account for longer z moves

	if((err = LS_WaitForAxisStop (7, timeout, &timed_out)) > 0) {
		LogLStepError(err);  
		return Z_DRIVE_ERROR;  
	}
	
	if(timed_out) {
		send_z_drive_error_text(lstep_zd, "Timeout occurred, Z drive failed to move");
		return Z_DRIVE_ERROR;
	}
*/
	return Z_DRIVE_SUCCESS;
}

static int lstep_get_speed (Z_Drive* zd, double *speed)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;   

	stage_get_speed((XYStage*) lstep_zd->lstep_stage, ZAXIS, speed);

	return Z_DRIVE_SUCCESS;
}

static int lstep_get_accel (Z_Drive* zd, double *accel)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;   

	stage_get_acceleration((XYStage*) lstep_zd->lstep_stage, ZAXIS, accel);

	return Z_DRIVE_SUCCESS;
}

static int lstep_focus_set_zero (Z_Drive* zd)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;   

	double x, y;  
	int err;
	
	//insert proper x y direction flags below
	stage_get_xy_position((XYStage*) lstep_zd->lstep_stage, &x, &y);
	if((err= LS_SetPos(-x, -y, 0.0, 0.0)) > 0) {  // Make the current location the z datum 
		LogLStepError(err);
		return STAGE_ERROR;	
	}	

	lstep_focus_set_indicators(zd, 0.0);

	return Z_DRIVE_SUCCESS;
}

static int lstep_focus_get_min_max_in_microns(Z_Drive* zd, int* min_microns, int* max_microns)
{
	*min_microns = (int) zd->_min_microns;
	*max_microns = (int) zd->_max_microns; 
	
	return Z_DRIVE_SUCCESS; 	
}

static int lstep_focus_hardware_init(Z_Drive* zd)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;
	int err;
	char device[UIMODULE_NAME_LEN];
	
	ui_module_get_name(UIMODULE_CAST(zd), device);
	 
	get_device_int_param_from_ini_file   (device, "COM_Port", &(lstep_zd->_com_port));  
//	get_device_double_param_from_ini_file(device, "StepsPerMicron", &(zd->_steps_per_micron));  // not used in this device
	get_device_double_param_from_ini_file(device, "Min Microns", &(zd->_min_microns));
	get_device_double_param_from_ini_file(device, "Max Microns", &(zd->_max_microns));
	get_device_double_param_from_ini_file(device, "Speed", &(zd->_speed));

	lstep_zd->_range =  zd->_max_microns - zd->_min_microns;
	
	lstep_init_comport(lstep_zd->_com_port);
	
	if((err = LS_SetActiveAxes(7)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
	
	stage_set_speed((XYStage*) lstep_zd->lstep_stage, ZAXIS, zd->_speed);

	//Set gear for calibration.
	if((err = LS_SetGear(1.0, 1.0, 10.0, 1.0)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}

	z_drive_hide_autofocus_controls(zd);  
	
	return Z_DRIVE_SUCCESS;  
}


static int lstep_focus_init(Z_Drive* zd)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd; 
	
	return Z_DRIVE_SUCCESS;   
}


int lstep_focus_destroy (Z_Drive* zd)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;    

	CmtDiscardLock(lstep_zd->_lock); 
	
	return Z_DRIVE_SUCCESS;  
}
						  
static int lstep_enable_disable_timers(Z_Drive* zd, int enable)
{
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;    

	//SetAsyncTimerAttribute (lstep_zd->, ASYNC_ATTR_ENABLED,  enable);
	//SetAsyncTimerAttribute (lstep_zd->_adc_monitor_timer, ASYNC_ATTR_ENABLED,  enable); 

	return Z_DRIVE_SUCCESS; 
}

Z_Drive* marzhauser_zdrive_lstep_new(LStepXYStage *lstep_stage, const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	Z_Drive* zd = (Z_Drive*) malloc(sizeof(Z_DriveLStep));  
	Z_DriveLStep* lstep_zd = (Z_DriveLStep *) zd;    
	
	lstep_zd->lstep_stage = lstep_stage;

	z_drive_constructor(zd, name, description, data_dir);

	z_drive_set_is_part_of_stage(zd);

	zdrive_dont_respond_to_event_val_changed(zd);
	
	CmtNewLock(NULL, 0, &(lstep_zd->_lock));
	
	Z_DRIVE_VTABLE_PTR(zd, hw_initialise) = lstep_focus_hardware_init; 
	Z_DRIVE_VTABLE_PTR(zd, initialise) = lstep_focus_init; 
	Z_DRIVE_VTABLE_PTR(zd, destroy) = lstep_focus_destroy; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_set_position) = lstep_focus_set_position; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_set_zero) = lstep_focus_set_zero; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_get_position) = lstep_focus_get_position;  
	Z_DRIVE_VTABLE_PTR(zd, z_drive_wait_for_stop_moving) = lstep_wait_for_stop_moving;
	Z_DRIVE_VTABLE_PTR(zd, z_drive_get_speed) = lstep_get_speed;  
	Z_DRIVE_VTABLE_PTR(zd, z_drive_get_accel) = lstep_get_accel; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_enable_disable_timers) = lstep_enable_disable_timers;  

	return zd;
}
