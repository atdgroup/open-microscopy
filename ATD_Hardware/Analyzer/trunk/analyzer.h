#ifndef __ANALYZER__
#define __ANALYZER__

#include "HardWareTypes.h" 
#include "HardWareDevice.h"      
#include "gci_ui_module.h"
#include "device_list.h"  
#include "toolbox.h" 

#define ANALYZER_SUCCESS 0
#define ANALYZER_ERROR -1

#define ANALYZER_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define ANALYZER_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_ANALYZER_VTABLE_PTR(ob, member) if(ANALYZER_VTABLE_PTR(ob, member) == NULL) { \
    send_analyzer_error_text(ob, "member not implemented"); \
    return ANALYZER_ERROR; \
}  

#define CALL_ANALYZER_VTABLE_PTR(ob, member) if( ANALYZER_VTABLE(ob, member)(ob) == ANALYZER_ERROR ) { \
	send_analyzer_error_text(ob, "member failed");  \
	return ANALYZER_ERROR; \
}

typedef struct _Analyzer Analyzer;


typedef struct
{
	int (*destroy) (Analyzer* analyzer);
	int (*analyzer_out) (Analyzer* analyzer);
	int (*analyzer_in) (Analyzer* analyzer); 
	int (*analyzer_status) (Analyzer* analyzer, int *status);

} AnalyzerVtbl;


struct _Analyzer
{ 
  HardwareDevice parent; 
  AnalyzerVtbl vtable;
 
  int	 	 _i2c_port;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int        _initialised;
};


Analyzer* analyzer_new(char *name, char *description, size_t size);

int analyzer_initialise (Analyzer* analyzer);
int analyzer_hardware_initialise (Analyzer* analyzer);
int analyzer_hardware_is_initialised (Analyzer* analyzer);

int  send_analyzer_error_text (Analyzer* analyzer, char fmt[], ...);

void analyzer_set_error_handler(Analyzer* analyzer, void (*handler) (char *error_string, Analyzer *analyzer));

int  analyzer_set_i2c_port(Analyzer* analyzer, int port);

int  analyzer_destroy(Analyzer* analyzer);
int  analyzer_out(Analyzer* analyzer);
int  analyzer_in(Analyzer* analyzer);
int  analyzer_status(Analyzer* analyzer, int *status);

void analyzer_disable_timer(Analyzer* analyzer);
void analyzer_enable_timer(Analyzer* analyzer);
void analyzer_on_change(Analyzer* analyzer); 

// Signals
typedef void (*ANALYZER_EVENT_HANDLER) (Analyzer* analyzer, void *data); 
typedef void (*ANALYZER_CHANGE_EVENT_HANDLER) (Analyzer* analyzer, int status, void *data);

int analyzer_changed_handler_connect(Analyzer* analyzer, ANALYZER_CHANGE_EVENT_HANDLER handler, void *data );

int CVICALLBACK OnAnalyserStateChange (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);		
int CVICALLBACK OnAnalyzerClose (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
		
#endif

 
