#include "analyzer.h"
#include "analyzer_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "toolbox.h"

#include <utility.h>

#include "asynctmr.h"

#include <ansi_c.h> 

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Analyzer control.
////////////////////////////////////////////////////////////////////////////

int send_analyzer_error_text (Analyzer* analyzer, char fmt[], ...)
{
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(analyzer), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(analyzer), message);  
	
	ui_module_send_valist_error(UIMODULE_CAST(analyzer), "Analyzer Manager Error", fmt, ap);
	
	va_end(ap);  

	return ANALYZER_SUCCESS;
}


static void error_handler (char *error_string, Analyzer *analyzer)
{
	MessagePopup("Analyzer Error", error_string); 
}

static int ANALYZER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Analyzer*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Analyzer *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int ANALYZER_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Analyzer*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Analyzer *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}



void analyzer_on_change(Analyzer* analyzer)
{
	int status;
	static int prev_status=-1;
	
	CmtGetLock (analyzer->_lock);
			
	if (analyzer_status(analyzer, &status) == ANALYZER_SUCCESS) {
		SetCtrlVal (analyzer->_main_ui_panel, ANALYZER_STATUS, status);
		SetCtrlVal (analyzer->_main_ui_panel, ANALYZER_LED, status);
	}
			
   	if (status != prev_status)
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(analyzer), "AnalyzerChanged", GCI_VOID_POINTER, analyzer, GCI_INT, status);  
			
	prev_status = status;
			
	CmtReleaseLock(analyzer->_lock);	
}


int CVICALLBACK OnAnalyzerTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Analyzer *analyzer = (Analyzer *) callbackData;
	
    switch (event)
    {
        case EVENT_TIMER_TICK:
        
        	analyzer_on_change(analyzer);
			
            break;
    }
    
    return 0;
}

static void OnPanelsClosedOrHidden (UIModule *module, void *data)
{
	Analyzer* analyzer = (Analyzer*) data; 

	analyzer_disable_timer(analyzer);
}

static void OnPanelsDisplayed (UIModule *module, int panel_id, void *data)
{
	Analyzer* analyzer = (Analyzer*) data; 

	analyzer_enable_timer(analyzer);
}

int analyzer_hardware_initialise (Analyzer* analyzer)
{
  	return ANALYZER_SUCCESS;
}

int analyzer_initialise (Analyzer* analyzer)
{
	analyzer->_timer = -1;
	analyzer->_i2c_port = 1;

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(analyzer), "AnalyzerChanged", ANALYZER_PTR_INT_MARSHALLER);

	analyzer->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(analyzer), "analyzer_ui.uir", ANALYZER, 1); 

    if ( InstallCtrlCallback (analyzer->_main_ui_panel, ANALYZER_STATUS, OnAnalyserStateChange, analyzer) < 0)
		return ANALYZER_ERROR;
		
	if ( InstallCtrlCallback (analyzer->_main_ui_panel, ANALYZER_CLOSE, OnAnalyzerClose, analyzer) < 0)
		return ANALYZER_ERROR;	
			
	#ifdef ENABLE_ANALYZER_STATUS_POLLING
	analyzer->_timer = NewAsyncTimer (2.0, -1, 1, OnAnalyzerTimerTick, analyzer);
	SetAsyncTimerAttribute (analyzer->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
	
	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(analyzer), OnPanelsClosedOrHidden, analyzer);
	ui_module_panel_show_handler_connect (UIMODULE_CAST(analyzer), OnPanelsDisplayed, analyzer);

    analyzer->_initialised = 1;

  	return ANALYZER_SUCCESS;
}

int analyzer_hardware_is_initialised (Analyzer* analyzer)
{
    return analyzer->_initialised;
}

int analyzer_changed_handler_connect(Analyzer* analyzer, ANALYZER_CHANGE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(analyzer), "AnalyzerChanged", handler, data) == SIGNAL_ERROR) {
		send_analyzer_error_text(analyzer, "Can not connect signal handler for Analyzer Change signal");
		return ANALYZER_ERROR;
	}

	return ANALYZER_SUCCESS;
}

Analyzer* analyzer_new(char *name, char *description, size_t size)
{
	Analyzer* analyzer = (Analyzer*) malloc(size);
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(analyzer), name); 
	ui_module_set_description(UIMODULE_CAST(analyzer), description);       
	
	ANALYZER_VTABLE_PTR(analyzer, destroy) = NULL; 
	ANALYZER_VTABLE_PTR(analyzer, analyzer_out) = NULL; 
	ANALYZER_VTABLE_PTR(analyzer, analyzer_in) = NULL; 
	ANALYZER_VTABLE_PTR(analyzer, analyzer_status) = NULL;
	
	CmtNewLock (NULL, 0, &(analyzer->_lock) );  
	
	return analyzer;
}


int analyzer_destroy(Analyzer* analyzer)
{
	analyzer_disable_timer(analyzer);
	
	analyzer_out(analyzer);	//Leave out
		
	CHECK_ANALYZER_VTABLE_PTR(analyzer, destroy) 
  	
	CALL_ANALYZER_VTABLE_PTR(analyzer, destroy) 
	
	ui_module_destroy(UIMODULE_CAST(analyzer));
  	
  	CmtDiscardLock (analyzer->_lock);
  	
  	free(analyzer);
  	
  	return ANALYZER_SUCCESS;
}


int analyzer_set_i2c_port(Analyzer* analyzer, int port)
{
	analyzer->_i2c_port = port;
	
	return ANALYZER_SUCCESS;  
}

int analyzer_out(Analyzer* analyzer)
{
	CHECK_ANALYZER_VTABLE_PTR(analyzer, analyzer_out) 
  	
	CALL_ANALYZER_VTABLE_PTR(analyzer, analyzer_out)

  	return ANALYZER_SUCCESS;
}


int analyzer_in(Analyzer* analyzer)
{
  	CHECK_ANALYZER_VTABLE_PTR(analyzer, analyzer_in) 
  	
	CALL_ANALYZER_VTABLE_PTR(analyzer, analyzer_in)

  	return ANALYZER_SUCCESS;
}


int analyzer_status(Analyzer* analyzer, int *status)
{
  	CHECK_ANALYZER_VTABLE_PTR(analyzer, analyzer_status) 

	if( (*analyzer->vtable.analyzer_status)(analyzer, status) == ANALYZER_ERROR ) {
		send_analyzer_error_text(analyzer, "analyzer_status failed");
		return ANALYZER_ERROR;
	}

  	return ANALYZER_SUCCESS;
}

void analyzer_disable_timer(Analyzer* analyzer)
{
	#ifdef ENABLE_ANALYZER_STATUS_POLLING 
	SetAsyncTimerAttribute (analyzer->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void analyzer_enable_timer(Analyzer* analyzer)
{
	#ifdef ENABLE_ANALYZER_STATUS_POLLING 
	SetAsyncTimerAttribute (analyzer->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}