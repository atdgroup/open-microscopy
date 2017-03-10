#include "HardWareTypes.h" 

#include "TemperatureMonitor.h"
#include "TemperatureMonitorUI.h" 
#include "string_utils.h"
#include "gci_utils.h"
#include "iniparser.h"  

#include "GL_CVIRegistry.h"

#include <utility.h>
#include "toolbox.h"

#include "asynctmr.h"

#include <ansi_c.h> 

int send_temperature_monitor_error_text (TemperatureMonitor* tm, char fmt[], ...)
{
	int ret=0;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(tm), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(tm), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(tm), "Temp Monitor Error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}

static int TEMPERATURE_MONITOR_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (TemperatureMonitor*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (TemperatureMonitor *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int temperature_monitor_read_temperatures (TemperatureMonitor* tm, double *value1, double *value2, double *value3, double *value4)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_TEMPERATURE_MONITOR_VTABLE_PTR(tm, read_temperatures) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( TEMPERATURE_MONITOR_VTABLE(tm, read_temperatures)(tm, value1, value2, value3, value4) == TEMPERATURE_MONITOR_ERROR ) {
			status = send_temperature_monitor_error_text(tm, "read_temperature failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				tm->_hw_initialised = 0;   
				return TEMPERATURE_MONITOR_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
	tm->_hw_initialised = 1;
	
  	return TEMPERATURE_MONITOR_SUCCESS;
}


int temperature_monitor_get_status_flags (TemperatureMonitor* tm, int *thermostat_error,
											int *heater_over_temp, int *enclosure_stat)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_TEMPERATURE_MONITOR_VTABLE_PTR(tm, get_status_flags) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( TEMPERATURE_MONITOR_VTABLE(tm, get_status_flags)(tm, thermostat_error, heater_over_temp, enclosure_stat) == TEMPERATURE_MONITOR_ERROR ) {
			status = send_temperature_monitor_error_text(tm, "get_status_flags failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return TEMPERATURE_MONITOR_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
  	return TEMPERATURE_MONITOR_SUCCESS;
}

int temperature_monitor_add_settings_panel (TemperatureMonitor* tm, int panel_id)
{
	tm->_setup_panel_id = panel_id;

	return TEMPERATURE_MONITOR_SUCCESS;
}

int temperature_monitor_clear_errors (TemperatureMonitor* tm)
{
	ListClear(tm->_error_list);

	return TEMPERATURE_MONITOR_SUCCESS;
}

void temperature_monitor_set_error (TemperatureMonitor* tm, TempError *error, TEMP_ERROR type, char *error_str)
{
	
}

int temperature_monitor_add_error (TemperatureMonitor* tm, int log, TEMP_ERROR type, char *error_str)
{
	TempError error;

	error.type = type;
	strncpy(error.error, error_str, 200);

	if(log) {
		if(type == TEMP_ERROR_CRITICAL)
			logger_log(UIMODULE_LOGGER(tm), LOGGER_ERROR, "%s: %s", UIMODULE_GET_DESCRIPTION(tm), error_str);
		else
			logger_log(UIMODULE_LOGGER(tm), LOGGER_INFORMATIONAL, "%s: %s", UIMODULE_GET_DESCRIPTION(tm), error_str);
	}

	ListInsertItem(tm->_error_list, &error, END_OF_LIST);

	return TEMPERATURE_MONITOR_SUCCESS;
}

static void temperature_monitor_display_errors (TemperatureMonitor* tm)
{
	char buffer[100] = "";
	int i;
	TempError *node = NULL;
	
	ClearListCtrl (tm->_panel_id, TEMP_PNL_ERRORBOX);
	SetCtrlAttribute (tm->_panel_id, TEMP_PNL_ERRORBOX, ATTR_HILITE_CURRENT_ITEM, 0);

	for(i=1; i <= ListNumItems(tm->_error_list); i++) {
		node = ListGetPtrToItem(tm->_error_list, i);

		if(node->type == TEMP_ERROR_CRITICAL) {
			sprintf(buffer, "\033fgFFFFFF\033bgFF0000%s", node->error);
		}
		else
			sprintf(buffer, "\033fg000000\033bgFFA500%s", node->error);
		
		InsertListItem (tm->_panel_id, TEMP_PNL_ERRORBOX, 0, buffer, 0);
	}

	ProcessDrawEvents();
}

int temperature_monitor_check_for_error (TemperatureMonitor* tm, char *string)
{
	int i;
	TempError *node = NULL;

	for(i=1; i <= ListNumItems(tm->_error_list); i++) {
		node = ListGetPtrToItem(tm->_error_list, i);

		if (strncmp(string, node->error, strlen(string) - 1)==0)
			return 1;
	}

	return 0;
}

int CVICALLBACK OnTemperatureMonitorTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			TemperatureMonitor *tm = (TemperatureMonitor*) callbackData;
			double temp1 = 0.0, temp2 = 0.0, temp3 = 0.0, temp4 = 0.0;

			// Give derived objects chance to set errors
			if(tm->lpVtbl.set_errors != NULL) {

				CALL_TEMPERATURE_MONITOR_VTABLE_PTR(tm, set_errors);
			}

			temperature_monitor_read_temperatures (tm, &temp1, &temp2, &temp3, &temp4);

			SetCtrlVal(tm->_panel_id, TEMP_PNL_TEMP1, temp1);
			SetCtrlVal(tm->_panel_id, TEMP_PNL_TEMP2, temp2);
			SetCtrlVal(tm->_panel_id, TEMP_PNL_TEMP3, temp3);
			SetCtrlVal(tm->_panel_id, TEMP_PNL_TEMP4, temp4);

			temperature_monitor_display_errors (tm);
		}
	}

    return 0;
}

static int CVICALLBACK OnTemperatureMonitor_ClosePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			TemperatureMonitor *tm = (TemperatureMonitor*) callbackData;  

			temperature_monitor_disable_timer (tm);

			ui_module_hide_main_panel(UIMODULE_CAST(tm));

			break;
		}
	}
	
	return 0;
}


static int CVICALLBACK OnTemperatureMonitor_SetupPressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			TemperatureMonitor *tm = (TemperatureMonitor*) callbackData;    

			ui_module_display_panel(UIMODULE_CAST(tm), tm->_setup_panel_id);
		
			break;
		}
	}
	return 0;
}

int temperature_monitor_hardware_initialise (TemperatureMonitor* tm)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_TEMPERATURE_MONITOR_VTABLE_PTR(tm, hw_initialise) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( TEMPERATURE_MONITOR_VTABLE(tm, hw_initialise)(tm) == TEMPERATURE_MONITOR_ERROR ) {
			status = send_temperature_monitor_error_text(tm, "Z Drive hardware initialisation failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				tm->_hw_initialised = 0;   
				return TEMPERATURE_MONITOR_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
	tm->_hw_initialised = 1;
	
  	return TEMPERATURE_MONITOR_SUCCESS;
}

int temperature_monitor_initialise (TemperatureMonitor* tm)
{	
	// Call specific device initialisation if necessary.
	if( (tm->lpVtbl.initialise != NULL)) {
		
		if( (tm->lpVtbl.initialise)(tm) == TEMPERATURE_MONITOR_ERROR )
			return TEMPERATURE_MONITOR_ERROR;  	
	}
	
	tm->_initialised = 1;
	
	//temperature_monitor_enable_timer(tm);

  	return TEMPERATURE_MONITOR_SUCCESS;
}

int temperature_monitor_is_initialised(TemperatureMonitor* tm)
{
	return (tm->_initialised && tm->_hw_initialised);	
}

void temperature_monitor_set_setup_panel(TemperatureMonitor *tm, int panel_id)
{
	tm->_setup_panel_id = panel_id;
}

static int TEMP_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (TemperatureMonitor*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (TemperatureMonitor *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int temperature_monitor_signal_enclosure_state_handler_connect (TemperatureMonitor *tm, TEMPERATURE_MONITOR_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(tm), "TemperatureStateChanged", handler, callback_data) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(tm),  "TemperatureMonitor", "Can not connect signal handler for TemperatureError signal");
		return TEMPERATURE_MONITOR_ERROR;
	}

	return TEMPERATURE_MONITOR_SUCCESS;
}


void temperature_monitor_constructor(TemperatureMonitor* tm, const char *name, const char *description, const char *data_dir)
{
	tm->_initialised = 0;  
	tm->_hw_initialised = 0;
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(tm), name);
//	ui_module_constructor(UIMODULE_CAST(tm), name);       
	ui_module_set_description(UIMODULE_CAST(tm), description);
	ui_module_set_data_dir(UIMODULE_CAST(tm), data_dir);
	
	tm->_error_list = ListCreate (sizeof(TempError));	// Holds error strings of lenth 200	

	TEMPERATURE_MONITOR_VTABLE_PTR(tm, hw_initialise) = NULL;   
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, initialise) = NULL;  
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, destroy) = NULL; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, read_temperatures) = NULL; 

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(tm), "TemperatureStateChanged", TEMP_PTR_MARSHALLER); 

    tm->_panel_id = ui_module_add_panel(UIMODULE_CAST(tm), "TemperatureMonitorUI.uir", TEMP_PNL, 1); 

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TimerInterval", &(tm->_timer_interval))<0)
		tm->_timer_interval=2.0;

	#ifdef SINGLE_THREADED_POLLING
	
	tm->_timer = NewCtrl(tm->_panel_id, CTRL_TIMER, "", 0, 0);
		
	if ( InstallCtrlCallback (tm->_panel_id, tm->_timer, OnTemperatureMonitorTimerTick, tm) < 0)
		return TEMPERATURE_MONITOR_ERROR; 
		
	SetCtrlAttribute(tm->_panel_id, tm->_timer, ATTR_INTERVAL, tm->_timer_interval);  
	SetCtrlAttribute(tm->_panel_id, tm->_timer, ATTR_ENABLED, 0);
	
	#else
	
	tm->_timer = NewAsyncTimer (tm->_timer_interval, -1, 1, OnTemperatureMonitorTimerTick, tm);
	SetAsyncTimerAttribute (tm->_timer, ASYNC_ATTR_ENABLED,  0);
	
	#endif

	InstallCtrlCallback (tm->_panel_id, TEMP_PNL_SETUP, OnTemperatureMonitor_SetupPressed, tm);
	InstallCtrlCallback (tm->_panel_id, TEMP_PNL_CLOSE, OnTemperatureMonitor_ClosePressed, tm);

	ui_module_set_main_panel_title (UIMODULE_CAST(tm));     
	// Panel is small so set an additional title
	SetCtrlVal(tm->_panel_id, TEMP_PNL_TITLE, description);
}


int temperature_monitor_destroy(TemperatureMonitor* tm)
{
	temperature_monitor_disable_timer(tm);

	CHECK_TEMPERATURE_MONITOR_VTABLE_PTR(tm, destroy) 
  	
	CALL_TEMPERATURE_MONITOR_VTABLE_PTR(tm, destroy) 

	ListDispose(tm->_error_list);

	ui_module_destroy(UIMODULE_CAST(tm));  
  	
  	free(tm);
  	
  	return TEMPERATURE_MONITOR_SUCCESS;
}

void temperature_monitor_set_error_handler(TemperatureMonitor* tm, UI_MODULE_ERROR_HANDLER handler, void *callback_data)
{
	ui_module_set_error_handler(UIMODULE_CAST(tm), handler, callback_data);	

}

void temperature_monitor_disable_timer(TemperatureMonitor* tm)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute(tm->_panel_id, tm->_timer, ATTR_ENABLED, 0);
	#else
	SetAsyncTimerAttribute (tm->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void temperature_monitor_enable_timer(TemperatureMonitor* tm)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute(tm->_panel_id, tm->_timer, ATTR_ENABLED, 1);
	#else
	SetAsyncTimerAttribute (tm->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}
