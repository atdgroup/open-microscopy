#include "PowerSwitch.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "iniparser.h"  
#include "ThreadDebug.h"

#include "GL_CVIRegistry.h"

#include <utility.h>
#include "toolbox.h"

#include "asynctmr.h"

#include <ansi_c.h> 

int send_power_switch_error_text (PowerSwitch* ps, char fmt[], ...)
{
	int ret=0;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(ps), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(ps), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(ps), "Shutter Error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}

	
static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	GCI_MessagePopup("Shutter Error", error_string); 
	
	return UIMODULE_ERROR_NONE;    
}


static int POWER_SWITCH_PTR_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (PowerSwitch*, int, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (PowerSwitch *) args[0].void_ptr_data, (int) args[1].int_data, (int) args[2].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int power_switch_initialise (PowerSwitch* ps)
{
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, hw_init) 
		
	if( POWER_SWITCH_VTABLE(ps, hw_init)(ps) == POWER_SWITCH_ERROR ) {
		ps->_initialised = 0;
		return POWER_SWITCH_ERROR; 
	}
		
	ps->_initialised = 1;
	
  	return POWER_SWITCH_SUCCESS;
}


int power_switch_device_name_can_use_load(PowerSwitch *ps, const char* name)
{
	int ret;

	CHECK_POWER_SWITCH_VTABLE_PTR(ps, name_can_use_load)  
		
	GciCmtGetLock(ps->_lock);

	ret = POWER_SWITCH_VTABLE(ps, name_can_use_load)(ps, name);	

	GciCmtReleaseLock(ps->_lock);

	return ret;
}

int power_switch_can_use_load(PowerSwitch *ps, int the_switch)
{
	int ret;

	CHECK_POWER_SWITCH_VTABLE_PTR(ps, can_use_load)  
	
	GciCmtGetLock(ps->_lock);

	ret = POWER_SWITCH_VTABLE(ps, can_use_load)(ps, the_switch);	

	GciCmtReleaseLock(ps->_lock);

	return ret;
}

int power_switch_is_initialised(PowerSwitch *ps)
{
	return ps->_initialised;	
}

int power_switch_pre_change_handler_connect(PowerSwitch* ps, POWER_SWITCH_CHANGE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchPreChange", handler, data) == SIGNAL_ERROR) {
		send_power_switch_error_text(ps, "Can not connect signal handler for power switch signal");
		return POWER_SWITCH_ERROR;
	}

	return POWER_SWITCH_SUCCESS;
}

int power_switch_changed_handler_connect(PowerSwitch* ps, POWER_SWITCH_CHANGE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchChanged", handler, data) == SIGNAL_ERROR) {
		send_power_switch_error_text(ps, "Can not connect signal handler for power switch signal");
		return POWER_SWITCH_ERROR;
	}

	return POWER_SWITCH_SUCCESS;
}


void power_switch_constructor(PowerSwitch* ps, const char *name, const char *description, const char *data_dir)
{
	ps->_initialised = 0;  
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(ps), name);
	ui_module_set_description(UIMODULE_CAST(ps), description);       
	ui_module_set_data_dir(UIMODULE_CAST(ps), data_dir);

	GciCmtNewLock ("PowerSwitch", 0, &(ps->_lock) );  

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchChanged", POWER_SWITCH_PTR_INT_INT_MARSHALLER);   
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchPreChange", POWER_SWITCH_PTR_INT_INT_MARSHALLER); 

	POWER_SWITCH_VTABLE_PTR(ps, hw_init) = NULL;   
	POWER_SWITCH_VTABLE_PTR(ps, destroy) = NULL; 
	POWER_SWITCH_VTABLE_PTR(ps, perform_switch) = NULL;
	POWER_SWITCH_VTABLE_PTR(ps, switch_on) = NULL; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_off) = NULL; 
	POWER_SWITCH_VTABLE_PTR(ps, switch_off_all) = NULL;
	POWER_SWITCH_VTABLE_PTR(ps, switch_status) = NULL;
	POWER_SWITCH_VTABLE_PTR(ps, can_use_load) = NULL;  
}


int power_switch_destroy(PowerSwitch* ps)
{
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, destroy) 
  	
	CALL_POWER_SWITCH_VTABLE_PTR(ps, destroy) 
	
	CmtDiscardLock (ps->_lock);
 
	ui_module_destroy(UIMODULE_CAST(ps));  
  	
  	free(ps);
  	
  	return POWER_SWITCH_SUCCESS;
}


void power_switch_set_error_handler(PowerSwitch* ps, UI_MODULE_ERROR_HANDLER handler, void *callback_data)
{
	ui_module_set_error_handler(UIMODULE_CAST(ps), handler, callback_data);	

}


int power_switch_perform_switch(PowerSwitch *ps, int the_switch, int value)
{
	int err_status = UIMODULE_ERROR_NONE;  
	
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, perform_switch)  

	logger_log(UIMODULE_LOGGER(ps), LOGGER_INFORMATIONAL, "%s switch %d to %d", UIMODULE_GET_DESCRIPTION(ps), the_switch, value);  
		
	GciCmtGetLock(ps->_lock);

	//power_switch_status(ps, the_switch, &status);

	//if(status != value)
	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchPreChange",
	//		GCI_VOID_POINTER, ps, GCI_INT, the_switch, GCI_INT, 1); 

	do {
		err_status = UIMODULE_ERROR_NONE;
		
		if( POWER_SWITCH_VTABLE(ps, perform_switch)(ps, the_switch, value) == POWER_SWITCH_ERROR ) {
			err_status = send_power_switch_error_text(ps, "perform_switch failed");
		
			if(err_status == UIMODULE_ERROR_IGNORE) {
				GciCmtReleaseLock(ps->_lock);
				return POWER_SWITCH_ERROR; 
			}
		}
		
	} 
	while(err_status == UIMODULE_ERROR_RETRY);
	
	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchChanged",
	//	GCI_VOID_POINTER, ps, GCI_INT, the_switch, GCI_INT, 1); 

	GciCmtReleaseLock(ps->_lock);

  	return POWER_SWITCH_SUCCESS;
}

int power_switch_on(PowerSwitch* ps, int the_switch)
{
	int err_status = UIMODULE_ERROR_NONE, status;  
	
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, switch_on)  

	logger_log(UIMODULE_LOGGER(ps), LOGGER_INFORMATIONAL, "%s switch on %d", UIMODULE_GET_DESCRIPTION(ps), the_switch);  

	GciCmtGetLock(ps->_lock);

	power_switch_status(ps, the_switch, &status);

	//if(status == 0) {
	//	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchPreChange",
	//		GCI_VOID_POINTER, ps, GCI_INT, the_switch, GCI_INT, 1); 
	//}

	do {
		err_status = UIMODULE_ERROR_NONE;
		
		if( POWER_SWITCH_VTABLE(ps, switch_on)(ps, the_switch) == POWER_SWITCH_ERROR ) {
			err_status = send_power_switch_error_text(ps, "power_switch_on failed");
		
			if(err_status == UIMODULE_ERROR_IGNORE) {
				GciCmtReleaseLock(ps->_lock);
				return POWER_SWITCH_ERROR; 
			}
		}
		
	} 
	while(err_status == UIMODULE_ERROR_RETRY);
	
	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchChanged",
	//	GCI_VOID_POINTER, ps, GCI_INT, the_switch, GCI_INT, 1); 

	GciCmtReleaseLock(ps->_lock);

  	return POWER_SWITCH_SUCCESS;
}

/*
// This function returns the bits representing switches that the user wants to turn off.
// It asks the user with a dialog for each switch.
int power_switch_ask_for_switches_to_turn_off(PowerSwitch *ps, int *switches)
{
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, ask_user_for_switches_to_turn_off)  

	POWER_SWITCH_VTABLE(ps, switch_off)(ps, switches);

	return POWER_SWITCH_SUCCESS;
}
*/

/*
int power_switch_turn_switches_off (PowerSwitch* ps, int switches)
{

}
*/
/*
int power_switch_ask_for_switches_to_turn_off(PowerSwitch *ps, int *switches)
{
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, ask_user_for_switches_to_turn_off)  

	POWER_SWITCH_VTABLE(ps, switch_off)(ps, switches);

	return POWER_SWITCH_SUCCESS;
}
*/
int power_switch_off(PowerSwitch* ps, int the_switch)
{
	int err_status = UIMODULE_ERROR_NONE;  
	
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, switch_off)  

	logger_log(UIMODULE_LOGGER(ps), LOGGER_INFORMATIONAL, "%s switch off %d", UIMODULE_GET_DESCRIPTION(ps), the_switch);  
		
	GciCmtGetLock(ps->_lock);

	//power_switch_status(ps, the_switch, &status);

	//if(status == 1) {
	//	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchPreChange", GCI_VOID_POINTER, ps,
	//				GCI_INT, the_switch, GCI_INT, 0); 
	//}

	do {
		err_status = UIMODULE_ERROR_NONE;
		
		if( POWER_SWITCH_VTABLE(ps, switch_off)(ps, the_switch) == POWER_SWITCH_ERROR ) {
			err_status = send_power_switch_error_text(ps, "power_switch_off failed");
		
			if(err_status == UIMODULE_ERROR_IGNORE) {
				GciCmtReleaseLock(ps->_lock);
				return POWER_SWITCH_ERROR;
			}
		}
		
	} 
	while(err_status == UIMODULE_ERROR_RETRY);
	
	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps),
	//			"PowerSwitchChanged", GCI_VOID_POINTER, ps, GCI_INT, the_switch, GCI_INT, 0); 

	GciCmtReleaseLock(ps->_lock);


  	return POWER_SWITCH_SUCCESS;
}


int power_switch_off_all(PowerSwitch* ps)
{
	int err_status = UIMODULE_ERROR_NONE;  
	
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, switch_off_all)  

	logger_log(UIMODULE_LOGGER(ps), LOGGER_INFORMATIONAL, "%s switch off all", UIMODULE_GET_DESCRIPTION(ps));  

	GciCmtGetLock(ps->_lock);

	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchPreChange",
	//		GCI_VOID_POINTER, ps, GCI_INT, PS_SWITCH_ALL, GCI_INT, 0); 

	do {
		err_status = UIMODULE_ERROR_NONE;
		
		if( POWER_SWITCH_VTABLE(ps, switch_off_all)(ps) == POWER_SWITCH_ERROR ) {
			err_status = send_power_switch_error_text(ps, "power_switch_off_all failed");
		
			if(err_status == UIMODULE_ERROR_IGNORE) {
				GciCmtReleaseLock(ps->_lock);
				return POWER_SWITCH_ERROR;
			}
		}
		
	} 
	while(err_status == UIMODULE_ERROR_RETRY);
	
	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(ps), "PowerSwitchChanged",
	//		GCI_VOID_POINTER, ps, GCI_INT, PS_SWITCH_ALL, GCI_INT, 0); 

	GciCmtReleaseLock(ps->_lock);

  	return POWER_SWITCH_SUCCESS;	
}


int power_switch_status(PowerSwitch* ps, int the_switch, int *status)
{
	int err_status = UIMODULE_ERROR_NONE;  
	
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, switch_status)  
		
	GciCmtGetLock(ps->_lock);

	do {
		err_status = UIMODULE_ERROR_NONE;
		
		if( POWER_SWITCH_VTABLE(ps, switch_status)(ps, the_switch, status) == POWER_SWITCH_ERROR ) {
			err_status = send_power_switch_error_text(ps, "power_switch_status failed");
		
			if(err_status == UIMODULE_ERROR_IGNORE) {
				GciCmtReleaseLock(ps->_lock);
				return POWER_SWITCH_ERROR;
			}
		}
		
	} 
	while(err_status == UIMODULE_ERROR_RETRY);
	
	GciCmtReleaseLock(ps->_lock);

  	return POWER_SWITCH_SUCCESS;
}

int power_switch_status_for_name(PowerSwitch* ps, const char *name, int *status)
{
	int err_status = UIMODULE_ERROR_NONE;  
	
	CHECK_POWER_SWITCH_VTABLE_PTR(ps, switch_status_for_name)  
		
	GciCmtGetLock(ps->_lock);

	do {
		err_status = UIMODULE_ERROR_NONE;
		
		if( POWER_SWITCH_VTABLE(ps, switch_status_for_name)(ps, name, status) == POWER_SWITCH_ERROR ) {
			err_status = send_power_switch_error_text(ps, "switch_status_for_name failed");
		
			if(err_status == UIMODULE_ERROR_IGNORE) {
				GciCmtReleaseLock(ps->_lock);
				return POWER_SWITCH_ERROR;
			}
		}
		
	} 
	while(err_status == UIMODULE_ERROR_RETRY);
	
	GciCmtReleaseLock(ps->_lock);

  	return POWER_SWITCH_SUCCESS;
}