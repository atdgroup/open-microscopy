#include "lamp.h"
#include "string_utils.h"
#include "gci_utils.h"

#include "toolbox.h"

#include <ansi_c.h>

#ifndef SINGLE_THREADED_POLLING
#include "asynctmr.h"
#endif

#include <utility.h>

// Generic interface for all kinds on lamp / led etc
static int LAMP_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Lamp*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Lamp *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int CVICALLBACK OnLampTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			Lamp *lamp = (Lamp *) callbackData;
   
        	LampStatus status;
	        double intensity;
	        static int prev_status;
	        static double prev_intensity;
		
			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

	        if(lamp_off_on_status(lamp, &status) == LAMP_ERROR)
                goto Error;

            if (lamp_get_intensity(lamp, &intensity) == LAMP_ERROR)
                goto Error;
 
            // If status has changed emit a signal
            if(prev_status != status || prev_intensity != intensity) {

                GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
            }

		    prev_status = status;
	        prev_intensity = intensity;
	   
            break;    
        }
    }
    
	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

	return 0;

Error:

	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

	return -1;
}


int lamp_set_main_panel (Lamp* lamp, int panel_id)
{
    lamp->_main_ui_panel = panel_id;
		
    SetPanelAttribute (lamp->_main_ui_panel, ATTR_TITLE, UIMODULE_GET_DESCRIPTION(lamp));
	
    #ifdef SINGLE_THREADED_POLLING
    lamp->_timer = NewCtrl(lamp->_main_ui_panel, CTRL_TIMER, "", 0, 0);
		
	if ( InstallCtrlCallback (lamp->_main_ui_panel, lamp->_timer, OnLampTimerTick, lamp) < 0)
        return LAMP_ERROR;	
		
	SetCtrlAttribute(lamp->_main_ui_panel, lamp->_timer, ATTR_INTERVAL, 5.0);  
	SetCtrlAttribute(lamp->_main_ui_panel, lamp->_timer, ATTR_ENABLED, 0);

    #else
	lamp->_timer = NewAsyncTimer (5.0, -1, 1, OnLampTimerTick, lamp);
	SetAsyncTimerName(lamp->_timer, "Lamp");
	SetAsyncTimerAttribute (lamp->_timer, ASYNC_ATTR_ENABLED,  0);
    #endif
    
  	return LAMP_SUCCESS;
}


int lamp_changed_handler_connect(Lamp* lamp, LAMP_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", handler, data) == SIGNAL_ERROR) {
		ui_module_send_error (UIMODULE_CAST(lamp), UIMODULE_GET_DESCRIPTION(lamp), "Can not connect signal handler for Lamp Change signal"); 
		return LAMP_ERROR;
	}

	return LAMP_SUCCESS;
}


int lamp_constructor(Lamp *lamp, char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
    memset(lamp, 0, sizeof(Lamp));	

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(lamp), name);
	ui_module_set_description(UIMODULE_CAST(lamp), description);         
	ui_module_set_data_dir(UIMODULE_CAST(lamp), data_dir);
	
    lamp->_timer = -1;
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", LAMP_PTR_MARSHALLER);

	ui_module_set_error_handler(UIMODULE_CAST(lamp), handler, lamp); 
	
	return LAMP_SUCCESS;    
}


int lamp_destroy(Lamp* lamp)
{
	lamp_disable_timer(lamp);
		
    if(lamp->vtable.destroy != NULL)
	    CALL_LAMP_VTABLE_PTR(lamp, destroy) 
	
	#ifndef SINGLE_THREADED_POLLING   
	DiscardAsyncTimer(lamp->_timer);
	#endif

	ui_module_destroy(UIMODULE_CAST(lamp));

  	free(lamp);
  	
  	return LAMP_SUCCESS;
}


int lamp_set_intensity_range(Lamp *lamp, double min, double max, double increment)
{
	CHECK_LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range)

	if( (*lamp->vtable.lamp_set_intensity_range)(lamp, min, max, increment) == LAMP_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "lamp_set_intensity_range failed");
		return LAMP_ERROR;
	}

	if(lamp->_main_ui_panel > 0) {
////		SetCtrlAttribute(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_MIN_VALUE, min);
////		SetCtrlAttribute(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_MAX_VALUE, max);
////		SetCtrlAttribute(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_INCR_VALUE, increment);
	}
	
	return LAMP_SUCCESS;
}


void lamp_set_error_handler(Lamp* lamp, UI_MODULE_ERROR_HANDLER handler )
{
    ui_module_set_error_handler(UIMODULE_CAST(lamp), handler, lamp);                                 
}


int lamp_off(Lamp* lamp)
{
    int status = UIMODULE_ERROR_NONE;    
	
	logger_log(UIMODULE_LOGGER(lamp), LOGGER_INFORMATIONAL, "%s turned off", UIMODULE_GET_DESCRIPTION(lamp));

	CHECK_LAMP_VTABLE_PTR(lamp, lamp_off)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (lamp->vtable.lamp_off)(lamp) == LAMP_ERROR ) {
			status = ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "lamp_off failed");               
		
			if(status == UIMODULE_ERROR_IGNORE)
				return LAMP_ERROR;     ; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

  	return LAMP_SUCCESS;
}


int lamp_on(Lamp* lamp)
{
  	int status = UIMODULE_ERROR_NONE;    
//	double intensity = 0.0;
	
	logger_log(UIMODULE_LOGGER(lamp), LOGGER_INFORMATIONAL, "%s turned on", UIMODULE_GET_DESCRIPTION(lamp));

	CHECK_LAMP_VTABLE_PTR(lamp, lamp_on)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (lamp->vtable.lamp_on)(lamp) == LAMP_ERROR ) {
			status = ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "lamp_on failed");               
		
			if(status == UIMODULE_ERROR_IGNORE)
				return LAMP_ERROR;     ; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
  	return LAMP_SUCCESS;
}


int lamp_off_on_status(Lamp* lamp, LampStatus *lamp_status)
{
  	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_LAMP_VTABLE_PTR(lamp, lamp_off_on_status)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (lamp->vtable.lamp_off_on_status)(lamp, lamp_status) == LAMP_ERROR ) {
			status = ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "lamp_off_on_status failed");               
		
			if(status == UIMODULE_ERROR_IGNORE)
				return LAMP_ERROR;     ; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

  	return LAMP_SUCCESS;
}

int lamp_set_intensity(Lamp* lamp, double intensity)
{
    int status = UIMODULE_ERROR_NONE;    
	
	CHECK_LAMP_VTABLE_PTR(lamp, lamp_set_intensity)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (lamp->vtable.lamp_set_intensity)(lamp, intensity) == LAMP_ERROR ) {
			status = ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "lamp_set_intensity failed");               
		
			if(status == UIMODULE_ERROR_IGNORE)
				return LAMP_ERROR;     ; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	lamp_get_intensity(lamp, &intensity);
    
  	return LAMP_SUCCESS;
}

int lamp_get_intensity(Lamp* lamp, double *intensity)
{
    int status = UIMODULE_ERROR_NONE;    
	
	CHECK_LAMP_VTABLE_PTR(lamp, lamp_get_intensity)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (lamp->vtable.lamp_get_intensity)(lamp, intensity) == LAMP_ERROR ) {
			status = ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "lamp_get_intensity failed");               
		
			if(status == UIMODULE_ERROR_IGNORE)
				return LAMP_ERROR;     ; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

  	return LAMP_SUCCESS;
}

void lamp_disable_timer(Lamp* lamp)
{
	#ifdef SINGLE_THREADED_POLLING
    SetCtrlAttribute(lamp->_main_ui_panel, lamp->_timer, ATTR_ENABLED, 0);
    #else
	SetAsyncTimerAttribute (lamp->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void lamp_enable_timer(Lamp* lamp)
{
	#ifdef SINGLE_THREADED_POLLING
    	if(lamp->_main_ui_panel > 0)  	
			SetCtrlAttribute(lamp->_main_ui_panel, lamp->_timer, ATTR_ENABLED, 1);
    #else
	SetAsyncTimerAttribute (lamp->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}

int lamp_display_main_ui(Lamp* lamp)
{
	ui_module_display_main_panel(UIMODULE_CAST(lamp));
	
	return LAMP_SUCCESS;
}


int lamp_hide_main_ui(Lamp* lamp)
{
	ui_module_hide_main_panel(UIMODULE_CAST(lamp));

	return LAMP_SUCCESS;
}

int lamp_load_settings(Lamp *lamp, const char *filename)
{
	if(filename == NULL)
    	return LAMP_ERROR;

	CHECK_LAMP_VTABLE_PTR(lamp, load_settings);

  	if( (lamp->vtable.load_settings)(lamp, filename) == LAMP_ERROR)
  		return LAMP_ERROR;
	
	return LAMP_SUCCESS;
}


int lamp_save_settings(Lamp *lamp, const char *filename, const char *flags)
{
	if(filename == NULL)
    	return LAMP_ERROR;

	CHECK_LAMP_VTABLE_PTR(lamp, save_settings);

  	if((lamp->vtable.save_settings)(lamp, filename, flags) == LAMP_ERROR) {
  		ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "Can not save settings for device %s\n",
            UIMODULE_GET_DESCRIPTION(lamp));
  		return LAMP_ERROR;
  	}
	
	return LAMP_SUCCESS;
}


/*
int lamp_save_default_settings(Lamp *lamp)
{
	char path[GCI_MAX_PATHNAME_LEN];

    sprintf(path, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(lamp), UIMODULE_GET_NAME(lamp), DEFAULT_LAMP_FILENAME_SUFFIX);
	
	if(lamp_save_settings(lamp, path, "w") == LAMP_ERROR)
		return LAMP_ERROR;
		
	return LAMP_SUCCESS;
}


int lamp_load_default_settings(Lamp *lamp)
{
	char path[GCI_MAX_PATHNAME_LEN];

    sprintf(path, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(lamp), UIMODULE_GET_NAME(lamp), DEFAULT_LAMP_FILENAME_SUFFIX);
	
	if(lamp_load_settings(lamp, path) == LAMP_ERROR)
		return LAMP_ERROR;
		
	return LAMP_SUCCESS;
}
*/

int lamp_hardware_is_initialised(Lamp* lamp)
{
	if(lamp == 0)
		return 0;

	return lamp->_initialised;	
}


int lamp_hardware_initialise (Lamp* lamp)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_LAMP_VTABLE_PTR(lamp, init)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (lamp->vtable.hardware_init)(lamp) == LAMP_ERROR ) {
			status = ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "lamp_initialise failed");               
		
			if(status == UIMODULE_ERROR_IGNORE)
				return LAMP_ERROR;
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);

	lamp->_initialised = 1;

  	return LAMP_SUCCESS;
}

int lamp_initialise (Lamp* lamp)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_LAMP_VTABLE_PTR(lamp, init)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (lamp->vtable.init)(lamp) == LAMP_ERROR ) {
			status = ui_module_send_error(UIMODULE_CAST(lamp), "Lamp Error", "lamp_initialise failed");               
		
			if(status == UIMODULE_ERROR_IGNORE)
				return LAMP_ERROR;
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);

	lamp->_initialised = 1;

  	return LAMP_SUCCESS;
}