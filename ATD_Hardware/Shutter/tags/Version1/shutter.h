#ifndef __SHUTTER__
#define __SHUTTER__

#include "signals.h"

#include <ansi_c.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Shutter control.
////////////////////////////////////////////////////////////////////////////

//#define ENABLE_SHUTTER_STATUS_POLLING 
	
#define SHUTTER_SUCCESS 0
#define SHUTTER_ERROR -1

#define SHUTTER_VTABLE_PTR(ob, member) ((ob)->lpVtbl->member)
#define SHUTTER_VTABLE(ob, member) (*((ob)->lpVtbl->member))

#define CHECK_SHUTTER_VTABLE_PTR(ob, member) if(SHUTTER_VTABLE_PTR(ob, member) == NULL) { \
    send_shutter_error_text(ob, "member not implemented"); \
    return SHUTTER_ERROR; \
}  

#define CALL_SHUTTER_VTABLE_PTR(ob, member) if( SHUTTER_VTABLE(ob, member)(ob) == SHUTTER_ERROR ) { \
	send_shutter_error_text(ob, "member failed");  \
	return SHUTTER_ERROR; \
}


typedef struct _Shutter Shutter;


typedef struct
{
	int (*destroy) (Shutter* shutter);
	int (*shutter_open) (Shutter* shutter);
	int (*shutter_close) (Shutter* shutter); 
	int (*shutter_status) (Shutter* shutter, int *status);
	int (*shutter_set_open_time) (Shutter* shutter, int time);
	int (*shutter_get_open_time) (Shutter* shutter, int *time);
	int (*shutter_trigger) (Shutter *shutter);

} ShutterVtbl;


struct _Shutter {
 
  ShutterVtbl *lpVtbl;
 
  char  	*_name;
  char  	*_description;
  
  int	 	 _mounted;
  int	 	 _i2c_port;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int	 	 _open_time;
  int		 _open;
  
  signal_table signal_table;
  
  void (*_error_handler) (char *error_string, Shutter* shutter);  
};


Shutter* shutter_new(char *name, char *description, size_t size);

int  send_shutter_error_text (Shutter* shutter, char fmt[], ...);

void shutter_set_error_handler(Shutter* shutter, void (*handler) (char *error_string, Shutter *shutter));

int  shutter_set_i2c_port(Shutter* shutter, int port);
int  shutter_set_description(Shutter* shutter, const char* description);
int  shutter_get_description(Shutter* shutter, char *description);
int  shutter_set_name(Shutter* shutter, char* name);
int  shutter_get_name(Shutter* shutter, char *name);

int  shutter_destroy(Shutter* shutter);
int  shutter_open(Shutter* shutter);
int  shutter_close(Shutter* shutter);
int  shutter_status(Shutter* shutter, int *status);
int  shutter_set_open_time(Shutter* shutter, int time);
int  shutter_get_open_time(Shutter* shutter, int *time);
int  shutter_trigger(Shutter *shutter);

void shutter_disable_timer(Shutter* shutter);
void shutter_enable_timer(Shutter* shutter);
void shutter_on_change(Shutter* shutter);
void shutter_changed(Shutter* shutter, int status);

int  shutter_display_main_ui (Shutter* shutter);
int  shutter_hide_main_ui (Shutter* shutter);
int  shutter_is_main_ui_visible (Shutter* shutter);


// Signals
typedef void (*SHUTTER_EVENT_HANDLER) (Shutter* shutter, void *data); 

int shutter_signal_hide_handler_connect (Shutter* shutter, SHUTTER_EVENT_HANDLER handler, void *callback_data);
int shutter_changed_handler_connect(Shutter* shutter, SHUTTER_EVENT_HANDLER handler, void *data );

#endif

 
