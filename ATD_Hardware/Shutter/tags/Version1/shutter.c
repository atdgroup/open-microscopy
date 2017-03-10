#include <utility.h>

#include "shutter.h"
#include "shutter_ui.h"
#include "string_utils.h"
#include "gci_utils.h"

#include "GL_CVIRegistry.h"
#include "toolbox.h"

#include "asynctmr.h"

#include <ansi_c.h> 

int send_shutter_error_text (Shutter* shutter, char fmt[], ...)
{
	char *buf, *buffer_start;
	char tmp_buf[256];
	
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;
	
	if(shutter == NULL || shutter->_error_handler == NULL)
		return SHUTTER_ERROR;
	
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
	shutter->_error_handler(buffer_start, shutter);
	free(buffer_start);
	
	return SHUTTER_SUCCESS;
}


static void error_handler (char *error_string, Shutter *shutter)
{
	MessagePopup("Shutter Error", error_string); 
}


static void shutter_read_or_write_main_panel_registry_settings(Shutter *shutter, int write)
{
	char buffer[500];
	int visible;

	// load or save panel positions
	
	if(shutter == NULL || shutter->_main_ui_panel == -1)
		return;

	// make sure the panel is visible and not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {
		GetPanelAttribute (shutter->_main_ui_panel, ATTR_VISIBLE, &visible);
		if(!visible) return;

		SetPanelAttribute (shutter->_main_ui_panel, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	}
	
	sprintf(buffer, "software\\GCI\\Microscope\\Shutter\\%s\\MainPanel\\", shutter->_name);
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer,  "top",    shutter->_main_ui_panel, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer,  "left",   shutter->_main_ui_panel, ATTR_LEFT);
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


void shutter_on_change(Shutter* shutter)
{
	int status;
	static int prev_status;	
	
	CmtGetLock (shutter->_lock);
			
	if (shutter_status(shutter, &status) == SHUTTER_SUCCESS) {
		SetCtrlVal (shutter->_main_ui_panel, SHUTTER_OPEN_LED, status);   //1 = on
		SetCtrlVal (shutter->_main_ui_panel, SHUTTER_CLOSED_LED, !status);
	}
			
    if (status != prev_status) 
		GCI_Signal_Emit(&(shutter->signal_table), "ShutterChanged", GCI_VOID_POINTER, shutter);  
			
	prev_status = status;
	shutter->_open = status;
			
	CmtReleaseLock(shutter->_lock);
}

void shutter_changed(Shutter* shutter, int status)
{
	SetCtrlVal (shutter->_main_ui_panel, SHUTTER_OPEN_LED, status);   //1 = open
	SetCtrlVal (shutter->_main_ui_panel, SHUTTER_CLOSED_LED, !status);
	shutter->_open = status;
	
	GCI_Signal_Emit(&(shutter->signal_table), "ShutterChanged", GCI_VOID_POINTER, shutter);  
}


int CVICALLBACK cbShutturStatusTimer (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Shutter *shutter = (Shutter *) callbackData;
	
    switch (event)
    {
        case EVENT_TIMER_TICK:
        
        	shutter_on_change(shutter); 
            
            break;
    }
    
    return 0;
}


static int shutter_init (Shutter* shutter)
{
	shutter->_timer = -1;
	shutter->_i2c_port = 1;
	shutter->_open_time = 1000;	//ms
	
	GCI_SignalSystem_Create(&(shutter->signal_table), 10);
	
	GCI_Signal_New(&(shutter->signal_table), "Hide", SHUTTER_PTR_MARSHALLER);
	GCI_Signal_New(&(shutter->signal_table), "ShutterChanged", SHUTTER_PTR_MARSHALLER);

	shutter_set_error_handler(shutter, error_handler);

    shutter->_main_ui_panel = FindAndLoadUIR(0, "shutter_ui.uir", SHUTTER);  
 
    if ( InstallCtrlCallback (shutter->_main_ui_panel, SHUTTER_OPEN, cbShutterOpenButton, shutter) < 0)
		return SHUTTER_ERROR;
  	
    if ( InstallCtrlCallback (shutter->_main_ui_panel, SHUTTER_CLOSE, cbShutterCloseButton, shutter) < 0)
		return SHUTTER_ERROR;
  	
  	if ( InstallCtrlCallback (shutter->_main_ui_panel, SHUTTER_TRIGGER, cbShutterTriggerButton, shutter) < 0)
		return SHUTTER_ERROR;
		
  	if ( InstallCtrlCallback (shutter->_main_ui_panel, SHUTTER_OPEN_TIME, cbShutterOpenTime, shutter) < 0)
		return SHUTTER_ERROR;
		
	if ( InstallCtrlCallback (shutter->_main_ui_panel, SHUTTER_EXIT, cbShutterClose, shutter) < 0)
		return SHUTTER_ERROR;	
		
	if ( InstallCtrlCallback (shutter->_main_ui_panel, SHUTTER_TEST, cbShutterTest, shutter) < 0)
		return SHUTTER_ERROR;	
		
	SetPanelAttribute (shutter->_main_ui_panel, ATTR_TITLE, shutter->_description);
	
	#ifdef ENABLE_SHUTTER_STATUS_POLLING 
	shutter->_timer = NewAsyncTimer (2.0, -1, 1, cbShutturStatusTimer, shutter);
	SetAsyncTimerAttribute (shutter->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
  	
  	return SHUTTER_SUCCESS;
}


int shutter_signal_hide_handler_connect (Shutter* shutter, SHUTTER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(&(shutter->signal_table), "Hide", handler, callback_data) == SIGNAL_ERROR) {
		send_shutter_error_text(shutter, "Can not connect signal handler for Shutter Hide signal");
		return SHUTTER_ERROR;
	}

	return SHUTTER_SUCCESS;
}


int shutter_changed_handler_connect(Shutter* shutter, SHUTTER_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(&(shutter->signal_table), "ShutterChanged", handler, data) == SIGNAL_ERROR) {
		send_shutter_error_text(shutter, "Can not connect signal handler for Shutter Change signal");
		return SHUTTER_ERROR;
	}

	return SHUTTER_SUCCESS;
}


Shutter* shutter_new(char *name, char *description, size_t size)
{
	Shutter* shutter = (Shutter*) malloc(size);
	
	shutter->lpVtbl = (ShutterVtbl *) malloc(sizeof(ShutterVtbl)); 
	
	SHUTTER_VTABLE_PTR(shutter, destroy) = NULL; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = NULL; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = NULL; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = NULL;
	SHUTTER_VTABLE_PTR(shutter, shutter_trigger) = NULL;
	
	CmtNewLock (NULL, 0, &(shutter->_lock) );  
	
	shutter_set_description(shutter, description);
    shutter_set_name(shutter, name);
	
	shutter_init (shutter);
	
	return shutter;
}


int shutter_destroy(Shutter* shutter)
{
	shutter_disable_timer(shutter);

	shutter_close(shutter);	//Leave closed
		
	shutter_read_or_write_main_panel_registry_settings(shutter, 1);

	CHECK_SHUTTER_VTABLE_PTR(shutter, destroy) 
  	
	CALL_SHUTTER_VTABLE_PTR(shutter, destroy) 
	
	DiscardPanel(shutter->_main_ui_panel);

	shutter->_main_ui_panel = -1;

	if(shutter->_description != NULL) {
	
  		free(shutter->_description);
  		shutter->_description = NULL;
  	}
  	
  	if(shutter->_name != NULL) {
  	
  		free(shutter->_name);
  		shutter->_name = NULL;
  	}
  	
  	CmtDiscardLock (shutter->_lock);
  	
  	free(shutter->lpVtbl);
  	free(shutter);
  	
  	return SHUTTER_SUCCESS;
}


int shutter_set_i2c_port(Shutter* shutter, int port)
{
	shutter->_i2c_port = port;
	
	return SHUTTER_SUCCESS;  
}


void shutter_set_error_handler(Shutter* shutter, void (*handler) (char *error_string, Shutter *shutter) )
{
	shutter->_error_handler = handler;
}


int  shutter_set_description(Shutter* shutter, const char* description)
{
  	shutter->_description = (char *) malloc(strlen(description) + 1);

  	if(shutter->_description != NULL) {
    	strcpy(shutter->_description, description);
  	}
  	
  	return SHUTTER_SUCCESS;
}


int  shutter_get_description(Shutter* shutter, char *description)
{
  	if(shutter->_description != NULL) {
    
    	strcpy(description, shutter->_description);
    
    	return SHUTTER_SUCCESS;
  	}
  
  	return SHUTTER_ERROR;
}


int shutter_set_name(Shutter* shutter, char* name)
{
  	shutter->_name = (char *)malloc(strlen(name) + 1);

  	if(shutter->_name != NULL) {
    	strcpy(shutter->_name, name);
  	}
  	
  	return SHUTTER_SUCCESS;
}


int shutter_get_name(Shutter* shutter, char *name)
{
  	if(shutter->_name != NULL) {
    
    		strcpy(name, shutter->_name);
    
    		return SHUTTER_SUCCESS;
  	}
  
  	return SHUTTER_ERROR;
}


int shutter_open(Shutter* shutter)
{
	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_open) 
  	
	return CALL_SHUTTER_VTABLE_PTR(shutter, shutter_open)
}


int shutter_close(Shutter* shutter)
{
  	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_close) 
  	
	return CALL_SHUTTER_VTABLE_PTR(shutter, shutter_close)
}


int shutter_status(Shutter* shutter, int *status)
{
  	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_status) 

	if( (*shutter->lpVtbl->shutter_status)(shutter, status) == SHUTTER_ERROR ) {
		send_shutter_error_text(shutter, "shutter_status failed");
		return SHUTTER_ERROR;
	}

  	return SHUTTER_SUCCESS;
}

int shutter_set_open_time(Shutter* shutter, int open_time)
{
  	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) 

	if( (*shutter->lpVtbl->shutter_set_open_time)(shutter, open_time) == SHUTTER_ERROR ) {
		send_shutter_error_text(shutter, "shutter_set_open_time failed");
		return SHUTTER_ERROR;
	}

  	return SHUTTER_SUCCESS;
}

int shutter_get_open_time(Shutter* shutter, int *open_time)
{
  	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) 

	if( (*shutter->lpVtbl->shutter_get_open_time)(shutter, open_time) == SHUTTER_ERROR ) {
		send_shutter_error_text(shutter, "shutter_get_open_time failed");
		return SHUTTER_ERROR;
	}

  	return SHUTTER_SUCCESS;
}

int shutter_trigger(Shutter* shutter)
{
  	CHECK_SHUTTER_VTABLE_PTR(shutter, shutter_trigger) 
  	
	if( (*shutter->lpVtbl->shutter_trigger)(shutter) == SHUTTER_ERROR ) {
		send_shutter_error_text(shutter, "shutter_trigger failed");
		return SHUTTER_ERROR;
	}

  	return SHUTTER_SUCCESS;
}

void shutter_disable_timer(Shutter* shutter)
{
	#ifdef ENABLE_SHUTTER_STATUS_POLLING      
	SetAsyncTimerAttribute (shutter->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void shutter_enable_timer(Shutter* shutter)
{
	#ifdef ENABLE_SHUTTER_STATUS_POLLING      
	SetAsyncTimerAttribute (shutter->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}

int shutter_display_main_ui(Shutter* shutter)
{
	if(shutter->_main_ui_panel != -1) {
		shutter_read_or_write_main_panel_registry_settings(shutter, 0); 
		DisplayPanel(shutter->_main_ui_panel);
		shutter_enable_timer(shutter);
	}
	
	return SHUTTER_SUCCESS;
}


int shutter_hide_main_ui(Shutter* shutter)
{
	if(shutter->_main_ui_panel != -1) {
		shutter_read_or_write_main_panel_registry_settings(shutter, 1);
		HidePanel(shutter->_main_ui_panel);
	}

	GCI_Signal_Emit(&(shutter->signal_table), "Hide", GCI_VOID_POINTER, shutter); 

	return SHUTTER_SUCCESS;
}


int shutter_is_main_ui_visible(Shutter* shutter)
{
	int visible;
	
	GetPanelAttribute(shutter->_main_ui_panel, ATTR_VISIBLE, &visible);
	
	return visible;
}
