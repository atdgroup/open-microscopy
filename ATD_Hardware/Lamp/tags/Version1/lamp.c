#include <utility.h>

#include "lamp.h"
#include "lamp_ui.h"
#include "string_utils.h"
#include "gci_utils.h"

#include "GL_CVIRegistry.h"
#include "toolbox.h"

#include "asynctmr.h"

#include <ansi_c.h> 

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Brightness control.
////////////////////////////////////////////////////////////////////////////

int send_lamp_error_text (Lamp* lamp, char fmt[], ...)
{
	char *buf, *buffer_start;
	char tmp_buf[256];
	
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;
	
	if(lamp == NULL || lamp->_error_handler == NULL)
		return LAMP_ERROR;
	
	buffer_start = (char*) malloc(1024);
	buf = buffer_start;
	
	va_start(ap, fmt);
	
	for (p = fmt; *p; p++) {
	
		if (*p != '%') {
			*buf++ = *p;
			continue;
		}
		
		*buf = '\0';
		
		switch (*++p) {
			case 'd':
			case 'i':
				ival = va_arg(ap, int);
				sprintf(tmp_buf, "%d", ival);
				strcat(buf, tmp_buf);
				buf+=strlen(tmp_buf);
				break;
				
			case 'x':
				ival = va_arg(ap, int);
				sprintf(tmp_buf, "%x", ival);
				strcat(buf, tmp_buf);
				buf+=strlen(tmp_buf);
				break;
				
			case 'f':
				dval = va_arg(ap, double);
				sprintf(tmp_buf, "%f", dval);
				strcat(buf, tmp_buf);
				buf+=strlen(tmp_buf);
				break;
				
			case 's':
				sval = va_arg(ap, char *);
				strcat(buf, sval);
				buf+=strlen(sval);
				break;
				
			default:
				*buf++ = *p;
				break;
		}
		
	}
	
	*buf = '\0';
	va_end(ap);
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	lamp->_error_handler(buffer_start, lamp);
	free(buffer_start);
	
	return LAMP_SUCCESS;
}


static void error_handler (char *error_string, Lamp *lamp)
{
	MessagePopup("Lamp Error", error_string); 
}


static void lamp_read_or_write_main_panel_registry_settings(Lamp *lamp, int write)
{
	char buffer[500];
	int visible;

	// load or save panel positions
	
	if(lamp == NULL || lamp->_main_ui_panel == -1)
		return;

	// make sure the panel is visible and not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {
		GetPanelAttribute (lamp->_main_ui_panel, ATTR_VISIBLE, &visible);
		if(!visible) return;

		SetPanelAttribute (lamp->_main_ui_panel, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	}
	
	sprintf(buffer, "software\\GCI\\Microscope\\Lamp\\%s\\MainPanel\\", lamp->_name);
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer,  "top",    lamp->_main_ui_panel, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer,  "left",   lamp->_main_ui_panel, ATTR_LEFT);
}


static int LAMP_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Lamp*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Lamp *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


void lamp_on_change(Lamp* lamp)
{
	int status;
	double intensity;
	static int prev_status;
	static double prev_intensity;
	
	CmtGetLock (lamp->_lock);
			
	if (lamp_status(lamp, &status) == LAMP_SUCCESS) {
		SetCtrlVal (lamp->_main_ui_panel, LAMP_PNL_ONOFF, status);
		SetCtrlVal (lamp->_main_ui_panel, LAMP_PNL_LED, status);
		SetCtrlAttribute (lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_DIMMED, !status);
	}
			
	if (lamp_get_intensity(lamp, &intensity) == LAMP_SUCCESS) 
		SetCtrlVal (lamp->_main_ui_panel, LAMP_PNL_INTENSITY, intensity);

    if ((status != prev_status) || (intensity != prev_intensity)) 
		GCI_Signal_Emit(&(lamp->signal_table), "LampChanged", GCI_VOID_POINTER, lamp);  
			
	prev_status = status;
	prev_intensity = intensity;
			
	CmtReleaseLock(lamp->_lock);
}

int CVICALLBACK OnLampTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Lamp *lamp = (Lamp *) callbackData;

    switch (event)
    {
        case EVENT_TIMER_TICK:
        
        	lamp_on_change(lamp);
            
            break;
    }
    
    return 0;
}


static int lamp_init (Lamp* lamp)
{
	lamp->_timer = -1;
	lamp->_i2c_port = 1;
	lamp->_min_intensity = 0.0;
  	lamp->_max_intensity = 0.0;
  	lamp->_intesity_increment = 0.0;
	
	GCI_SignalSystem_Create(&(lamp->signal_table), 10);
	
	GCI_Signal_New(&(lamp->signal_table), "Hide", LAMP_PTR_MARSHALLER);
	GCI_Signal_New(&(lamp->signal_table), "LampChanged", LAMP_PTR_MARSHALLER);

	lamp_set_error_handler(lamp, error_handler);

    lamp->_main_ui_panel = FindAndLoadUIR(0, "lamp_ui.uir", LAMP_PNL);  
 
    if ( InstallCtrlCallback (lamp->_main_ui_panel, LAMP_PNL_ONOFF, OnLampOnOffToggle, lamp) < 0)
		return LAMP_ERROR;
  	
  	if ( InstallCtrlCallback (lamp->_main_ui_panel, LAMP_PNL_INTENSITY, OnLampIntensity, lamp) < 0)
		return LAMP_ERROR;
		
	if ( InstallCtrlCallback (lamp->_main_ui_panel, LAMP_PNL_CLOSE, OnLampClose, lamp) < 0)
		return LAMP_ERROR;	
		
	SetPanelAttribute (lamp->_main_ui_panel, ATTR_TITLE, lamp->_description);
	
	#ifdef ENABLE_LAMP_STATUS_POLLING
	lamp->_timer = NewAsyncTimer (2.0, -1, 1, OnLampTimerTick, lamp);
	SetAsyncTimerAttribute (lamp->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
	
  	return LAMP_SUCCESS;
}


int lamp_signal_hide_handler_connect (Lamp* lamp, LAMP_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(&(lamp->signal_table), "Hide", handler, callback_data) == SIGNAL_ERROR) {
		send_lamp_error_text(lamp, "Can not connect signal handler for Lamp Hide signal");
		return LAMP_ERROR;
	}

	return LAMP_SUCCESS;
}


int lamp_changed_handler_connect(Lamp* lamp, LAMP_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(&(lamp->signal_table), "LampChanged", handler, data) == SIGNAL_ERROR) {
		send_lamp_error_text(lamp, "Can not connect signal handler for Lamp Change signal");
		return LAMP_ERROR;
	}

	return LAMP_SUCCESS;
}


Lamp* lamp_new(char *name, char *description, size_t size)
{
	Lamp* lamp = (Lamp*) malloc(size);
	
	lamp->lpVtbl = (LampVtbl *) malloc(sizeof(LampVtbl)); 
	
	LAMP_VTABLE_PTR(lamp, destroy) = NULL; 
	LAMP_VTABLE_PTR(lamp, lamp_off) = NULL; 
	LAMP_VTABLE_PTR(lamp, lamp_on) = NULL; 
	LAMP_VTABLE_PTR(lamp, lamp_status) = NULL;
	
	CmtNewLock (NULL, 0, &(lamp->_lock) );  
	
	lamp_set_description(lamp, description);
    lamp_set_name(lamp, name);
	
	lamp_init (lamp);
	
	return lamp;
}


int lamp_destroy(Lamp* lamp)
{
	lamp_disable_timer(lamp);
	
	lamp_on(lamp);	//Leave on so that it may be controlled manually
		
	lamp_read_or_write_main_panel_registry_settings(lamp, 1);

	CHECK_LAMP_VTABLE_PTR(lamp, destroy) 
  	
	CALL_LAMP_VTABLE_PTR(lamp, destroy) 
	
	DiscardPanel(lamp->_main_ui_panel);

	lamp->_main_ui_panel = -1;

	if(lamp->_description != NULL) {
	
  		free(lamp->_description);
  		lamp->_description = NULL;
  	}
  	
  	if(lamp->_name != NULL) {
  	
  		free(lamp->_name);
  		lamp->_name = NULL;
  	}
  	
  	CmtDiscardLock (lamp->_lock);
  	
  	free(lamp->lpVtbl);
  	free(lamp);
  	
  	return LAMP_SUCCESS;
}


int lamp_set_i2c_port(Lamp* lamp, int port)
{
	lamp->_i2c_port = port;
	
	return LAMP_SUCCESS;  
}


int lamp_set_intensity_range(Lamp *lamp, double min, double max, double increment)
{
	CHECK_LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range)

	if( (*lamp->lpVtbl->lamp_set_intensity_range)(lamp, min, max, increment) == LAMP_ERROR ) {
		send_lamp_error_text(lamp, "lamp_set_intensity_range failed");
		return LAMP_ERROR;
	}

	lamp->_min_intensity = min;
  	lamp->_max_intensity = max;
  	lamp->_intesity_increment = increment;
	
	//Want to display zero volts when the lamp is off
	SetCtrlAttribute(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_MIN_VALUE, 0.0);
	SetCtrlAttribute(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_MAX_VALUE, max);
	SetCtrlAttribute(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_INCR_VALUE, increment);
	
	SetCtrlAttribute(lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_DIMMED, 0); 
	
	return LAMP_SUCCESS;
}

int lamp_get_intensity_range(Lamp *lamp, double *min, double *max)
{
	if (FP_Compare(lamp->_max_intensity, 0.0) == 0) return LAMP_ERROR;
	
	*min = lamp->_min_intensity;
	*max = lamp->_max_intensity;
	return LAMP_SUCCESS;
}

void lamp_set_error_handler(Lamp* lamp, void (*handler) (char *error_string, Lamp *lamp) )
{
	lamp->_error_handler = handler;
}


int  lamp_set_description(Lamp* lamp, const char* description)
{
  	lamp->_description = (char *) malloc(strlen(description) + 1);

  	if(lamp->_description != NULL) {
    	strcpy(lamp->_description, description);
  	}
  	
  	return LAMP_SUCCESS;
}


int  lamp_get_description(Lamp* lamp, char *description)
{
  	if(lamp->_description != NULL) {
    
    	strcpy(description, lamp->_description);
    
    	return LAMP_SUCCESS;
  	}
  
  	return LAMP_ERROR;
}


int lamp_set_name(Lamp* lamp, char* name)
{
  	lamp->_name = (char *)malloc(strlen(name) + 1);

  	if(lamp->_name != NULL) {
    	strcpy(lamp->_name, name);
  	}
  	
  	return LAMP_SUCCESS;
}


int lamp_get_name(Lamp* lamp, char *name)
{
  	if(lamp->_name != NULL) {
    
    	strcpy(name, lamp->_name);
    
    	return LAMP_SUCCESS;
  	}
  
  	return LAMP_ERROR;
}


int lamp_off(Lamp* lamp)
{
	CHECK_LAMP_VTABLE_PTR(lamp, lamp_off) 
  	
	CALL_LAMP_VTABLE_PTR(lamp, lamp_off)

  	return LAMP_SUCCESS;
}


int lamp_on(Lamp* lamp)
{
  	CHECK_LAMP_VTABLE_PTR(lamp, lamp_on) 
  	
	CALL_LAMP_VTABLE_PTR(lamp, lamp_on)

  	return LAMP_SUCCESS;
}


int lamp_status(Lamp* lamp, int *status)
{
  	CHECK_LAMP_VTABLE_PTR(lamp, lamp_status) 

	if( (*lamp->lpVtbl->lamp_status)(lamp, status) == LAMP_ERROR ) {
		send_lamp_error_text(lamp, "lamp_status failed");
		return LAMP_ERROR;
	}

  	return LAMP_SUCCESS;
}

int lamp_set_intensity(Lamp* lamp, double intensity)
{
	int status;
	
	//Don't attempt to change the intensity if the lamp is off
	if (lamp_status(lamp, &status) == LAMP_ERROR )
		return LAMP_ERROR;
	if (status == 0)
		return LAMP_SUCCESS;
	
  	CHECK_LAMP_VTABLE_PTR(lamp, lamp_set_intensity) 

	if( (*lamp->lpVtbl->lamp_set_intensity)(lamp, intensity) == LAMP_ERROR ) {
		send_lamp_error_text(lamp, "lamp_set_intensity failed");
		return LAMP_ERROR;
	}

  	return LAMP_SUCCESS;
}

int lamp_get_intensity(Lamp* lamp, double *intensity)
{
  	CHECK_LAMP_VTABLE_PTR(lamp, lamp_get_intensity) 

	if( (*lamp->lpVtbl->lamp_get_intensity)(lamp, intensity) == LAMP_ERROR ) {
		send_lamp_error_text(lamp, "lamp_get_intensity failed");
		return LAMP_ERROR;
	}

  	return LAMP_SUCCESS;
}

void lamp_disable_timer(Lamp* lamp)
{
	#ifdef ENABLE_LAMP_STATUS_POLLING
	SetAsyncTimerAttribute (lamp->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void lamp_enable_timer(Lamp* lamp)
{
	#ifdef ENABLE_LAMP_STATUS_POLLING  
	SetAsyncTimerAttribute (lamp->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}

int lamp_display_main_ui(Lamp* lamp)
{
	if(lamp->_main_ui_panel != -1) {
		lamp_read_or_write_main_panel_registry_settings(lamp, 0); 
		DisplayPanel(lamp->_main_ui_panel);
		lamp_enable_timer(lamp);
	}
	
	return LAMP_SUCCESS;
}


int lamp_hide_main_ui(Lamp* lamp)
{
	if(lamp->_main_ui_panel != -1) {
		lamp_read_or_write_main_panel_registry_settings(lamp, 1);
		HidePanel(lamp->_main_ui_panel);
	}

	GCI_Signal_Emit(&(lamp->signal_table), "Hide", GCI_VOID_POINTER, lamp); 

	return LAMP_SUCCESS;
}


int lamp_is_main_ui_visible(Lamp* lamp)
{
	int visible;
	
	GetPanelAttribute(lamp->_main_ui_panel, ATTR_VISIBLE, &visible);
	
	return visible;
}
