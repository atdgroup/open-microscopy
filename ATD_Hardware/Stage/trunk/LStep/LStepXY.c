#include "gci_utils.h"
#include "status.h"

#include "profile.h"
#include "stage\stage_ui.h"
#include "LStep\LStep4.h"
#include "LStep\LStepXY.h"
#include "LStep\LStep_UserInterface.h"

#include "iniparser.h"
#include <rs232.h>
#include <utility.h>
#include <formatio.h>

/////////////////////////////////////////////////////////////////////////////
// Test program for LStep with Marzhauser stage - RJL Sept 2001
// Adapted from Graham West's program for the MC 2000 controller
//
// I had to modify LStep4.h to remove all the c++ bits and make an include
// library, LStep4.lib for cvi from the DLL. 
/////////////////////////////////////////////////////////////////////////////
//Mods - v1.2 -> v1.3
//Rosalind Locke - July 2003
//Added controls such that acceleration, speed and backlash are visible for 
//all three axes. Found that for diagonal moves it always uses the x settings
//if x and y are different.
//Very slow moves caused problems because the software gives up waiting after
//30 seconds. Therefore need to calculate all the move times.
////////////////////////////////////////////////////////////////////////////
// RJL June 2004
// Added XY direction reversal
// RJL April 2004
// Bug in xy moves resulted in uneccesary backlash moves
/////////////////////////////////////////////////////////////////////////////

//NOTES:

//1) All position coords displayed are given relative to the datum
//2) The z-drive is disabled during calibration (no limits)
//3) All position and move values are given in um
//4) The datum position is given relative to the bottom right hand corner of the stage
//5) The region of interest is given in terms of datum position and lengths of each side
//6) The range of the stage was found to be approx 100mm (Y) by 124mm (X)
//7) Backlash (X and Y directions) - always approach from the origin direction, ie increasing values of distance
//8) Z-axis - initially setting to no limits option.  If z drive present then set to limits option
//9) If any axis drive not present then disable that axis 
//10) Stage has 1mm pitch ie one revolution = 1mm in x and y
//11) Velocity range xy = 1um/sec to 40mm/sec, max acceleration = 800mm/sec/sec)
//12) Velocity range z = 1um/sec to 40mm/sec, max acceleration = 2300mm/sec/sec)

char* ErrorStrings[] = {
					"Valid axis designation missing",
					"Non-executable function",
					"Command string is too long",
					"Invalid command",
					"Number outside valid range",
					"Incorrect number of parameters",
					"None ! or ?",
					"TVR not possible because axis is active",
					"Axes cannot be switched on or off bescause TVR is active",
					"Function not configured",
					"Joystick is set to manual",
					"Limit switch tripped",
					"Internal error",
					"Internal error",
					"Undefined error",
					"Interface type unknown",
					"Cannot access controller. Is it switched on?",
					"Lost communication with controller",
					"Timeout while reading from controller",
					"Command transmission error",
					"Command terminated",
					"Command not supported",
					"Joystick is set to Manual",
					"Joystick is set to Manual",
					"Controller Timeout"
				 	};

int ErrorValueToString(int error, char *string)
{
	if(error == 0)
		return -1;
		
	if(error >= 1 && error <= 12) {
		strcpy(string, ErrorStrings[error - 1]);
		return error - 1;
	}	
		
	if(error >= 4001 && error <= 4013) {
		strcpy(string, ErrorStrings[error - 3989]); 
		return error - 3989;
	}
		
	strcpy(string, "Unknown Error");          
	
	return -1;
}

void LogLStepError(int val)
{
	char buffer[500];
	
	ErrorValueToString(val, buffer);  
	
	MessagePopup("LStep Error", buffer);
}

void lstep_update_ui (XYStage* stage)
{
	LStepXYStage *this = (LStepXYStage *) stage;     

	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_X_ENABLED,  stage->_enabled_axis[XAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_X_SPEED,    stage->_speed[XAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_X_ACC,      stage->_acceleration[XAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_BACKLASH_X, this->_backlash[XAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_X_PITCH,    this->_pitch[XAXIS]);	

	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_Y_ENABLED,  stage->_enabled_axis[YAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_Y_SPEED,    stage->_speed[YAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_Y_ACC,      stage->_acceleration[YAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_BACKLASH_Y, this->_backlash[YAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_Y_PITCH,    this->_pitch[YAXIS]);	
				
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_BACKLASH_X, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_X_PITCH, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_X_SPEED, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_X_ACC, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_TEXTMSG_15, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_TEXTMSG_2, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_TEXTMSG_9, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);

	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_BACKLASH_Y, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_Y_PITCH, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_Y_SPEED, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_Y_ACC, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_TEXTMSG_17, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_TEXTMSG_18, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, LS_PARAMS_TEXTMSG_22, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
}

static int lstep_get_info_from_controller (XYStage* stage)
{
	int err;
	char serial[500];
	char version[500];
	
	LStepXYStage *this = (LStepXYStage *) stage;  
	
	// Check RS232 line by sending request for Version number
	if((err = LS_GetSerialNr(serial, 64)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
	
	if((err = LS_GetVersionStr(version, 64)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}

	sprintf(this->_info, "Serial %s Version %s", serial, version);
	
	return STAGE_SUCCESS;
}

int lstep_init_comport(int port)
{
	//Initialise rs232 communications
	int err;
	char version[64];
	char comport_name[10];
	
	//Let us have our error messages in English
	if((err = LS_SetLanguage("ENG")) > 0) {
		LogLStepError(err);
		return -1;	
	}
	
	sprintf(comport_name, "COM%d", port);
	
	if((err = LS_ConnectSimple(1, comport_name, 9600, 0)) > 0) {
		LogLStepError(err);
		return -1;	
	}
	
	// Check RS232 line by sending request for Version number
	if((err = LS_GetVersionStr(version, 64)) > 0) {
		LogLStepError(err);
		return -1;	
	}

	SetComTime(port, 10); 
		
	return 0;	
}


int lstep_xy_hw_init (XYStage* stage)
{
	LStepXYStage *this = (LStepXYStage *) stage;     
	int err;
	double start;
	char status[50]="";     
	
	CmtGetLock (this->_lock);   
	
	if(lstep_init_comport(this->_com_port) != 0)
		goto STAGE_INIT_ERROR;     	
	
	//SetComTime(this->_com_port, 5.0);
	
	// This has to be done at 9600 baud
	if((err = LS_SoftwareReset()) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
	
	// Wait for "OK"
	start = Timer();
	while (strncmp (status, "OK", 2)) {
		LS_GetStatus(status, 20);
		if ((Timer() - start) > 10.0) break;
	}

	//Check that it's not in manual joystick mode
	while (1) {
	
		if((err = LS_GetStatusAxis(status, 50)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;	
		}
	
		if ( FindPattern (status, 0, strlen(status), "J", 1, 0) == -1)
			break;   //not in manual mode
		
		MessagePopup("", "Please put joystick 'CONTROL' switch to the 'AUTO' position.");
		
		ProcessSystemEvents();
	}
	
	lstep_get_info_from_controller (stage);

	//Don't want Auto Status
	if((err = LS_SetAutoStatus(0)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
		
	//Set active axis X, Y, Z
	if((err = LS_SetActiveAxes(7)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
	
	//Set units to um for moves. um all axes 
	if((err = LS_SetDimensions(1, 1, 1, 1)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
	
	//Set active limit switches for x and y only
	if((err = LS_SetSwitchActive(5, 5, 0, 0)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
	
	
	//Don't want the velocity to be affected by the pot
	if((err = LS_SetSpeedPoti(0)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
	
/*	
	//Set software limits 2mm from limit switches.
	//If it's any less than this and we initialise at full speed it all goes horribly wrong.
	err = LS_SetCalibOffset(2000, 2000, 2000, 2000);	//2 mm all axes
	GCI_CheckLstepErr(err);
	if (err) return -1;
	
	err = LS_SetRMOffset(2000, 2000, 2000, 2000);		//2 mm all axes
	GCI_CheckLstepErr(err);
	if (err) return -1;
*/

	stage_load_default_settings(stage);
	
	// set joystick speed (2 mm/sec)   
	stage_set_joystick_speed (stage, 2.0);  
	
	CmtReleaseLock(this->_lock);    
	
	return STAGE_SUCCESS;
	
	STAGE_INIT_ERROR:
	
	CmtReleaseLock(this->_lock); 
		
	return STAGE_ERROR; 
}


int lstep_destroy(XYStage* stage)
{
	LStepXYStage *this = (LStepXYStage *) stage;       
	
	LS_Disconnect(); 
	
	CmtDiscardLock(this->_lock);     
	
  	return STAGE_SUCCESS;
}

static int lstep_get_pitch (XYStage* stage, Axis axis, double* pitch)
{
	int err = 0;
	double x, y, z, a;
	
	LStepXYStage *this = (LStepXYStage *) stage;  
	
	if((err = LS_GetPitch(&x, &y, &z, &a)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;
	}

	if (err)
        return STAGE_ERROR;
	
	if(axis == XAXIS)
		*pitch = x;
	else if(axis == YAXIS)
		*pitch = y;
	
	return STAGE_SUCCESS;
}


static int lstep_set_pitch (XYStage* stage, Axis axis, double pitch)
{
	int err;

	double x, y, z, a;

	LStepXYStage *this = (LStepXYStage *) stage;  
	
	if((err = LS_GetPitch(&x, &y, &z, &a)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;
	}
	
	if(axis == XAXIS) {
	
		if((err = LS_SetPitch(pitch, y, z, 1)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}
	}
	else if(axis == YAXIS) {
	
		if((err = LS_SetPitch(x, pitch, z, 1)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}
	}

	return STAGE_SUCCESS;
}

static int lstep_set_speed (XYStage* stage, Axis axis, double speed)
{
	int err;

	LStepXYStage *this = (LStepXYStage *) stage;  
	
	if(axis != XAXIS && axis != YAXIS && axis != ZAXIS && axis != ALL_AXIS)
		return STAGE_ERROR;
	
	if(axis == ALL_AXIS) {
	
		if((err = LS_SetVelSingleAxis(1, speed)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}
		
		if((err = LS_SetVelSingleAxis(2, speed)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}
				
		return STAGE_SUCCESS;    
	}
	else {
		
		if((err = LS_SetVelSingleAxis(axis, speed)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}
	}
		
	return STAGE_SUCCESS;
}

static int lstep_get_speed (XYStage* stage, Axis axis, double *speed)
{
	int err;
	LStepXYStage *this = (LStepXYStage *) stage;  

	double x, y, z, r;
	
	if((err = LS_GetVel(&x, &y, &z, &r)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;
	}

	if(axis == XAXIS)
		*speed = x;
	else if(axis == YAXIS)
		*speed = y;
	else if(axis == ZAXIS)
		*speed = z;

	return STAGE_SUCCESS;
}

// Acceleration is passed as mm / s^2
static int lstep_set_acceleration (XYStage* stage, Axis axis, double acceleration)
{
	int err;
	LStepXYStage *this = (LStepXYStage *) stage;  
	
	if(axis != XAXIS && axis != YAXIS && axis != ZAXIS && axis != ALL_AXIS)   
		return STAGE_ERROR;
	
	if(axis == ALL_AXIS) {
	
		if((err = LS_SetAccelSingleAxis(1, acceleration / 1000.0)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}
		
		if((err = LS_SetAccelSingleAxis(2, acceleration / 1000.0)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}

		if((err = LS_SetAccelSingleAxis(3, acceleration / 1000.0)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}
		
		return STAGE_SUCCESS;    
	}
	else {
		
		if((err = LS_SetAccelSingleAxis(axis, acceleration / 1000.0)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;
		}
	}
		
	return STAGE_SUCCESS;
}

static int lstep_get_acceleration (XYStage* stage, Axis axis, double *acceleration)
{
	int err;
	LStepXYStage *this = (LStepXYStage *) stage;  
	
	double x, y, z, r;
	
	if((err = LS_GetAccel(&x, &y, &z, &r)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;
	}
	
	// Convert to mm /s ^2
	if(axis == XAXIS)
		*acceleration = (x * 1000.0);
	else if(axis == YAXIS)
		*acceleration = (y * 1000.0);
	else if(axis == ZAXIS)
		*acceleration = (z * 1000.0);

	return STAGE_SUCCESS;
}

static int lstep_get_info (XYStage* stage, char *info)
{
	int err;
	char serial[500];
	char version[500];
	
	LStepXYStage *this = (LStepXYStage *) stage;  

	strcpy(info, this->_info, 199);
	
	return STAGE_SUCCESS;
}


int lstep_get_xyz_position (XYStage* stage, double *x, double *y, double *z)
{
	double dummy;
	int err;
	
	//Read returned coordinate values (x, y and z).
	if((err = LS_GetPos(x, y, z, &dummy)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;		
	}
	
  	return STAGE_SUCCESS;
}

	
static int lstep_set_xyz_datum (XYStage* stage, double x, double y, double z)
{
	// set x,y,f as the logical origin
	int err;
	LStepXYStage *this = (LStepXYStage *) stage;  
	
	if((err= LS_SetPos(0.0, 0.0, 0.0, 0.0)) > 0) {  // Make the current location the origin 
		LogLStepError(err);
		return STAGE_ERROR;	
	}	

	return STAGE_SUCCESS;
}


static int lstep_calibrate_extents (XYStage* stage, double *min_x, double *min_y,
		                             double *max_x, double *max_y)
{
	int err, joyOn;
	double xStageMin, yStageMin, zStageMin, dummy;
	double xStageMax, yStageMax, zStageMax; 
	double xspeed, yspeed, xaccel, yaccel;
	LStepXYStage *this = (LStepXYStage *) stage;     

	CmtGetLock (this->_lock); 
	
	joyOn = stage_get_joystick_status(stage);
	if(joyOn)
		stage_set_joystick_off(stage);
	
	//Need a long timeout for stage initialisation.
	//SetComTime (this->_com_port, 20.0); 
	
	stage_get_speed(stage, XAXIS, &xspeed);
	stage_get_speed(stage, YAXIS, &yspeed);     
	stage_get_acceleration(stage, XAXIS, &xaccel);    
	stage_get_acceleration(stage, YAXIS, &yaccel);    
	
	stage_set_speed(stage, ALL_AXIS, this->_cal_velocity);
	stage_set_acceleration(stage, ALL_AXIS, this->_cal_acceleration);
		
	if((err = LS_CalibrateEx(3)) > 0) {	// Calibrate x,y only
		LogLStepError(err);
		goto Error;
	}
	
	if((err= LS_SetPos(0.0, 0.0, 0.0, 0.0)) > 0) {  // Make the current location the origin 
		LogLStepError(err);
		goto Error;;	
	}
	
	if((err = LS_RMeasureEx(3)) > 0) {		// RMeasure x,y only   > 0)	
		LogLStepError(err);
		goto Error;
	}
	
	// Return previous speed and acceleration values.
	stage_set_speed(stage, XAXIS, xspeed);
	stage_set_speed(stage, YAXIS, yspeed);     
	stage_set_acceleration(stage, XAXIS, xaccel);
	stage_set_acceleration(stage, YAXIS, yaccel);  
	
	// Get hard limits, set during calibration and rmeasure commands (values returned in um)   
	xStageMin = 0.0; yStageMin = 0.0; zStageMin = 0.0;
	
	if((err = LS_GetPos(&xStageMax, &yStageMax, &zStageMax, &dummy)) > 0) {	// Calibrate x,y only
		LogLStepError(err);
		goto Error;
	}

	// Go to centre and call it zero
	if((err = LS_MoveAbs(xStageMax/2.0, yStageMax/2.0, 0.0, 0.0, 1)) > 0) {   //Move stage to middle
		LogLStepError(err);
		goto Error;
	}

	// Make the current location the origin 
	if((err= LS_SetPos(0.0, 0.0, 0.0, 0.0)) > 0) {  // Make the current location the origin 
		LogLStepError(err);
		goto Error;
	}
	
	//SetComTime (this->_com_port, 5.0);  
	CmtReleaseLock (this->_lock);    

	*min_x = -xStageMax/2;
	*min_y = -yStageMax/2;
	*max_x = xStageMax/2;
	*max_y = yStageMax/2;

	if (joyOn)
		stage_set_joystick_on(stage);

  	return STAGE_SUCCESS;
  	
Error:
	
	CmtReleaseLock (this->_lock); 
	//SetComTime (this->_com_port, 5.0);  
	
	return STAGE_ERROR;
}


static int lstep_is_moving (XYStage* stage, int *status)
{
	int err;
	char str_status[50] = "";
	
	LStepXYStage *this = (LStepXYStage *) stage;
	
	if((err = LS_GetStatusAxis(str_status, 50)) > 0) { 
		LogLStepError(err);
		*status = 0;
		return STAGE_ERROR;
	}
	
	if ( FindPattern (str_status, 0, strlen(str_status), "M", 1, 0) == -1) {
		*status = 0;    
		return STAGE_SUCCESS;
	}
	
	*status = 1;
  	return STAGE_SUCCESS;
}	

//////////////////////////////////////////////////////////////////////////
static int lstep_set_joystick_speed (XYStage* stage, double speed)
{
	int err;
	LStepXYStage *this = (LStepXYStage *) stage;     

	if (stage->_joystick_status==0) { // joystick should be on to do this, but return success so the speed is saved for when needed (i.e. when joy stick is turned on)
		return STAGE_SUCCESS;
	}
	
	if((err = LS_SetVel (speed, speed, stage->_speed[ZAXIS], 0)) > 0) { 
		LogLStepError(err);
		return STAGE_ERROR;
	} 

  	return STAGE_SUCCESS;
}	

static int lstep_get_joystick_speed (XYStage* stage, double* speed)
{
	int err;
	double dummy;
	LStepXYStage *this = (LStepXYStage *) stage;  
	
	*speed = stage->_joystick_speed;
	
	return STAGE_SUCCESS;
}	
	
static int lstep_set_joystick_on (XYStage* stage)
{
	double xvel, yvel, zvel, dummy;
	int err;

	// set joystick speed	
	if((err = LS_SetVel (stage->_joystick_speed, stage->_joystick_speed, stage->_speed[ZAXIS], 0)) > 0) { 
		LogLStepError(err);
		return STAGE_ERROR;
	} 		

	if((err = LS_SetJoystickOn(1,1)) > 0) {
		LogLStepError(err);      
	}
  	
	return STAGE_SUCCESS;
}	

	
static int lstep_set_joystick_off (XYStage* stage)
{
	int err = 0;

	// set normal speed	
	if((err = LS_SetVel (stage->_speed[XAXIS], stage->_speed[YAXIS], stage->_speed[ZAXIS], 0)) > 0) { 
		LogLStepError(err);
		return STAGE_ERROR;
	} 		

	if((err = LS_SetJoystickOff()) > 0) {
		LogLStepError(err);      
	}
    	
	return STAGE_SUCCESS;	
}		
	

static int lstep_async_rel_move_by (XYStage* stage, double x, double y)          
{
	int err, timed_out = 0, backlash_applied_x = 0, backlash_applied_y = 0, joyOn;
	double second_x_move = 0.0, second_y_move = 0.0, minimum_move = 0.1; //in um, to account for rounding in lstep controller
	LStepXYStage *this = (LStepXYStage *) stage;  
	
	joyOn = stage_get_joystick_status (stage);
	if (joyOn)
		stage_set_joystick_off (stage);   
	
	//for negative moves add in the backlash
	if ((x < 0) && (abs(x) >= 0.1)) {
		backlash_applied_x = 1;
 		x -= this->_backlash[XAXIS]; 
	}
	if ((y < 0) && (abs(y) >= 0.1)) {
		backlash_applied_y = 1;
		y -= this->_backlash[YAXIS];
	}

	//perform the resultant move
	if((err = LS_MoveRel(x, y, 0, 0, 0)) > 0) {
		LogLStepError(err);
		return STAGE_ERROR;	
	}
	
	if((err = LS_WaitForAxisStop (7, this->_timeout, &timed_out)) > 0) {
		LogLStepError(err);  
		return -1;  
	}

	if(timed_out) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "stage failed to move");   
		return -1;
	}

	if(backlash_applied_x || backlash_applied_y) { //if backlash has been applied therefore a second move necessary
		second_x_move = (double)backlash_applied_x*this->_backlash[XAXIS]; 	
		second_y_move = (double)backlash_applied_y*this->_backlash[YAXIS]; 
		//perform the resultant move
		if((err = LS_MoveRel(second_x_move, second_y_move, 0, 0, 0)) > 0) {
			LogLStepError(err);
			return STAGE_ERROR;	
		}
	}

	if (joyOn) 
		stage_set_joystick_on (stage);   
	
	return STAGE_SUCCESS;
}

static int lstep_abort_move (XYStage* stage)
{
	//LStepXYStage *this = (LStepXYStage *) stage;     
	
	LS_SetAbortFlag();
	LS_StopAxes();
	LS_SoftwareReset();
	
	return STAGE_SUCCESS;	
}		
	

static void get_backlash_correction(XYStage *stage, double *x, double *y, double *z)
{
	LStepXYStage *this = (LStepXYStage *) stage;
	double last_x = 0.0, last_y = 0.0, last_z = 0.0;

	lstep_get_xyz_position (stage, &last_x, &last_y, &last_z);

	//Check to see if backlash compensation if required
	if (FP_Compare (this->_backlash[XAXIS], 0.0) != 0) {	//backlash X is not zero
		//If moving in negative direction do backlash
		if (FP_Compare (last_x, *x) == 1)
			if ((last_x - *x) >= 0.1)
				*x -= this->_backlash[XAXIS]; 
	}

	if (FP_Compare (this->_backlash[YAXIS], 0.0) != 0) {	//backlashY is not zero
		//If moving in negative direction do backlash
		if (FP_Compare (last_y, *y) == 1)
			if ((last_y - *y) >= 0.1)
				*y -= this->_backlash[YAXIS];
	}

	if (FP_Compare (this->_backlash[ZAXIS], 0.0) != 0) {	//backlashY is not zero
		//If moving in negative direction do backlash
		if (FP_Compare (last_z, *z) == 1)
			if ((last_z - *z) >= 0.1)
				*z -= this->_backlash[ZAXIS];
	}
	return;
}

static int lstep_goto_xyz_position (XYStage* stage, double x, double y, double z)
{
	int i, moving = 0, timed_out = 0, err;
	double target_x, target_y, target_z;
	LStepXYStage *this = (LStepXYStage *) stage;
	int joyOn;
	
	joyOn = stage_get_joystick_status(stage);
	if(joyOn)
		stage_set_joystick_off (stage); //otherwise stage will not move

	target_x = x;
	target_y = y;
	target_z = z;
	
	get_backlash_correction(stage, &x, &y, &z);

	//move to target plus backlash position, a 1 as final parameter to LS_MoveAbs means wait for move to finish
	if((err = LS_MoveAbs(x, y, z, 0.0, 1)) > 0) {
		LogLStepError(err);
		if(joyOn)
			stage_set_joystick_on (stage);
		return STAGE_ERROR;  
	}

	if( (x == target_x) && (y == target_y) && (z == target_z)) { //no need for second move
		if(joyOn)
			stage_set_joystick_on (stage);
		return STAGE_SUCCESS;
	}

	//move to target position
	if((err = LS_MoveAbs(target_x, target_y, target_z, 0.0, 1)) > 0) {
		LogLStepError(err);  
		if(joyOn)
			stage_set_joystick_on (stage);
		return STAGE_ERROR;  
	}

	if(joyOn)
		stage_set_joystick_on (stage);

	return STAGE_SUCCESS;

/*  Old code from Stuart

	// This move function errors about the controller being in manual mode ?
	// Originally I was passing the wait parameter to LS_MoveAbs but his 
	// sometimes failed sometimes and did not return.
	
	get_backlash_correction(stage, &x, &y, &z);

	for(i=0; i < 3; i++) {
		
		//move to target plus backlash position
		if((err = LS_MoveAbs(x, y, z, 0.0, 1)) > 0) {
			LogLStepError(err);
			if(joyOn)
				stage_set_joystick_on (stage);
			return -1;  
		}
	
		ProcessSystemEvents();

		if((err = LS_WaitForAxisStop (7, this->_timeout, &timed_out)) > 0) {
			LogLStepError(err);  
			if(joyOn)
				stage_set_joystick_on (stage);
			return -1;  
		}

		if(timed_out) {
			logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "stage failed to move");   
		}
		else
			break;

	}

	if(timed_out) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "stage failed to move three times");   
		if(joyOn)
			stage_set_joystick_on (stage);
		return -1;
	}

	if( (x == target_x) && (y == target_y) && (z == target_z)) { //no need for second move
		if(joyOn)
			stage_set_joystick_on (stage);
		return 0;
	}

	for(i=0; i < 3; i++) {
		
		//move to target position
		if((err = LS_MoveAbs(target_x, target_y, target_z, 0.0, 1)) > 0) {
			LogLStepError(err);  
			if(joyOn)
				stage_set_joystick_on (stage);
			return -1;  
		}
	
		ProcessSystemEvents();

		if((err = LS_WaitForAxisStop (7, this->_timeout, &timed_out)) > 0) {
			LogLStepError(err);  
			if(joyOn)
				stage_set_joystick_on (stage);
			return -1;  
		}

		if(timed_out) {
			logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "stage failed to move");   
		}
		else {
			if(joyOn)
				stage_set_joystick_on (stage);
			return 0;
		}
		
	}

	if(timed_out) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "stage failed to move three times");   
		if(joyOn)
				stage_set_joystick_on (stage);
		return -1;
	}

	if(joyOn)
		stage_set_joystick_on (stage);

	return 0; 
	*/
}

static int lstep_goto_xy_position (XYStage* stage, double x, double y)
{
	LStepXYStage *this = (LStepXYStage *) stage;
	int i, moving = 0, timed_out = 0, err;
	double z, dummy;

	static double last_x = 0.0, last_y = 0.0;
	double t = 0.0;
	double target_x, target_y;
	int joyOn;

	stage_get_z_position(stage, &z);
	STAGE_CORRECT_VAL_FOR_ORIENTATION(stage, z, ZAXIS);

	return lstep_goto_xyz_position (stage, x, y, z);

	/*  Old code from Stuart

	joyOn = stage_get_joystick_status(stage);
	if(joyOn)
		stage_set_joystick_off (stage); //otherwise stage will not move

	stage_get_z_position(stage, &z);

	target_x = x;
	target_y = y;

	get_backlash_correction(stage, &x, &y, &dummy);

	PROFILE_START("lstep_goto_xy_position");

	// This move function errors about the controller being in manual mode ?
	// Originally I was passing the wait parameter to LS_MoveAbs but his 
	// sometimes failed sometimes and did not return.

	for(i=0; i < 3; i++) {
		
		//move to target plus backlash position
		if((err = LS_MoveAbs(x, y, z, 0.0, 1)) > 0) {
			LogLStepError(err);  
			PROFILE_STOP("lstep_goto_xy_position"); 
			if(joyOn)
				stage_set_joystick_on (stage);
			return -1;  
		}

		ProcessSystemEvents();
	
		PROFILE_START("LS_WaitForAxisStop"); 

		if((err = LS_WaitForAxisStop (5, this->_timeout, &timed_out)) > 0) {
			LogLStepError(err);  
			PROFILE_STOP("LS_WaitForAxisStop"); 
			PROFILE_STOP("lstep_goto_xy_position"); 
			if(joyOn)
				stage_set_joystick_on (stage);
			return -1;  
		}

		PROFILE_STOP("LS_WaitForAxisStop"); 

		if(timed_out) {
			logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "timeout occurred, stage failed to move");   
		}
		else {
			PROFILE_STOP("lstep_goto_xy_position"); 
			break;
//			return 0;
		}
		
	}

	if(timed_out) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "stage failed to move three times");  
		PROFILE_STOP("lstep_goto_xy_position"); 
		if(joyOn)
			stage_set_joystick_on (stage);
		return -1;
	}

	PROFILE_STOP("lstep_goto_xy_position"); 
	
	if( (x == target_x) && (y == target_y)) { //no need for second move
		if(joyOn)
			stage_set_joystick_on (stage);
		return 0;
	}

	for(i=0; i < 3; i++) {
		
		//move to target position
		if((err = LS_MoveAbs(target_x, target_y, z, 0.0, 1)) > 0) {
			LogLStepError(err);  
			if(joyOn)
				stage_set_joystick_on (stage);
			return -1;  
		}
	
		ProcessSystemEvents();

		if((err = LS_WaitForAxisStop (7, 10000, &timed_out)) > 0) {
			LogLStepError(err);  
			if(joyOn)
				stage_set_joystick_on (stage);
			return -1;  
		}

		if(timed_out) {
			logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "stage failed to move");   
		}
		else {
			if(joyOn)
				stage_set_joystick_on (stage);
			return 0;
		}
		
	}

	if(timed_out) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "stage failed to move three times");   
		if(joyOn)
			stage_set_joystick_on (stage);
		return -1;
	}

	if(joyOn)
		stage_set_joystick_on (stage);



	return 0;  	
*/
}

static int lstep_async_goto_xyz_position (LStepXYStage *lstep_stage, double x, double y, double z)
{
	// Return straight away
	int err, was_on = 0, joyOn;
	double target_x, target_y, target_z;

	XYStage* stage = (XYStage*) lstep_stage;

	// cannot do async as we want to do backlash
	lstep_goto_xyz_position (stage, x, y,z);
	return STAGE_SUCCESS;

	
	/*  Old code from Stuart
	joyOn = stage_get_joystick_status(stage);
	if(joyOn)
		stage_set_joystick_off (stage); //otherwise stage will not move

/////////////////////////////////////////////////////// remove!
//	lstep_goto_xyz_position (stage, x, y,z);
//	return STAGE_SUCCESS;	
//////////////////////////////////////////////////////  remove!

	target_x = x;
	target_y = y;
	target_z = z;

	get_backlash_correction(stage, &x, &y, &z);

	//move to target plus backlash position
	if((err = LS_MoveAbs(x, y, z, 0, 0)) > 0) {
		LogLStepError(err);  
		if(joyOn)
			stage_set_joystick_on (stage);
		return STAGE_ERROR;
	}

	//move to target position
	if((err = LS_MoveAbs(target_x, target_y, target_z, 0, 0)) > 0) {
		LogLStepError(err);   
		if(joyOn)
			stage_set_joystick_on (stage);
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
	*/
}

static int lstep_async_goto_xy_position (XYStage* stage, double x, double y)
{
	// Return straight away
	double read_x, read_y, read_z, dummy;
	LStepXYStage *this = (LStepXYStage *) stage;

	// cannot do async as we want to do backlash
	stage_get_z_position(stage, &read_z);
	STAGE_CORRECT_VAL_FOR_ORIENTATION(stage, read_z, ZAXIS);
	return lstep_goto_xyz_position (stage, x, y, read_z);


/*
	LS_GetPos(&read_x, &read_y, &read_z, &dummy);
	
	if(lstep_async_goto_xyz_position (this, x, y, read_z) == STAGE_ERROR) {
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;	
*/
}

static int lstep_set_xy_datum (XYStage* stage, double x, double y)
{
	int err;
	LStepXYStage *this = (LStepXYStage *) stage;   
	
	// Make the current location the origin 
	if((err = LS_SetPos(x, y, 0.0, 0.0)) > 0) {  // Make the current location the origin 
		LogLStepError(err);
		return STAGE_ERROR;   
	}
	
	return STAGE_SUCCESS;	
}

int lstep_set_axis_enable(XYStage* stage, Axis axis, int enable)
{
	LStepXYStage *this = (LStepXYStage *) stage;   

//	int err =  RS232CorvusSend(this, "%d %d setaxis\n", enable, axis); 
	
//	if (err)
//		return STAGE_ERROR;

	// the setaxis command diables the joystick
//	if (stage->_joystick_status == 1) stage_set_joystick_on (stage);
	
//	lstep_update_ui(stage);
	
	return STAGE_SUCCESS;	
}


int lstep_save_settings (XYStage* stage, const char *filepath)
{
	LStepXYStage *this = (LStepXYStage *) stage;  
	FILE *fd;
	dictionary *d = dictionary_new(20);
	
	fd = fopen(filepath, "w");
	
	// get generic settings
	stage_save_data_to_dictionary(stage, d);
	
	dictionary_setdouble(d, "Calibration Velocity", this->_cal_velocity);  
	dictionary_setdouble(d, "Calibration Acceleration", this->_cal_acceleration); 
	
	// get specific settings
    dictionary_setdouble(d, "X Axis Pitch",    this->_pitch[XAXIS]);
    dictionary_setdouble(d, "Y Axis Pitch",    this->_pitch[YAXIS]);
    dictionary_setdouble(d, "X Axis Backlash", this->_backlash[XAXIS]);
    dictionary_setdouble(d, "Y Axis Backlash", this->_backlash[YAXIS]);
    dictionary_setdouble(d, "Z Axis Backlash", this->_backlash[ZAXIS]);
	
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return STAGE_SUCCESS;	      
}

int lstep_load_settings (XYStage* stage, const char *filepath)
{
	LStepXYStage *this = (LStepXYStage *) stage;  
	dictionary* d = NULL;
	int file_size;
	
	if(!FileExists(filepath, &file_size))
		return STAGE_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {

		// generic settings
		stage_load_data_from_dictionary(stage, d); 
 
		this->_cal_velocity = dictionary_getdouble(d, "Stage:Calibration Velocity", 20);  
		this->_cal_acceleration = dictionary_getdouble(d, "Stage:Calibration Acceleration", 0.5); 

		// specific settings
	    this->_backlash[XAXIS] = dictionary_getdouble(d, "Stage:X Axis Backlash", 50);	 // 50 microns
	    this->_pitch[XAXIS]    = dictionary_getdouble(d, "Stage:X Axis Pitch", 4.0);
	    this->_backlash[YAXIS] = dictionary_getdouble(d, "Stage:Y Axis Backlash", 50);	 // 50 microns
	    this->_pitch[YAXIS]    = dictionary_getdouble(d, "Stage:Y Axis Pitch", 4.0);
	    this->_backlash[ZAXIS] = dictionary_getdouble(d, "Stage:Z Axis Backlash", 0);	 // 0 microns

		stage_set_pitch(stage, XAXIS, this->_pitch[XAXIS]);
		stage_set_pitch(stage, YAXIS, this->_pitch[YAXIS]);
		
		lstep_set_axis_enable(stage, XAXIS, stage->_enabled_axis[XAXIS]);
		lstep_set_axis_enable(stage, YAXIS, stage->_enabled_axis[YAXIS]);
		
		dictionary_del(d);
	}
	
	lstep_update_ui(stage);
	
	return STAGE_SUCCESS;	  
}
	
static int lstep_set_timeout(XYStage *stage, double timeout)
{
	LStepXYStage *this = (LStepXYStage *) stage;  
	
	// timeout will be in seconds but lstep timeout expects milliseconds
	this->_timeout = timeout * 1000.0;
	
	return STAGE_SUCCESS;
}

int lstep_init (XYStage *stage)
{
	lstep_update_ui(stage);

	return STAGE_SUCCESS;
}

XYStage* lstep_xy_stage_new(const char* name, const char* description, UI_MODULE_ERROR_HANDLER error_handler, void *data, const char *data_dir)
{
	LStepXYStage* this = (LStepXYStage *) malloc(sizeof(LStepXYStage));
	XYStage *stage = (XYStage*) this;

	stage_constructor(stage, name);

	CmtNewLock(NULL, 0, &(this->_lock));
	
	ui_module_set_description(UIMODULE_CAST(stage), description);  
	ui_module_set_error_handler(UIMODULE_CAST(stage), error_handler, data); 
	ui_module_set_data_dir(UIMODULE_CAST(stage), data_dir);   
	
	stage->_params_ui_panel = ui_module_add_panel(UIMODULE_CAST(stage), "LStep_UserInterface.uir", LS_PARAMS, 0); 
	
	get_device_param_from_ini_file("Stage", "COM_Port", &(this->_com_port));  
	
	STAGE_VTABLE(stage, init) = lstep_init;	  
	STAGE_VTABLE(stage, hw_init) = lstep_xy_hw_init;
	STAGE_VTABLE(stage, destroy) = lstep_destroy;
	STAGE_VTABLE(stage, get_info) = lstep_get_info;
	STAGE_VTABLE(stage, set_pitch) = lstep_set_pitch;
	STAGE_VTABLE(stage, get_pitch) = lstep_get_pitch;
	STAGE_VTABLE(stage, set_speed) = lstep_set_speed;
	STAGE_VTABLE(stage, get_speed) = lstep_get_speed;
	STAGE_VTABLE(stage, set_acceleration) = lstep_set_acceleration;
	STAGE_VTABLE(stage, get_acceleration) = lstep_get_acceleration;
	STAGE_VTABLE(stage, calibrate_extents) = lstep_calibrate_extents;
	STAGE_VTABLE(stage, async_goto_xy_position) = lstep_async_goto_xy_position;
	STAGE_VTABLE(stage, async_goto_xyz_position) = lstep_async_goto_xyz_position;
	STAGE_VTABLE(stage, goto_xy_position) = lstep_goto_xy_position;
	STAGE_VTABLE(stage, goto_xyz_position) = lstep_goto_xyz_position;
	STAGE_VTABLE(stage, async_rel_move_by) = lstep_async_rel_move_by;
	STAGE_VTABLE(stage, get_xyz_position) = lstep_get_xyz_position;
	STAGE_VTABLE(stage, set_xy_datum) = lstep_set_xy_datum;
	STAGE_VTABLE(stage, is_moving) = lstep_is_moving;
	STAGE_VTABLE(stage, set_joystick_speed) = lstep_set_joystick_speed;
	STAGE_VTABLE(stage, get_joystick_speed) = lstep_get_joystick_speed;  
	STAGE_VTABLE(stage, set_joystick_on) = lstep_set_joystick_on;
	STAGE_VTABLE(stage, set_joystick_off) = lstep_set_joystick_off;
	STAGE_VTABLE(stage, abort_move) = lstep_abort_move;
	STAGE_VTABLE(stage, load_settings) = lstep_load_settings;     
	STAGE_VTABLE(stage, save_settings) = lstep_save_settings;      
	STAGE_VTABLE(stage, set_timeout) = lstep_set_timeout;
	
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_LOAD, OnLStepParamsLoad, stage);
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_SAVE, OnLStepParamsSave, stage);
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_CLOSE, OnLStepParamsClose, stage);

	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_X_PITCH, OnLStepPitch, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_BACKLASH_X, OnLStepXbacklash, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_X_SPEED, OnLStepSpeed, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_X_ACC, OnLStepAcceleration, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_X_ENABLED, OnLStepXenabled, stage);

	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_Y_PITCH, OnLStepPitch, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_BACKLASH_Y, OnLStepYbacklash, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_Y_SPEED, OnLStepSpeed, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_Y_ACC, OnLStepAcceleration, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, LS_PARAMS_Y_ENABLED, OnLStepYenabled, stage);

	return stage;
}
