#include "ATD_PowerSwitch_B.h"
#include "ATD_PowerSwitch_B_UI.h" 
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include <utility.h>

// the switch goes from 1 to 4
static int is_switch_on(PowerSwitch *ps, int the_switch)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps;  

	if(the_switch == PS_SWITCH_1)
		return ps_b1->_switch1_can_status;
	else if(the_switch == PS_SWITCH_2)
		return ps_b1->_switch2_can_status;
	else if(the_switch == PS_SWITCH_3)
		return ps_b1->_switch3_can_status;
	else if(the_switch == PS_SWITCH_4)
		return ps_b1->_switch4_can_status;
	
	return -1;
}

static int atd_power_switch_b1_perform_switch(PowerSwitch *ps, int the_switch, int value)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps;  

	int mode, led_id, switch_id;
	unsigned char val[10];

	memset(val, 0, 10);

	//value = !value;
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchPreChange",
			GCI_VOID_POINTER, ps, GCI_INT, the_switch, GCI_INT, 1); 

	if(the_switch < PS_SWITCH_1 || the_switch > PS_SWITCH_4)
		return POWER_SWITCH_ERROR;
	
	if(the_switch == PS_SWITCH_1) {
		mode = 4;
		ps_b1->_switch1_can_status = value;
		led_id = POW_PANEL_LED1;
		switch_id = POW_PANEL_SW1;
	}
	else if (the_switch == PS_SWITCH_2) {
		mode = 5;
		ps_b1->_switch2_can_status = value;
		led_id = POW_PANEL_LED2; 
		switch_id = POW_PANEL_SW2;     
	}
	else if (the_switch == PS_SWITCH_3) {
		mode = 6;  
		ps_b1->_switch3_can_status = value;
		led_id = POW_PANEL_LED3; 
		switch_id = POW_PANEL_SW3;     
	}
	else if (the_switch == PS_SWITCH_4) {
		mode = 2;  
		ps_b1->_switch4_can_status = value;
		led_id = POW_PANEL_LED4; 
		switch_id = POW_PANEL_SW4;     
	}
		
	val[0]=ps_b1->_i2c_type;
   	val[1]=mode;
   	val[2]=value;
   	
	// PIC expects 6 bytes so even though we fill in only 3 bytes will send 6.
	if(GCI_writeI2C_multiPort(ps_b1->_com_port, 6, val, ps_b1->_i2c_bus, "perform_switch"))
		return  POWER_SWITCH_ERROR ; 

	Delay(0.5);
	
	if(is_switch_on(ps, the_switch)) {    
		SetCtrlVal(ps->_panel_id, led_id, 1); 
		SetCtrlVal(ps->_panel_id, switch_id, 1);   
	}
	else {
		SetCtrlVal(ps->_panel_id, led_id, 0);  
		SetCtrlVal(ps->_panel_id, switch_id, 0);
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchChanged",
		GCI_VOID_POINTER, ps, GCI_INT, the_switch, GCI_INT, 1); 

	return POWER_SWITCH_SUCCESS; 
}


static int atd_power_switch_b1_switch_on (PowerSwitch *ps, int the_switch)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps;   
	
	return power_switch_perform_switch(ps, the_switch, 1);   
}

static int ConfirmPowerShutdown(PowerSwitch *ps, int the_switch)
{
	// returns 1 if the user selected to turn the item off, or item should be turned off without asking
	// returns 0 if the user selected not to turn the item off
	// returns -1 if ask_shutdown is <0, meaning do not shut down and do not ask

	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps; 
	
	char buffer[500];
	char *name = "Unknown Device";
	int ask = 1, status = 0;
	
	power_switch_status(ps, the_switch, &status);

	if(the_switch == PS_SWITCH_1) {
			
		ask = ps_b1->_switch1_ask_shutdown;
		name = ps_b1->_switch1_name;		
	}
	else if(the_switch == PS_SWITCH_2) {
		ask = ps_b1->_switch2_ask_shutdown;   
		name = ps_b1->_switch2_name;
	}
	else if(the_switch == PS_SWITCH_3) {
		ask = ps_b1->_switch3_ask_shutdown;  
		name = ps_b1->_switch3_name;
	}
	else if(the_switch == PS_SWITCH_4) {
		ask = ps_b1->_switch4_ask_shutdown;  
		name = ps_b1->_switch4_name;
	}
	
	if(ask<0) { // do not allow this to turn off
		return -1;
	}
	
	if(ask && status != 0) {
		sprintf(buffer, "Do you want to switch off the %s ?", name);
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		return ConfirmPopup("Power Down", buffer);
	}
	
	return 1;
}

static int atd_power_switch_b1_switch_off (PowerSwitch *ps, int the_switch)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps;   
	
	if (ConfirmPowerShutdown(ps, the_switch)>0) {
		return power_switch_perform_switch(ps, the_switch, 0);  	
	}
	else
		return POWER_SWITCH_SUCCESS; 
}

static int atd_power_switch_b1_switch_off_all (PowerSwitch *ps)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps;   
	
	if(power_switch_off(ps, PS_SWITCH_1) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR;
		
	if(power_switch_off(ps, PS_SWITCH_2) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR;
			
	if(power_switch_off(ps, PS_SWITCH_3) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR; 
	
	if(power_switch_off(ps, PS_SWITCH_4) == POWER_SWITCH_ERROR)
		return POWER_SWITCH_ERROR; 

	return POWER_SWITCH_SUCCESS;
}

static int atd_power_switch_b1_switch_status (PowerSwitch *ps, int the_switch, int *status)
{
	atd_powerswitch_b* atd_shutter_b_shutter = (atd_powerswitch_b *) ps; 
	
	*status = is_switch_on(ps, the_switch);  
		
	return POWER_SWITCH_SUCCESS;
}


static int CVICALLBACK thread_can_use_check(void *callback)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) callback; 
    double elapsed_time;
	
	while(1) {
		
		elapsed_time = Timer() - ps_b1->_start_time;
		
		if(elapsed_time > ps_b1->_switch1_delay)
			ps_b1->_switch1_can_use = 1;
		
		if(elapsed_time > ps_b1->_switch2_delay)
			ps_b1->_switch2_can_use = 1;
		
		if(elapsed_time > ps_b1->_switch3_delay)
			ps_b1->_switch3_can_use = 1;

		if(elapsed_time > ps_b1->_switch4_delay)
			ps_b1->_switch4_can_use = 1;
		
		if(ps_b1->_switch1_can_use && ps_b1->_switch2_can_use && ps_b1->_switch3_can_use)
			break;
		
		if(elapsed_time > 15) {
			
			MessagePopup("Error", "Failed to reach switch delay");
			break;
		}
	}
	
	return POWER_SWITCH_SUCCESS;   
}

static int CVICALLBACK OnPowerSwitchClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) callbackData;   
			PowerSwitch* ps = (PowerSwitch*) ps_b1;  
	
			ui_module_hide_panel(UIMODULE_CAST(ps), panel);
				
			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnSwitchChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;
			atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) callbackData;    
			PowerSwitch* ps = (PowerSwitch*) ps_b1;  
	
			GetCtrlVal(panel, control, &val);
			
			if(control == POW_PANEL_SW1)
				power_switch_perform_switch(ps, PS_SWITCH_1, val);     
			
			if(control == POW_PANEL_SW2)
				power_switch_perform_switch(ps, PS_SWITCH_2, val);   
			
			if(control == POW_PANEL_SW3)
				power_switch_perform_switch(ps, PS_SWITCH_3, val);   

			if(control == POW_PANEL_SW4)
				power_switch_perform_switch(ps, PS_SWITCH_4, val); 
				
			break;
		}
	}
	return 0;
}

static int atd_shutter_b_power_switch_init(PowerSwitch *ps)
{
	int thread_id;
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps;
	
	ps_b1->_switch_state = 0xff;
	ps_b1->_switch1_can_use = 0;  
	ps_b1->_switch2_can_use = 0; 
	ps_b1->_switch3_can_use = 0; 
	ps_b1->_switch4_can_use = 0; 
	
	// only needs the com port the master pic is on, works directly off its fast lines.
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "COM_Port", &(ps_b1->_com_port)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_ChipAddress", &(ps_b1->_i2c_address));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_Bus", &(ps_b1->_i2c_bus));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "i2c_ChipType", &(ps_b1->_i2c_type));      
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Number_Of_Switches", &(ps_b1->_no_of_switches));
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Name", ps_b1->_switch1_name);
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Name", ps_b1->_switch2_name);
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Name", ps_b1->_switch3_name);
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch4_Name", ps_b1->_switch4_name);
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Startup", &(ps_b1->_switch1_startup)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Startup", &(ps_b1->_switch2_startup));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Startup", &(ps_b1->_switch3_startup)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch4_Startup", &(ps_b1->_switch4_startup));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_Delay", &(ps_b1->_switch1_delay)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_Delay", &(ps_b1->_switch2_delay));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_Delay", &(ps_b1->_switch3_delay)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch4_Delay", &(ps_b1->_switch4_delay));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch1_AskToShutdown", &(ps_b1->_switch1_ask_shutdown)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch2_AskToShutdown", &(ps_b1->_switch2_ask_shutdown));   
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch3_AskToShutdown", &(ps_b1->_switch3_ask_shutdown)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(ps), "Switch4_AskToShutdown", &(ps_b1->_switch4_ask_shutdown)); 

	SetCtrlAttribute(ps->_panel_id, POW_PANEL_STRING_1, ATTR_CTRL_VAL, ps_b1->_switch1_name); 
	SetCtrlAttribute(ps->_panel_id, POW_PANEL_STRING_2, ATTR_CTRL_VAL, ps_b1->_switch2_name);     
	SetCtrlAttribute(ps->_panel_id, POW_PANEL_STRING_3, ATTR_CTRL_VAL, ps_b1->_switch3_name);   
	SetCtrlAttribute(ps->_panel_id, POW_PANEL_STRING_4, ATTR_CTRL_VAL, ps_b1->_switch4_name); 

	initialise_comport(ps_b1->_com_port, 9600);

	if(ps_b1->_switch1_startup) {
		if(power_switch_on (ps, PS_SWITCH_1) == POWER_SWITCH_ERROR)
			return POWER_SWITCH_ERROR;
	} 
	
	if(ps_b1->_switch2_startup) {
		if(power_switch_on (ps, PS_SWITCH_2) == POWER_SWITCH_ERROR)
			return POWER_SWITCH_ERROR;
	} 
	
	if(ps_b1->_switch3_startup) {
		if(power_switch_on (ps, PS_SWITCH_3) == POWER_SWITCH_ERROR)
			return POWER_SWITCH_ERROR;
	} 
	
	if(ps_b1->_switch4_startup) {
		if(power_switch_on (ps, PS_SWITCH_4) == POWER_SWITCH_ERROR)
			return POWER_SWITCH_ERROR;
	}

	ps_b1->_start_time = Timer();
	
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, thread_can_use_check, ps, &thread_id);  
	
    InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW1, OnSwitchChanged, ps);
    InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW2, OnSwitchChanged, ps);    
  	InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW3, OnSwitchChanged, ps);
	InstallCtrlCallback (ps->_panel_id, POW_PANEL_SW4, OnSwitchChanged, ps);   
	InstallCtrlCallback (ps->_panel_id, POW_PANEL_CLOSE, OnPowerSwitchClose, ps);

	return POWER_SWITCH_SUCCESS;
}


static int atd_shutter_b_power_switch_destroy (PowerSwitch *ps)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps; 

	close_comport(ps_b1->_com_port);

	return POWER_SWITCH_SUCCESS;
}

static int get_switch_id_for_name(PowerSwitch* ps, const char *name)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps; 

	if(strncmp(name, ps_b1->_switch1_name, strlen(name) - 1) == 0)
		return 1;
	else if(strncmp(name, ps_b1->_switch2_name, strlen(name) - 1) == 0)
		return 2;
	else if(strncmp(name, ps_b1->_switch3_name, strlen(name) - 1) == 0)
		return 3;

	return -1;
}

static int atd_power_switch_b1_switch_status_for_name (PowerSwitch *ps, const char *name, int *status)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps; 
	
	int the_switch = get_switch_id_for_name(ps, name);

	*status = is_switch_on(ps, the_switch);  
		
	return POWER_SWITCH_SUCCESS;
}

static int atd_power_switch_b1_name_can_use_load (PowerSwitch* ps, const char *name)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps; 

	int the_switch = get_switch_id_for_name(ps, name);

	if(the_switch == 1)
		return ps_b1->_switch1_can_use;
	else if(the_switch == 2)
		return ps_b1->_switch2_can_use; 
	else if(the_switch == 3)
		return ps_b1->_switch3_can_use; 
	
	return 0;
}

static int atd_shutter_b_can_use_load (PowerSwitch* ps, int the_switch)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) ps; 
	
	if(the_switch == PS_SWITCH_1)
		return ps_b1->_switch1_can_use; 
	else if(the_switch == PS_SWITCH_2)
		return ps_b1->_switch2_can_use; 
	else if(the_switch == PS_SWITCH_3)
		return ps_b1->_switch3_can_use; 
	else if(the_switch == PS_SWITCH_4)
		return ps_b1->_switch4_can_use; 

	return 0;
}

PowerSwitch* atd_power_switch_b_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	atd_powerswitch_b* ps_b1 = (atd_powerswitch_b*) malloc(sizeof(atd_powerswitch_b));  
	PowerSwitch* ps = (PowerSwitch*) ps_b1;    
	
	memset(ps_b1, 0, sizeof(atd_powerswitch_b));

	power_switch_constructor(ps, name, description, data_dir);

	ui_module_set_error_handler(UIMODULE_CAST(ps), handler, NULL);   

	POWER_SWITCH_VTABLE_PTR(ps, hw_init) = atd_shutter_b_power_switch_init; 
	POWER_SWITCH_VTABLE_PTR(ps, destroy) = atd_shutter_b_power_switch_destroy; 
	POWER_SWITCH_VTABLE_PTR(ps, perform_switch) = atd_power_switch_b1_perform_switch; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_on) = atd_power_switch_b1_switch_on; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_off) = atd_power_switch_b1_switch_off; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_off_all) = atd_power_switch_b1_switch_off_all;     
	POWER_SWITCH_VTABLE_PTR(ps, switch_status) = atd_power_switch_b1_switch_status;
	POWER_SWITCH_VTABLE_PTR(ps, switch_status_for_name) = atd_power_switch_b1_switch_status_for_name;
	POWER_SWITCH_VTABLE_PTR(ps, can_use_load) = atd_shutter_b_can_use_load;
	POWER_SWITCH_VTABLE_PTR(ps, name_can_use_load) = atd_power_switch_b1_name_can_use_load;

	ps->_panel_id = ui_module_add_panel(UIMODULE_CAST(ps), "ATD_PowerSwitch_B_UI.uir", POW_PANEL, 1);  
						 
	return ps;
}

