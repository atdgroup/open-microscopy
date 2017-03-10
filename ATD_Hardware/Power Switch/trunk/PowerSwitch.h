#ifndef __POWER_SWITCH__
#define __POWER_SWITCH__

#include "HardwareTypes.h"
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"   

#include <ansi_c.h>

#define POWER_SWITCH_SUCCESS 0
#define POWER_SWITCH_ERROR -1

#define POWER_SWITCH_VTABLE_PTR(ob, member) ((ob)->lpVtbl.member)
#define POWER_SWITCH_VTABLE(ob, member) (*((ob)->lpVtbl.member))

typedef enum {PS_SWITCH_ALL = 0,
			  PS_SWITCH_1,
			  PS_SWITCH_2,
			  PS_SWITCH_3,
			  PS_SWITCH_4,
			  PS_SWITCH_5,
			 } PS_SWITCH;

#define CHECK_POWER_SWITCH_VTABLE_PTR(ob, member) if(POWER_SWITCH_VTABLE_PTR(ob, member) == NULL) { \
    send_power_switch_error_text(ob, "member not implemented"); \
    return POWER_SWITCH_ERROR; \
}  

#define CALL_POWER_SWITCH_VTABLE_PTR(ob, member) if( POWER_SWITCH_VTABLE(ob, member)(ob) == POWER_SWITCH_ERROR ) { \
	send_power_switch_error_text(ob, "member failed");  \
	return POWER_SWITCH_ERROR; \
}

typedef struct
{
	int (*hw_init) (PowerSwitch* ps); 
	int (*destroy) (PowerSwitch* ps);
	int (*can_use_load) (PowerSwitch* ps, int the_switch);
	int (*name_can_use_load) (PowerSwitch* ps, const char *name);
	int (*perform_switch) (PowerSwitch *ps, int the_switch, int value);
	int (*switch_on) (PowerSwitch* ps, int the_switch);
	int (*switch_off) (PowerSwitch* ps, int the_switch); 
	int (*turn_switches_off) (PowerSwitch* ps, int switches);
	int (*switch_off_all) (PowerSwitch* ps); 
	int (*switch_status) (PowerSwitch* ps, int the_switch, int *status);
	int (*switch_status_for_name) (PowerSwitch* ps, const char *name, int *status);
	int (*ask_user_for_switches_to_turn_off) (PowerSwitch* ps, int *switches);
	
} PowerSwitchVtbl;


struct _PowerSwitch
{
  HardwareDevice parent; 
  
  PowerSwitchVtbl lpVtbl;
 
  int		 _panel_id; 
  int		 _lock;
  int     	 _timer;
  int		 _initialised;
  int 		 _enabled;
};


void power_switch_constructor(PowerSwitch* ps, const char *name, const char *description, const char *data_dir);

int power_switch_initialise(PowerSwitch* ps);    
int power_switch_is_initialised(PowerSwitch* ps);
int  send_power_switch_error_text (PowerSwitch* ps, char fmt[], ...);
void power_switch_set_error_handler(PowerSwitch* ps, UI_MODULE_ERROR_HANDLER handler, void *callback_data);

int power_switch_destroy(PowerSwitch* ps);
int power_switch_perform_switch(PowerSwitch *ps, int the_switch, int value);
int power_switch_on(PowerSwitch* ps, int the_switch);
int power_switch_off(PowerSwitch* ps, int the_switch);  
int power_switch_off_all(PowerSwitch* ps); 
int power_switch_status(PowerSwitch* ps, int the_switch, int *status);
int power_switch_status_for_name(PowerSwitch* ps, const char *name, int *status);
int power_switch_can_use_load(PowerSwitch *ps, int the_switch);
int power_switch_device_name_can_use_load(PowerSwitch *ps, const char* name);
int power_switch_ask_for_switches_to_turn_off(PowerSwitch *ps, int *switches);

// Signals
typedef void (*POWER_SWITCH_CHANGE_EVENT_HANDLER) (PowerSwitch* ps, int the_switch, int status, void *data); 

int power_switch_pre_change_handler_connect(PowerSwitch* ps, POWER_SWITCH_CHANGE_EVENT_HANDLER handler, void *data );
int power_switch_changed_handler_connect(PowerSwitch* ps, POWER_SWITCH_CHANGE_EVENT_HANDLER handler, void *data );

#endif

 
