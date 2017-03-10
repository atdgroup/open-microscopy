#include "ATD_CoarseZDrive_A.h"
#include "ATD_CoarseZDrive_A_UI.h" 

#include "FTDI_Utils.h"
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 
#include "dictionary.h"
#include "iniparser.h"
#include "ThreadDebug.h"
#include "password.h"

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include "asynctmr.h"
#include <rs232.h>
#include <utility.h>

// PRB Oct 2012
// Whenever the position is read, it gets saved to a file in case of crash
// the final value will be useful to the user
// we force a final read and save in destroy, otherwise user will have the panel open 
// and the timer will read and save the position
#define DEFAULT_COARSEZD_USER_FILENAME_SUFFIX "Position.ini"

static int zdrive_set_datum (CoarseZDrive* zd)
{
	unsigned char vals[10] = ""; 
	char cmd=2; 		  // Reset datum

#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;

	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

#else

	vals[0] = zd->_i2c_chip_address;  
   	vals[1] = cmd;
   	
	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "zdrive_set_datum"))
		return COARSE_Z_DRIVE_ERROR; 

#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

static int zdrive_set_full_range_travel(CoarseZDrive* zd, double full_range_um)
{ 
	unsigned char vals[10] = ""; 
	int cmd = 6; 		  //Set down distance
	double range_nm = full_range_um * 1000.0;
	int range_in_counts = (int) (range_nm /  zd->_calibration);

	unsigned char msb = range_in_counts >> 16 & 0xFF;
	unsigned char lsb_1= range_in_counts >> 8 & 0xff;
	unsigned char lsb= range_in_counts & 0xff; 
				
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;
   	vals[1] = msb;
   	vals[2] = lsb_1;
   	vals[3] = lsb; 
   			
	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

   	vals[0] = zd->_i2c_chip_address;
   	vals[1] = cmd;
   	vals[2] = msb;
   	vals[3] = lsb_1;
   	vals[4] = lsb; 
   			
	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "zdrive_set_full_range_travel"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	zd->_full_range_um = (int)full_range_um;

	return COARSE_Z_DRIVE_SUCCESS;
}

static int zdrive_set_nudge(CoarseZDrive* zd, double nudge_um)
{  
	unsigned char vals[10] = ""; 
	int cmd = 3; 		  //Set nudge
	
	double increment_nm = nudge_um * 1000.0;
	int nudge_counts = (int) (increment_nm /  zd->_calibration);

	unsigned char msb = nudge_counts >> 8 & 0xff;
	unsigned char lsb = nudge_counts & 0xff;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;
   	vals[1] = msb;
   	vals[2] = lsb;
   				
   	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

   	vals[0] = zd->_i2c_chip_address;
    vals[1] = cmd;
   	vals[2] = msb;
   	vals[3] = lsb;
   				
   	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "zdrive_set_nudge"))
		return COARSE_Z_DRIVE_ERROR; 

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

static int zdrive_set_approach_distance(CoarseZDrive* zd, double approach_um)
{  
	unsigned char vals[10] = ""; 
	int cmd = 13; 		  //Set approach distance
	
	double approach_nm = approach_um * 1000.0;
	int counts = (int) (approach_nm /  zd->_calibration);

	unsigned char msb = counts >> 8 & 0xff;
	unsigned char lsb = counts & 0xff;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;
   	vals[1] = msb;
   	vals[2] = lsb;
   				
   	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

   	vals[0] = zd->_i2c_chip_address;
    vals[1] = cmd;
   	vals[2] = msb;
   	vals[3] = lsb;
   				
   	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "zdrive_set_nudge"))
		return COARSE_Z_DRIVE_ERROR; 

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

static int zdrive_goto_top_or_bottom(CoarseZDrive* zd, int top)
{ 
	unsigned char vals[10] = ""; 
	int cmd = 1; 			  // Move up or down

	// 0 for bottom and 1 for top
	if(top != 0)
		top = 1;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0]=cmd;
   	vals[1]=top;   
   		
   	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

   	vals[0]=zd->_i2c_chip_address;
   	vals[1]=cmd;
   	vals[2]=top;   
   		
   	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "zdrive_goto_bottom"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	SetCtrlAttribute(zd->_panel_id, MAIN_PNL_NUDGE_UP, ATTR_DIMMED, 1);
	SetCtrlAttribute(zd->_panel_id, MAIN_PNL_NUDGE_DOWN, ATTR_DIMMED, 1);
	SetCtrlAttribute(zd->_setup_panel_id, SETUP_PNL_DATUM, ATTR_DIMMED, 1);

	return COARSE_Z_DRIVE_SUCCESS;
}

static int zdrive_perform_nudge(CoarseZDrive* zd, int up)
{
	unsigned char vals[10] = ""; 
	int cmd = 4; 			//Increment up or down 

	// 0 for down and 1 for up
	if(up != 0)
		up = 1;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0]=cmd;
   	vals[1]=up;    // Down
   		
   	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

   	vals[0]=zd->_i2c_chip_address;
   	vals[1]=cmd;
   	vals[2]=up;    // Down
   		
   	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "zdrive_perform_nudge"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

// This drives forever
static int zdrive_perform_drive(CoarseZDrive* zd, int up)
{
	unsigned char vals[10] = ""; 
	int cmd = 11; 			//Increment up or down 
	int direction = 2;		// Default to down

	// 0 for down and 1 for up
	if(up != 0)
		direction = 1;
	else
		direction = 2;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0]=cmd;
   	vals[1]=direction;    
   		
	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

   	vals[0]=zd->_i2c_chip_address;
   	vals[1]=cmd;
   	vals[2]=direction;    
   		
   	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "zdrive_perform_nudge"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

int atd_coarse_zdrive_save_position_to_file (CoarseZDrive* zd, int position)
{
	char filepath[GCI_MAX_PATHNAME_LEN];
	FILE *fp = NULL;
	dictionary *d = dictionary_new(1);

	sprintf(filepath, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(zd), UIMODULE_GET_NAME(zd), DEFAULT_COARSEZD_USER_FILENAME_SUFFIX);
	
	fp = fopen(filepath, "w");
	
	if(fp == NULL)
		return COARSE_Z_DRIVE_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(zd), NULL);

	dictionary_setint(d, "Position", position);  

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return COARSE_Z_DRIVE_SUCCESS;
}

int atd_coarse_zdrive_read_position_from_file (CoarseZDrive* zd, int *position)
{
	char filepath[GCI_MAX_PATHNAME_LEN];
	char buffer[500] = "";
	dictionary* d = NULL;
	int file_size;    

	sprintf(filepath, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(zd), UIMODULE_GET_NAME(zd), DEFAULT_COARSEZD_USER_FILENAME_SUFFIX);

	if(!FileExists(filepath, &file_size)) {
		*position = 0;
		return COARSE_Z_DRIVE_SUCCESS;	 	  // assume all ok
	}
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		*position = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "Position"), 0.0); 
	}

    dictionary_del(d);

	return COARSE_Z_DRIVE_SUCCESS;
}

static int zdrive_read_position(CoarseZDrive* zd, int *position, unsigned int *counts, unsigned int *stop_ind, int save_value)
{ 
	byte vals[10] = "";  
	unsigned int msb,lsb_2,lsb_1,lsb;
	double position_um, position_nm;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	ftdi_controller_get_lock(zd->controller);

   	vals[0]=254; 	

	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		ftdi_controller_release_lock(zd->controller);
		return COARSE_Z_DRIVE_ERROR;
	}


	if(ftdi_controller_i2c_read_bytes(zd->controller, zd->_i2c_chip_address, 5, vals) != FT_OK) {
		ftdi_controller_release_lock(zd->controller);
		*position = 0;
		*counts = 0;
		*stop_ind = 0;
		return COARSE_Z_DRIVE_ERROR;
	}
   								
	ftdi_controller_release_lock(zd->controller);

	#else

	GetI2CPortLock(zd->_com_port, "zdrive_read_position");    
	
   	vals[0]=zd->_i2c_chip_address;
   	vals[1]=254; 	

	if(GCI_writeI2C_multiPort(zd->_com_port,6, vals, zd->_i2c_bus, "zdrive_read_position")) {
		ReleaseI2CPortLock(zd->_com_port, "zdrive_read_position");  
		return COARSE_Z_DRIVE_ERROR; 
	}
   				
   	vals[0]=zd->_i2c_chip_address | 0x01;
				
	if (GCI_readI2C_multiPort(zd->_com_port,5, vals, zd->_i2c_bus, "zdrive_read_position")) {
		
		ReleaseI2CPortLock(zd->_com_port, "zdrive_read_position");
		*position = 0;
		*counts = 0;
		*stop_ind = 0;

		logger_log(UIMODULE_LOGGER(zd), LOGGER_ERROR, "Coarse ZDrive Error", "GCI_readI2C_multiPort failed");

	    return COARSE_Z_DRIVE_ERROR;	
	}
			
	ReleaseI2CPortLock(zd->_com_port, "zdrive_read_position");  
	
	#endif

	msb = vals[0] & 0xff; 
    lsb_2 = vals[1] & 0xff; 
    lsb_1 = vals[2] & 0xff; 
    lsb = vals[3] & 0xff; 
    *stop_ind = vals[4] & 0xff;  
    		
	// The scale count of the device is around 4 billion so rob has add half scale.
	//Offset by 2x10^9 so counts will not go through zero and roll over
	*counts = ((msb<<24) | (lsb_2<<16) | (lsb_1<<8) | lsb) + 2000000000;    
    											
    //SetCtrlVal(setupPanel, SETUP_PNL_FB_POSITION ,counts);
    position_nm = *counts *  zd->_calibration;
	position_um = position_nm / 1000;

	// Offset taken off to get zero position - ROB
	// Not sure what this means. Seems to be another arbitary constant ?
	// dealing in mm where calibration is nm / count ?  - GLENN
    *position = (int) (position_um - (2000 * 1000 * zd->_calibration));

	// PRB
	// What is this above? If you do the simple maths:
	// *position = ((msb<<24) | (lsb_2<<16) | (lsb_1<<8) | lsb) * zd->_calibration / 1000.0;

	if (save_value) {
		// In an attempt to save the z drive state if a crash happens, save the position when it is read
		atd_coarse_zdrive_save_position_to_file(zd, *position);
	}

	return COARSE_Z_DRIVE_SUCCESS; 
}

int coarse_zdrive_move_to_top(CoarseZDrive* zd)
{
	// Dim Top button
	SetCtrlAttribute(zd->_panel_id, MAIN_PNL_MOVE_TO_TOP, ATTR_DIMMED, 1);

	zdrive_goto_top_or_bottom(zd, 1);

	return COARSE_Z_DRIVE_SUCCESS; 
}

int coarse_zdrive_move_to_bottom(CoarseZDrive* zd)
{
	// UnDim Top button
	SetCtrlAttribute(zd->_panel_id, MAIN_PNL_MOVE_TO_TOP, ATTR_DIMMED, 0);

	zdrive_goto_top_or_bottom(zd, 0);

	return COARSE_Z_DRIVE_SUCCESS; 
}

int coarse_zdrive_wait_for_stop_moving(CoarseZDrive* zd)
{
	double start_time;
	unsigned int counts;
	unsigned int stop_ind;
	int position;
		
	start_time = Timer();
	stop_ind = 0;

	while(stop_ind == 0) {

		// Timeout specified in minutes.
		if((Timer() - start_time) > (zd->_timeout * 60))
			break;

		Delay(0.1);
		ProcessSystemEvents();
		zdrive_read_position(zd, &position, &counts, &stop_ind, 0);
	}

	SetCtrlAttribute(zd->_panel_id, MAIN_PNL_NUDGE_UP, ATTR_DIMMED, 0);
	SetCtrlAttribute(zd->_panel_id, MAIN_PNL_NUDGE_DOWN, ATTR_DIMMED, 0);
	SetCtrlAttribute(zd->_setup_panel_id, SETUP_PNL_DATUM, ATTR_DIMMED, 0);

	// final read and save of position	
	zdrive_read_position(zd, &position, &counts, &stop_ind, 1);

	return COARSE_Z_DRIVE_SUCCESS;
}

static int CVICALLBACK OnMoveToTop (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			
			coarse_zdrive_move_to_top(zd);
   	
			break;
		}
	}

	return 0;
}

static int coarse_zdrive_stop(CoarseZDrive* zd)
{
	int cmd = 0;
	char vals[10] = "";

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;

	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

	vals[0]=zd->_i2c_chip_address;
	vals[1] = cmd;

	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "coarse_zdrive_stop"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

static int CVICALLBACK OnMoveToBottom (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 

			coarse_zdrive_move_to_bottom(zd);
   	
			break;
		}
	}

	return 0;
}

static int CVICALLBACK drive_thread(void *callback)
{
	CoarseZDrive* zd = (CoarseZDrive* ) callback; 
	double start_time = Timer();

	GciCmtGetLock (zd->_driving_lock);

	zd->_driving = 0;

	while((Timer() - start_time) < 2.0) {

		ProcessSystemEvents();

		if ( GetAsyncKeyState(VK_LBUTTON) & 0x8000 ) {
			continue;
		}
		else {
			GciCmtReleaseLock (zd->_driving_lock);
			return 0;
		}
	}

	zdrive_perform_drive(zd, zd->_drive_direction);
	zd->_driving = 1;

	GciCmtReleaseLock (zd->_driving_lock);

	return 0;   
}

static int CVICALLBACK OnMovePositiveIncrement (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 

			if(zd->_driving) {
				coarse_zdrive_stop(zd);
				printf("performing stop\n");
			}
			else {
				zdrive_perform_nudge(zd, 1);
				printf("performing nudge\n");
			}

			zd->_driving = 0;

			break;
		}

		case EVENT_LEFT_CLICK:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			int thread_id, return_value;
	
			zd->_drive_direction = 1;

			if(zd->_driving)
				return 0;

			CmtScheduleThreadPoolFunction (gci_thread_pool(), drive_thread, zd, &thread_id);
			CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_RETURN_VALUE, &return_value);

			break;
		}
	}

	return 0;
}



static int CVICALLBACK OnMoveNegativeIncrement (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 

			if(zd->_driving) {
				coarse_zdrive_stop(zd);
				printf("performing stop\n");
			}
			else {
				zdrive_perform_nudge(zd, 0);

				printf("performing nudge\n");
			}

			zd->_driving = 0;

			break;
		}

		case EVENT_LEFT_CLICK:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			int thread_id, return_value;
	
			zd->_drive_direction = 0;

			if(zd->_driving)
				return 0;

			CmtScheduleThreadPoolFunction (gci_thread_pool(), drive_thread, zd, &thread_id);
			CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_RETURN_VALUE, &return_value);

			break;
		}
	}

	return 0;


	return 0;
}

static int CVICALLBACK OnSetDatumValue (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			double range;
			int position;
			unsigned int stop_ind, counts;

			zdrive_set_datum (zd);
   	
			// Now we have set the datum we can also set the top and botton values
			zdrive_read_position(zd, &position, &counts, &stop_ind, 1);

			GetCtrlVal(panel, SETUP_PNL_RANGE, &range);  

			break;
		}
	}

	return 0;
}

static int coarse_zdrive_set_backlash(CoarseZDrive* zd, double backlash_um)
{
	int lsb, msb;
	int cmd = 10;
	char vals[10] = "";

	double backlash_nm = backlash_um * 1000.0;
	unsigned int backlash_counts = (unsigned int) (backlash_nm /  zd->_calibration);

	//msb = HIWORD(backlash_counts);
	//lsb = LOWORD(backlash_counts);

	msb = backlash_counts >> 8 & 0x00ff;
	lsb = backlash_counts & 0x00ff;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;
	vals[1] = msb;
	vals[2] = lsb;

	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

	vals[0] = zd->_i2c_chip_address;
	vals[1] = cmd;
	vals[2] = msb;
	vals[3] = lsb;

	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "coarse_zdrive_set_nudge_speed"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

static int coarse_zdrive_set_minimum_speed(CoarseZDrive* zd, int minimum_speed_percentage)
{
	int lsb, msb;
	int cmd = 8, minimum_speed;
	char vals[10] = "";

	minimum_speed = (int) (minimum_speed_percentage * 1023.0 / 100.0);

	msb = minimum_speed >> 8 & 0x00ff;
	lsb = minimum_speed & 0x00ff;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;
	vals[1] = msb;
	vals[2] = lsb;

	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

	vals[0] = zd->_i2c_chip_address;
	vals[1] = cmd;
	vals[2] = msb;
	vals[3] = lsb;

	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "coarse_zdrive_set_nudge_speed"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

static int coarse_zdrive_set_nudge_speed(CoarseZDrive* zd, int nudge_speed_percentage)
{
	int lsb, msb;
	int cmd = 9, nudge_speed;
	char vals[10] = "";

	nudge_speed = (int) (nudge_speed_percentage * 1023.0 / 100.0);

	msb = nudge_speed >> 8 & 0xff;
	lsb = nudge_speed & 0xff;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;
	vals[1] = msb;
	vals[2] = lsb;

	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

	vals[0] = zd->_i2c_chip_address;
	vals[1] = cmd;
	vals[2] = msb;
	vals[3] = lsb;

	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "coarse_zdrive_set_nudge_speed"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

static int coarse_zdrive_set_drive_speed(CoarseZDrive* zd, int drive_speed_percentage)
{
	int lsb, msb;
	int cmd = 13, drive_speed;
	char vals[10] = "";

	drive_speed = (int) (drive_speed_percentage * 1023.0 / 100.0);

	msb = drive_speed >> 8 & 0xff;
	lsb = drive_speed & 0xff;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0] = cmd;
	vals[1] = msb;
	vals[2] = lsb;

	if(ftdi_controller_i2c_write_bytes(zd->controller, zd->_i2c_chip_address, 6, vals) != FT_OK) {
		return COARSE_Z_DRIVE_ERROR;
	}

	#else

	vals[0] = zd->_i2c_chip_address;
	vals[1] = cmd;
	vals[2] = msb;
	vals[3] = lsb;

	if(GCI_writeI2C_multiPort(zd->_com_port, 6, vals, zd->_i2c_bus, "coarse_zdrive_set_drive_speed"))
		return COARSE_Z_DRIVE_ERROR;

	#endif

	return COARSE_Z_DRIVE_SUCCESS;
}

static int CVICALLBACK OnSetApproachSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			int value = 0;

			GetCtrlVal(panel, control, &value);  

			coarse_zdrive_set_minimum_speed(zd, value);
			
			break;
		}
	}

	return 0;
}

static int CVICALLBACK OnSetBacklash (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			int value = 0;

			GetCtrlVal(panel, control, &value);  

			coarse_zdrive_set_backlash(zd, (double) value);
			
			break;
		}
	}

	return 0;
}

static int CVICALLBACK OnSetDriveSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			int value = 0;

			GetCtrlVal(panel, control, &value);  

			coarse_zdrive_set_drive_speed(zd, value);
			
			break;
		}
	}

	return 0;
}


static int CVICALLBACK OnSetTimeout (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 

			GetCtrlVal(panel, control, &(zd->_timeout));  

			break;
		}
	}

	return 0;
}

static int CVICALLBACK OnSetNudgeSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData;
			int val = 0;

			GetCtrlVal(panel, control, &val);
			coarse_zdrive_set_nudge_speed(zd, val);

			break;
		}
	}

	return 0;
}

static int CVICALLBACK OnFullRangeTravelSetCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			int val = 0;

			GetCtrlVal(panel, control, &val);  
			zdrive_set_full_range_travel(zd, (double) val);

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnNudgeDistanceSetCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			int val = 0;

			GetCtrlVal(panel, control, &val);  

			zdrive_set_nudge(zd, (double) val);

			break;
		}
	}
	
	return 0;
}

static  int CVICALLBACK OnCalibrationCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int int_val;
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 

			GetCtrlVal(panel, SETUP_PNL_CAL, &(zd->_calibration));  

			GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_RANGE, &int_val);
			zdrive_set_full_range_travel(zd, (double) int_val);

			GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_NUDGE_DST, &int_val);
			zdrive_set_nudge(zd, (double) int_val);

			break;
		}
	}
	
	return 0;
}


static  int CVICALLBACK OnCoarseZDriveClosedClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 

			ui_module_hide_panel(UIMODULE_CAST(zd), zd->_setup_panel_id);
			
			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnCoarseZDrive_ClosePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 

			ui_module_hide_main_panel(UIMODULE_CAST(zd));

			break;
		}
	}
	
	return 0;
}


static int CVICALLBACK OnCoarseZDrive_SetupPressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 

			//ui_module_display_panel(UIMODULE_CAST(zd), zd->_setup_panel_id);
		
			GCI_ShowPasswordProtectedPanel(zd->_setup_panel_id, zd->_panel_id);  

			break;
		}
	}
	return 0;
}

static int atd_b_zdrive_on_timer_tick (CoarseZDrive* zd)
{
	int position;
	unsigned int counts;
	unsigned int stop_ind;
	
	zdrive_read_position(zd, &position, &counts, &stop_ind, 1);

	SetCtrlVal(zd->_panel_id, MAIN_PNL_POS, position); 
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_POS, position);
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_COUNTS, counts);

	//Set indicators
	switch(stop_ind)
	{		 
		case 0:
			SetCtrlVal(zd->_panel_id, MAIN_PNL_LED_UP, 0);
			SetCtrlVal(zd->_panel_id, MAIN_PNL_LED_DOWN ,0); 
			break;
		case 1:
			SetCtrlVal(zd->_panel_id, MAIN_PNL_LED_UP, 0); 
			SetCtrlVal(zd->_panel_id, MAIN_PNL_LED_DOWN, 1); 

			SetCtrlAttribute(zd->_panel_id, MAIN_PNL_NUDGE_UP, ATTR_DIMMED, 0);
			SetCtrlAttribute(zd->_panel_id, MAIN_PNL_NUDGE_DOWN, ATTR_DIMMED, 0);
			SetCtrlAttribute(zd->_setup_panel_id, SETUP_PNL_DATUM, ATTR_DIMMED, 0);

			break;
		case 2:
			SetCtrlVal(zd->_panel_id, MAIN_PNL_LED_UP, 1);
			SetCtrlVal(zd->_panel_id, MAIN_PNL_LED_DOWN ,0); 

			SetCtrlAttribute(zd->_panel_id, MAIN_PNL_NUDGE_UP, ATTR_DIMMED, 0);
			SetCtrlAttribute(zd->_panel_id, MAIN_PNL_NUDGE_DOWN, ATTR_DIMMED, 0);
			SetCtrlAttribute(zd->_setup_panel_id, SETUP_PNL_DATUM, ATTR_DIMMED, 0);

			break;
	}

	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(zd), "CoarseZDrivePositionChanged", GCI_VOID_POINTER, zd); 

	return COARSE_Z_DRIVE_SUCCESS;
}

static int atd_coarse_zdrive_destroy (CoarseZDrive* zd)
{
	int position=0;
	unsigned int counts;
	unsigned int stop_ind;
	
	
	// move to normal position on exit, so that next time this remains the datum
	// But make sure it gets to the datum, even if starting from above the datum
	// by driving down to some -ve value and then up again
	// (hw ignores commands to go to top if already above the top)
	coarse_zdrive_move_to_bottom(zd);
	do {
		zdrive_read_position(zd, &position, &counts, &stop_ind, 0);
	} while (position > -100);
	coarse_zdrive_move_to_top(zd);

	// finally read and save the position
	coarse_zdrive_wait_for_stop_moving(zd);

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(zd->controller);
	#else
	close_comport(zd->_com_port);
	#endif

	return COARSE_Z_DRIVE_SUCCESS;  
}

static int CVICALLBACK OnCoarseZDriveTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {	
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			
			atd_b_zdrive_on_timer_tick (zd);

            break;
		}
    }
    
    return 0;
}

void coarse_zdrive_disable_timer(CoarseZDrive* zd)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute(zd->_panel_id, zd->_timer, ATTR_ENABLED, 0);
	#else
	SetAsyncTimerAttribute (zd->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void coarse_zdrive_enable_timer(CoarseZDrive* zd)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute(zd->_panel_id, zd->_timer, ATTR_ENABLED, 1);
	#else
	SetAsyncTimerAttribute (zd->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}

static int COARSE_ZDRIVE_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (CoarseZDrive*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (CoarseZDrive *) args[0].void_ptr_data,  (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

static int atd_coarse_zdrive_hardware_device_initialise (HardwareDevice* device)
{
	CoarseZDrive* zd = (CoarseZDrive *) device;  

	char device_name[UIMODULE_NAME_LEN];
	char device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH] = "";

	ui_module_get_name(UIMODULE_CAST(zd), device_name);
		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	get_device_string_param_from_ini_file(device_name, "FTDI_SN", device_sn);  
	get_device_int_param_from_ini_file   (device_name, "I2C_DEVICE_BUS", &(zd->_i2c_bus));
	get_device_int_param_from_ini_file   (device_name, "I2C_DEVICE_ADDRESS", &(zd->_i2c_chip_address));  
	
	zd->controller = ftdi_controller_new();

	//ftdi_controller_set_debugging(zd->controller, 1);
	ftdi_controller_set_error_handler(zd->controller, ftdi_error_handler, NULL);
	ftdi_controller_open(zd->controller, device_sn);

	#else

	get_device_int_param_from_ini_file   (device_name, "COM_Port", &(zd->_com_port));  
	get_device_int_param_from_ini_file   (device_name, "i2c_Bus", &(zd->_i2c_bus));
	get_device_int_param_from_ini_file   (device_name, "i2c_ChipAddress", &(zd->_i2c_chip_address));  
	get_device_int_param_from_ini_file   (device_name, "i2c_ChipType", &(zd->_i2c_chip_type));  
	
	if (initialise_comport(zd->_com_port, 9600) < 0)
		return COARSE_Z_DRIVE_ERROR;

	#endif

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(device_name), "TimerInterval", &(zd->_timer_interval))<0) 
		zd->_timer_interval=0.2;

	// need to load state here for initialisation of stage as Z drive must be used there, UI not created yet - call function later as well
	hardware_load_state_from_file(HARDWARE_DEVICE_CAST(zd), zd->_settings_file);
	zdrive_set_datum (zd);

	return COARSE_Z_DRIVE_SUCCESS;   
}

int coarse_z_drive_destroy(CoarseZDrive* zd)
{
	coarse_zdrive_disable_timer(zd);

	Delay(0.2);

	CHECK_COARSE_Z_DRIVE_VTABLE_PTR(zd, destroy) 
  	
	CALL_COARSE_Z_DRIVE_VTABLE_PTR(zd, destroy) 
	
	CmtDiscardLock(zd->_lock);
	CmtDiscardLock(zd->_driving_lock);

	ui_module_destroy(UIMODULE_CAST(zd));  
  	
  	free(zd);
  	
  	return COARSE_Z_DRIVE_SUCCESS;
}

static int CVICALLBACK OnLoad (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			char path[GCI_MAX_PATHNAME_LEN] = "";

            if (FileSelectPopup (UIMODULE_GET_DATA_DIR(zd), zd->_settings_file, "*.ini", "Load", VAL_LOAD_BUTTON, 0, 1, 1, 0, path) <= 0)
				return 0;
	
			if (!FileExists(path, NULL))
                return 0;
                
			hardware_load_state_from_file(HARDWARE_DEVICE_CAST(zd), path);

			break;  
		}
	}
	
	return 0;
}

static int CVICALLBACK OnSave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CoarseZDrive* zd = (CoarseZDrive* ) callbackData; 
			int ret;

			ret = hardware_save_state_to_file(HARDWARE_DEVICE_CAST(zd), zd->_settings_file, "w");

			if (HARDWARE_SUCCESS==ret) {
				GCIDialog(0, "Configuration Saved", IDI_INFORMATION, GCI_OK_BUTTON, "Configuration saved successfully to:\n%s", zd->_settings_file);
			}
			else {
				GCIDialog(0, "Configuration Save Error", IDI_EXCLAMATION, GCI_OK_BUTTON, "Configuration FAILED to save to:\n%s", zd->_settings_file);
			}
      
			break; 
		}
	}
	
	return 0;
}

//
static void OnCoarseZDrivePanelsClosedOrHidden (UIModule *module, void *data)
{
	CoarseZDrive* zd = (CoarseZDrive*) data; 

	coarse_zdrive_disable_timer(zd);
}

static void OnCoarseZDrivePanelsDisplayed (UIModule *module, int panel_id, void *data)
{
	CoarseZDrive* zd = (CoarseZDrive*) data; 

	coarse_zdrive_enable_timer(zd);
}

int coarse_z_drive_initialise(CoarseZDrive* zd)
{
	// This overrides the default main panel
	zd->_panel_id = ui_module_add_panel(UIMODULE_CAST(zd), "ATD_CoarseZDrive_A_UI.uir", MAIN_PNL, 1); 

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(zd), OnCoarseZDrivePanelsClosedOrHidden, zd);
	ui_module_panel_show_handler_connect (UIMODULE_CAST(zd), OnCoarseZDrivePanelsDisplayed, zd);

	if(InstallCtrlCallback (zd->_panel_id, MAIN_PNL_MOVE_TO_TOP, OnMoveToTop, zd) < 0)
		return COARSE_Z_DRIVE_ERROR;

	if(InstallCtrlCallback (zd->_panel_id, MAIN_PNL_MOVE_TO_BOTT, OnMoveToBottom, zd) < 0)
		return COARSE_Z_DRIVE_ERROR;

	InstallCtrlCallback (zd->_panel_id, MAIN_PNL_NUDGE_UP, OnMovePositiveIncrement, zd);
	InstallCtrlCallback (zd->_panel_id, MAIN_PNL_NUDGE_DOWN, OnMoveNegativeIncrement, zd);
	InstallCtrlCallback (zd->_panel_id, MAIN_PNL_SETUP, OnCoarseZDrive_SetupPressed, zd);
	InstallCtrlCallback (zd->_panel_id, MAIN_PNL_CLOSE, OnCoarseZDrive_ClosePressed, zd);

	// Setup the setup panel
	zd->_setup_panel_id = ui_module_add_panel(UIMODULE_CAST(zd), "ATD_CoarseZDrive_A_UI.uir", SETUP_PNL, 0);
	hardware_load_state_from_file(HARDWARE_DEVICE_CAST(zd), zd->_settings_file);  // load here for the ui
	
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_NUDGE_DST, OnNudgeDistanceSetCallback, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_RANGE, OnFullRangeTravelSetCallback, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_CAL, OnCalibrationCallback, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_DATUM, OnSetDatumValue, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_CLOSE, OnCoarseZDriveClosedClicked, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_APPROACH_SPD, OnSetApproachSpeed, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_NUDGE_SPEED, OnSetNudgeSpeed, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_DRIVE_SPEED, OnSetDriveSpeed, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_BACKLASH, OnSetBacklash, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_TIMEOUT, OnSetTimeout, zd);

	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_SAVE, OnSave, zd);
	InstallCtrlCallback (zd->_setup_panel_id, SETUP_PNL_LOAD, OnLoad, zd);
	
	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_CAL, &(zd->_calibration)); 
	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_RANGE, &(zd->_full_range_um)); 

	zd->_timer = NewAsyncTimer (zd->_timer_interval, -1, 1, OnCoarseZDriveTimerTick, zd);
	SetAsyncTimerName(zd->_timer, "CoarseZDrive");
	SetAsyncTimerAttribute (zd->_timer, ASYNC_ATTR_ENABLED,  0);

	// Call specific device initialisation if necessary.
//	if( (zd->vtable.initialise != NULL)) {
		
//		if( (zd->vtable.initialise)(zd) == COARSE_Z_DRIVE_ERROR )
//			return COARSE_Z_DRIVE_ERROR;  	
//	}
	
	zd->_initialised = 1;

	return COARSE_Z_DRIVE_SUCCESS;   
}

int coarse_z_drive_hardware_initialise(CoarseZDrive* zd)
{
	if(hardware_device_hardware_initialise(HARDWARE_DEVICE_CAST(zd)) != HARDWARE_SUCCESS)
		return COARSE_Z_DRIVE_ERROR;  	

	return COARSE_Z_DRIVE_SUCCESS;   
}

int coarse_z_drive_hardware_is_initialised(CoarseZDrive* zd)
{
	return hardware_device_hardware_is_initialised (HARDWARE_DEVICE_CAST(zd));
}


int coarse_z_drive_is_initialised(CoarseZDrive* zd)
{
	return zd->_initialised;
}

int atd_coarse_zdrive_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	CoarseZDrive* zd = (CoarseZDrive*)device;
	int int_val;
	double double_val;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return COARSE_Z_DRIVE_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(zd), NULL);

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_CAL ,&double_val);
	dictionary_setdouble(d, "Calibration", double_val);  

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_NUDGE_DST ,&int_val);
	dictionary_setint(d, "Nudge", int_val);      

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_APPROACH_DIST ,&int_val);
	dictionary_setint(d, "ApproachDistance", int_val);      

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_RANGE, &int_val);
	dictionary_setint(d, "FullRangeTravel", int_val);   

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_APPROACH_SPD, &int_val);
	dictionary_setint(d, "ApproachSpeed", int_val);

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_NUDGE_SPEED, &int_val);
	dictionary_setint(d, "NudgeSpeed", int_val);     

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DRIVE_SPEED, &int_val);
	dictionary_setint(d, "PostNudgeSpeed", int_val);     

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_NUDGE_SPEED ,&int_val);
	dictionary_setint(d, "MinimumIncrementSpeed", int_val);      

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_BACKLASH ,&int_val);
	dictionary_setint(d, "Backlash", int_val);      

	GetCtrlVal(zd->_setup_panel_id, SETUP_PNL_TIMEOUT ,&double_val);
	dictionary_setdouble(d, "Timeout", double_val);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return COARSE_Z_DRIVE_SUCCESS;
}

int atd_coarse_zdrive_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	CoarseZDrive* zd = (CoarseZDrive*)device;
	dictionary* d = NULL;
	int file_size, int_val;    
	char buffer[500] = "";
	double double_val;

	if(!FileExists(filepath, &file_size))
		return COARSE_Z_DRIVE_ERROR;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {

		double_val = dictionary_getdouble(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "Calibration"), -1.0); 

		if(double_val > 0) {
			zd->_calibration = double_val;
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_CAL, double_val);
		}

		int_val = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "Nudge"), -1); 

		if(int_val > 0) {
			zdrive_set_nudge(zd, int_val);
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_NUDGE_DST, int_val);
		}

		int_val = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "ApproachDistance"), -1); 

		if(int_val > 0) {
			zdrive_set_approach_distance(zd, int_val);
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_APPROACH_DIST, int_val);
		}

		int_val = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "FullRangeTravel"), -1); 

		if(int_val > 0) {
			zdrive_set_full_range_travel(zd, int_val);
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_RANGE, int_val);
		}

		int_val = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "ApproachSpeed"), -1); 

		if(int_val > 0) {
			coarse_zdrive_set_minimum_speed(zd, int_val);
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_APPROACH_SPD, int_val);
		}

		int_val = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "PostNudgeSpeed"), -1); 

		if(int_val > 0) {
			coarse_zdrive_set_drive_speed(zd, int_val);
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DRIVE_SPEED, int_val);
		}

		int_val = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "NudgeSpeed"), -1); 

		if(int_val > 0) {
			coarse_zdrive_set_nudge_speed(zd, int_val);
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_NUDGE_SPEED, int_val);
		}

		int_val = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "Backlash"), -1); 

		if(int_val > 0) {
			coarse_zdrive_set_backlash(zd, int_val);
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_BACKLASH, int_val);
		}

		double_val = dictionary_getdouble(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(zd), "Timeout"), -1.0); 

		if(double_val > 0) {
			zd->_timeout = double_val;
			SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_TIMEOUT, double_val);
		}
	}

    dictionary_del(d);

	return COARSE_Z_DRIVE_SUCCESS;
}

CoarseZDrive* atd_coarse_zdrive_a_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir, const char *data_file)
{
	CoarseZDrive* zd = (CoarseZDrive*) malloc(sizeof(CoarseZDrive));  
	int last_position=0.0;

	memset(zd, 0, sizeof(CoarseZDrive));  
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(zd), name);
	ui_module_set_description(UIMODULE_CAST(zd), description);
	ui_module_set_data_dir(UIMODULE_CAST(zd), data_dir);   
	
	zd->_timeout = 2.0;
	sprintf(zd->_settings_file, "%s\\%s", data_dir, data_file);

	GciCmtNewLock ("CoarseZDriveDriveLock", 0, &zd->_driving_lock);

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(zd), "CoarseZDrivePositionChanged", COARSE_ZDRIVE_PTR_INT_MARSHALLER); 
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(zd), hardware_initialise) = atd_coarse_zdrive_hardware_device_initialise; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(zd), hardware_save_state_to_file) = atd_coarse_zdrive_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(zd), hardware_load_state_from_file) = atd_coarse_zdrive_hardware_load_state_from_file; 

	COARSE_Z_DRIVE_VTABLE_PTR(zd, initialise) = coarse_z_drive_initialise; 
	COARSE_Z_DRIVE_VTABLE_PTR(zd, destroy) = atd_coarse_zdrive_destroy; 

	// read last position from file and see if we shutdown properly last time
	atd_coarse_zdrive_read_position_from_file(zd, &last_position);
	if (abs(last_position) > 5) {  // more that some value from zero
		GCI_MessagePopup("Warning", "%s did not return to datum at the end of the last session.\nPosition:%d um\nYou may wish to drive it to %d and reset the datum\nbefore you allow the XY stage to initialise.", UIMODULE_GET_DESCRIPTION(zd), last_position, -last_position);
		logger_log(UIMODULE_LOGGER(zd), LOGGER_WARNING, UIMODULE_GET_DESCRIPTION(zd), "non-zero initial position: %d", last_position);
	}

	return zd;
}
