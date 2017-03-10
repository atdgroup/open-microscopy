#include <cviauto.h>
#include "OfflineImager_Shutter.h"
#include "ShutterUI.h"
#include "ATD_UsbInterface_A.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "asynctmr.h"

#include "profile.h"

#define FAST_SHUTTER 0

static int dirs[8] = {0, 0xff, 0, 0, 0, 0, 0, 0x0f};

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Optical shutter control for manual microscope.
////////////////////////////////////////////////////////////////////////////

static int offlineimager_shutter_get_shutter_open_time (Shutter* shutter, double *open_time);
static int offlineimager_shutter_shutter_close(Shutter* shutter);

static int GCI_Out_Bit_multiPort(int port, int bus, byte chip_type, byte address, byte dirs, int bit, int data)
{
	byte val[3], mask; 		
    int err;
    
	GetI2CPortLock(port, "GCI_Out_Bit_multiPort");

	//Read byte first
    val[0] = chip_type | (address <<1) | 0x01;

    if (GCI_readI2C_multiPort(port, 2, val, bus, "GCI_Out_Bit_multiPort")) {				 //problem
		ReleaseI2CPortLock(port, "GCI_Out_Bit_multiPort"); 
    	return -1;	
    }

	val[1] |= dirs;	//keeps input lines high
    
    if (data)		//set bit
    	val[1] = val[1] | ((int)pow (2.0, (double)bit));
    else {			//clear bit
    	mask = ~((int)pow (2.0, (double)bit));
    	val[1] = val[1] & mask;	
    }
    
    val[0] = chip_type | (address <<1);
    
    err = GCI_writeI2C_multiPort(port, 2, val, bus, "GCI_Out_Bit_multiPort");
    
	ReleaseI2CPortLock(port, "GCI_Out_Bit_multiPort"); 

    return err;
}

static int GCI_In_Byte_multiPort (int port, int bus, byte chip_type, byte address, byte *i_patt)
{
	byte val[3]; 
 
	GetI2CPortLock(port, "GCI_In_Byte_multiPort");

    val[0] = chip_type | (address <<1) | 0x01;
    if (GCI_readI2C_multiPort(port, 2, val, bus, "GCI_In_Byte_multiPort")) {
		ReleaseI2CPortLock(port, "GCI_In_Byte_multiPort"); 
    	return -1;	//problem
    }
    *i_patt = (int)val[1];

	ReleaseI2CPortLock(port, "GCI_In_Byte_multiPort"); 

    return 0;
}

static int offlineimager_read_shutter_status (Shutter* shutter, int *enabled, int *charging, int *status)
{
	OfflineImagerShutter* ol_shutter = (OfflineImagerShutter *) shutter; 

	unsigned char stat = 0;
	unsigned char data[2];
	*status = 0;
	*enabled = 0;

	//Read address 1 of the chip. 
	//bit 6 indicates shutter enabled. bit 7 indicates shutter state. bit 3 indicates charging
	if (GCI_In_Byte_multiPort (ol_shutter->_com_port, ol_shutter->_i2c_bus,
		ol_shutter->_i2c_chip_type, 1, &stat))
		return SHUTTER_ERROR;
	
 	*enabled = stat & ((int)pow (2.0, (double)6));
 	*enabled = (*enabled >> 6);

 	*charging = stat & ((int)pow (2.0, (double)3));
 	*charging = (*charging >> 3);
	
	if (!FAST_SHUTTER) {				  //conventional I2C control
 		*status = stat & ((int)pow (2.0, (double)7));
 		*status = (*status >> 7);
	}
	else {
		//Read one byte using "fast" mechanism
		if (GCI_readFAST_multiPort(ol_shutter->_com_port, data, 1, "offlineimager_read_shutter_status"))
			return SHUTTER_ERROR;

		*status = data[0];
	}
	
	*status = !(*status);

	return SHUTTER_SUCCESS;
}

int offlineimager_shutter_shutter_init (Shutter* shutter)
{
	OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter; 
	
	int enabled, charging, closed, tries=0;
	
	if(offlineimager_read_shutter_status (shutter, &enabled, &charging, &closed) == SHUTTER_ERROR)
		return SHUTTER_ERROR;

	if(!enabled) {
		//send_shutter_error_text (shutter,
		//	"The shutter PC control does not seem to be enabled. "
		//	"Is the shutter module control switch set to the PC position?");
		shutter_set_computer_control(shutter, 0);
	}
	else
		shutter_set_computer_control(shutter, 1);

	// Hide inhibit control and make the shutter on / off control an indicator.
	if (!(offlineimager_shutter->has_inhibit_line))
		SetCtrlAttribute(shutter->_panel_id, SHUTTER_INHIBIT, ATTR_VISIBLE, 0);
	SetCtrlAttribute(shutter->_panel_id, SHUTTER_COMP_CTRL, ATTR_CTRL_MODE, VAL_INDICATOR);
	SetCtrlAttribute(shutter->_panel_id, SHUTTER_COMP_CTRL, ATTR_DIMMED, 1);

	return SHUTTER_SUCCESS;
}

int offlineimager_shutter_shutter_destroy (Shutter* shutter)
{
	OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter; 
	
	SetAsyncTimerAttribute (offlineimager_shutter->_monitor_timer, ASYNC_ATTR_ENABLED,  0);

	return SHUTTER_SUCCESS;
}

static int offlineimager_timed_shutter(Shutter* shutter)
{
	int ret = 0;
	double open_time;
	
	offlineimager_shutter_get_shutter_open_time (shutter, &open_time);
	if(open_time <= 0.0)
		return ret;
	
	// open_time is is milli seconds
	Delay(open_time / 1000.0);

	return offlineimager_shutter_shutter_close(shutter); // Close the shutter
}

static int offlineimager_shutter_shutter_open(Shutter* shutter)
{
	double open_time;
	double time, timeout=1.0;
	OfflineImagerShutter* ol_shutter = (OfflineImagerShutter *) shutter; 
	byte data[3]= {0x00,0xff,0x00};
	int enabled, charging, fb, warned=0;

	if (!(ol_shutter->has_charge_status)) {
		while (Timer() - ol_shutter->last_closed_time < 0.4)
			;	   // min recharge time
	}
	else {

		offlineimager_read_shutter_status (shutter, &enabled, &charging, &fb);

		if(enabled == 0) {
			logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s not enabled.", UIMODULE_GET_DESCRIPTION(shutter));
			return SHUTTER_SUCCESS;
		}

		if(fb == 1) {
			logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s already open.", UIMODULE_GET_DESCRIPTION(shutter));
			return SHUTTER_SUCCESS;
		}
			
		if (charging) time=Timer();
			
		while(charging && (Timer()-time)<timeout) {
				
			if (!warned)
			{
				logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s open request whilst still charging.", UIMODULE_GET_DESCRIPTION(shutter));
				warned = 1;
			}
		
			Delay(0.1);
			
			offlineimager_read_shutter_status (shutter, &enabled, &charging, &fb);
		}
			
		if (charging) {
			logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s is still charging.", UIMODULE_GET_DESCRIPTION(shutter));
			return  SHUTTER_ERROR ;
		}
	}

	// Ok to open the shutter from here...

	if (!FAST_SHUTTER) {		//conventional I2C control
		GCI_Out_Bit_multiPort(ol_shutter->_com_port, ol_shutter->_i2c_bus,
				ol_shutter->_i2c_chip_type, 0, dirs[0], 7, 0);   //modify bit 7 of address 0

		offlineimager_shutter_get_shutter_open_time (shutter, &open_time);
		if(open_time > 0.0)
			offlineimager_timed_shutter(shutter);  
		return SHUTTER_SUCCESS;
	}
//	if (!FAST_SHUTTER) 		//conventional I2C control
//		return GCI_Out_Bit_multiPort(ol_shutter->_com_port, ol_shutter->_i2c_bus,
//						ol_shutter->_i2c_chip_type, 0, dirs[0], 7, 0);   //modify bit 7 of address 0

	//Using Andreas's fast lines
	data[2] = 0;				  //data byte
	GCI_writeFAST_multiPort(ol_shutter->_com_port, data, 3, "offlineimager_shutter_shutter_open");

	return SHUTTER_SUCCESS;
}

static int offlineimager_shutter_shutter_close(Shutter* shutter)
{
	OfflineImagerShutter* ol_shutter = (OfflineImagerShutter *) shutter; 

	byte data[3]= {0x00,0xff,0x00};

	if (!FAST_SHUTTER) 		//conventional I2C control
		return GCI_Out_Bit_multiPort(ol_shutter->_com_port, ol_shutter->_i2c_bus,
							ol_shutter->_i2c_chip_type, 0, dirs[0], 7, 1);   //modify bit 7 of address 0

	//Using Andreas's fast lines
	data[2] = 1;				  //data byte
	GCI_writeFAST_multiPort(ol_shutter->_com_port, data, 3, "offlineimager_shutter_shutter_close");

	if (!(ol_shutter->has_charge_status))
		ol_shutter->last_closed_time = Timer();

	return SHUTTER_SUCCESS;
}

static int offlineimager_shutter_get_shutter_status (Shutter* shutter, int *status)
{
	OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter; 
	int enabled, charging;

	return offlineimager_read_shutter_status (shutter, &enabled, &charging, status);
}

static int offlineimager_shutter_set_shutter_open_time (Shutter* shutter, double open_time)
{
	OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter;      
	
	offlineimager_shutter->open_time = open_time; 
	
	return SHUTTER_SUCCESS;
}

static int offlineimager_shutter_get_shutter_open_time (Shutter* shutter, double *open_time)
{
	OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter;      
	
	*open_time = offlineimager_shutter->open_time; 

	return SHUTTER_SUCCESS;
}

static int offlineimager_shutter_inhibit(Shutter* shutter, int inhibit)
{
	OfflineImagerShutter* ol_shutter = (OfflineImagerShutter *) shutter;
	int data=1;

	if (ol_shutter->has_inhibit_line) {

		if (inhibit)
			data = 0;

		if (GCI_Out_Bit_multiPort(ol_shutter->_com_port, ol_shutter->_i2c_bus,
			ol_shutter->_i2c_chip_type, 0, dirs[0], 6, data)<0) {   //modify bit 6 of address 0
		
				return SHUTTER_ERROR;
		}

		ol_shutter->inhibited = inhibit;
	}
	else 
		ol_shutter->inhibited = 0;

	return  SHUTTER_SUCCESS;  
}

static int offlineimager_shutter_is_inhibited(Shutter* shutter, int *inhibit)
{
	OfflineImagerShutter* ol_shutter = (OfflineImagerShutter *) shutter;  
	
	*inhibit = ol_shutter->inhibited;

	return  SHUTTER_SUCCESS;  
}

static int offlineimager_shutter_set_computer_control  (Shutter* shutter, int compCtrl)
{
	OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter;      
	
	offlineimager_shutter->computer_controlled = compCtrl;

	return  SHUTTER_SUCCESS; 
}

static int CVICALLBACK OnOfflineShutterMonitorTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			Shutter *shutter = (Shutter *) callbackData;
			OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter; 
	
			int enabled, charging, fb;
	
			if(offlineimager_read_shutter_status (shutter, &enabled, &charging, &fb) == SHUTTER_ERROR)
				return SHUTTER_ERROR;

			if(!enabled) {
				offlineimager_shutter_set_computer_control(shutter, 0);
			}
			else
				offlineimager_shutter_set_computer_control(shutter, 1);
     	   
			break;   
		}
    }
    
    return 0;
}

static int offlineimager_get_info (Shutter* shutter, char* info)
{
	if(info != NULL)  
		strcpy(info, "Uniblitz VS25S2ZM1");

	return SHUTTER_SUCCESS;	
}

Shutter* offline_imager_shutter_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler)
{
	Shutter* shutter = (Shutter*) malloc(sizeof(OfflineImagerShutter));  	
	OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter;
	
	memset(shutter, 0, sizeof(OfflineImagerShutter));

	shutter_constructor(shutter, name, description);

	ui_module_set_error_handler(UIMODULE_CAST(shutter), handler, NULL);   
	
	SHUTTER_VTABLE_PTR(shutter, hw_init) = offlineimager_shutter_shutter_init;    
	SHUTTER_VTABLE_PTR(shutter, destroy) = offlineimager_shutter_shutter_destroy; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = offlineimager_shutter_shutter_open; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = offlineimager_shutter_shutter_close; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = offlineimager_shutter_get_shutter_status;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = offlineimager_shutter_set_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = offlineimager_shutter_get_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_inhibit) = offlineimager_shutter_inhibit;
	SHUTTER_VTABLE_PTR(shutter, shutter_is_inhibited) = offlineimager_shutter_is_inhibited;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_computer_control) = offlineimager_shutter_set_computer_control;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_info) = offlineimager_get_info; 
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "i2c_Bus", &(offlineimager_shutter->_i2c_bus));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "i2c_ChipAddress", &(offlineimager_shutter->_i2c_chip_address));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "i2c_ChipType", &(offlineimager_shutter->_i2c_chip_type));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "COM_Port", &(offlineimager_shutter->_com_port));  

	offlineimager_shutter->has_charge_status = 1;
	offlineimager_shutter->has_inhibit_line = 1;

	offlineimager_shutter->_monitor_timer = NewAsyncTimer (2.0, -1, 1, OnOfflineShutterMonitorTimerTick, shutter);
	SetAsyncTimerAttribute (offlineimager_shutter->_monitor_timer, ASYNC_ATTR_ENABLED,  1);

	return shutter;
}

Shutter* offline_imager_shutter2_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler)
{	// version 2 of this shutter has mods for an inhibit line and charging feedback
	Shutter* shutter = (Shutter*) malloc(sizeof(OfflineImagerShutter));  	
	OfflineImagerShutter* offlineimager_shutter = (OfflineImagerShutter *) shutter;
	
	memset(shutter, 0, sizeof(OfflineImagerShutter));

	shutter_constructor(shutter, name, description);

	ui_module_set_error_handler(UIMODULE_CAST(shutter), handler, NULL);   
	
	SHUTTER_VTABLE_PTR(shutter, hw_init) = offlineimager_shutter_shutter_init;    
	SHUTTER_VTABLE_PTR(shutter, destroy) = offlineimager_shutter_shutter_destroy; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = offlineimager_shutter_shutter_open; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = offlineimager_shutter_shutter_close; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = offlineimager_shutter_get_shutter_status;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = offlineimager_shutter_set_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = offlineimager_shutter_get_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_inhibit) = offlineimager_shutter_inhibit;
	SHUTTER_VTABLE_PTR(shutter, shutter_is_inhibited) = offlineimager_shutter_is_inhibited;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_computer_control) = offlineimager_shutter_set_computer_control;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_info) = offlineimager_get_info; 
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "i2c_Bus", &(offlineimager_shutter->_i2c_bus));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "i2c_ChipAddress", &(offlineimager_shutter->_i2c_chip_address));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "i2c_ChipType", &(offlineimager_shutter->_i2c_chip_type));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "COM_Port", &(offlineimager_shutter->_com_port));  

	offlineimager_shutter->has_charge_status = 1;
	offlineimager_shutter->has_inhibit_line = 1;

	offlineimager_shutter->_monitor_timer = NewAsyncTimer (2.0, -1, 1, OnOfflineShutterMonitorTimerTick, shutter);
	SetAsyncTimerAttribute (offlineimager_shutter->_monitor_timer, ASYNC_ATTR_ENABLED,  1);

	return shutter;
}
