#include "OfflineImager_PowerSwitch.h"
#include "OfflineImager_PowerSwitch_UI.h" 
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 
#include "Microscope.h"

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include <utility.h>

/////////////////////////////////////////////////////////////////////////////
// Offline Imager Application - BV/RGN control box module
//
//July 2002 - RJL and BV
/////////////////////////////////////////////////////////////////////////////
//COM port 3
//Address 0: all outputs (Chip PCF8574)
// bit 0		camera power	 (0 for on, 1 for off)
// bit 1		lamp power		 (0 for on, 1 for off)
// bit 2		stage power		 (0 for on, 1 for off)
// bit 3		Hg lamp power	 (0 for on, 1 for off)
// bit 7		shutter state	 (0 to open, 1 to shut)
//
//Address 1: all inputs
// bit 5		lamp enabled	 (1 enabled, 0 disabled)
// bit 6		shutter enabled	 (1 enabled, 0 disabled)
// bit 7		shutter state	 (1 closed, 0 open)
//
//Address 7:
// bits 0-2		Turret pos feedback		output  (0-7)
// bit 3		Turret moving			output  (0 moving)
// bits 4-6		Turret pos				input   (0-7)
// bit 7		Turret enable			input   (1 enabled, 0 disabled) 
//
//Address 0: (Chip MAX521)
// DAC 3		Lamp on				(0 for on, 255 for off)
// DAC 4		Scope lamp control  (0 to enable, 255 to disable)
// DAC 5		Scope int'y control (0 to enable, 255 to disable)
// DAC 6		Lamp fine control   (255 max)
// DAC 7		Lamp coarse control (255 max)
//

// For this module the config needs to be set up as 
//Switch1_Name = Camera
//Switch2_Name = Lamp
//Switch3_Name = Stage
//Switch4_Name = HG Lamp

//safer to assume mercury lamp, (bit 4), is already on 
//otherwise we get an off-on which kills the lamp
//static byte command0 = 0xfe;   
static int dirs[8] = {0, 0xff, 0, 0, 0, 0, 0, 0x0f};

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

static int GCI_Out_Byte_multiPort (int port, int bus, byte chip_type, byte address, byte dirs, byte patt )
{
	int err = 0;
	byte val[3]; 									  
 
	//Send a byte. Any input lines must be set high in dirs.
	
	GetI2CPortLock(port, "GCI_Out_Byte_multiPort");

   	val[0] = chip_type | (address <<1);
   	val[1] = patt | dirs;						//keeps input lines high

   	err = GCI_writeI2C_multiPort(port, 2, val, bus, "GCI_Out_Byte_multiPort");

	ReleaseI2CPortLock(port, "GCI_Out_Byte_multiPort"); 

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

// the switch goes from 1 to 3
static int is_switch_on(PowerSwitch *ps, int the_switch)
{
	byte data, mask;
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps; 

	//Read power status
	if (GCI_In_Byte_multiPort (ps_a1->_com_port, ps_a1->_i2c_bus, ps_a1->_i2c_type, 0, &data))
		return -1;
	
	switch(the_switch)
	{
		case 1:
		{
			// Camera
			mask = 0x01;
			break;
		}

		case 2:
		{
			// Lamp
			mask = 0x02;
			break;
		}

		case 3:
		{
			// Stage
			mask = 0x04;
			break;
		}

		case 4:
		{
			// HG Lamp
			mask = 0x08;
			break;
		}

	}

	if (data & mask) {
		// Device off
		return 0;
	}

	return 1;
}


static int perform_switch(PowerSwitch *ps, int the_switch, int value)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps;  

	int led_id, switch_id;

	// Is Device already on.
	// If so do nothing.
	if(value == 1 && is_switch_on(ps, the_switch))
		return POWER_SWITCH_SUCCESS;

	value = !value;
	
	if(the_switch < 1 || the_switch > 4)
		return POWER_SWITCH_ERROR;
	
	if(the_switch == 1) {
		led_id = POW_PANEL_LED1;
		switch_id = POW_PANEL_SW1;
		GCI_Out_Bit_multiPort(ps_a1->_com_port, ps_a1->_i2c_bus, ps_a1->_i2c_type, 0, dirs[0], 0, value);
	}
	else if (the_switch == 2) {
		// Lamp
		led_id = POW_PANEL_LED2;
		switch_id = POW_PANEL_SW2;
		GCI_Out_Bit_multiPort(ps_a1->_com_port, ps_a1->_i2c_bus, ps_a1->_i2c_type, 0, dirs[0], 1, value); 
	}
	else if (the_switch == 3) {
		// Stage
		led_id = POW_PANEL_LED3;
		switch_id = POW_PANEL_SW3;
		GCI_Out_Bit_multiPort(ps_a1->_com_port, ps_a1->_i2c_bus, ps_a1->_i2c_type, 0, dirs[0], 2, value);
	}
	else if (the_switch == 4) {
		// HG Lamp
		led_id = POW_PANEL_LED4;
		switch_id = POW_PANEL_SW4;
		GCI_Out_Bit_multiPort(ps_a1->_com_port, ps_a1->_i2c_bus, ps_a1->_i2c_type, 0, dirs[0], 3, value);
	}

	Delay(0.5);
	
	if(is_switch_on(ps, the_switch)) {    
		SetCtrlVal(ps->_panel_id, led_id, 1); 
		SetCtrlVal(ps->_panel_id, switch_id, 1);   
	}
	else {
		SetCtrlVal(ps->_panel_id, led_id, 0);  
		SetCtrlVal(ps->_panel_id, switch_id, 0);
	}

	return POWER_SWITCH_SUCCESS; 
}


static int atd_power_switch_a1_switch_on (PowerSwitch *ps, int the_switch)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps;   
	
	return perform_switch(ps, the_switch, 1);   
}

static int ConfirmPowerShutdown(PowerSwitch *ps, int the_switch)
{
	// returns 1 if the user selected to turn the item off, or item should be turned off without asking
	// returns 0 if the user selected not to turn the item off
	// returns -1 if ask_shutdown is <0, meaning do not shut down and do not ask

	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps; 
	
	char buffer[500];
	char *name;
	int ask = 1;
	
	if(!is_switch_on(ps, the_switch))
		return 1;

	if(the_switch == 1) {
			
		ask = ps_a1->_switch1_ask_shutdown;
		name = ps_a1->_switch1_name;		
	}
	else if(the_switch == 2) {
		ask = ps_a1->_switch2_ask_shutdown;   
		name = ps_a1->_switch2_name;
	}
	else if(the_switch == 3) {
		ask = ps_a1->_switch3_ask_shutdown;  
		name = ps_a1->_switch3_name;
	}
	else if(the_switch == 4) {
		ask = ps_a1->_switch4_ask_shutdown;  
		name = ps_a1->_switch4_name;
	}
	
	if(ask<0) { // do not allow this to turn off
		return -1;
	}
	
	if(ask) {
		sprintf(buffer, "Do you want to switch off the %s ?", name); 
		return ConfirmPopup("Power Down", buffer);
	}
	
	return 1;
}

static int offline_imager_switch_a1_switch_off (PowerSwitch *ps, int the_switch)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps;   
	
	if (ConfirmPowerShutdown(ps, the_switch)>0) {
		return perform_switch(ps, the_switch, 0);  	
	}
	else
		return POWER_SWITCH_SUCCESS; 
}

static int offline_imager_switch_a1_switch_off_all (PowerSwitch *ps)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps;   
	
	if(offline_imager_switch_a1_switch_off(ps, 1) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR;
		
	if(offline_imager_switch_a1_switch_off(ps, 2) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR;
			
	if(offline_imager_switch_a1_switch_off(ps, 3) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR; 
	
	if(offline_imager_switch_a1_switch_off(ps, 4) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR; 

	return POWER_SWITCH_SUCCESS;
}

static int offline_imager_switch_a1_switch_status (PowerSwitch *ps, int the_switch, int *status)
{
	OfflineImager_PowerSwitch* offline_imager_shutter = (OfflineImager_PowerSwitch *) ps; 
	
	*status = is_switch_on(ps, the_switch);  
		
	return POWER_SWITCH_SUCCESS;
}


static int CVICALLBACK thread_can_use_check(void *callback)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) callback; 
    double elapsed_time;
	
	while(1) {
		
		elapsed_time = Timer() - ps_a1->_start_time;
		
		if(elapsed_time > ps_a1->_switch1_delay)
			ps_a1->_switch1_can_use = 1;
		
		if(elapsed_time > ps_a1->_switch2_delay)
			ps_a1->_switch2_can_use = 1;
		
		if(elapsed_time > ps_a1->_switch3_delay)
			ps_a1->_switch3_can_use = 1;

		if(elapsed_time > ps_a1->_switch4_delay)
			ps_a1->_switch4_can_use = 1;
		
		if(ps_a1->_switch1_can_use && ps_a1->_switch2_can_use && ps_a1->_switch3_can_use && ps_a1->_switch4_can_use)
			break;
		
		if(elapsed_time > 30) {
			
			MessagePopup("Error", "Failed to reach switch delay");
			break;
		}
	}
	
	return POWER_SWITCH_SUCCESS;   
}

static int offline_imager_power_switch_init(PowerSwitch *ps)
{
	int thread_id;
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps;
	Microscope *ms = NULL;

	ps_a1->_switch_state = 0xff;
	ps_a1->_switch1_can_use = 0;  
	ps_a1->_switch2_can_use = 0; 
	ps_a1->_switch3_can_use = 0; 
	ps_a1->_switch4_can_use = 0;
	
	// only needs the com port the master pic is on, works directly off its fast lines.
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "COM_Port", &(ps_a1->_com_port)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_ChipAddress", &(ps_a1->_i2c_chip_address));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_Bus", &(ps_a1->_i2c_bus));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_ChipType", &(ps_a1->_i2c_type));      
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Number_Of_Switches", &(ps_a1->_no_of_switches));
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Name", ps_a1->_switch1_name);
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Name", ps_a1->_switch2_name);
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Name", ps_a1->_switch3_name);
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch4_Name", ps_a1->_switch4_name);
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Startup", &(ps_a1->_switch1_startup)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Startup", &(ps_a1->_switch2_startup));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Startup", &(ps_a1->_switch3_startup));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch4_Startup", &(ps_a1->_switch4_startup));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Delay", &(ps_a1->_switch1_delay)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Delay", &(ps_a1->_switch2_delay));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Delay", &(ps_a1->_switch3_delay)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch4_Delay", &(ps_a1->_switch4_delay));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_AskToShutdown", &(ps_a1->_switch1_ask_shutdown)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_AskToShutdown", &(ps_a1->_switch2_ask_shutdown));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_AskToShutdown", &(ps_a1->_switch3_ask_shutdown)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch4_AskToShutdown", &(ps_a1->_switch4_ask_shutdown));

	SetCtrlAttribute(ps->_panel_id, POW_PANEL_SW1, ATTR_LABEL_TEXT, ps_a1->_switch1_name); 
	SetCtrlAttribute(ps->_panel_id, POW_PANEL_SW2, ATTR_LABEL_TEXT, ps_a1->_switch2_name);     
	SetCtrlAttribute(ps->_panel_id, POW_PANEL_SW3, ATTR_LABEL_TEXT, ps_a1->_switch3_name);  
	SetCtrlAttribute(ps->_panel_id, POW_PANEL_SW4, ATTR_LABEL_TEXT, ps_a1->_switch4_name);
	
	ms = microscope_get_microscope();

	initialise_comport(ps_a1->_com_port, 9600);

	//Set inputs
	if (GCI_Out_Byte_multiPort (ps_a1->_com_port, ps_a1->_i2c_bus, ps_a1->_i2c_type, 1, dirs[1], 0xff))
		return POWER_SWITCH_ERROR;
	
	if(ps_a1->_switch1_startup) {

		if(!is_switch_on(ps, 1))
			atd_power_switch_a1_switch_on (ps, 1); 
		else
			ps_a1->_switch1_can_use = 1;
	}

	if(ps_a1->_switch2_startup) {

		if(!is_switch_on(ps, 2))
			atd_power_switch_a1_switch_on (ps, 2); 
		else
			ps_a1->_switch2_can_use = 1;
	}

	if(ps_a1->_switch3_startup){
	
		if(!is_switch_on(ps, 3))
			atd_power_switch_a1_switch_on (ps, 3); 
		else
			ps_a1->_switch3_can_use = 1;
	}

	if(ps_a1->_switch4_startup){
	
		if(!is_switch_on(ps, 4))
			atd_power_switch_a1_switch_on (ps, 4); 
		else
			ps_a1->_switch4_can_use = 1;
	}

	// Update switch UI to match switch state.
	SetCtrlVal(ps->_panel_id, POW_PANEL_LED1, is_switch_on(ps, 1)); 
	SetCtrlVal(ps->_panel_id, POW_PANEL_SW1, is_switch_on(ps, 1));   

	SetCtrlVal(ps->_panel_id, POW_PANEL_LED2, is_switch_on(ps, 2)); 
	SetCtrlVal(ps->_panel_id, POW_PANEL_SW2, is_switch_on(ps, 2));   

	SetCtrlVal(ps->_panel_id, POW_PANEL_LED3, is_switch_on(ps, 3)); 
	SetCtrlVal(ps->_panel_id, POW_PANEL_SW3, is_switch_on(ps, 3));   

	SetCtrlVal(ps->_panel_id, POW_PANEL_LED4, is_switch_on(ps, 4)); 
	SetCtrlVal(ps->_panel_id, POW_PANEL_SW4, is_switch_on(ps, 4));   
	
	ps_a1->_start_time = Timer();
	
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, thread_can_use_check, ps, &thread_id);  
	
    InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW1, OnSwitchChanged, ps);
    InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW2, OnSwitchChanged, ps);    
  	InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW3, OnSwitchChanged, ps);    
	InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW4, OnSwitchChanged, ps);
	InstallCtrlCallback (ps->_panel_id, POW_PANEL_CLOSE, OnPowerSwitchClose, ps);

	return POWER_SWITCH_SUCCESS;
}


static int offline_imager_power_switch_destroy (PowerSwitch *ps)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps; 

	return POWER_SWITCH_SUCCESS;
}

static int get_switch_id_for_name(PowerSwitch* ps, const char *name)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps; 

	if(strncmp(name, ps_a1->_switch1_name, strlen(name) - 1) == 0)
		return 1;
	else if(strncmp(name, ps_a1->_switch2_name, strlen(name) - 1) == 0)
		return 2;
	else if(strncmp(name, ps_a1->_switch3_name, strlen(name) - 1) == 0)
		return 3;

	return -1;
}

static int offline_imager_switch_status_for_name (PowerSwitch *ps, const char *name, int *status)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps; 
	
	int the_switch = get_switch_id_for_name(ps, name);

	*status = is_switch_on(ps, the_switch);  
		
	return POWER_SWITCH_SUCCESS;
}

static int offline_imager_name_can_use_load (PowerSwitch* ps, const char *name)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps; 

	int the_switch = get_switch_id_for_name(ps, name);

	if(the_switch == 1)
		return ps_a1->_switch1_can_use;
	else if(the_switch == 2)
		return ps_a1->_switch2_can_use; 
	else if(the_switch == 3)
		return ps_a1->_switch3_can_use; 
	
	return 0;
}

static int offline_imager_can_use_load (PowerSwitch* ps, int the_switch)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) ps; 
	
	if(the_switch == 1)
		return ps_a1->_switch1_can_use; 
	else if(the_switch == 2)
		return ps_a1->_switch2_can_use; 
	else if(the_switch == 3)
		return ps_a1->_switch3_can_use;
	else if(the_switch == 4)
		return ps_a1->_switch4_can_use; 
	
	return 0;
}

PowerSwitch* offline_imager_power_switch_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir, void *callback_data)
{
	OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) malloc(sizeof(OfflineImager_PowerSwitch));  
	PowerSwitch* ps = (PowerSwitch*) ps_a1;    
	
	power_switch_constructor(ps, name, description, data_dir);
	
	ui_module_set_error_handler(UIMODULE_CAST(ps), handler, callback_data);   

	POWER_SWITCH_VTABLE_PTR(ps, hw_init) = offline_imager_power_switch_init; 
	POWER_SWITCH_VTABLE_PTR(ps, destroy) = offline_imager_power_switch_destroy; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_on) = atd_power_switch_a1_switch_on; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_off) = offline_imager_switch_a1_switch_off; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_off_all) = offline_imager_switch_a1_switch_off_all;     
	POWER_SWITCH_VTABLE_PTR(ps, switch_status) = offline_imager_switch_a1_switch_status;
	POWER_SWITCH_VTABLE_PTR(ps, switch_status_for_name) = offline_imager_switch_status_for_name;
	POWER_SWITCH_VTABLE_PTR(ps, can_use_load) = offline_imager_can_use_load;
	POWER_SWITCH_VTABLE_PTR(ps, name_can_use_load) = offline_imager_name_can_use_load;

	ps->_panel_id = ui_module_add_panel(UIMODULE_CAST(ps), "OfflineImager_PowerSwitch_UI.uir", POW_PANEL, 1);  
						 
	return ps;
}


int CVICALLBACK OnSwitchChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;
			OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) callbackData;    
			PowerSwitch* ps = (PowerSwitch*) ps_a1;  
	
			GetCtrlVal(panel, control, &val);
			
			if(control == POW_PANEL_SW1)
				perform_switch(ps, 1, val);     
			
			if(control == POW_PANEL_SW2)
				perform_switch(ps, 2, val);   
			
			if(control == POW_PANEL_SW3)
				perform_switch(ps, 3, val);   

			if(control == POW_PANEL_SW4)
				perform_switch(ps, 4, val);   
				
			break;
		}
	}
	return 0;
}

int CVICALLBACK OnPowerSwitchClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OfflineImager_PowerSwitch* ps_a1 = (OfflineImager_PowerSwitch*) callbackData;   
			PowerSwitch* ps = (PowerSwitch*) ps_a1;  
	
			ui_module_hide_panel(UIMODULE_CAST(ps), panel);
				
			break;
		}
	}
	return 0;
}
