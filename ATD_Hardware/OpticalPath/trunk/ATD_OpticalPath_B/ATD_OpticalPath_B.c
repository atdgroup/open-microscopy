#include <cviauto.h>

#include "ATD_OpticalPath_B.h"
#include "ATD_OpticalPath_B_UI.h"
#include "OpticalPathUI.h"
#include "ATD_UsbInterface_A.h"
#include "gci_utils.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define EPROM_WRITE_TIME	0.05	   // Time to write to ROM on pic	 
#define MAX_POSITION 31500

// Set does not move the lift. This is unfortunate terminology from Rob.
// I think it temporalily sets a the position of a point to move to
static int optical_path_send_position(OpticalPathManager* op, int position)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;    
	byte vals[10] = ""; 

	GetI2CPortLock(atd_b_op->_com_port, "optical_path_send_position");    

	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=0;		// Set position mode
   	vals[2]=position>>8;			
   	vals[3]=position & 0xff; 
	
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_send_position"); 

	ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_send_position");    
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int optical_path_store_current_pid(OpticalPathManager* op)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;  
	byte vals[10] = ""; 

	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=6;	 //Save PID factors
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_store_current_pid"); 
	
	Delay(EPROM_WRITE_TIME);
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int optical_path_set_deadband(OpticalPathManager* op, int deadband)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;  
	byte vals[10] = ""; 

	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=10;	 // Set deadband
	vals[2]=deadband>>8;	 // Set deadband
	vals[3]=deadband & 0xff;

   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_store_current_pid"); 
	
	Delay(EPROM_WRITE_TIME);
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int optical_path_send_position_to_memory(OpticalPathManager* op, int position, int point)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;    
	byte vals[10] = ""; 
		
   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=8;	 //Store position in memory
   	vals[2]=position;
   	vals[3]=point>>8; 
   	vals[4]=point & 0xff; 
	
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_send_position"); 
	
	Delay(EPROM_WRITE_TIME);
		
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int optical_path_send_deadband_memory(OpticalPathManager* op, int deadband)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;    
	byte vals[10] = ""; 
		
	GetI2CPortLock(atd_b_op->_com_port, "optical_path_send_deadband_memory");    

   	optical_path_set_deadband(op, deadband);

	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=11;	 // Store deadband

   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_send_position"); 
	
	ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_send_deadband_memory");    

	Delay(EPROM_WRITE_TIME);
		
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int optical_path_enable(OpticalPathManager* op, int enable)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;  
	byte vals[6] = ""; 

   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=5;	
   	vals[2]=enable;			
   				
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_enable"); 	
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int atd_optical_path_moveto_position(OpticalPathManager* op, int position)
{			 
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;    
	byte vals[10] = ""; 

	GetI2CPortLock(atd_b_op->_com_port, "atd_optical_path_moveto_position");    

	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=9;	 //Go to position mode
   	vals[2]=position;			
   				
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "atd_optical_path_moveto_position"); 
	ReleaseI2CPortLock(atd_b_op->_com_port, "atd_optical_path_moveto_position");    
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int optical_path_read_position_advanced(OpticalPathManager* op, int *position, int *lsb, int *msb)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;    
	byte vals[10] = "";  

	GetI2CPortLock(atd_b_op->_com_port, "optical_path_read_position_advanced");    
	
   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=255;  //Set mode   
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_read_position"); 
   				
   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1) | 0x01;
				
	if (GCI_readI2C_multiPort(atd_b_op->_com_port,2, vals, atd_b_op->_i2c_bus, "optical_path_read_position")) {
		ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_read_position_advanced");  
	    return OPTICAL_PATH_MANAGER_ERROR;	
	}
			
	ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_read_position_advanced");  
	
    *msb = vals[0] & 0xff; 
   	*lsb = vals[1] & 0xff; 
	*position= ((*msb<<8) | *lsb); 
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int optical_path_read_pid_values_from_eprom(OpticalPathManager* op, OPB_PID_PARAM_TYPE type, int *val)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;  
	unsigned char vals[20] = "";  
	int propGain,intGain,dirGain,propGain_lsb,intGain_lsb,dirGain_lsb;
	
	GetI2CPortLock(atd_b_op->_com_port, "optical_path_read_pid_values_from_eprom");    

   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=254;
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_read_pid_values_from_eprom"); 
   				
   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1) | 0x01;
    
	if (GCI_readI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_read_pid_values_from_eprom")) {
		ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_read_pid_values_from_eprom"); 	
		return -1;	
	}

	ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_read_pid_values_from_eprom"); 

    propGain = vals[0] & 0xff; 
    propGain_lsb = vals[1] & 0xff;  
    intGain = vals[2] & 0xff; 
    intGain_lsb = vals[3] & 0xff; 
    dirGain = vals[4] & 0xff;
    dirGain_lsb = vals[5] & 0xff; 
    			
    propGain=(propGain<<8) |  propGain_lsb;
    intGain=(intGain<<8) | intGain_lsb; 
    dirGain=(dirGain<<8) |  dirGain_lsb; 
    			
	if(type == OPB_PID_PARAM_PROPORTIONAL) {
		*val = propGain;	
	} 
	else if(type == OPB_PID_PARAM_INTEGAL) {
		*val = intGain;	
	}
	else if(type == OPB_PID_PARAM_DERIVITIVE) {
		*val = dirGain;	
	}
	else {
		*val = 0;
	}

	return OPTICAL_PATH_MANAGER_SUCCESS; 
}


static int optical_path_read_deadband(OpticalPathManager* op, int *deadband, int *position)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;    
	byte vals[10] = "";  
	int db, db_lsb;

	GetI2CPortLock(atd_b_op->_com_port, "optical_path_read_deadband");    
	
   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=253;  // Set cmd to read deadband  
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_read_deadband"); 
   				
   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1) | 0x01;
				
	if (GCI_readI2C_multiPort(atd_b_op->_com_port,3, vals, atd_b_op->_i2c_bus, "optical_path_read_deadband")) {
		ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_read_deadband");  
	    return OPTICAL_PATH_MANAGER_ERROR;	
	}
			
	ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_read_deadband");  
	
    db = vals[0] & 0xff; 
    db_lsb = vals[1] & 0xff; 
	*position =  vals[2] & 0xff;				
	*deadband=(db<<8) | db_lsb; 
				
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}
    			  

static int optical_path_read_values_from_eprom(OpticalPathManager* op, int *prop_gain, int *int_gain, int *dir_gain,
											   int *position1, int *position2, int *position3, int *position4)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;  
	unsigned char vals[20] = "";  
	int val, lsb;
	
	GetI2CPortLock(atd_b_op->_com_port, "optical_path_read_values_from_eprom");    

   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=254;
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_read_values_from_eprom"); 
   				
   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1) | 0x01;
    
	if (GCI_readI2C_multiPort(atd_b_op->_com_port,14, vals, atd_b_op->_i2c_bus, "optical_path_read_values_from_eprom")) {
		ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_read_values_from_eprom"); 	
		return -1;	
	}

	ReleaseI2CPortLock(atd_b_op->_com_port, "optical_path_read_values_from_eprom"); 

    val = vals[0] & 0xff; 
    lsb = vals[1] & 0xff;  
	*prop_gain=(val<<8) | lsb;

    val = vals[2] & 0xff; 
    lsb = vals[3] & 0xff; 
	*int_gain=(val<<8) | lsb; 

    val = vals[4] & 0xff;
    lsb = vals[5] & 0xff; 
    *dir_gain=(val<<8) |  lsb; 

	val= vals[6] & 0xff; 
    lsb = vals[7] & 0xff;
    *position1 = (val<<8) | lsb;

	val= vals[8] & 0xff; 
    lsb = vals[9] & 0xff;
	*position2 = (val<<8) | lsb;  

	val= vals[10] & 0xff; 
   	lsb = vals[11] & 0xff;
	*position3 = (val<<8) | lsb;

	val= vals[12] & 0xff; 
    lsb = vals[13] & 0xff;
	*position4 = (val<<8) | lsb;  
    
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int optical_path_set_gain(OpticalPathManager* op, OPB_PID_PARAM_TYPE type, int val)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;  
	byte vals[6] = ""; 

   	vals[0]=atd_b_op->_i2c_chip_type | (atd_b_op->_i2c_chip_address <<1);
   	vals[1]=type;	
   	vals[2]=val>>8;			
   	vals[3]=val & 0xff;		
	
   	GCI_writeI2C_multiPort(atd_b_op->_com_port,6, vals, atd_b_op->_i2c_bus, "optical_path_set_gain"); 
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

static int atd_op_b_optical_path_manager_hw_init (OpticalPathManager* op, int move_to_default)
{
	ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;    
	char device[UIMODULE_NAME_LEN];
	
	ui_module_get_name(UIMODULE_CAST(op), device);
		
	get_device_int_param_from_ini_file   (device, "COM_Port", &(atd_b_op->_com_port));  
	get_device_int_param_from_ini_file   (device, "i2c_Bus", &(atd_b_op->_i2c_bus));
	get_device_int_param_from_ini_file   (device, "i2c_ChipAddress", &(atd_b_op->_i2c_chip_address));  
	get_device_int_param_from_ini_file   (device, "i2c_ChipType", &(atd_b_op->_i2c_chip_type));  

	if (initialise_comport(atd_b_op->_com_port, 9600) == OPTICAL_PATH_MANAGER_ERROR)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	return OPTICAL_PATH_MANAGER_SUCCESS;  
}

static int atd_op_b_move_to_optical_path_position(OpticalPathManager* op, int position)
{
	ATD_OPTICALPATH_B *atd_b_op =  (ATD_OPTICALPATH_B *) op;   

	atd_optical_path_moveto_position(op, position);

	atd_b_op->_last_position = position;

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int atd_op_b_optical_path_manager_destroy (OpticalPathManager* op)
{
	atd_op_b_move_to_optical_path_position(op, 1); // should be setup as the laser position on Surrey Endstation

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int atd_op_b_get_current_optical_path_position (OpticalPathManager* op, int *position)
{
	ATD_OPTICALPATH_B *atd_b_op =  (ATD_OPTICALPATH_B *) op;   

	*position = atd_b_op->_last_position;

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int atd_op_b_optical_path_setup (OpticalPathManager* op)
{
	ATD_OPTICALPATH_B *atd_b_op =  (ATD_OPTICALPATH_B *) op;   
    
	ui_module_display_panel(UIMODULE_CAST(op), atd_b_op->_setup_panel_id);

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int atd_op_b_hide_optical_path_calib (OpticalPathManager* op)
{
	ATD_OPTICALPATH_B *atd_b_op =  (ATD_OPTICALPATH_B *) op;   
    
	ui_module_hide_panel(UIMODULE_CAST(op), atd_b_op->_setup_panel_id);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int CVICALLBACK OnPropCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* op = (OpticalPathManager* ) callbackData; 
			int val = 0;
			
		   	GetCtrlVal(panel, control, &val);
			optical_path_set_gain(op, OPB_PID_PARAM_PROPORTIONAL, val);
			
			break;
		}
	}
	
	return 0;
}



static int CVICALLBACK OnIntergralCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* op = (OpticalPathManager* ) callbackData; 
			int val = 0;
			
		   	GetCtrlVal(panel, control, &val);
			optical_path_set_gain(op, OPB_PID_PARAM_INTEGAL, val);
			
			break;
		}
	}
	
	return 0;
}

static  int CVICALLBACK OnDerivitiveCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* op = (OpticalPathManager* ) callbackData; 
			int val = 0;
			
		   	GetCtrlVal(panel, control, &val);
			optical_path_set_gain(op, OPB_PID_PARAM_DERIVITIVE, val);
			
			break;
		}
	}
	
	return 0;
}


static  int CVICALLBACK OnOpticalPathClosedClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* op = (OpticalPathManager* ) callbackData; 
			ATD_OPTICALPATH_B *atd_b_op =  (ATD_OPTICALPATH_B *) op;   

			ui_module_hide_panel(UIMODULE_CAST(op), atd_b_op->_setup_panel_id);
			
			break;
		}
	}
	
	return 0;
}


static  int CVICALLBACK OnOpticalPathWriteToEPROM (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* op = (OpticalPathManager* ) callbackData;    
			ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;  
			int val;
			
			GetI2CPortLock(atd_b_op->_com_port, "OnOpticalPathWriteToEPROM");    

			GetCtrlVal(panel, SETUP_PNL_POSITION1, &val);
			optical_path_send_position_to_memory(op, 1, val);
			
			GetCtrlVal(panel, SETUP_PNL_POSITION2, &val);
			optical_path_send_position_to_memory(op, 2, val);

			GetCtrlVal(panel, SETUP_PNL_POSITION3, &val);
			optical_path_send_position_to_memory(op, 3, val);

			GetCtrlVal(panel, SETUP_PNL_POSITION4, &val);
			optical_path_send_position_to_memory(op, 4, val);
			
			GetCtrlVal(panel, SETUP_PNL_PROP, &val);
			optical_path_set_gain(op, OPB_PID_PARAM_PROPORTIONAL, val);
		
			GetCtrlVal(panel, SETUP_PNL_INTERGRAL, &val);
			optical_path_set_gain(op, OPB_PID_PARAM_INTEGAL, val);

			GetCtrlVal(panel, SETUP_PNL_DERIVITIVE, &val);
			optical_path_set_gain(op, OPB_PID_PARAM_DERIVITIVE, val);
					
   			optical_path_store_current_pid(op);

			GetCtrlVal(panel, SETUP_PNL_DEADBAND, &val);
			optical_path_send_deadband_memory(op, val);

			ReleaseI2CPortLock(atd_b_op->_com_port, "OnOpticalPathWriteToEPROM");   

			break;
		}
	}
	
	return 0;
}

static  int CVICALLBACK OnOpticalPathReadFromEPROM (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* op = (OpticalPathManager* ) callbackData;   
			ATD_OPTICALPATH_B* atd_b_op = (ATD_OPTICALPATH_B *) op;  
			int prop_gain, int_gain, dir_gain, position1, position2, position3, position4, deadband, pos;

			GetI2CPortLock(atd_b_op->_com_port, "OnOpticalPathReadFromEPROM");    

			optical_path_read_values_from_eprom(op, &prop_gain, &int_gain, &dir_gain, &position1, &position2, &position3, &position4);
			
			optical_path_read_deadband(op, &deadband, &pos);

			ReleaseI2CPortLock(atd_b_op->_com_port, "OnOpticalPathReadFromEPROM");

			SetCtrlVal(panel, SETUP_PNL_POSITION1_R, position1);
			SetCtrlVal(panel, SETUP_PNL_POSITION2_R, position2);
			SetCtrlVal(panel, SETUP_PNL_POSITION3_R, position3);
			SetCtrlVal(panel, SETUP_PNL_POSITION4_R, position4);

			SetCtrlVal(panel, SETUP_PNL_PROP_R, prop_gain);
			SetCtrlVal(panel, SETUP_PNL_INTERGRAL_R, int_gain);
			SetCtrlVal(panel, SETUP_PNL_DERIVITIVE_R, dir_gain);
			SetCtrlVal(panel, SETUP_PNL_DEADBAND_R, deadband);

			SetCtrlVal(panel, SETUP_PNL_POS, pos);
	
			break;
		}
	}
	
	return 0;
}


static int CVICALLBACK OnEnableMotor (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* op = (OpticalPathManager* ) callbackData; 
			ATD_OPTICALPATH_B *atd_b_op =  (ATD_OPTICALPATH_B *) op; 
			int val;
			
			GetCtrlVal(panel, control, &val);

			optical_path_enable(op, val);
   	
			break;
		}
	}

	return 0;
}

static int CVICALLBACK OnSetPositionValue (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* op = (OpticalPathManager* ) callbackData;
			ATD_OPTICALPATH_B *atd_b_op =  (ATD_OPTICALPATH_B *) op; 
			int val;
			
			GetCtrlVal(panel, control, &val);
			
			if(val >= 31500)
				val=31500;
	
			if(val<=500)
				val=500;

			optical_path_send_position(op, val);
   	
			break;
		}
	}

	return 0;
}

OpticalPathManager* atd_op_b_optical_path_new(const char *name, const char *description, const char* data_dir, const char *file)
{
	OpticalPathManager* op = optical_path_manager_new(name, description, data_dir, file, sizeof(ATD_OPTICALPATH_B));
	
	ATD_OPTICALPATH_B *atd_b_op =  (ATD_OPTICALPATH_B *) op; 
	
	op->_initialised = 0;
	atd_b_op->_last_position = -1;

	OPTICAL_PATH_MANAGER_VTABLE_PTR(op, hw_init) = atd_op_b_optical_path_manager_hw_init; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(op, destroy) = atd_op_b_optical_path_manager_destroy;
	OPTICAL_PATH_MANAGER_VTABLE_PTR(op, move_to_optical_path_position) = atd_op_b_move_to_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(op, get_current_optical_path_position) = atd_op_b_get_current_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(op, setup_optical_path) = atd_op_b_optical_path_setup; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(op, hide_optical_path_calib) = atd_op_b_hide_optical_path_calib; 

	atd_b_op->_setup_panel_id = ui_module_add_panel(UIMODULE_CAST(atd_b_op), "ATD_OpticalPath_B_UI.uir", SETUP_PNL, 0);
	if (atd_b_op->_setup_panel_id < 0) goto Error;

	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_MOTOR_ENABLE, OnEnableMotor, op);
	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_POSITION1, OnSetPositionValue, op);
	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_POSITION2, OnSetPositionValue, op);
	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_PROP, OnPropCallback, op);
	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_INTERGRAL, OnIntergralCallback, op);
	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_DERIVITIVE, OnDerivitiveCallback, op);
	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_EPROM_BUTTON, OnOpticalPathWriteToEPROM, op);
	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_READ_EPROM_BUTTON, OnOpticalPathReadFromEPROM, op);
	InstallCtrlCallback (atd_b_op->_setup_panel_id, SETUP_PNL_CLOSE, OnOpticalPathClosedClicked, op);

	SetCtrlAttribute (op->_main_ui_panel, OPATH_PNL_CALIBRATE, ATTR_VISIBLE, 1);

	return op;
	
	Error:
	
	return NULL;
}
