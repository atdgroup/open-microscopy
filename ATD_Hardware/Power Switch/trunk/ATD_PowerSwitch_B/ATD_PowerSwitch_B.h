#ifndef __ATD_POWER_SWITCH_B__
#define __ATD_POWER_SWITCH_B__

#include "PowerSwitch.h"

typedef struct
{
	PowerSwitch parent;

	int 	 _switch_state;  
	int		 _no_of_switches;
	int	 	 _com_port;
	int 	 _i2c_address;
	int 	 _i2c_bus;
	int 	 _i2c_type;
  	int 	 _lock;
	int 	 _switch1_can_status;    
	int 	 _switch2_can_status;    
	int 	 _switch3_can_status;
	int 	 _switch4_can_status;
	int 	 _switch1_can_use;    
	int 	 _switch2_can_use;    
	int 	 _switch3_can_use;
	int 	 _switch4_can_use;
	int 	 _switch1_startup;    
	int 	 _switch2_startup;    
	int 	 _switch3_startup;
	int 	 _switch4_startup;
	int 	 _switch1_delay;    
	int 	 _switch2_delay;    
	int 	 _switch3_delay;
	int 	 _switch4_delay;
	int 	 _switch1_ask_shutdown;
	int 	 _switch2_ask_shutdown;  
	int 	 _switch3_ask_shutdown;  
	int 	 _switch4_ask_shutdown;
	char 	 _switch1_name[100];
	char 	 _switch2_name[100]; 
	char 	 _switch3_name[100]; 
	char 	 _switch4_name[100]; 
	double 	 _start_time;
	
	
} atd_powerswitch_b;

PowerSwitch* atd_power_switch_b_new(const char *name, const char *description,
	UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

int CVICALLBACK OnSwitchChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnPowerSwitchClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

#endif
