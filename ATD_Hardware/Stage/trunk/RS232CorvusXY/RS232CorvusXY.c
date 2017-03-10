#include "gci_utils.h"
#include "status.h"

#include "RS232CorvusXY_UserInterface.h"	 
#include "RS232CorvusXY.h"
#include "RS232Corvus_Communication.h"  

#include "iniparser.h"
#include <rs232.h>
#include <utility.h>
#include <formatio.h>

#include "ThreadDebug.h"

/////////////////////////////////////////////////////////////////////////////
// XYZ stage module for Corvus with Marzhauser stage - GP & RJL Jan 2006
//
/////////////////////////////////////////////////////////////////////////////

//NOTES:

//2) All position and move values are given in um
//3) The range of the stage is set by variable limit switches
//4) Backlash (X and Y directions) - always approach from the origin direction, ie increasing values of distance
//5) If any axis drive not present then disable that axis 
//6) Stage has 1mm pitch ie one revolution = 1mm 
//7) Velocity range xy = 1um/sec to 40mm/sec, max acceleration = 700mm/sec/sec)
//8) DIP switch:- Closed loop - All off (down). Open loop - all off except switch 3

/////////////////////////////////////////////////////////////////////////////

void update_ui (XYStage* stage)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;     

	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_X_ENABLED,  stage->_enabled_axis[XAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_X_SPEED,    stage->_speed[XAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_X_ACC,      stage->_acceleration[XAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_BACKLASH_X, this->_backlash[XAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_X_PITCH,    this->_pitch[XAXIS]);	

	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_Y_ENABLED,  stage->_enabled_axis[YAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_Y_SPEED,    stage->_speed[YAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_Y_ACC,      stage->_acceleration[YAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_BACKLASH_Y, this->_backlash[YAXIS]);	
	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_Y_PITCH,    this->_pitch[YAXIS]);	
				
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_BACKLASH_X, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_X_PITCH, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_X_SPEED, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_X_ACC, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_TEXTMSG_15, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_TEXTMSG_2, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_TEXTMSG_9, ATTR_DIMMED, !stage->_enabled_axis[XAXIS]);

	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_BACKLASH_Y, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_Y_PITCH, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_Y_SPEED, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);     
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_Y_ACC, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_TEXTMSG_17, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_TEXTMSG_18, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
	SetCtrlAttribute(stage->_params_ui_panel, XY_PARAMS_TEXTMSG_22, ATTR_DIMMED, !stage->_enabled_axis[YAXIS]);
}

int corvus_rs232_xy_send_closed_loop_params(XYStage* stage)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;     

	char command[200]="";
	
	if(stage->_closed_loop_enabled) {

		// Set closed loop mode for x axis
		if (RS232CorvusSend(this, "1 1 setcloop\n"))
			return STAGE_ERROR;

		// Set closed loop mode for y axis
		if (RS232CorvusSend(this, "1 2 setcloop\n"))
			return STAGE_ERROR;
	}
	else {

		// Set open loop mode for x axis
		if (RS232CorvusSend(this, "0 1 setcloop\n"))
			return STAGE_ERROR;

		// Set closed loop mode for y axis
		if (RS232CorvusSend(this, "0 2 setcloop\n"))
			return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;
}


int corvus_rs232_xy_hw_init (XYStage* stage)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;     

	GciCmtGetLock (this->_lock);   
	
	if(Corvus_initRS232Port(this) != 0)
		goto STAGE_INIT_ERROR;     	
	
	// Clear parameter Stack
	if (RS232CorvusSend(this, "clear\n"))
		return STAGE_ERROR;

	// disable joystick
	if (RS232CorvusSend(this, "0 j\n"))
		return STAGE_ERROR;

	// set units for velocity (um)
	if (RS232CorvusSend(this, "1 0 setunit\n"))
		goto STAGE_INIT_ERROR;    

	// set units for axes (um). position of each axis is set in um
	if (RS232CorvusSend(this, "1 1 setunit\n"))
		goto STAGE_INIT_ERROR;    	// x axis
	
	if (RS232CorvusSend(this, "1 2 setunit\n"))
		goto STAGE_INIT_ERROR;    	// y axis

	// set number of dimensions to 2, i.e. x, y
	if (RS232CorvusSend(this, "2 setdim\n"))
		goto STAGE_INIT_ERROR;
	
	//TODO
	// In Ros code seems to be undocumented
	if (RS232CorvusSend(this, "4.0 0 setpitch\n"))
		goto STAGE_INIT_ERROR;
	
	// set joystick speed (2 mm/sec)   
	stage_set_joystick_speed (stage, 2.0);  
	
	stage_load_default_settings(stage);
	
	corvus_rs232_xy_send_closed_loop_params(stage);

	// enable joystick
	//RS232CorvusSend(this, "1 j\n");
	
	GciCmtReleaseLock(this->_lock);    
	
	Delay(2.0);

	return STAGE_SUCCESS;
	
	STAGE_INIT_ERROR:
	
	GciCmtReleaseLock(this->_lock); 
		
	return STAGE_ERROR; 
}


int corvus_rs232_destroy(XYStage* stage)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;       
	
	// Clear parameter Stack
	RS232CorvusSend(this, "clear\n");
	
	close_comport(this->_com_port);

	CmtDiscardLock(this->_lock);     
	
  	return STAGE_SUCCESS;
}

static int corvus_rs232_set_pitch (XYStage* stage, Axis axis, double pitch)
{
	int err;

	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	
	//if (axis != ZAXIS) {
//		sprintf(str, "%.1f 0 setpitch\n", pitch);	
//		err = STAGE_SendNoAnswer(stage, str);
	//}	
	
	// Undocumented ?	
	err = RS232CorvusSend(this, "%.1f 0 setpitch\n", pitch);

	if (err)
		return STAGE_ERROR;
	
	err = RS232CorvusSend(this, "%.1f %d setpitch\n", pitch, axis);
//	Delay(0.2);	//Need to wait this long before sending another command
	
	if (err)
		return STAGE_ERROR;
	
//	Delay(0.2);	//Need to wait this long before sending another command   
	
//	stage->_pitch[0] = pitch;
	this->_pitch[axis] = pitch;
	
	return STAGE_SUCCESS;
}

static int corvus_rs232_get_pitch (XYStage* stage, Axis axis, double* pitch)
{
	int err;

	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	err = RS232CorvusSend(this, "%d getpitch\n", axis);
	
	if (err)
        return STAGE_ERROR;
		
	err = STAGE_ReadDouble(this, pitch);

	if (err)
        return STAGE_ERROR;
	
	this->_pitch[axis] = *pitch;
	
	return STAGE_SUCCESS;
}

static int corvus_rs232_set_speed (XYStage* stage, Axis axis, double speed)
{
	char str[50] = "";
	int err;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	sprintf(str,"%.1f sv\n", speed*1000.0);  //speed in um
	err = RS232CorvusSend(this, str);
	
//	Delay(0.2);	//Need to wait this long before sending another command

	if (err)
        return STAGE_ERROR;
	
//	stage->_speed[0] = speed;

	return STAGE_SUCCESS;
}

static int corvus_rs232_get_speed (XYStage* stage, Axis axis, double *speed)
{
	int err;
	double um_speed;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	err = STAGE_SendReadDouble(this, "getvel\n", &um_speed);

	if (err)
        return STAGE_ERROR;
	
	*speed = um_speed/1000;   //um to mm

	return STAGE_SUCCESS;
}


static int corvus_rs232_set_acceleration (XYStage* stage, Axis axis, double acceleration)
{
	char str[20];
	int err;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	//Send acceleration to the Corvus controller in mm/sec/sec
	sprintf(str,"%.1f sa\n", acceleration*1000.0);   //um
	err = RS232CorvusSend(this, str);
	
//	Delay(0.2);	//Need to wait this long before sending another command

	if (err)
        return STAGE_ERROR;

//	stage->_acceleration[0] = acceleration;

	return STAGE_SUCCESS;
}

static int corvus_rs232_get_acceleration (XYStage* stage, Axis axis, double *acceleration)
{
	int err;
	double um_acc;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	err = STAGE_SendReadDouble(this, "ga\n", &um_acc);

	if (err)
        return STAGE_ERROR;
	
	*acceleration = um_acc/1000;   //um to mm

	return STAGE_SUCCESS;
}


static int corvus_rs232_get_info (XYStage* stage, char *info)
{
	char serial_str[RS232_ARRAY_SIZE];
	double version;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
   	if (STAGE_SendReadString(this, "getserialno\n",serial_str) != 0)
   		return STAGE_ERROR;
	
   	if (STAGE_SendReadDouble(this, "version\n", &version) != 0)
   		return STAGE_ERROR;

	sprintf(info, "S/N %s Ver %f", serial_str, version);
	
	return STAGE_SUCCESS;
}


int corvus_rs232_get_xyz_position (XYStage* stage, double *x, double *y, double *z)
{
	char *stop_string;
	char read_data[RS232_ARRAY_SIZE] = "", stop_str[RS232_ARRAY_SIZE] = "";
	int err;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	*z = 0;
	
	// gets x,y,z coordinates of stage
	stop_string = stop_str;

	err = STAGE_SendReadString(this, "p\n",read_data); 

	if (err)
		return STAGE_ERROR;
	
	strcpy(stop_string," "); 				// coordinates separated by spaces
	*x = strtod(read_data, &stop_string); 	// picks up x coordinate and converts to double
	// strtod puts the a pointer to the first instance of stop_string into &&stop_string.
	// thus we can use stop_string as the parameter for the search for the y coordinate

	*y = strtod(stop_string, &stop_string); // get y coord
	
  	return STAGE_SUCCESS;
}

	
static int corvus_rs232_async_rel_move_by (XYStage* stage, double x, double y)          
{
	int err;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
//	memset(command, 0, RS232_ARRAY_SIZE);

//	sprintf(command,"%.1f %.1f r st\n", x, y);   //controller will respond to st when the move is complete
//	err = STAGE_SendReadString(this, command, answer);

	// WHY CAN'T WE JUST USE THIS COMMAND?
	err = RS232CorvusSend(this, "%.1f %.1f r\n", x, y);

// This blocking command will upset the status checks and "is_moving" checks	
//	if (stage->_joystick_status == 1)	//Moves disable the joystick
//		err |= RS232CorvusSend(this, "1 j\n");

	if (err)
		return STAGE_ERROR;

	return STAGE_SUCCESS;
}


static int corvus_rs232_set_xyz_datum (XYStage* stage, double x, double y, double z)
{
	// set x,y,f as the logical origin
	double cur_x, cur_y;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
		
	if(stage_get_xy_position_without_orientation_correction(stage, &cur_x, &cur_y) == STAGE_ERROR)
		return STAGE_ERROR;
	
	// Set the datam. It is relative from the current position
	if (RS232CorvusSend(this, "%.1f %.1f sp\n",cur_x - x, cur_y - y))
		return STAGE_ERROR;

//	Delay(0.2);	//	setpos takes longer than other commands          
	
	return STAGE_SUCCESS;
}


static int corvus_rs232_calibrate_extents (XYStage* stage, double *min_x, double *min_y,
		                             double *max_x, double *max_y)
{
	char command[RS232_ARRAY_SIZE]="";
	CorvusXYStage *this = (CorvusXYStage *) stage;     

 	//Set calibration velocities in revs/sec. Default values from Winpos are 10.0, 0.25
	//However we have an MR encoder system. This means that at the end of "cal" 
	//there is a 2mm move to calibrate the encoders. 
	//With a speed of 0.25 rev/sec this took 16 seconds!
	
	double xpitch;
	
	//GciCmtGetLock (this->_lock); 
		
	if( stage_check_user_has_aborted(stage))
		goto Error;
	
	stage_get_pitch(stage, XAXIS, &xpitch);
	
	if (RS232CorvusSend(this, "%.2f 1 setcalvel\n", this->_cal_velocity_toward/xpitch)) //towards limit switches
		goto Error;
	
	if (RS232CorvusSend(this, "%.2f 2 setcalvel\n", this->_cal_velocity_away/xpitch)) //away from limit switches
		goto Error;
	
	if (RS232CorvusSend(this, "%.2f 1 setrmvel\n", this->_rm_velocity_toward/xpitch)) //towards limit switches
		goto Error;
	
	if (RS232CorvusSend(this, "%.2f 2 setrmvel\n", this->_rm_velocity_away/xpitch))  //away from limit switches
		goto Error;

	stage_set_joystick_off(stage);
	
	*min_x = *min_y = 0.0;
	*max_x = *max_y = 0.0;

	// Need a long timeout for stage initialisation.
	SetComTime (this->_com_port, STAGE_LONG_TIMEOUT); 
		
	if( stage_check_user_has_aborted(stage))
		goto Error;

	//Move to x, y "minus" limit switches. Position is automatically set to zero.
    // This causes the stage to go to the left top limit switch.
	if (RS232CorvusSend(this, "cal\n"))
		goto Error;
	
	RS232CorvusBlockUntilFinished(this);
	
	Delay(0.5);							 						//wait for it to start moving
		
	stage_wait_for_stop_moving(stage); 
		
    // The stage 
	if (stage_get_xy_position_without_orientation_correction(stage, min_x, min_y) == STAGE_ERROR)
		goto Error;

	if (stage_check_user_has_aborted(stage))
		goto Error;

	// rm moves to maximum range values. Ie the right / bottom limit switches
	if (RS232CorvusSendandBlockUntilFinished(this, "rm\n"))
		goto Error;
		
	Delay(0.5);
	
	stage_wait_for_stop_moving(stage);       
		
	if( stage_check_user_has_aborted(stage))
		goto Error;
		
		// Get hard limits, set during calibration and rmeasure commands (values returned in um)   
	if (stage_get_xy_position_without_orientation_correction(stage, max_x, max_y) == STAGE_ERROR)
		goto Error;
  	
	SetComTime (this->_com_port, STAGE_NORMAL_TIMEOUT);  
	GciCmtReleaseLock (this->_lock);    
	
  	return STAGE_SUCCESS;
  	
Error:
	
	//GciCmtReleaseLock (this->_lock); 
	SetComTime (this->_com_port, STAGE_NORMAL_TIMEOUT);  
	
	return STAGE_ERROR;
}


static int corvus_rs232_is_moving (XYStage* stage, int *status)
{
	int err = 0, retval = 0;
	CorvusXYStage *this = (CorvusXYStage *) stage;
	
	err = STAGE_SendReadInt(this, "st\n", &retval);

	if (err)
		return STAGE_ERROR;
	
	if ((retval & 1) == 0)				//bit 0 is clear, controller is not busy
		*status = 0;
	else
		*status = 1;
	
  	return STAGE_SUCCESS;
}	

//////////////////////////////////////////////////////////////////////////
static int corvus_rs232_set_joystick_speed (XYStage* stage, double speed)
{
	int err;
	CorvusXYStage *this = (CorvusXYStage *) stage;     
		
	// set joystick speed in um
	err = RS232CorvusSend(this, "%.1f setjoyspeed\n", speed*1000.0);

   	if (err)
        return STAGE_ERROR;
  	
  	return STAGE_SUCCESS;
}	

static int corvus_rs232_get_joystick_speed (XYStage* stage, double* speed)
{
	int err;
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	err = STAGE_SendReadDouble(this, "getjoyspeed\n", speed);

	if (err)
        return STAGE_ERROR;
	
	return STAGE_SUCCESS;
}	
	
static int corvus_rs232_set_joystick_on (XYStage* stage)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	int err =  RS232CorvusSend(this, "1 j\n");

	if (err)
        return STAGE_ERROR;
  	
	corvus_rs232_set_joystick_speed(stage, stage->_joystick_speed);
		
  	return STAGE_SUCCESS;
}	

	
static int corvus_rs232_set_joystick_off (XYStage* stage)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;     
	
	int err = RS232CorvusSend(this, "0 j\n");

	if (err)
        return STAGE_ERROR;

	return STAGE_SUCCESS;	
}		
	

static int corvus_rs232_abort_move (XYStage* stage)
{
	int err;
	CorvusXYStage *this = (CorvusXYStage *) stage;     
	
	//Send ctrl C
	err = RS232CorvusSend(this, "%c\n",3);

	if (err)
        return STAGE_ERROR;

	return STAGE_SUCCESS;	
}		
	

static void get_backlash_correction(XYStage *stage, double *x, double *y)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;
	static double last_x = 0.0, last_y = 0.0;

	//Check to see if backlash compensation if required
	if (FP_Compare (this->_backlash[XAXIS], 0.0) != 0) {	//backlash X is not zero
		//If moving in negative direction do backlash
		if (FP_Compare (last_x, *x) == 1)
			*x -= this->_backlash[XAXIS]; 
	}

	if (FP_Compare (this->_backlash[YAXIS], 0.0) != 0) {	//backlashY is not zero
		//If moving in negative direction do backlash
		if (FP_Compare (last_y, *y) == 1)
			*y -= this->_backlash[YAXIS];
	}

	last_x = *x;
	last_y = *y;
}

static int corvus_rs232_async_goto_xy_position (XYStage* stage, double x, double y)
{
	// Return straight away
	int err;
	CorvusXYStage *this = (CorvusXYStage *) stage;

    get_backlash_correction(stage, &x, &y);
	
	err =  RS232CorvusSend(this, "%.1f %.1f m\n", x, y);

	// This blocking command will upset the status checks and "is_moving" checks	
	//	if (stage->_joystick_status == 1)	//Moves disable the joystick
	//		err |= RS232CorvusSend(this, "1 j\n");

	if (err)
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}


static int corvus_rs232_set_xy_datum (XYStage* stage, double x, double y)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;   
	
	// set x,y,f as the logical origin  
	int err =  RS232CorvusSend(this, "%.1f %.1f sp\n",x, y); 
	
	if (err)
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}

static int corvus_rs232_set_controller_safe_region (XYStage* stage, Roi roi)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;   
	
	// set roi as the hardware safe limit
	int err =  RS232CorvusSend(this, "%.1f %.1f %.1f %.1f setlimit\n", roi.min_x, roi.min_y, roi.max_x, roi.max_y); 
	
	if (err)
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}

int corvus_rs232_set_axis_enable(XYStage* stage, Axis axis, int enable)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;   

	int err =  RS232CorvusSend(this, "%d %d setaxis\n", enable, axis); 
	
	if (err)
		return STAGE_ERROR;

	// the setaxis command diables the joystick
	if (stage->_joystick_status == 1) stage_set_joystick_on (stage);
	
	update_ui(stage);
	
	return STAGE_SUCCESS;	
}


int corvus_rs232_save_settings (XYStage* stage, const char *filepath)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	FILE *fd=NULL;
	dictionary *d = dictionary_new(20);
	char temp_dir[GCI_MAX_PATHNAME_LEN] = "", temp_filepath[GCI_MAX_PATHNAME_LEN] = "";

	// Get temp dir
	if(!GetEnvironmentVariable("Temp", temp_dir, 500))
		ui_module_send_error(UIMODULE_CAST(stage), "Stage Error", "Can not get temporary directory");  

	sprintf(temp_filepath, "%s\\%s", temp_dir, "\\stage_calibration_temp.ini");	

	fd = fopen(temp_filepath, "w");

	if (fd==NULL)
		return STAGE_ERROR;
	
	// get generic settings
	stage_save_data_to_dictionary(stage, d);
	
	dictionary_setdouble(d, "Calibration Velocity Toward EndSwitch", this->_cal_velocity_toward);  
	dictionary_setdouble(d, "Calibration Velocity Away From EndSwitch", this->_cal_velocity_away); 
	dictionary_setdouble(d, "RM Calibration Velocity Toward EndSwitch", this->_rm_velocity_toward);  
	dictionary_setdouble(d, "RM Calibration Velocity Away From EndSwitch", this->_rm_velocity_away);  

	// get specific settings
    dictionary_setdouble(d, "X Axis Pitch",    this->_pitch[XAXIS]);
    dictionary_setdouble(d, "Y Axis Pitch",    this->_pitch[YAXIS]);
    dictionary_setdouble(d, "X Axis Backlash", this->_backlash[XAXIS]);
    dictionary_setdouble(d, "Y Axis Backlash", this->_backlash[YAXIS]);
	
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	if(CopyFile (temp_filepath, filepath) < 0)
		return STAGE_ERROR;

	return STAGE_SUCCESS;	      
}

int corvus_rs232_load_settings (XYStage* stage, const char *filepath)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	dictionary* d = NULL;
	int file_size;
	
	if(!FileExists(filepath, &file_size))
		return STAGE_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(iniparser_getnsec(d) == 0)
	{
		stage->_do_not_initialise = 1;
		GCI_MessagePopup("Error", "Failed to read stage configuration. The file exists but no data was read.\n"
							  "I am not going to initialise the stage extents as this could cause damage.");
		return STAGE_ERROR;
	}

	if(d != NULL) {

		// generic settings
		stage_load_data_from_dictionary(stage, d); 
 
		this->_cal_velocity_toward = dictionary_getdouble(d, "Stage:Calibration Velocity Toward EndSwitch", 10);  
		this->_cal_velocity_away = dictionary_getdouble(d, "Stage:Calibration Velocity Away From EndSwitch", 1.5); 
		this->_rm_velocity_toward = dictionary_getdouble(d, "Stage:RM Calibration Velocity Toward EndSwitch", 10);  
		this->_rm_velocity_away = dictionary_getdouble(d, "Stage:RM Calibration Velocity Away From EndSwitch", 1.5); 
	
		// specific settings
	    this->_backlash[XAXIS] = dictionary_getdouble(d, "Stage:X Axis Backlash", 0);	 // 0 microns
	    this->_pitch[XAXIS]    = dictionary_getdouble(d, "Stage:X Axis Pitch", 1.0);
	    this->_backlash[YAXIS] = dictionary_getdouble(d, "Stage:Y Axis Backlash", 0);	 // 0 microns
	    this->_pitch[YAXIS]    = dictionary_getdouble(d, "Stage:Y Axis Pitch", 1.0);

		stage_set_pitch(stage, XAXIS, this->_pitch[XAXIS]);
		stage_set_pitch(stage, YAXIS, this->_pitch[YAXIS]);
		
		corvus_rs232_set_axis_enable(stage, XAXIS, stage->_enabled_axis[XAXIS]);
		corvus_rs232_set_axis_enable(stage, YAXIS, stage->_enabled_axis[YAXIS]);
		
		dictionary_del(d);
	}
	
	update_ui(stage);
	
	return STAGE_SUCCESS;	  
}
	
static int corvus_rs232_set_timeout(XYStage *stage, double timeout)
{
	CorvusXYStage *this = (CorvusXYStage *) stage;  
	
	SetComTime(this->_com_port, timeout); 
	
	return STAGE_SUCCESS;
}

int corvus_rs232_init (XYStage *stage)
{
	update_ui(stage);

	return STAGE_SUCCESS;
}

XYStage* corvus_rs232_xy_stage_new(const char* name, const char* description, UI_MODULE_ERROR_HANDLER error_handler, void *data, const char *data_dir)
{
	CorvusXYStage* this = (CorvusXYStage *) malloc(sizeof(CorvusXYStage));
	XYStage *stage = (XYStage*) this;

	stage_constructor(stage, name);

	GciCmtNewLock("CorvusStage", 0, &(this->_lock));
	
	ui_module_set_description(UIMODULE_CAST(stage), description);  
	ui_module_set_error_handler(UIMODULE_CAST(stage), error_handler, data); 
	ui_module_set_data_dir(UIMODULE_CAST(stage), data_dir);   
	
	stage->_params_ui_panel = ui_module_add_panel(UIMODULE_CAST(stage), "RS232CorvusXY_UserInterface.uir", XY_PARAMS, 0); 
	
	get_device_param_from_ini_file("Stage", "COM_Port", &(this->_com_port));
	
	if(get_device_param_from_ini_file("Stage", "BaudRate", &(this->_baud_rate)) < 0)
		this->_baud_rate = 115200;
	
	STAGE_VTABLE(stage, init) = corvus_rs232_init;	  
	STAGE_VTABLE(stage, hw_init) = corvus_rs232_xy_hw_init;
	STAGE_VTABLE(stage, destroy) = corvus_rs232_destroy;
	STAGE_VTABLE(stage, get_info) = corvus_rs232_get_info;
	STAGE_VTABLE(stage, set_pitch) = corvus_rs232_set_pitch;
	STAGE_VTABLE(stage, get_pitch) = corvus_rs232_get_pitch;
	STAGE_VTABLE(stage, set_speed) = corvus_rs232_set_speed;
	STAGE_VTABLE(stage, get_speed) = corvus_rs232_get_speed;
	STAGE_VTABLE(stage, set_acceleration) = corvus_rs232_set_acceleration;
	STAGE_VTABLE(stage, get_acceleration) = corvus_rs232_get_acceleration;
	STAGE_VTABLE(stage, calibrate_extents) = corvus_rs232_calibrate_extents;
	STAGE_VTABLE(stage, async_goto_xy_position) = corvus_rs232_async_goto_xy_position;
	STAGE_VTABLE(stage, async_rel_move_by) = corvus_rs232_async_rel_move_by;
	STAGE_VTABLE(stage, get_xyz_position) = corvus_rs232_get_xyz_position;
	STAGE_VTABLE(stage, set_xy_datum) = corvus_rs232_set_xy_datum;
	STAGE_VTABLE(stage, set_controller_safe_region) = corvus_rs232_set_controller_safe_region;
	STAGE_VTABLE(stage, is_moving) = corvus_rs232_is_moving;
	STAGE_VTABLE(stage, set_joystick_speed) = corvus_rs232_set_joystick_speed;
	STAGE_VTABLE(stage, get_joystick_speed) = corvus_rs232_get_joystick_speed;  
	STAGE_VTABLE(stage, set_joystick_on) = corvus_rs232_set_joystick_on;
	STAGE_VTABLE(stage, set_joystick_off) = corvus_rs232_set_joystick_off;
	STAGE_VTABLE(stage, abort_move) = corvus_rs232_abort_move;
	STAGE_VTABLE(stage, load_settings) = corvus_rs232_load_settings;     
	STAGE_VTABLE(stage, save_settings) = corvus_rs232_save_settings;      
	STAGE_VTABLE(stage, set_timeout) = corvus_rs232_set_timeout;
	
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_LOAD, OnParamsLoad, stage);
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_SAVE, OnParamsSave, stage);
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_RESETSAFEREGION, OnResetSafeRegion, stage);
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_TOGGLECLOOP, OnToggleClosedLoop, stage);
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_CLOSE, OnParamsClose, stage);

	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_X_PITCH, OnPitch, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_BACKLASH_X, OnXbacklash, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_X_SPEED, OnSpeed, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_X_ACC, OnAcceleration, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_X_ENABLED, OnXenabled, stage);

	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_Y_PITCH, OnPitch, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_BACKLASH_Y, OnYbacklash, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_Y_SPEED, OnSpeed, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_Y_ACC, OnAcceleration, stage); 
	InstallCtrlCallback (stage->_params_ui_panel, XY_PARAMS_Y_ENABLED, OnYenabled, stage);

	return stage;
}
