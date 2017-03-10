#ifndef __SHUTTER__
#define __SHUTTER__

#include "HardwareTypes.h"
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"   

//#define ENABLE_SHUTTER_STATUS_POLLING 
	
#define SHUTTER_CLOSE_OPEN_ARRAY_SIZE 10
#define SHUTTER_SHOULD_CLOSE_THRESHOLD 0.5		// 0.5 second threshold

#define SHUTTER_SUCCESS 0
#define SHUTTER_ERROR -1

#define SHUTTER_VTABLE_PTR(ob, member) ((ob)->lpVtbl.member)
#define SHUTTER_VTABLE(ob, member) (*((ob)->lpVtbl.member))

#define CHECK_SHUTTER_VTABLE_PTR(ob, member) if(SHUTTER_VTABLE_PTR(ob, member) == NULL) { \
    send_shutter_error_text(ob, "member not implemented"); \
    return SHUTTER_ERROR; \
}  

#define CALL_SHUTTER_VTABLE_PTR(ob, member) if( SHUTTER_VTABLE(ob, member)(ob) == SHUTTER_ERROR ) { \
	send_shutter_error_text(ob, "member failed");  \
	return SHUTTER_ERROR; \
}

typedef struct
{
	int (*hw_init) (Shutter* shutter); 
	int (*destroy) (Shutter* shutter);
	int (*shutter_open) (Shutter* shutter);
	int (*shutter_close) (Shutter* shutter); 
	int (*shutter_status) (Shutter* shutter, int *status);
	int (*shutter_set_open_time) (Shutter* shutter, double time);
	int (*shutter_get_open_time) (Shutter* shutter, double *time);
	int (*shutter_inhibit) (Shutter* shutter, int inhibit);
	int (*shutter_is_inhibited) (Shutter* shutter, int *inhibit);
	int (*shutter_set_computer_control) (Shutter* shutter, int comp_ctrl);
	int (*shutter_get_info) (Shutter* shutter, char* info);

} ShutterVtbl;


struct _Shutter
{
  HardwareDevice parent; 
  
  ShutterVtbl lpVtbl;
 
  int		 _panel_id; 
  int     	 _timer;
  int		 _initialised;
  int 		 _enabled;

  double	_last_close_time;
  double	_time_until_open[SHUTTER_CLOSE_OPEN_ARRAY_SIZE];
};


void shutter_constructor(Shutter* shutter, const char *name, const char *description);

int shutter_initialise(Shutter *shutter);  
int shutter_hardware_initialise (Shutter* shutter);
int shutter_is_initialised(Shutter *shutter);

int  send_shutter_error_text (Shutter* shutter, char fmt[], ...);

void shutter_set_error_handler(Shutter* shutter, UI_MODULE_ERROR_HANDLER handler, void *callback_data);

int  shutter_destroy(Shutter* shutter);
int  shutter_open(Shutter* shutter);
int  shutter_close(Shutter* shutter);
int  shutter_reset_previous_open_close_experience(Shutter *shutter);
int  shutter_intelligent_close(Shutter *shutter);
int  shutter_status(Shutter* shutter, int *status);

// A time <= 0 will be interpreted as infinity
// open_time is is milli seconds  
int  shutter_set_open_time(Shutter* shutter, double time);
int  shutter_get_open_time(Shutter* shutter, double *time);
int  shutter_inhibit(Shutter* shutter, int inhibit);
int  shutter_is_inhibited(Shutter* shutter, int *inhibit);
int  shutter_set_computer_control(Shutter* shutter, int compCtrl);
int  shutter_save_settings_in_ini_fmt (Shutter* shutter, const char *filepath, const char *mode);    
int  shutter_load_settings_from_ini_fmt (Shutter* shutter, const char *filepath); 
int  shutter_get_info (Shutter* shutter, char* info);

void shutter_disable_timer(Shutter* shutter);
void shutter_enable_timer(Shutter* shutter);

void shutter_emit_if_shutter_changed(Shutter* shutter);

// Signals
typedef void (*SHUTTER_EVENT_HANDLER) (Shutter* shutter, void *data); 
typedef void (*SHUTTER_CHANGE_EVENT_HANDLER) (Shutter* shutter, int status, void *data); 

int shutter_signal_hide_handler_connect (Shutter* shutter, SHUTTER_EVENT_HANDLER handler, void *callback_data);
int shutter_changed_handler_connect(Shutter* shutter, SHUTTER_CHANGE_EVENT_HANDLER handler, void *data );

#endif

 
