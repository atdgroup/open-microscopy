#include "ATD_PowerSwitch_A.h"
#include "ATD_PowerSwitch_A_UI.h" 
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include <utility.h>

// the switch goes from 1 to 3
static int is_switch_on(PowerSwitch *ps, int the_switch)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps;  
	unsigned char inVal[10];
	unsigned int readVal; 

	if(the_switch < 1 || the_switch > 3)
		return -1;
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_i2c_read_bytes(ps_a->_controller, ps_a->_i2c_address, 1, inVal) != FT_OK) {
		return -1;
	}

	#else

	inVal[0]=ps_a->_i2c_type | ps_a->_i2c_address<<1 | 0x01;	   //for read
	
	GCI_readI2C_multiPort(ps_a->_com_port, 1, inVal, ps_a->_i2c_bus, "is_switch_on");
	
	#endif

	readVal = inVal[0];
	
	if(the_switch == 1)
		return ((readVal & 0x10)>>4)^1;   //bit shift and invert
	else if(the_switch == 2)
		return ((readVal & 0x20)>>5)^1;   //bit shift and invert
	else if(the_switch == 3)
		return ((readVal & 0x40)>>6)^1;   //bit shift and invert
	
	return -1;
}


static int atd_power_switch_a_perform_switch(PowerSwitch *ps, int the_switch, int value)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps;  

	int bit, led_id, switch_id;
	byte val[10];

	memset(val, 0, 10);

	value = !value;
	
	if(the_switch < 1 || the_switch > 3)
		return POWER_SWITCH_ERROR;
	
	if(the_switch == 1) {
		bit = 0xfe;
		led_id = POW_PANEL_LED1;
		switch_id = POW_PANEL_SW1;
	}
	else if (the_switch == 2) {
		bit = 0xfd;
		value = value << 1;
		led_id = POW_PANEL_LED2; 
		switch_id = POW_PANEL_SW2;     
	}
	else if (the_switch == 3) {
		bit = 0xfb;
		value = value << 2;       
		led_id = POW_PANEL_LED3; 
		switch_id = POW_PANEL_SW3;     
	}
		
	ps_a->_switch_state = ps_a->_switch_state & bit;   			//clear the bit
	ps_a->_switch_state = ps_a->_switch_state | value;			//set the bit
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	val[0]=(byte) ps_a->_switch_state;
   		
	if(ftdi_controller_i2c_write_bytes(ps_a->_controller, ps_a->_i2c_address, 1, val) != FT_OK) {
		return POWER_SWITCH_ERROR;
	}

	#else

	val[0] = ps_a->_i2c_type | ps_a->_i2c_address<<1;			//device address
	val[1] = (byte) ps_a->_switch_state;
	
	if(GCI_writeI2C_multiPort(ps_a->_com_port, 2, val, ps_a->_i2c_bus, "perform_switch") < 0)
		return POWER_SWITCH_ERROR;

	#endif

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


static int atd_power_switch_a_switch_on (PowerSwitch *ps, int the_switch)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps;   
	
	return atd_power_switch_a_perform_switch(ps, the_switch, 1);   
}

static int ConfirmPowerShutdown(PowerSwitch *ps, int the_switch)
{
	// returns 1 if the user selected to turn the item off, or item should be turned off without asking
	// returns 0 if the user selected not to turn the item off
	// returns -1 if ask_shutdown is <0, meaning do not shut down and do not ask

	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps; 
	
	char buffer[500];
	char *name;
	int ask = 1;
	
	if(the_switch == 1) {
			
		ask = ps_a->_switch1_ask_shutdown;
		name = ps_a->_switch1_name;		
	}
	else if(the_switch == 2) {
		ask = ps_a->_switch2_ask_shutdown;   
		name = ps_a->_switch2_name;
	}
	else if(the_switch == 3) {
		ask = ps_a->_switch3_ask_shutdown;  
		name = ps_a->_switch3_name;
	}
	
	if(ask<0) { // do not allow this to turn off
		return -1;
	}

	if(ask) {
		sprintf(buffer, "Do you want to switch off the %s ?", name); 
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		return GCI_ConfirmPopup("Power Down", IDI_WARNING, buffer);
	}
	
	return 1;
}

static int atd_power_switch_a_switch_off (PowerSwitch *ps, int the_switch)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps;   
	
	if (ConfirmPowerShutdown(ps, the_switch)>0) {
		return atd_power_switch_a_perform_switch(ps, the_switch, 0);  	
	}
	else {
		ps_a->_keep_box_on_reminder = 1;
		return POWER_SWITCH_SUCCESS; 
	}
}

static int atd_power_switch_a_switch_off_all (PowerSwitch *ps)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps;   
	
	if(atd_power_switch_a_switch_off(ps, 1) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR;
		
	if(atd_power_switch_a_switch_off(ps, 2) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR;
			
	if(atd_power_switch_a_switch_off(ps, 3) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR; 
	
	if(ps_a->_keep_box_on_reminder) {

		GCI_MessagePopup("Warning", "Please keep the control box switched on.");
	}

	return POWER_SWITCH_SUCCESS;
}

static int atd_power_switch_a_switch_status (PowerSwitch *ps, int the_switch, int *status)
{
	atd_powerswitch_a* atd_shutter_a_shutter = (atd_powerswitch_a *) ps; 
	
	*status = is_switch_on(ps, the_switch);  
		
	return POWER_SWITCH_SUCCESS;
}

static int CVICALLBACK thread_can_use_check(void *callback)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) callback; 
    double elapsed_time;
	
	while(1) {
		
		elapsed_time = Timer() - ps_a->_start_time;
		
		if(elapsed_time > ps_a->_switch1_delay)
			ps_a->_switch1_can_use = 1;
		
		if(elapsed_time > ps_a->_switch2_delay)
			ps_a->_switch2_can_use = 1;
		
		if(elapsed_time > ps_a->_switch3_delay)
			ps_a->_switch3_can_use = 1;
		
		if(ps_a->_switch1_can_use && ps_a->_switch2_can_use && ps_a->_switch3_can_use)
			break;
		
		if(elapsed_time > 15) {
			
			GCI_MessagePopup("Error", "Failed to reach switch delay");
			break;
		}
	}
	
	return POWER_SWITCH_SUCCESS;   
}

static int CVICALLBACK OnSwitchChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;
			atd_powerswitch_a* ps_a = (atd_powerswitch_a*) callbackData;    
			PowerSwitch* ps = (PowerSwitch*) ps_a;  
	
			GetCtrlVal(panel, control, &val);
			
			if(control == POW_PANEL_SW1)
				power_switch_perform_switch(ps, 1, val);     
			
			if(control == POW_PANEL_SW2)
				power_switch_perform_switch(ps, 2, val);   
			
			if(control == POW_PANEL_SW3)
				power_switch_perform_switch(ps, 3, val);   
				
			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnPowerSwitchClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			atd_powerswitch_a* ps_a = (atd_powerswitch_a*) callbackData;   
			PowerSwitch* ps = (PowerSwitch*) ps_a;  
	
			ui_module_hide_panel(UIMODULE_CAST(ps), panel);
				
			break;
		}
	}
	return 0;
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

static int atd_power_switch_a_switch_init(PowerSwitch *ps)
{
	int thread_id;
	char device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH] = "";
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps;
	
	ps_a->_keep_box_on_reminder = 0;
	ps_a->_switch_state = 0xff;
	ps_a->_switch1_can_use = 0;  
	ps_a->_switch2_can_use = 0; 
	ps_a->_switch3_can_use = 0; 
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Number_Of_Switches", &(ps_a->_no_of_switches));
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Name", ps_a->_switch1_name);
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Name", ps_a->_switch2_name);
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Name", ps_a->_switch3_name);
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Startup", &(ps_a->_switch1_startup)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Startup", &(ps_a->_switch2_startup));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Startup", &(ps_a->_switch3_startup));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Delay", &(ps_a->_switch1_delay)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Delay", &(ps_a->_switch2_delay));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Delay", &(ps_a->_switch3_delay)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_AskToShutdown", &(ps_a->_switch1_ask_shutdown)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_AskToShutdown", &(ps_a->_switch2_ask_shutdown));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_AskToShutdown", &(ps_a->_switch3_ask_shutdown)); 

	SetCtrlAttribute(ps->_panel_id, POW_PANEL_STRING_1, ATTR_CTRL_VAL, ps_a->_switch1_name); 
	SetCtrlAttribute(ps->_panel_id, POW_PANEL_STRING_2, ATTR_CTRL_VAL, ps_a->_switch2_name);     
	SetCtrlAttribute(ps->_panel_id, POW_PANEL_STRING_3, ATTR_CTRL_VAL, ps_a->_switch3_name);   

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "FTDI_SN", device_sn);  
	get_device_int_param_from_ini_file   (UIMODULE_GET_NAME(ps), "I2C_DEVICE_BUS", &(ps_a->_i2c_bus));
	get_device_int_param_from_ini_file   (UIMODULE_GET_NAME(ps), "I2C_DEVICE_ADDRESS", &(ps_a->_i2c_address));  
	
	// Real address. The I2C_DEVICE_ADDRESS is the hardcoded address
	// In this case it can be one of 7 dufferent values depending of set hardware
	// switches. We are using the 7th.
	ps_a->_i2c_address = ps_a->_i2c_address | 7;

	ps_a->_controller = ftdi_controller_new();

	//ftdi_controller_set_debugging(ps_a->controller, 1);
	ftdi_controller_set_error_handler(ps_a->_controller, ftdi_error_handler, NULL);
	ftdi_controller_open(ps_a->_controller, device_sn);

	#else
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "COM_Port", &(ps_a->_com_port)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_ChipAddress", &(ps_a->_i2c_address));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_Bus", &(ps_a->_i2c_bus));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_ChipType", &(ps_a->_i2c_type));      
	
	initialise_comport(ps_a->_com_port, 9600);

	#endif

	// Rob's code does not do this.
	// I get the status of the switches / latches and reconstruct the 
	// ps_a->_switch_state variable from last program run.
	if(is_switch_on(ps, 1)) {
		ps_a->_switch_state &= 0xFE;

		SetCtrlVal(ps->_panel_id, POW_PANEL_LED1, 1); 
		SetCtrlVal(ps->_panel_id, POW_PANEL_SW1, 1);  
	}

	if(is_switch_on(ps, 2)) {
		ps_a->_switch_state &= 0xFD;

		SetCtrlVal(ps->_panel_id, POW_PANEL_LED2, 1); 
		SetCtrlVal(ps->_panel_id, POW_PANEL_SW2, 1);  
	}

	if(is_switch_on(ps, 3)) {
		ps_a->_switch_state &= 0xFB;

		SetCtrlVal(ps->_panel_id, POW_PANEL_LED3, 1); 
		SetCtrlVal(ps->_panel_id, POW_PANEL_SW3, 1);  
	}

	// Now that ps_a->_switch_state is in a valid state we can switch

	if(ps_a->_switch1_startup) {
		if(atd_power_switch_a_switch_on (ps, 1) == POWER_SWITCH_ERROR)
			return POWER_SWITCH_ERROR;
	}

	if(ps_a->_switch2_startup) {
		if(atd_power_switch_a_switch_on (ps, 2) == POWER_SWITCH_ERROR)
			return POWER_SWITCH_ERROR;
	}

	if(ps_a->_switch3_startup) {
		if(atd_power_switch_a_switch_on (ps, 3) == POWER_SWITCH_ERROR)
			return POWER_SWITCH_ERROR;
	}

	ps_a->_start_time = Timer();
	
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, thread_can_use_check, ps, &thread_id);  
	
    InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW1, OnSwitchChanged, ps);
    InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW2, OnSwitchChanged, ps);    
  	InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW3, OnSwitchChanged, ps);    
	InstallCtrlCallback (ps->_panel_id, POW_PANEL_CLOSE, OnPowerSwitchClose, ps);

	return POWER_SWITCH_SUCCESS;
}


static int atd_power_switch_a_switch_destroy (PowerSwitch *ps)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps; 

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(ps_a->_controller);
	#else
	close_comport(ps_a->_com_port);
	#endif

	return POWER_SWITCH_SUCCESS;
}


static int get_switch_id_for_name(PowerSwitch* ps, const char *name)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps; 

	if(strncmp(name, ps_a->_switch1_name, strlen(name) - 1) == 0)
		return 1;
	else if(strncmp(name, ps_a->_switch2_name, strlen(name) - 1) == 0)
		return 2;
	else if(strncmp(name, ps_a->_switch3_name, strlen(name) - 1) == 0)
		return 3;

	return -1;
}

static int atd_power_switch_a_switch_status_for_name (PowerSwitch *ps, const char *name, int *status)
{
	atd_powerswitch_a* atd_shutter_a_shutter = (atd_powerswitch_a *) ps; 
	
	int the_switch = get_switch_id_for_name(ps, name);

	*status = is_switch_on(ps, the_switch);  
		
	return POWER_SWITCH_SUCCESS;
}

static int atd_power_switch_a_name_can_use_load (PowerSwitch* ps, const char *name)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps; 

	int the_switch = get_switch_id_for_name(ps, name);

	if(the_switch == 1)
		return ps_a->_switch1_can_use;
	else if(the_switch == 2)
		return ps_a->_switch2_can_use; 
	else if(the_switch == 3)
		return ps_a->_switch3_can_use; 
	
	return 0;
}

static int atd_power_switch_a_can_use_load (PowerSwitch* ps, int the_switch)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) ps; 
	
	if(the_switch == 1)
		return ps_a->_switch1_can_use; 
	else if(the_switch == 2)
		return ps_a->_switch2_can_use; 
	else if(the_switch == 3)
		return ps_a->_switch3_can_use; 
	
	return 0;
}


PowerSwitch* atd_power_switch_a_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	atd_powerswitch_a* ps_a = (atd_powerswitch_a*) malloc(sizeof(atd_powerswitch_a));  
	PowerSwitch* ps = (PowerSwitch*) ps_a;    
	
	power_switch_constructor(ps, name, description, data_dir);

	ui_module_set_error_handler(UIMODULE_CAST(ps), handler, NULL);   

	POWER_SWITCH_VTABLE_PTR(ps, hw_init) = atd_power_switch_a_switch_init; 
	POWER_SWITCH_VTABLE_PTR(ps, destroy) = atd_power_switch_a_switch_destroy; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_on) = atd_power_switch_a_switch_on; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_off) = atd_power_switch_a_switch_off; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_off_all) = atd_power_switch_a_switch_off_all;     
	POWER_SWITCH_VTABLE_PTR(ps, switch_status) = atd_power_switch_a_switch_status;
	POWER_SWITCH_VTABLE_PTR(ps, switch_status_for_name) = atd_power_switch_a_switch_status_for_name;
	POWER_SWITCH_VTABLE_PTR(ps, can_use_load) = atd_power_switch_a_can_use_load;
	POWER_SWITCH_VTABLE_PTR(ps, name_can_use_load) = atd_power_switch_a_name_can_use_load;
	POWER_SWITCH_VTABLE_PTR(ps, perform_switch) = atd_power_switch_a_perform_switch;

	ps->_panel_id = ui_module_add_panel(UIMODULE_CAST(ps), "ATD_PowerSwitch_A_UI.uir", POW_PANEL, 1);  
						 
	return ps;
}