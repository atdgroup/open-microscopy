#include "shutter.h"
#include "shutterUI.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "iniparser.h"  

#include "GL_CVIRegistry.h"

#include <utility.h>
#include "toolbox.h"

#include "profile.h"

#include "asynctmr.h"

#include <ansi_c.h> 

int send_shutter_error_text (Shutter* shutter, char fmt[], ...)
{
	int ret=0;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(shutter), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(shutter), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(shutter), "Shutter Error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}

	
static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	GCI_MessagePopup("Shutter Error", error_string); 
	
	return UIMODULE_ERROR_NONE;    
}


static int SHUTTER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Shutter*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Shutter *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int SHUTTER_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Shutter*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Shutter *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


void shutter_emit_if_shutter_changed(Shutter* shutter)
{
	int status=0;
	static int prev_status;	
	
	if (shutter_status(shutter, &status) == SHUTTER_ERROR) 
		return;
	
   	if (status != prev_status) 
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(shutter), "ShutterChanged", GCI_VOID_POINTER, shutter, GCI_INT, status); 

	prev_status = status;
}


int CVICALLBACK OnShutterTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			Shutter *shutter = (Shutter *) callbackData;
	
			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

        	shutter_emit_if_shutter_changed(shutter);  
         	   
			break;
		}
    }
    
	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

    return 0;
}


int shutter_hardware_initialise (Shutter* shutter)
{
	int status = UIMODULE_ERROR_NONE;  
	
	shutter->_timer = -1;

	CHECK_SHUTTER_VTABLE_PTR(shutter, hw_init) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( SHUTTER_VTABLE(shutter, hw_init)(shutter) == SHUTTER_ERROR ) {
			status = send_shutter_error_text(shutter, "shutter initialisation failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				shutter->_initialised = 0;   
				return SHUTTER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
	shutter->_initialised = 1;
	
  	return SHUTTER_SUCCESS;
}

static void OnShutterPanelsClosedOrHidden (UIModule *module, void *data)
{
	Shutter* shutter = (Shutter*) data; 

	shutter_disable_timer(shutter);
}

static void OnShutterPanelDisplayed (UIModule *module, void *data)
{
	Shutter* shutter = (Shutter*) data; 

	shutter_enable_timer(shutter);
}

int shutter_initialise (Shutter* shutter)
{
	shutter->_timer = NewAsyncTimer (0.5, -1, 1, OnShutterTimerTick, shutter);
	SetAsyncTimerName(shutter->_timer, "Shutter");
	SetAsyncTimerAttribute (shutter->_timer, ASYNC_ATTR_ENABLED,  0);
	
	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(shutter), OnShutterPanelsClosedOrHidden, shutter);
	ui_module_main_panel_show_mainpanel_handler_connect (UIMODULE_CAST(shutter), OnShutterPanelDisplayed, shutter);

  	return SHUTTER_SUCCESS;
}

int shutter_is_initialised(Shutter *shutter)
{
	return shutter->_initialised;	
}

int shutter_changed_handler_connect(Shutter* shutter, SHUTTER_CHANGE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(shutter), "ShutterChanged", handler, data) == SIGNAL_ERROR) {
		send_shutter_error_text(shutter, "Can not connect signal handler for Shutter Change signal");
		return SHUTTER_ERROR;
	}

	return SHUTTER_SUCCESS;
}


int CVICALLBACK OnShutterOpen (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Shutter *shutter = (Shutter *) callbackData;
	
			int fb;
			
			shutter_set_open_time(shutter, 0);
			shutter_open(shutter);
			
			Delay(0.02);
			
			// update shutter status
			shutter_status(shutter, &fb);   
			
			ProcessSystemEvents();       
			
			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnShutterClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Shutter *shutter = (Shutter *) callbackData;
	
			int fb;
			
			shutter_close(shutter);      
	
			Delay(0.02);
	
			// update shutter status
			shutter_status(shutter, &fb);   
			
			ProcessSystemEvents();
			
			break;
		}
	}
	return 0;
}

int CVICALLBACK OnShutterInhitbit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			Shutter *shutter = (Shutter *) callbackData;
	
			int val = 0;
			
			GetCtrlVal(panel, control, &val);
			
			shutter_inhibit(shutter, val);   
				
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnShutterQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			Shutter *shutter = (Shutter *) callbackData;
	
			ui_module_hide_all_panels(UIMODULE_CAST(shutter));  
				
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnShutterAutomaticControl (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			Shutter *shutter = (Shutter *) callbackData;
	
			int val = 0;
			
			GetCtrlVal(panel, control, &val);
			
			shutter_set_computer_control(shutter, val); 
				
			break;
		}
	}
	
	return 0;
}

void shutter_constructor(Shutter* shutter, const char *name, const char *description)
{
	shutter->_initialised = 0;  
	shutter->_last_close_time = -1.0;

	shutter_reset_previous_open_close_experience(shutter);

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(shutter), name); 
	ui_module_set_description(UIMODULE_CAST(shutter), description);       
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(shutter), "ShutterChanged", SHUTTER_PTR_INT_MARSHALLER);   
	
	SHUTTER_VTABLE_PTR(shutter, hw_init) = NULL;   
	SHUTTER_VTABLE_PTR(shutter, destroy) = NULL; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = NULL; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = NULL; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_inhibit) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_is_inhibited) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_computer_control) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_info) = NULL;
	
    shutter->_panel_id = ui_module_add_panel(UIMODULE_CAST(shutter), "ShutterUI.uir", SHUTTER, 1); 
	
    InstallCtrlCallback (shutter->_panel_id, SHUTTER_OPEN, OnShutterOpen, shutter);
    InstallCtrlCallback (shutter->_panel_id, SHUTTER_CLOSE, OnShutterClose, shutter);
  	InstallCtrlCallback (shutter->_panel_id, SHUTTER_INHIBIT, OnShutterInhitbit, shutter);
	InstallCtrlCallback (shutter->_panel_id, SHUTTER_COMP_CTRL, OnShutterAutomaticControl, shutter);
	InstallCtrlCallback (shutter->_panel_id, SHUTTER_QUIT, OnShutterQuit, shutter);

	ui_module_set_main_panel_title (UIMODULE_CAST(shutter));
}


int shutter_destroy(Shutter* shutter)
{
	shutter_disable_timer(shutter);

	DiscardAsyncTimer(shutter->_timer);

	shutter_close(shutter);		 // Leave closed
	shutter_inhibit(shutter, 0); // Leave not inhidited	
	shutter_set_computer_control(shutter, 0);	// Leave under manual control
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, destroy) 
  	
	CALL_SHUTTER_VTABLE_PTR(shutter, destroy) 
	
	ui_module_destroy(UIMODULE_CAST(shutter));  
  	
  	free(shutter);
  	
  	return SHUTTER_SUCCESS;
}


void shutter_set_error_handler(Shutter* shutter, UI_MODULE_ERROR_HANDLER handler, void *callback_data)
{
	ui_module_set_error_handler(UIMODULE_CAST(shutter), handler, callback_data);	

}
 
static int CVICALLBACK trigger_fault(void *callback)
{
	Shutter* shutter = (Shutter*) callback;
	int status=0;
	double delay = 0.020; // the time in which we expect the shutter to open 
	
	shutter_is_inhibited (shutter, &status);
	if (status) return 0;
	
	Delay(delay);
	shutter_status(shutter, &status);

	if(status==0) {
		send_shutter_error_text(shutter, "shutter failed to open in %f ms", delay*1000.0);       
	}
	
	return 0;
}

int shutter_open(Shutter* shutter)
{
	const int max_errors = 5;
	static int ring_buffer_index = 0;
	int status = UIMODULE_ERROR_NONE, inhibit, open_status;
	double open_time;  
	
	if(!shutter->_enabled)
	{
		logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s attempted to open when not under computer control.", UIMODULE_GET_DESCRIPTION(shutter));	
		return SHUTTER_SUCCESS;
	}
	
	shutter_is_inhibited(shutter, &inhibit);
	shutter_get_open_time(shutter, &open_time);

	shutter_status(shutter, &open_status);

	// Already Opened
	if(open_status == 1)
		return SHUTTER_SUCCESS;

	if (!inhibit) 
	{	
		if (open_time==0.0)
			logger_log(UIMODULE_LOGGER(shutter), LOGGER_INFORMATIONAL, "%s Opened", UIMODULE_GET_DESCRIPTION(shutter));
		else
			logger_log(UIMODULE_LOGGER(shutter), LOGGER_INFORMATIONAL, "%s Opened for %.1f ms", UIMODULE_GET_DESCRIPTION(shutter), open_time);
	}
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_open)     
		
	do {
		status = UIMODULE_ERROR_NONE;

		if( SHUTTER_VTABLE(shutter, shutter_open)(shutter) == SHUTTER_ERROR ) {
			
			if(hardware_device_get_number_of_consecutive_errors(HARDWARE_DEVICE_CAST(shutter)) >= max_errors) {
				return SHUTTER_ERROR;
			}

			status = send_shutter_error_text(shutter, "shutter open failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	if(shutter->_last_close_time < 0.0)
		return SHUTTER_SUCCESS;

	if(ring_buffer_index >= SHUTTER_CLOSE_OPEN_ARRAY_SIZE - 1)
		ring_buffer_index = 0;

	shutter->_time_until_open[ring_buffer_index++] = Timer() - shutter->_last_close_time;

	shutter_emit_if_shutter_changed(shutter);  

	hardware_device_reset_consecutive_errors(HARDWARE_DEVICE_CAST(shutter));

  	return SHUTTER_SUCCESS;
}

int shutter_reset_previous_open_close_experience(Shutter *shutter)
{
    int i;
    
	//Set elements to threshold so the intelignet shutter should close by default
    for(i=0; i < SHUTTER_CLOSE_OPEN_ARRAY_SIZE; i++) {
      shutter->_time_until_open[i] = SHUTTER_SHOULD_CLOSE_THRESHOLD;
    }

	return SHUTTER_SUCCESS;
}

int shutter_intelligent_close(Shutter *shutter)
{
	int i = 0, should_close = 0;

	// check the last times from close to open are less than the threshold value
	for(i=0; i < SHUTTER_CLOSE_OPEN_ARRAY_SIZE - 1; i++) {
		if(shutter->_time_until_open[i] > SHUTTER_SHOULD_CLOSE_THRESHOLD) {
			should_close = 1;
			break;
		}
	}
		
	// Judging from the last 10 times the shutter has opened we are expecting an open very soon
	// so lets not close the shutter
	if(should_close)
		return shutter_close(shutter);

	return SHUTTER_SUCCESS;
}

int shutter_close(Shutter* shutter)
{
	int status = UIMODULE_ERROR_NONE, inhibit, open_status;  
	double open_time;  
	
	if(!shutter->_enabled)
	{
		logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s attempted to close when not under computer control.", UIMODULE_GET_DESCRIPTION(shutter));	
		return SHUTTER_SUCCESS;
	}
	
	shutter_is_inhibited(shutter, &inhibit);
	shutter_get_open_time(shutter, &open_time);

	shutter_status(shutter, &open_status);

	// Already Closed
	if(open_status == 0)
		return SHUTTER_SUCCESS;

	if (!inhibit && open_time==0.0)
		logger_log(UIMODULE_LOGGER(shutter), LOGGER_INFORMATIONAL, "%s Closed", UIMODULE_GET_DESCRIPTION(shutter));
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_close)    
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( SHUTTER_VTABLE(shutter, shutter_close)(shutter) == SHUTTER_ERROR ) {
			status = send_shutter_error_text(shutter, "shutter close failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	shutter->_last_close_time = Timer();

	shutter_emit_if_shutter_changed(shutter);  
	
  	return SHUTTER_SUCCESS;
}

int shutter_status(Shutter* shutter, int *status)
{
	const int max_errors = 5;

	int err_status = UIMODULE_ERROR_NONE;  
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_status)  
		
	do {
		err_status = UIMODULE_ERROR_NONE;
		
		if(hardware_device_get_number_of_consecutive_errors(HARDWARE_DEVICE_CAST(shutter)) >= max_errors) {
			return SHUTTER_ERROR;
		}

		if( SHUTTER_VTABLE(shutter, shutter_status)(shutter, status) == SHUTTER_ERROR ) {
			err_status = send_shutter_error_text(shutter, "shutter_status failed");
		
			hardware_device_increment_consecutive_errors(HARDWARE_DEVICE_CAST(shutter));

			if(err_status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
		
	} 
	while(err_status == UIMODULE_ERROR_RETRY);
	
	SetCtrlVal(shutter->_panel_id, SHUTTER_FB, *status);  
	
	hardware_device_reset_consecutive_errors(HARDWARE_DEVICE_CAST(shutter));

  	return SHUTTER_SUCCESS;
}

int shutter_set_open_time(Shutter* shutter, double open_time)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time)  
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( SHUTTER_VTABLE(shutter, shutter_set_open_time)(shutter, open_time) == SHUTTER_ERROR ) {
			status = send_shutter_error_text(shutter, "shutter_set_open_time failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
  	return SHUTTER_SUCCESS;
}

int shutter_set_computer_control(Shutter* shutter, int enable)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_set_computer_control)  
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( SHUTTER_VTABLE(shutter, shutter_set_computer_control)(shutter, enable) == SHUTTER_ERROR ) {
			status = send_shutter_error_text(shutter, "shutter_set_computer_control failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	SetCtrlVal(shutter->_panel_id, SHUTTER_COMP_CTRL, enable);
	shutter->_enabled = enable;
	logger_log(UIMODULE_LOGGER(shutter), LOGGER_INFORMATIONAL, "%s computer control set to %d", UIMODULE_GET_DESCRIPTION(shutter), enable);	
	
  	return SHUTTER_SUCCESS;
}

int shutter_get_open_time(Shutter* shutter, double *open_time)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time)  
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( SHUTTER_VTABLE(shutter, shutter_get_open_time)(shutter, open_time) == SHUTTER_ERROR ) {
			status = send_shutter_error_text(shutter, "shutter_get_open_time failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

  	return SHUTTER_SUCCESS;
}

int shutter_is_inhibited(Shutter* shutter, int *inhibit)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_is_inhibited)  
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( SHUTTER_VTABLE(shutter, shutter_is_inhibited)(shutter, inhibit) == SHUTTER_ERROR ) {
			status = send_shutter_error_text(shutter, "shutter_is_inhibited failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

  	return SHUTTER_SUCCESS;
}


int shutter_inhibit(Shutter* shutter, int inhibit)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_inhibit)  
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( SHUTTER_VTABLE(shutter, shutter_inhibit)(shutter, inhibit) == SHUTTER_ERROR ) {
			status = send_shutter_error_text(shutter, "shutter_inhibit failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	SetCtrlVal(shutter->_panel_id, SHUTTER_INHIBIT, inhibit);  
	logger_log(UIMODULE_LOGGER(shutter), LOGGER_INFORMATIONAL, "%s inhibit set to %d", UIMODULE_GET_DESCRIPTION(shutter), inhibit);	
 
	return SHUTTER_SUCCESS;	
}

int  shutter_get_info (Shutter* shutter, char* info)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_inhibit)  
	
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( SHUTTER_VTABLE(shutter, shutter_get_info)(shutter, info) == SHUTTER_ERROR ) {
			status = send_shutter_error_text(shutter, "shutter_get_info failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return SHUTTER_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	return SHUTTER_SUCCESS;	
}

int  shutter_save_settings_in_ini_fmt (Shutter* shutter, const char *filepath, const char *mode)
{
	int status, inhibited;
	double open_time;
	char buffer[256];
	
	shutter_is_inhibited(shutter, &inhibited);
	shutter_status(shutter, &status);
	shutter_get_open_time (shutter, &open_time);      
	
	sprintf(buffer, "Status=%d\nInhibit=%d\nEnabled=%d\nOpenTime=%.2f\n\n",
		status, inhibited, shutter->_enabled, open_time);
	
	str_change_char(buffer, '\n', '\0'); 
	
	if(!WritePrivateProfileSection(UIMODULE_GET_NAME(shutter), buffer, filepath))
		return SHUTTER_ERROR;	  
	
	return SHUTTER_SUCCESS;	  
}

int  shutter_load_settings_from_ini_fmt (Shutter* shutter, const char *filepath)
{
	int tmp;
	double dtmp;
	char buffer[64];

	dictionary* ini = iniparser_load(filepath);   

	sprintf (buffer, "%s:Status", UIMODULE_GET_NAME(shutter));
	tmp = iniparser_getint(ini, buffer, -1);
	
	PROFILE_START("shutter - shutter_open - shutter_close");

	if(tmp >= 0) {
		if(tmp) // Open 
			shutter_open(shutter);
		else
			shutter_close(shutter);
	}
		
	PROFILE_STOP("shutter - shutter_open - shutter_close");

	sprintf (buffer, "%s:Inhibit", UIMODULE_GET_NAME(shutter));
	tmp = iniparser_getint(ini, buffer, -1);
	
	PROFILE_START("shutter - shutter_inhibit");

	if(tmp >= 0)
		shutter_inhibit(shutter, tmp);

	PROFILE_STOP("shutter - shutter_inhibit");

	sprintf (buffer, "%s:OpenTime", UIMODULE_GET_NAME(shutter));
	dtmp = iniparser_getdouble(ini, buffer, 0);
	
	PROFILE_START("shutter - shutter_set_open_time");

	shutter_set_open_time(shutter, dtmp);

	PROFILE_STOP("shutter - shutter_set_open_time");

	sprintf (buffer, "%s:Enabled", UIMODULE_GET_NAME(shutter));
	tmp = iniparser_getint(ini, buffer, -1);
	
	PROFILE_START("shutter - shutter_set_computer_control");

	if(tmp >= 0)
		shutter_set_computer_control(shutter, tmp);
	
	PROFILE_STOP("shutter - shutter_set_computer_control");

	iniparser_freedict(ini);
	
	return SHUTTER_SUCCESS;		
}


void shutter_disable_timer(Shutter* shutter)
{
	SetAsyncTimerAttribute (shutter->_timer, ASYNC_ATTR_ENABLED,  0);
}

void shutter_enable_timer(Shutter* shutter)
{
	SetAsyncTimerAttribute (shutter->_timer, ASYNC_ATTR_ENABLED,  1);
}
