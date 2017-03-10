#include <rs232.h>

#include "deviceFinder.h"
#include <utility.h>
#include "com_port_control.h"
#include "gci_utils.h"
#include "status.h"
//#include "power.h"
#include "Corvus_ui.h"	 
#include "Corvus_stage.h"
#include "hardware.h"

#include <formatio.h>

/////////////////////////////////////////////////////////////////////////////
// XYZ stage module for Corvus with Marzhauser stage - GP & RJL Jan 2006
//
/////////////////////////////////////////////////////////////////////////////

//NOTES:

//1) The z-drive is disabled during calibration (no limits)
//2) All position and move values are given in um
//3) The range of the stage is set by variable limit switches
//4) Backlash (X and Y directions) - always approach from the origin direction, ie increasing values of distance
//5) If any axis drive not present then disable that axis 
//6) Stage has 1mm pitch ie one revolution = 1mm 
//7) Velocity range xy = 1um/sec to 40mm/sec, max acceleration = 700mm/sec/sec)
//8) DIP switch:- Closed loop - All off (down). Open loop - all off except switch 3

/////////////////////////////////////////////////////////////////////////////

//#define DEBUG

#define DELAY 0.02

#ifdef XY_ONLY
	#define CORVUS_F_FACTOR 1.0
#else 
	//1 rev of the z axis is 100um. However we set z pitch to 1mm otherwise it can't
	//move as fast for some reason. This means we have to adjust z moves and readings. 
	#define CORVUS_F_FACTOR 10.0
#endif

#define CAL_VEL_XY	10.0			//Initial velocity for xy calibration rev/sec towards limits
#define CAL_VEL_XY2	0.25			//Initial velocity for xy calibration rev/sec away from limits

static double gStageOnTime = -6.0;

/* Function pointers used as virtual functions */
static struct stage_operations Corvus_ops;

/****************************************************************************************************************/ 
//Basic communications

static int STAGE_CommsErrorMessage(Stage *stage)
{
	char *msg;
	int err;
	
	//Break down in communication has occured
	err = ReturnRS232Err();
	if (err < 0) {
		if (!stage->_show_errors)
			return -1;

		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		msg = GetRS232ErrorString (err);
		LogError("XY Stage Controller Communication Error", msg);
		stage->_no_errors++;
		if (stage->_no_errors > 2) {
			GCI_ShowStatus("XY Stage Controller Communication Error", msg);
		}	
		return -1;
	}
	else 
		stage->_no_errors = 0;
	return 0;
}

static int STAGE_SendString(Stage *stage, char* str) 
{
	CmtGetLock (stage->_lock);					 //for multi-threading

#ifdef DEBUG
	printf(str);
#else	
	// flush the Qs
	FlushInQ(stage->_port);
	FlushOutQ(stage->_port);	

	// send string 
	if (ComWrt(stage->_port, str, strlen(str)) <= 0) {
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}
#endif

	CmtReleaseLock (stage->_lock);
	return 0;
}

static int STAGE_SendNoAnswer(Stage *stage, char* str)
{
	//If we are sending a command to which there is no reply we need
	//a small delay before sending the next command otherwise calamity WILL occur
	if (STAGE_SendString(stage, str)) return -1;
	Delay(DELAY);
	return 0;
}

static int STAGE_ReadString(Stage *stage, char *retval)
{
	int no_bytes_read;
	char read_data[200]="";
	
	// Reads the Stage Port and returns a string
	// Returned character string is always terminated with CR, (ASCII 13)
	// will return 0 if read is successful, -1 otherwise

	CmtGetLock (stage->_lock);					 //for multi-threading

	FillBytes (read_data, 0, 200, 0);
#ifndef DEBUG
	no_bytes_read = ComRdTerm(stage->_port, read_data, 199, 13);  
	if (no_bytes_read <= 0) {
		strcpy(retval,"");
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}
#endif	
	strcpy(retval, read_data);

	CmtReleaseLock (stage->_lock);
	return 0;
}

static int STAGE_SendReadString(Stage *stage, char* str, char* retval)
{
	int no_bytes_read;
	char read_data[200]="";
	
	// sends str to gStagePort and returns a string
	CmtGetLock (stage->_lock);					 //for multi-threading

#ifdef DEBUG
	//printf(str);
#else	
	// flush the Qs
	FlushInQ(stage->_port);
	FlushOutQ(stage->_port);	

	// send string 
	if (ComWrt(stage->_port, str, strlen(str)) <= 0) {
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}
#endif

	FillBytes (read_data, 0, 200, 0);
#ifndef DEBUG
	no_bytes_read = ComRdTerm(stage->_port, read_data, 199, 13);  
	if (no_bytes_read <= 0) {
		strcpy(retval,"");
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}
#endif	
	strcpy(retval, read_data);

	CmtReleaseLock (stage->_lock);
	return 0;
}

static int STAGE_ReadInt(Stage *stage, int *retval)
{
	// Reads the gStagePort and converts returned value to int
	// will return 0 if read is successful, -1 otherwise
	int no_bytes_read;
	char read_data[20]="";
	
	CmtGetLock (stage->_lock);					 //for multi-threading

	*retval = 0;
	FillBytes (read_data, 0, 20, 0);
#ifndef DEBUG
	no_bytes_read = ComRdTerm(stage->_port, read_data, 19, 13);
	if (no_bytes_read <= 0) {
		*retval=0;
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}

	*retval = atoi(read_data);
#endif

	CmtReleaseLock (stage->_lock);
	return 0;
}

static int STAGE_SendReadInt(Stage *stage, char* str, int *retval)
{
	int no_bytes_read;
	char read_data[20]="";
	
	// sends str to gStagePort and returns an int
	CmtGetLock (stage->_lock);					 //for multi-threading

#ifdef DEBUG
	printf(str);
#else	
	// flush the Qs
	FlushInQ(stage->_port);
	FlushOutQ(stage->_port);	

	// send string 
	if (ComWrt(stage->_port, str, strlen(str)) <= 0) {
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}
#endif

	*retval = 0;
	FillBytes (read_data, 0, 20, 0);
#ifndef DEBUG
	no_bytes_read = ComRdTerm(stage->_port, read_data, 19, 13);
	if (no_bytes_read <= 0) {
		*retval=0;
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}

	*retval = atoi(read_data);
#endif

	CmtReleaseLock (stage->_lock);
	return 0;
}

static int STAGE_ReadDouble(Stage *stage, double *retval)
{	
	// Reads the gStagePort and converts returned value to double
	// will return 0 if read is successful, -1 otherwise
	int no_bytes_read;
	char read_data[20]="";
	
	CmtGetLock (stage->_lock);					 //for multi-threading

	*retval = 0.0;
	FillBytes (read_data, 0, 20, 0);
#ifndef DEBUG
	no_bytes_read = ComRdTerm(stage->_port, read_data, 19, 13);
	if (no_bytes_read <= 0) {
		*retval=0.0;
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}

	*retval = atof(read_data);
#endif

	CmtReleaseLock (stage->_lock);
	return 0;
}

static int STAGE_SendReadDouble(Stage *stage, char* str, double *retval)
{
	int no_bytes_read;
	char read_data[20]="";
	
	// sends str to gStagePort and returns a double
	CmtGetLock (stage->_lock);					 //for multi-threading

#ifdef DEBUG
	printf(str);
#else	
	// flush the Qs
	FlushInQ(stage->_port);
	FlushOutQ(stage->_port);	

	// send string 
	if (ComWrt(stage->_port, str, strlen(str)) <= 0) {
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}
#endif

	*retval = 0.0;
	FillBytes (read_data, 0, 20, 0);
#ifndef DEBUG
	no_bytes_read = ComRdTerm(stage->_port, read_data, 19, 13);
	if (no_bytes_read <= 0) {
		*retval=0.0;
		CmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}

	*retval = atof(read_data);
#endif

	CmtReleaseLock (stage->_lock);
	return 0;
}

//////////////////////////////////////////////////////////////////
//Stage Controller errors

int STAGE_ErrorMessage(Stage *stage)
{
	int retval=-1, err=0;
	char msg[200]="";
	
	if (!stage->_show_errors)
		return STAGE_SUCCESS;

	while (retval != 0) {
		if (STAGE_SendReadInt(stage, "ge\n",&retval)) return -1;
		if (retval == 0) {
			stage->_no_errors = 0;
			return err;	//No more errors on stack
		}
		
		err = -1;						//At least one stage error
		switch (retval) {
			case 1001:
				sprintf(msg, "Command sent with wrong parameter.");		  
			break;
			case 1002:
				sprintf(msg, "Not enough parameters on stack.");		  
			break;
			case 1003:
				sprintf(msg, "Range of parameter exceeded.");		  
			break;
			case 1004:
				sprintf(msg, "Move stopped working range should run over.");		  
			break;
			case 1008:
				sprintf(msg, "Not enough parameters on stack.");		  
			break;
			case 1009:
				sprintf(msg, "Stack overflow");		  
			break;
			case 1010:
				sprintf(msg, "Parameter memory full");		  
			break;
			case 1015:
				sprintf(msg, "Parameter outside working range");		  
			break;
			case 2000:
				sprintf(msg, "Unrecognised command.");		  
			break;
			default:
				sprintf(msg, "Unknown error code %d.", retval);		  
			break;
		}
		
		LogError("XY Stage Controller Error", msg);
		stage->_no_errors++;
		if (stage->_no_errors > 2) {
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			GCI_ShowStatus("XY Stage Controller Error", msg); 
		}	
	}
	return err;
}

/////////////////////////////////////////////////////////////////////////
//Stage Initialisation

static int Corvus_set_baud_rate (Stage* stage, int baud_rate)
{
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	MessagePopup("XY Stage Warning", "You must use the DIP switch on the rear of the controller to change the baud rate.");
	
	return STAGE_SUCCESS;
}


static int Corvus_power_up(Stage* stage)
{
	stage->_powered_up = 1;
	#ifdef POWER_VIA_I2C
	gStageOnTime = Timer();
	return GCI_PowerMicroscope_and_StageOn();
	#else
	return 0;
	#endif
}

static int Corvus_power_down(Stage* stage)
{
	// Leave hardware joystick enabled
	stage_set_joystick_on (stage);
	
	//STAGE_SendNoAnswer(stage, "save\n");  //save settings in non-volatile memory
	
	stage->_powered_up = 0;
	#ifdef POWER_VIA_I2C
	return GCI_PowerMicroscope_and_StageOff();
	#else
	return STAGE_SUCCESS;
	#endif
}

static int Corvus_powered_up(Stage* stage)
{
	#ifdef POWER_VIA_I2C
	stage->_powered_up = !GCI_Power_GetMicroscope_and_StageStatus();
	#else
	stage->_powered_up = 0;
	#endif
	return stage->_powered_up;
}

static int Corvus_reset (Stage* stage) 
{
	return STAGE_SUCCESS;
}

static int Corvus_getFTDIport(int *port)
{
	char path[MAX_PATHNAME_LEN], id[20];
	int id_panel, pnl, ctrl;
	
	//If we are using an FTDI gizmo Device Finder should give us the port number
	GetPrivateDataFolder("Microscope Data", path);
	strcat(path, "StageDeviceID.txt");
	return selectPortForDevice(path, port, "Select Port for Stage Controller"); 
}

static int Corvus_initRS232Port(Stage* stage)
{
	//Initialise rs232 communications
	int err, attempts = 0, baud=9600;
	int parity=0, dataBits=8, stopBits=1, inputQueueSize=164, outputQueueSize=164;
	double retval;
	
	//If we are using an FTDI gizo Device Finder should give us the port number
	if (Corvus_getFTDIport(&stage->_port) == 0) 
		sprintf(stage->_port_string, "COM%d", stage->_port);
	else {
		//Otherwise use Glenn's COM port allocation module
		GCI_ComPortControlAddDevice("XYStage");
		GCI_ComPortControlLoadConfig();
		GCI_ComPortControlGetDeviceProperties("XYStage", &stage->_port, stage->_port_string, &parity, &baud,
					&dataBits, &stopBits, &inputQueueSize, &outputQueueSize);
	}
	
	//GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BAUD, &baud);
	
#ifndef DEBUG
	while (attempts < 2) {
		//If the stage controller is off switch it on and wait for it to initialise
        stage->_powered_up = Corvus_powered_up(stage);
        if (!stage->_powered_up) 
			if (Corvus_power_up(stage) != 0) return -1;   	//Some I2C problem
        while ((Timer() - gStageOnTime) < 6.0) ProcessSystemEvents();

		CloseCom(stage->_port);	//In case it was open
		err = OpenComConfig (stage->_port, stage->_port_string, baud, 0, 8, 1, 512, 512);
		//Short timeout so we can tell at once if it's switched off or something
		SetComTime (stage->_port, 1.0);
		
		STAGE_SendNoAnswer(stage, "mode 0\n");	  //Set "host" mode. "terminal" mode is for use with Hyperterminal only.
	
		//Get the version number from the controller
    	if (STAGE_SendReadDouble(stage, "version\n",&retval) == 0) {
    		if (retval > 0) break;	//success
		}
		
		attempts ++;
	}
	if (attempts > 1) return -1;	//Failed
	
	SetComTime (stage->_port, 5.0); 
#endif

	return 0;
}

static int Corvus_set_port_timeout (Stage* stage, double timeout)
{
	SetComTime (stage->_port, timeout);
	return 0;
}

static int Corvus_init (Stage* stage)
{
	char string[30];

	//If the stage controller is off switch it on and wait for it to initialise
    stage->_powered_up = Corvus_powered_up(stage);
    if (!stage->_powered_up) 
		if (Corvus_power_up(stage) != 0) return STAGE_ERROR;   	//Some I2C problem
    while ((Timer() - gStageOnTime) < 6.0) ProcessSystemEvents();

	CmtGetLock (stage->_lock);

	if (Corvus_initRS232Port(stage) != 0) {
	    CmtReleaseLock(stage->_lock);
		return STAGE_ERROR;
	}
	
	// disable joystick
	if (STAGE_SendNoAnswer(stage, "0 j\n")) return STAGE_ERROR;

	// set units for velocity (um)
	if (STAGE_SendNoAnswer(stage, "1 0 setunit\n")) return STAGE_ERROR;

	// set units for axes (um). position of each axis is set in um
	if (STAGE_SendNoAnswer(stage, "1 1 setunit\n")) return STAGE_ERROR;	// x axis
	if (STAGE_SendNoAnswer(stage, "1 2 setunit\n")) return STAGE_ERROR;	// y axis
#ifndef XY_ONLY
	if (STAGE_SendNoAnswer(stage, "1 3 setunit\n")) return STAGE_ERROR;	// f axis
#endif
	
#ifndef XY_ONLY
	// set number of dimensions to 3, i.e. x, y and f
	if (STAGE_SendNoAnswer(stage, "3 setdim\n")) return STAGE_ERROR;
#else
	// set number of dimensions to 2, i.e. x, y
	if (STAGE_SendNoAnswer(stage, "2 setdim\n")) return STAGE_ERROR;
#endif
	
	// set pitches
	sprintf(string, "%.1f 0 setpitch\n", stage->_pitch[XAXIS]);	
	if (STAGE_SendNoAnswer(stage, string)) return  STAGE_ERROR; // set stage pitch, vel
	sprintf(string, "%.1f 1 setpitch\n", stage->_pitch[XAXIS]);	
	if (STAGE_SendNoAnswer(stage, string)) return  STAGE_ERROR; // set stage pitch, x axis
	sprintf(string, "%.1f 2 setpitch\n", stage->_pitch[YAXIS]);	
	if (STAGE_SendNoAnswer(stage, string)) return  STAGE_ERROR; // set stage pitch, y axis
#ifndef XY_ONLY
	//Pitch of our z drive is 100um in reality. However, for some reason if we
	//send a pitch of "0.1" it greatly reduces the speed we can achieve without loosing steps.
	sprintf(string, "%.1f 3 setpitch\n", stage->_pitch[ZAXIS]*CORVUS_F_FACTOR);	
	if (STAGE_SendNoAnswer(stage, string)) return  STAGE_ERROR; // set stage pitch, f axis
#endif

//	if (STAGE_SendNoAnswer(stage, "1 0 setpitch\n")) return  STAGE_ERROR; // set stage pitch, vel
//	if (STAGE_SendNoAnswer(stage, "1 1 setpitch\n")) return  STAGE_ERROR; // set stage pitch, x axis
//	if (STAGE_SendNoAnswer(stage, "1 2 setpitch\n")) return  STAGE_ERROR; // set stage pitch, y axis
//#ifndef XY_ONLY
//	if (STAGE_SendNoAnswer(stage, "1 3 setpitch\n")) return  STAGE_ERROR; // set stage pitch, f axis
//#endif

	// set joystick speed (2 mm/sec)
	sprintf(string,"%d setjoyspeed\n",2000); 
	if (STAGE_SendNoAnswer(stage, string)) return STAGE_ERROR;

	// enable joystick
	STAGE_SendNoAnswer(stage, "1 j\n");

	stage_set_description(stage, "Corvus Stage");
	stage_set_name(stage, "Corvus Stage");
	
	CmtReleaseLock(stage->_lock);

	return STAGE_SUCCESS;
}

int Corvus_destroy(Stage* stage)
{
	CorvusStage* Corvus_stage = (CorvusStage*) stage;
	
	Corvus_power_down(stage);
	
  	free(Corvus_stage);
  	
  	return STAGE_SUCCESS;
}

static int Corvus_set_pitch (Stage* stage, Axis axis, double pitch)
{
	char str[20];
	int err;
	
	CmtGetLock (stage->_lock);

	if (axis != ZAXIS) {
		sprintf(str, "%.1f 0 setpitch\n", pitch);	
		err = STAGE_SendNoAnswer(stage, str);
	}	
	sprintf(str, "%.1f %d setpitch\n", pitch, axis);  //pitch in mm
	err = STAGE_SendNoAnswer(stage, str);
	Delay(0.2);	//Need to wait this long before sending another command
	
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;
	
	stage->_pitch[0] = pitch;
	return STAGE_SUCCESS;
}

static int Corvus_set_speed (Stage* stage, Axis axis, double speed)
{
	char str[20];
	int err;
	
	CmtGetLock (stage->_lock);

	sprintf(str,"%.1f sv\n", speed*1000.0);  //speed in um
	err = STAGE_SendNoAnswer(stage, str);
	Delay(0.2);	//Need to wait this long before sending another command
	
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;
	
	stage->_speed[0] = speed;
	return STAGE_SUCCESS;
}

static int Corvus_get_speed (Stage* stage, Axis axis, double *speed)
{
	int err;
	double um_speed;
	
	CmtGetLock (stage->_lock);

	err = STAGE_SendReadDouble(stage, "gv\n", &um_speed);
	
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;
	
	*speed = um_speed/1000;   //um to mm
	return STAGE_SUCCESS;
}


static int Corvus_set_acceleration (Stage* stage, Axis axis, double acceleration)
{
	char str[20];
	int err;
	
	//Send acceleration to the Corvus controller in mm/sec/sec
	
	CmtGetLock (stage->_lock);

	sprintf(str,"%.1f sa\n", acceleration*1000.0);   //um
	err = STAGE_SendNoAnswer(stage, str);
	Delay(0.2);	//Need to wait this long before sending another command
	
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;

	stage->_acceleration[0] = acceleration;

	return STAGE_SUCCESS;
}

static int Corvus_get_acceleration (Stage* stage, Axis axis, double *acceleration)
{
	int err;
	double um_acc;
	
	CmtGetLock (stage->_lock);

	err = STAGE_SendReadDouble(stage, "ga\n", &um_acc);
	
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;
	
	*acceleration = um_acc/1000;   //um to mm
	return STAGE_SUCCESS;
}


static int Corvus_get_info (Stage* stage, char *info)
{
	char serial_str[30];
	double version;
	
	CmtGetLock (stage->_lock);

   	if (STAGE_SendReadString(stage, "getserialno\n",serial_str) != 0)
   		return STAGE_ERROR;
	
   	if (STAGE_SendReadDouble(stage, "version\n", &version) != 0)
   		return STAGE_ERROR;
	
	CmtReleaseLock(stage->_lock);

	sprintf(info, "Serial %s\nVersion %f\n", serial_str, version);
	
	return STAGE_SUCCESS;
}


static int Corvus_self_test (Stage* stage)
{
  	return STAGE_SUCCESS;
}


static int Corvus_enable_axis (Stage* stage, Axis axis)
{
	int err;
	
	CmtGetLock (stage->_lock);

	switch(axis) {
		case XAXIS: 
			err =STAGE_SendNoAnswer(stage, "1 1 setaxis\n");   //enable x
			if (!err) stage->_enabled_axis[XAXIS] = 1;
			break;
			
		case YAXIS:
			err = STAGE_SendNoAnswer(stage, "1 2 setaxis\n");  //enable y
			if (!err) stage->_enabled_axis[YAXIS] = 1;
			break;
	
		case ZAXIS:
#ifndef XY_ONLY
			err =STAGE_SendNoAnswer(stage, "1 3 setaxis\n");   //enable z
			if (!err) stage->_enabled_axis[ZAXIS]= 1;
#endif
			break;
			
		default:
			err = 1;	
	}
	
	CmtReleaseLock(stage->_lock);
  	
  	if (err) return STAGE_ERROR;
  	return STAGE_SUCCESS;
}


static int Corvus_disable_axis (Stage* stage, Axis axis)
{
	int err;
	
	CmtGetLock (stage->_lock);

	switch(axis) {
		case XAXIS: 
			err =STAGE_SendNoAnswer(stage, "0 1 setaxis\n");   //disable x
			if (!err) stage->_enabled_axis[XAXIS] = 1;
			break;
			
		case YAXIS:
			err =STAGE_SendNoAnswer(stage, "0 2 setaxis\n");   //disable y
			if (!err) stage->_enabled_axis[YAXIS] = 1;
			break;
	
		case ZAXIS:
#ifndef XY_ONLY
			err =STAGE_SendNoAnswer(stage, "0 3 setaxis\n");   //disable z
			if (!err) stage->_enabled_axis[ZAXIS] = 1;
#endif
			break;
			
		default:
			err = 1;	
	}
	
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;
	return STAGE_SUCCESS;	
}

static int Corvus_get_xyz_position (Stage* stage, double *x, double *y, double *z)
{
	char *stop_string;
	char read_data[50], stop_str[50];
	int err;
	
	// gets x,y,z coordinates of stage
	
	stop_string = stop_str;

	CmtGetLock (stage->_lock);
	err = STAGE_SendReadString(stage, "p\n",read_data); 
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;
	
	strcpy(stop_string," "); 				// coordinates separated by spaces
	*x = strtod(read_data, &stop_string); 	// picks up x coordinate and converts to double
	// strtod puts the a pointer to the first instance of stop_string into &&stop_string.
	// thus we can use stop_string as the parameter for the search for the y coordinate

	*y = strtod(stop_string, &stop_string); // get y coord

#ifndef XY_ONLY
	*z = strtod(stop_string, &stop_string); // get z coord
    (*z) /= CORVUS_F_FACTOR;	  
#else
    (*z) = 0.0;
#endif
	
  	return STAGE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////

static int Corvus_async_goto_xyz_position (Stage* stage, double x, double y, double z)
{
	// Return straight away
	int err;
	char command[50];
	
#ifdef XY_ONLY
	sprintf(command,"%.1f %.1f m\n", x, y);
#else	
	sprintf(command,"%.1f %.1f %.1f m\n", x, y, z*CORVUS_F_FACTOR);
#endif
	
	CmtGetLock (stage->_lock);
	err =  STAGE_SendNoAnswer(stage, command);

	if (stage->_joystick_status == 1)	//Moves disable the joystick
		err |= STAGE_SendNoAnswer(stage, "1 j\n");
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;

	return STAGE_SUCCESS;	
}
	
static int Corvus_async_goto_xy_position (Stage* stage, double x, double y)
{
	// Return straight away

#ifdef XY_ONLY
	int err;
	char command[50];
	
	sprintf(command,"%.1f %.1f m\n", x, y);
	
	CmtGetLock (stage->_lock);
	err =  STAGE_SendNoAnswer(stage, command);
	if (stage->_joystick_status == 1)	//Moves disable the joystick
		err |= STAGE_SendNoAnswer(stage, "1 j\n");
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;

	return STAGE_SUCCESS;
#else
	double xCur, yCur, zCur;

	if (Corvus_get_xyz_position(stage, &xCur, &yCur, &zCur) == 0)
		return Corvus_async_goto_xyz_position(stage, x, y, zCur);
	return STAGE_ERROR;
#endif
}
	
static int Corvus_async_goto_x_position (Stage* stage, double x)
{
	// Return straight away
	double xCur, yCur, zCur;

	if (Corvus_get_xyz_position(stage, &xCur, &yCur, &zCur) == 0)
		return Corvus_async_goto_xyz_position(stage, x, yCur, zCur);

	return STAGE_ERROR;
}
	
static int Corvus_async_goto_y_position (Stage* stage, double y)
{
	// Return straight away
	double xCur, yCur, zCur;

	if (Corvus_get_xyz_position(stage, &xCur, &yCur, &zCur) == 0)
		return Corvus_async_goto_xyz_position(stage, xCur, y, zCur);

	return STAGE_ERROR;
}
	
static int Corvus_async_goto_z_position (Stage* stage, double z)
{
	// Return straight away
	double xCur, yCur, zCur;

	if (Corvus_get_xyz_position(stage, &xCur, &yCur, &zCur) == 0)
		return Corvus_async_goto_xyz_position(stage, xCur, yCur, z);

	return STAGE_ERROR;
}
	

	
static int Corvus_async_rel_move_by (Stage* stage, double x, double y, double z)          
{
	char command[100], answer[100];
	int err;
	
	FillBytes (command, 0, 100, 0);

#ifdef XY_ONLY
	sprintf(command,"%.1f %.1f r st\n", x, y);   //controller will respond to st when the move is complete
#else	
	sprintf(command,"%.1f %.1f %.1f r st\n", x, y, z*CORVUS_F_FACTOR);   //controller will respond to st when the move is complete
#endif
	
	CmtGetLock (stage->_lock);
	err = STAGE_SendReadString(stage, command, answer);

	if (stage->_joystick_status == 1)	//Moves disable the joystick
		err |= STAGE_SendNoAnswer(stage, "1 j\n");
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;

	return STAGE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////
int Corvus_DatumXYF(Stage* stage, double x, double y, double f)
{
	// set x,y,f as the logical origin
	char command[100];

#ifdef XY_ONLY
	sprintf(command,"%.1f %.1f sp\n",x,y);
#else	
	sprintf(command,"%.1f %.1f %.1f sp\n",x,y,f);
#endif
	
	CmtGetLock (stage->_lock);
	if (STAGE_SendNoAnswer(stage, command)) {
		CmtReleaseLock(stage->_lock);
		return STAGE_ERROR;
	}
	Delay(0.2);	//setpos takes longer than other commands
	CmtReleaseLock(stage->_lock);
	
	return STAGE_SUCCESS;
}

static int Corvus_set_xyz_datum (Stage* stage, double x, double y, double z)
{
	//Tell stage controller that current xyz position is to become zero
	return Corvus_DatumXYF(stage, 0, 0, 0);
}

static int Corvus_set_xy_datum (Stage* stage, double x, double y)
{
	double xCur, yCur, zCur;
	
	if (Corvus_get_xyz_position(stage, &xCur, &yCur, &zCur) != 0) 
		return STAGE_ERROR;
	
	return Corvus_DatumXYF(stage, 0, 0, -zCur*CORVUS_F_FACTOR); 
}


static int Corvus_calibrate_extents (Stage* stage, int full, double *min_x, double *min_y, double *max_x, double *max_y)
{
	int retval;
	double zCur;
	char command[50]="";
	
	CmtGetLock (stage->_lock);

 	//Set calibration velocities in revs/sec. Default values from Winpos are 10.0, 0.25
	//However we have an MR encoder system. This means that at the end of "cal" 
	//there is a 2mm move to calibrate the encoders. 
	//With a speed of 0.25 rev/sec this took 16 seconds!
	sprintf(command, "%.2f 1 setcalvel\n", stage->_cal_speed_1/stage->_pitch[XAXIS]);		//towards limit switches
	if (STAGE_SendNoAnswer(stage, command)) goto Error;
	sprintf(command, "%.2f 2 setcalvel\n", stage->_cal_speed_2/stage->_pitch[XAXIS]);	//away from limit switches
	if (STAGE_SendNoAnswer(stage, command)) goto Error;
	sprintf(command, "%.2f 1 setrmvel\n", stage->_cal_speed_1/stage->_pitch[XAXIS]);		//towards limit switches
	if (STAGE_SendNoAnswer(stage, command)) goto Error;
	sprintf(command, "%.2f 2 setrmvel\n", stage->_cal_speed_2/stage->_pitch[XAXIS]);		//away from limit switches
	if (STAGE_SendNoAnswer(stage, command)) goto Error;

	if (STAGE_SendNoAnswer(stage, "0 j\n")) goto Error;  // disable joystick

	//Process disabled axes
	if (!stage->_enabled_axis[XAXIS]) {
		if (STAGE_SendNoAnswer(stage, "0 1 setaxis\n")) goto Error;
	}
	if (!stage->_enabled_axis[YAXIS]) {
		if (STAGE_SendNoAnswer(stage, "0 2 setaxis\n")) goto Error;
	}
	if (!stage->_enabled_axis[ZAXIS]) {
		if (STAGE_SendNoAnswer(stage, "0 3 setaxis\n")) goto Error;
	}
	else {
		// restrict Z axis to prevent calamity during calibration
		if (STAGE_SendNoAnswer(stage, "2 3 setaxis\n")) return -1;
	}
	
	*min_x = *min_y = 0.0;
	*max_x = *max_y = 0.0;

	if (full) {
		//Need a long timeout for stage initialisation.
		SetComTime (stage->_port, 20.0); 

		//Move to x, y "minus" limit switches. Position is automatically set to zero.
		if (STAGE_SendNoAnswer(stage, "cal\n")) goto Error;
		Delay(0.5);							 						//wait for it to start moving
		if (STAGE_SendReadInt(stage, "st\n",&retval)) goto Error;	//wait for it to stop moving

		if (Corvus_get_xyz_position(stage, min_x, min_y, &zCur) != 0) goto Error;

		if (stage_check_user_has_aborted(stage))   goto Error;

		// rm moves to maximum range values. 
		if (STAGE_SendNoAnswer(stage, "rm\n")) goto Error;
		Delay(0.5);
		if (STAGE_SendReadInt(stage, "st\n",&retval)) goto Error; 		

		if( stage_check_user_has_aborted(stage))   goto Error;

		// Get hard limits, set during calibration and rmeasure commands (values returned in um)   
		if (Corvus_get_xyz_position(stage, max_x, max_y, &zCur) != 0) goto Error;
	}
  	
	CmtReleaseLock(stage->_lock);
  	return STAGE_SUCCESS;
  	
Error:
	CmtReleaseLock(stage->_lock);
	return STAGE_ERROR;
}


static int Corvus_is_moving (Stage* stage, int *status)
{
	int err, retval;
	
	CmtGetLock (stage->_lock);
	err = STAGE_SendReadInt(stage, "st\n", &retval);
	CmtReleaseLock(stage->_lock);
	
	if (err) return STAGE_ERROR;
	
	if ((retval & 1) == 0)				//bit 0 is clear, controller is not busy
		*status = 0;
	else
		*status = 1;
	
  	return STAGE_SUCCESS;
}	

//////////////////////////////////////////////////////////////////////////
static int Corvus_set_joystick_speed (Stage* stage, double speed)
{
	char string[50];
	int err;
	
	// set joystick speed in um
	sprintf(string, "%.1f setjoyspeed\n", speed*1000.0); 
	
	CmtGetLock (stage->_lock);
	err = STAGE_SendNoAnswer(stage, string);
	CmtReleaseLock(stage->_lock);
	
   	if (err) return STAGE_ERROR;
  	
  	return STAGE_SUCCESS;
}	

	
static int Corvus_set_joystick_on (Stage* stage)
{
	int err;
	
	CmtGetLock (stage->_lock);
	err =  STAGE_SendNoAnswer(stage, "1 j\n");
	CmtReleaseLock(stage->_lock);
  	
	if (err) return STAGE_ERROR;
  	
  	return STAGE_SUCCESS;
}	

	
static int Corvus_set_joystick_off (Stage* stage)
{
	int err;
	
	CmtGetLock (stage->_lock);
	err =  STAGE_SendNoAnswer(stage, "0 j\n");
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;

	return STAGE_SUCCESS;	
}		
	
static int Corvus_send_command (Stage* stage, char* command)
{
	int err;
	char str[100];
	
	sprintf(str, "%s\n", command);
	
	CmtGetLock (stage->_lock);
	err =  STAGE_SendNoAnswer(stage, str);
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;

	return STAGE_SUCCESS;	
}		
	
int Corvus_get_response (Stage* stage, char* response)
{
	return STAGE_ReadString(stage, response);
}	
	
///////////////////////////////////////////////////////////////////////////
static int Corvus_abort_move (Stage* stage)
{
	int err;
	char command[5];
	
	//Send ctrl C
	sprintf(command,"%c",3);

	CmtGetLock (stage->_lock);
	err = STAGE_SendNoAnswer(stage, command);
	CmtReleaseLock(stage->_lock);

	if (err) return STAGE_ERROR;

	return STAGE_SUCCESS;	
}		
	
	
static int Corvus_set_timeout (Stage* stage, double timeout)
{
	SetComTime (stage->_port, timeout); 
  	
  	return STAGE_SUCCESS;
}	


static  int Corvus_save_settings (Stage* stage, const char *filename)
{
	
  	return STAGE_SUCCESS;
}	


static int Corvus_load_settings (Stage* stage, const char *filename)
{
  	
  	return STAGE_SUCCESS;
}	

	
Stage* Corvus_stage_new(void)
{
	Stage* stage = stage_new();

	Corvus_ops.init = Corvus_init;
	Corvus_ops.destroy = Corvus_destroy;
	Corvus_ops.power_up = Corvus_power_up;
	Corvus_ops.power_down = Corvus_power_down;
	Corvus_ops.reset = Corvus_reset;
	Corvus_ops.set_baud_rate = Corvus_set_baud_rate;
	Corvus_ops.get_info = Corvus_get_info;
	Corvus_ops.self_test = Corvus_self_test;
	Corvus_ops.enable_axis = Corvus_enable_axis;
	Corvus_ops.disable_axis = Corvus_disable_axis;
	Corvus_ops.set_pitch = Corvus_set_pitch;
	Corvus_ops.set_speed = Corvus_set_speed;
	Corvus_ops.get_speed = Corvus_get_speed;
	Corvus_ops.set_acceleration = Corvus_set_acceleration;
	Corvus_ops.get_acceleration = Corvus_get_acceleration;
	Corvus_ops.calibrate_extents = Corvus_calibrate_extents;
  	Corvus_ops.async_goto_xyz_position = Corvus_async_goto_xyz_position;
  	Corvus_ops.async_goto_xy_position = Corvus_async_goto_xy_position;
  	Corvus_ops.async_goto_x_position = Corvus_async_goto_x_position;
  	Corvus_ops.async_goto_y_position = Corvus_async_goto_y_position;
#ifndef XY_ONLY
  	Corvus_ops.async_goto_z_position = Corvus_async_goto_z_position;
#endif

	Corvus_ops.get_xyz_position = Corvus_get_xyz_position;
	Corvus_ops.async_rel_move_by = Corvus_async_rel_move_by;
	Corvus_ops.set_xyz_datum = Corvus_set_xyz_datum;
	Corvus_ops.set_xy_datum = Corvus_set_xy_datum;
	Corvus_ops.is_moving = Corvus_is_moving;
	Corvus_ops.set_joystick_speed = Corvus_set_joystick_speed;
	Corvus_ops.set_joystick_on = Corvus_set_joystick_on;
	Corvus_ops.set_joystick_off = Corvus_set_joystick_off;
	Corvus_ops.abort_move = Corvus_abort_move;
	Corvus_ops.send_command = Corvus_send_command;
	Corvus_ops.get_response = Corvus_get_response;
	Corvus_ops.set_timeout = Corvus_set_timeout;
	Corvus_ops.save_settings = Corvus_save_settings; 
	Corvus_ops.load_settings = Corvus_load_settings; 
	Corvus_ops.set_port_timeout = Corvus_set_port_timeout;
	
	stage_set_operations(stage, &Corvus_ops);

	return stage;
}


