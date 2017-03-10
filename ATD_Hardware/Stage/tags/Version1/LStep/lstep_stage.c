#include <formatio.h>
#include <utility.h>

#include "hardware.h"
#include "com_port_control.h"
#include "power.h"
#include "LStep4.h"
#include "lstep_stage.h"

/////////////////////////////////////////////////////////////////////////////
// XYZ stage module for LStep with Marzhauser stage - GP & RJL Jan 2006
//
// Requires LStep4.h LStep4.lib and LStep4.dll
/////////////////////////////////////////////////////////////////////////////

//NOTES:

//1) The z-drive is disabled during calibration (no limits)
//2) All position and move values are given in um
//3) The range of the stage was found to be approx 100mm (Y) by 124mm (X)
//4) Backlash (X and Y directions) - always approach from the origin direction, ie increasing values of distance
//5) If any axis drive not present then disable that axis 
//6) Stage has 1mm pitch ie one revolution = 1mm 
//7) Velocity range xy = 1um/sec to 40mm/sec, max acceleration = 700mm/sec/sec)

/////////////////////////////////////////////////////////////////////////////

#define PORTXY 		1		// This has to be 1 for Lstep even if we use port 5!
#define PORTXYSTR 	"COM1"  // We would change this to "COM5" if using port 5

#define DEFAULT_VEL_XY	20.0		// Initial velocity for xy calibration mm/sec
#define DEFAULT_ACC_XY	500.0		// Initial acceleration for xy calibration mm/sec^2

static double gStageOnTime = -6.0;

/* Function pointers used as virtual functions */
static struct stage_operations lstep_ops;

int CheckLStepErr(Stage *stage, int err)
{
	char msg[60];
	
	if (err == 0)
		return 0;
	
	if (err == 1)
		send_stage_error_text(stage, "Valid axis designation missing");
	else if (err == 2)
		send_stage_error_text(stage, "Non-executable function");
	else if (err == 3)
		send_stage_error_text(stage, "Command string is too long");
	else if (err == 4)
		send_stage_error_text(stage, "Invalid command");
	else if (err == 5)
		send_stage_error_text(stage, "Number outside valid range");
	else if (err == 6)
		send_stage_error_text(stage, "Incorrect number of parameters");
	else if (err == 7)
		send_stage_error_text(stage, "None ! or ?");
	else if (err == 8)
		send_stage_error_text(stage, "TVR not possible because axis is active");
	else if (err == 9)
		send_stage_error_text(stage, "Axes cannot be switched on or off bescause TVR is active");
	else if (err == 10)
		send_stage_error_text(stage, "Function not configured");
	else if (err == 11)
		send_stage_error_text(stage, "Joystick is set to manual");
	else if (err == 12)
		send_stage_error_text(stage, "Limit switch tripped");
	else if (err == 4001)
		send_stage_error_text(stage, "Internal error");
	else if (err == 4002)
		send_stage_error_text(stage, "Internal error");
	else if (err == 4003)
		send_stage_error_text(stage, "Undefined error");
	else if (err == 4004)
		send_stage_error_text(stage, "Interface type unknown");
	else if (err == 4005)
		send_stage_error_text(stage, "Cannot access controller. Is it switched on?");
	else if (err == 4006)
		send_stage_error_text(stage, "Lost communication with controller");
	else if (err == 4007)
		send_stage_error_text(stage, "Timeout while reading from controller");
	else if (err == 4008)
		send_stage_error_text(stage, "Command transmission error");
	else if (err == 4009)
		send_stage_error_text(stage, "Command terminated");
	else if (err == 4010)
		send_stage_error_text(stage, "Command not supported");
	else if (err == 4011)
		send_stage_error_text(stage, "Joystick is set to Manual");
	else if (err == 4012)
		send_stage_error_text(stage, "Joystick is set to Manual");
	else if (err == 4013)
		send_stage_error_text(stage, "Controller Timeout");
	else {
		sprintf(msg, "Unknown error code %d", err);
		send_stage_error_text(stage, msg);
	}

	return 1;
}


static int lstep_wait_for_ok_status(Stage *stage)
{
	char status[50] = "";
	double start = Timer();

	while (strncmp (status, "OK", 2)) {
		
		CmtGetLock (stage->_lock);
		LS_GetStatus(status, 20);
	    CmtReleaseLock(stage->_lock);
		
		// Don't wait forever
		if ((Timer() - start) > 10.0) 
			return STAGE_ERROR;
	}

	return STAGE_SUCCESS;	
}


static int lstep_set_baud_rate (Stage* stage, int baud_rate)
{
	int err = 0;
	char dummy[50], send_str[50];
	
	CmtGetLock (stage->_lock);

	sprintf(send_str, "!baud %d\r", baud_rate);  
	err = LS_SendString(send_str, dummy, 20, 0, 1000);
	if(CheckLStepErr(stage, err)) {
	    CmtReleaseLock(stage->_lock);
		return STAGE_ERROR;
	}
	
	err = LS_Disconnect();
	if(CheckLStepErr(stage, err)) {
	    CmtReleaseLock(stage->_lock);
		return STAGE_ERROR;
	}
	
	err = LS_ConnectSimple(1, stage->_port_string, baud_rate, 0);
	if(CheckLStepErr(stage, err)) {
	    CmtReleaseLock(stage->_lock);
		return STAGE_ERROR;
	}
	
    CmtReleaseLock(stage->_lock);
	
	return STAGE_SUCCESS;
}


static int lstep_power_up(Stage* stage)
{
	stage->_powered_up = 1;
	#ifdef POWER_VIA_I2C
	gStageOnTime = Timer();
	return GCI_PowerMicroscope_and_StageOn();
	#else
	return 0;
	#endif
}

static int lstep_power_down(Stage* stage)
{
	//stage_goto_xy_position (stage, 0.0, 0.0);

	// Leave hardware joystick enabled
	stage_set_joystick_on (stage);
	
	stage_set_baud_rate (stage, 9600);
	
	CmtGetLock (stage->_lock);
	LS_Disconnect();
    CmtReleaseLock(stage->_lock);

	stage->_powered_up = 0;
	#ifdef POWER_VIA_I2C
	return GCI_PowerMicroscope_and_StageOff();
	#else
	return STAGE_SUCCESS;
	#endif
}

static int lstep_powered_up(Stage* stage)
{
	#ifdef POWER_VIA_I2C
	stage->_powered_up = !GCI_Power_GetMicroscope_and_StageStatus();
	#else
	stage->_powered_up = 0;
	#endif
	return stage->_powered_up;
}

static int lstep_reset (Stage* stage) 
{
	int err;	

	CmtGetLock (stage->_lock);
	err = LS_SoftwareReset();
    CmtReleaseLock(stage->_lock);
	
	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	lstep_wait_for_ok_status(stage);

	return STAGE_SUCCESS;

}

static int lstep_init (Stage* stage)
{
	int parity, baud, dataBits, stopBits, inputQueueSize, outputQueueSize;
	char port_string[10], version[64], status[30];
	int i, err, rates[4] = {9600, 19200, 38400, 57600};

	//Use Glen's COM port allocation module
	GCI_ComPortControlAddDevice("XYStage");
	GCI_ComPortControlLoadConfig();
	GCI_ComPortControlGetDeviceProperties("XYStage", &stage->_port, stage->_port_string, &parity, &baud,
				&dataBits, &stopBits, &inputQueueSize, &outputQueueSize);
	
	//If the stage controller is off switch it on and wait for it to initialise
    stage->_powered_up = lstep_powered_up(stage);
    if (!stage->_powered_up) 
		if (lstep_power_up(stage) != 0) return STAGE_ERROR;   	//Some I2C problem
    while ((Timer() - gStageOnTime) < 6.0) ProcessSystemEvents();

	//Let us have our error messages in English
	err = LS_SetLanguage("ENG");
	if (CheckLStepErr(stage, err))
		return STAGE_ERROR;

	CmtGetLock (stage->_lock);

	for (i=0; i<4; i++) {
		if (LS_ConnectSimple(1, stage->_port_string, rates[i], 0) == 0)
			break;
	}
		
	// Check RS232 line by sending request for Version number
	err = LS_GetVersionStr(version, 64);
	if (CheckLStepErr(stage, err)) {
	    CmtReleaseLock(stage->_lock);
		return STAGE_ERROR;
	}
	
	CmtReleaseLock(stage->_lock);

	//Controller needs to operate at 9600 for successful initialisation
	if (lstep_set_baud_rate (stage, 9600) == STAGE_ERROR) return STAGE_ERROR;
	
	if (lstep_reset (stage) == STAGE_ERROR) return STAGE_ERROR;
	
	CmtGetLock (stage->_lock);

	//Check that it's not in manual joystick mode
	while (1) {
		err = LS_GetStatusAxis(status, 50);
		CheckLStepErr(stage, err);
		if (err) return STAGE_ERROR;
		
		if ( FindPattern (status, 0, strlen(status), "J", 1, 0) == -1) break;   //not in manual mode
		
		MessagePopup("", "Please put 'JOY-STICK' control to the 'AUTO' position.");
	}
	
	stage_set_description(stage, "LStep Stage");
	stage_set_name(stage, "LStep Stage");
	
	CmtReleaseLock(stage->_lock);

	return STAGE_SUCCESS;
}

int lstep_destroy(Stage* stage)
{
	LStepStage* lstep_stage = (LStepStage*) stage;
	
	lstep_power_down(stage);
	//LS_Disconnect();
	
  	free(lstep_stage);
  	
  	return STAGE_SUCCESS;
}


static int lstep_set_speed (Stage* stage, Axis axis, double speed)
{
	int err;
	
	CmtGetLock (stage->_lock);

	if(axis == ALL_AXIS)
		err = LS_SetVel(speed, speed, speed, 0.0);	
	else
		err = LS_SetVelSingleAxis(axis, speed);
	
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;
}


static int lstep_set_acceleration (Stage* stage, Axis axis, double acceleration)
{
	int err;
	
	//Send acceleration to the LStep controller in m/sec/sec
	
	CmtGetLock (stage->_lock);

	acceleration /= 1000;	//mm -> m
	
	if(axis == ALL_AXIS)
		err = LS_SetAccel(acceleration, acceleration, acceleration, 0.0);	
	else
		err = LS_SetAccelSingleAxis(axis, acceleration);
	
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	*stage->_acceleration = acceleration;

	return STAGE_SUCCESS;
}


static int lstep_get_info (Stage* stage, char *info)
{
	char serial_str[30], info_str[30], version_str[30];
	
	CmtGetLock (stage->_lock);

	LS_GetSerialNr(info_str, 30);
	
	LS_GetVersionStr(version_str, 30);
	
	CmtReleaseLock(stage->_lock);

	sprintf(info, "Serial %s\nVersion %s\n", info_str, version_str);
	
	return STAGE_SUCCESS;
}


static int lstep_self_test (Stage* stage)
{
  	return STAGE_SUCCESS;
}


static int lstep_enable_axis (Stage* stage, Axis axis)
{
	int err, enabled = 0;
	
	enabled = stage->_enabled_axis[XAXIS];
	enabled |= stage->_enabled_axis[YAXIS] << 1;
	enabled |= stage->_enabled_axis[ZAXIS] << 2;

	switch(axis) {
		case XAXIS: 
			enabled |= 1;	//set bit 0	
			break;
			
		case YAXIS:
			enabled |= 2;	//set bit 1	
			break;
	
		case ZAXIS:
			enabled |= 4;	//set bit 2	
			
		default:
			return STAGE_ERROR;	
	}
	
	CmtGetLock (stage->_lock);
	err = LS_SetActiveAxes(enabled);
	CmtReleaseLock(stage->_lock);
	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;
  	
  	return STAGE_SUCCESS;
}


static int lstep_disable_axis (Stage* stage, Axis axis)
{
	int err, enabled = 0;
	
	enabled = stage->_enabled_axis[XAXIS];
	enabled |= stage->_enabled_axis[YAXIS] << 1;
	enabled |= stage->_enabled_axis[ZAXIS] << 2;

	switch(axis) {
		case XAXIS: 
			enabled &= 0xfe;	//clear bit 0	
			break;
			
		case YAXIS:
			enabled &= 0xfd;	//clear bit 1	
			break;
	
		case ZAXIS:
			enabled &= 0xfb;	//clear bit 2	
			
		default:
			return STAGE_ERROR;	
	}
	
	CmtGetLock (stage->_lock);
	err = LS_SetActiveAxes(enabled);
	CmtReleaseLock(stage->_lock);
	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;
  	
	return STAGE_SUCCESS;	
}

	
static int lstep_async_goto_xyz_position (Stage* stage, double x, double y, double z)
{
	// Return straight away
	int err;

	CmtGetLock (stage->_lock);
	err =  LS_MoveAbs(x, y, z, 0.0, 0);
	CmtReleaseLock(stage->_lock);
	
	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}
	
static int lstep_async_goto_x_position (Stage* stage, double x)
{
	// Return straight away
	int err;

	CmtGetLock (stage->_lock);
	err =  LS_MoveAbsSingleAxis(1, x, 0);
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}
	
static int lstep_async_goto_y_position (Stage* stage, double y)
{
	// Return straight away
	int err;

	CmtGetLock (stage->_lock);
	err =  LS_MoveAbsSingleAxis(2, y, 0);
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}
	
static int lstep_async_goto_z_position (Stage* stage, double z)
{
	// Return straight away
	int err;

	CmtGetLock (stage->_lock);
	err =  LS_MoveAbsSingleAxis(3, z, 0);
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}
	

static int lstep_get_xyz_position (Stage* stage, double *x, double *y, double *z)
{
	double a;
	int err;
	
	CmtGetLock (stage->_lock);
	err = LS_GetPos(x, y, z, &a); 
	CmtReleaseLock(stage->_lock);
  	
  	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;
  	
  	return STAGE_SUCCESS;
}

	
static int lstep_async_rel_move_by (Stage* stage, double x, double y, double z)          
{
	int err;
	
	CmtGetLock (stage->_lock);
	err =  LS_MoveRel(x, y, z, 0.0, 0);
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;
}


static int lstep_set_xyz_datum (Stage* stage, double x, double y, double z)
{
	int err;
	
	CmtGetLock (stage->_lock);
	err = LS_SetPos(x, y, z, 0.0);
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}

static int lstep_set_xy_datum (Stage* stage, double x, double y)
{
	int err;
	double cur_x, cur_y, cur_z, a;
	
	CmtGetLock (stage->_lock);
	err = LS_GetPos(&cur_x, &cur_y, &cur_z, &a); 
	if(CheckLStepErr(stage, err)) {
		CmtReleaseLock(stage->_lock);
		return STAGE_ERROR;
	}
	
	err = LS_SetPos(x, y, cur_z, 0.0);
	CmtReleaseLock(stage->_lock);
	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}


static int lstep_calibrate_extents (Stage* stage, int full, double *min_x, double *min_y, double *max_x, double *max_y)
{
	int err;
	double dummy;
	
	CmtGetLock (stage->_lock);

	// Don't want Auto Status
	err = LS_SetAutoStatus(0);	
	if (CheckLStepErr(stage, err)) goto Error;
	
	//Set active axes
	err = LS_SetActiveAxes(3);	// x, y
	if (CheckLStepErr(stage, err)) goto Error;
	
	// Set units to um for moves
	err = LS_SetDimensions(1, 1, 1, 1);
	
	// Set controller mode - controller off when not moving
	err = LS_SetController(0, 0, 0, 0);	
	if (CheckLStepErr(stage, err))  goto Error;

	err = LS_SetPitch(stage->_pitch[XAXIS], stage->_pitch[YAXIS], stage->_pitch[ZAXIS], 1);
	//err = LS_SetPitch(1.0, 1.0, 1, 1);
	if (CheckLStepErr(stage, err))  goto Error;
		

	err = LS_SetMotorCurrent(stage->_motor_current[XAXIS], stage->_motor_current[YAXIS], 1, 1);
	//err = LS_SetMotorCurrent(1.0, 1.0, 1, 1);
	if (CheckLStepErr(stage, err))  goto Error;
		
	err = LS_SetReduction(stage->_current_reduction[XAXIS]/100.0, stage->_current_reduction[YAXIS]/100.0, 1, 1);
	//err = LS_SetReduction(50.0/100.0, 50.0/100.0, 1, 1);
	if (CheckLStepErr(stage, err))  goto Error;

	// Set active limit switches for x and y only
	err = LS_SetSwitchActive(5, 5, 0, 0);
	if (CheckLStepErr(stage, err))  goto Error;
		
	// Set software limits 2mm from limit switches.
	// If it's any less than this and we initialise at full speed it all goes horribly wrong.
	err = LS_SetCalibOffset(stage->_software_limit, stage->_software_limit, 2000, 2000);	// 2 mm all axes
	//err = LS_SetCalibOffset(2000, 2000, 2000, 2000);	// 2 mm all axes
	if (CheckLStepErr(stage, err))  goto Error;
		
	err = LS_SetRMOffset(stage->_software_limit, stage->_software_limit, 2000, 2000);		// 2 mm all axes
	if (CheckLStepErr(stage, err))  goto Error;
  	
	CmtReleaseLock(stage->_lock); //These two functions get the lock for themselves
  	if(stage_set_speed (stage, ALL_AXIS, DEFAULT_VEL_XY) == STAGE_ERROR)
		return STAGE_ERROR;
  	
  	if(stage_set_acceleration (stage, ALL_AXIS, DEFAULT_ACC_XY) == STAGE_ERROR)
		return STAGE_ERROR;
	CmtGetLock (stage->_lock);
		
	// Don't want the velocity to be affected by the pot
	err = LS_SetSpeedPoti(0);
	if (CheckLStepErr(stage, err))  goto Error;
		
	*min_x = *min_y = 0.0;
	*max_x = *max_y = 0.0;

	if (full) {
	  	err = LS_CalibrateEx(3);	// Calibrate x,y only
		if (CheckLStepErr(stage, err))  goto Error;
	
		err = LS_SetPos(0.0, 0.0, 0.0, 0.0);  // Make the current location the origin 
		if (CheckLStepErr(stage, err))  goto Error;

		if(stage_check_user_has_aborted(stage))   goto Error;

		err = LS_RMeasureEx(3);		// RMeasure x,y only
		if (CheckLStepErr(stage, err))  goto Error;

		if(stage_check_user_has_aborted(stage))   goto Error;

		// Get hard limits, set during calibration and rmeasure commands (values returned in um)   
		err = LS_GetPos(max_x, max_y, &dummy, &dummy);
		if (CheckLStepErr(stage, err))  goto Error;
	}
  	
	CmtReleaseLock(stage->_lock);
  	return STAGE_SUCCESS;
  	
Error:
	CmtReleaseLock(stage->_lock);
	return STAGE_ERROR;
}


static int lstep_is_moving (Stage* stage, int *status)
{
	char axis_status[50];
	int err;
	
	CmtGetLock (stage->_lock);
	err = LS_GetStatusAxis(axis_status, 50);
	CmtReleaseLock(stage->_lock);
	
	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;
	
	if ( FindPattern (axis_status, 0, strlen(axis_status), "M", 1, 0) == -1)
		*status = 0;
	else
		*status = 1;
	
  	return STAGE_SUCCESS;
}	


static int lstep_set_joystick_speed (Stage* stage, double speed)
{
	int err;
	
	CmtGetLock (stage->_lock);
	err = LS_SetVel(speed, speed, speed, 0);
	CmtReleaseLock(stage->_lock);
	
   	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;
  	
  	return STAGE_SUCCESS;
}	

	
static int lstep_set_joystick_on (Stage* stage)
{
	int err;
	
	CmtGetLock (stage->_lock);
	err = LS_SetJoystickOn(1, 1); 
	CmtReleaseLock(stage->_lock);
  	
  	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;
  	
  	return STAGE_SUCCESS;
}	

	
static int lstep_set_joystick_off (Stage* stage)
{
	int err;
	
	CmtGetLock (stage->_lock);
	err =  LS_SetJoystickOff();
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}		
	
static int lstep_abort_move (Stage* stage)
{
	int err;
	
	CmtGetLock (stage->_lock);
	err =  LS_StopAxes();
	CmtReleaseLock(stage->_lock);

	if(CheckLStepErr(stage, err))
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}		
	
	
static int lstep_set_timeout (Stage* stage, double timeout)
{
	//printf("lstep_set_timeout\n");
  	
  	return STAGE_SUCCESS;
}	


static  int lstep_save_settings (Stage* stage, const char *filename)
{
	
  	return STAGE_SUCCESS;
}	


static int lstep_load_settings (Stage* stage, const char *filename)
{
  	
  	return STAGE_SUCCESS;
}	

	
Stage* lstep_stage_new(void)
{
	Stage* stage = stage_new();

	lstep_ops.init = lstep_init;
	lstep_ops.destroy = lstep_destroy;
	lstep_ops.power_up = lstep_power_up;
	lstep_ops.power_down = lstep_power_down;
	lstep_ops.reset = lstep_reset;
	lstep_ops.set_baud_rate = lstep_set_baud_rate;
	lstep_ops.get_info = lstep_get_info;
	lstep_ops.self_test = lstep_self_test;
	lstep_ops.enable_axis = lstep_enable_axis;
	lstep_ops.disable_axis = lstep_disable_axis;
	lstep_ops.set_speed = lstep_set_speed;
	lstep_ops.set_acceleration = lstep_set_acceleration;
	lstep_ops.calibrate_extents = lstep_calibrate_extents;
  	lstep_ops.async_goto_xyz_position = lstep_async_goto_xyz_position;
  	lstep_ops.async_goto_x_position = lstep_async_goto_x_position;
  	lstep_ops.async_goto_y_position = lstep_async_goto_y_position;
  	lstep_ops.async_goto_z_position = lstep_async_goto_z_position;

	lstep_ops.get_xyz_position = lstep_get_xyz_position;
	lstep_ops.async_rel_move_by = lstep_async_rel_move_by;
	lstep_ops.set_xyz_datum = lstep_set_xyz_datum;
	lstep_ops.set_xy_datum = lstep_set_xy_datum;
	lstep_ops.is_moving = lstep_is_moving;
	lstep_ops.set_joystick_speed = lstep_set_joystick_speed;
	lstep_ops.set_joystick_on = lstep_set_joystick_on;
	lstep_ops.set_joystick_off = lstep_set_joystick_off;
	lstep_ops.abort_move = lstep_abort_move;
	lstep_ops.set_timeout = lstep_set_timeout;
	lstep_ops.save_settings = lstep_save_settings; 
	lstep_ops.load_settings = lstep_load_settings; 
	
	stage_set_operations(stage, &lstep_ops);

	return stage;
}


