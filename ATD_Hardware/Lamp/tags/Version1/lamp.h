#ifndef __LAMP__
#define __LAMP__

#include "signals.h"

#include <ansi_c.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Brightness control.
////////////////////////////////////////////////////////////////////////////

//#define ENABLE_LAMP_STATUS_POLLING

#define LAMP_SUCCESS 0
#define LAMP_ERROR -1


#define LAMP_VTABLE_PTR(ob, member) ((ob)->lpVtbl->member)
#define LAMP_VTABLE(ob, member) (*((ob)->lpVtbl->member))

#define CHECK_LAMP_VTABLE_PTR(ob, member) if(LAMP_VTABLE_PTR(ob, member) == NULL) { \
    send_lamp_error_text(ob, "member not implemented"); \
    return LAMP_ERROR; \
}  

#define CALL_LAMP_VTABLE_PTR(ob, member) if( LAMP_VTABLE(ob, member)(ob) == LAMP_ERROR ) { \
	send_lamp_error_text(ob, "member failed");  \
	return LAMP_ERROR; \
}


typedef struct _Lamp Lamp;


typedef struct
{
	int (*destroy) (Lamp* lamp);
	int (*lamp_off) (Lamp* lamp);
	int (*lamp_on) (Lamp* lamp); 
	int (*lamp_status) (Lamp* lamp, int *status);
	int (*lamp_set_intensity) (Lamp* lamp, double intensity);
	int (*lamp_get_intensity) (Lamp* lamp, double *intensity);
	int (*lamp_set_intensity_range) (Lamp *lamp, double min, double max, double increment);

} LampVtbl;


struct _Lamp {
 
  LampVtbl *lpVtbl;
 
  char  	*_name;
  char  	*_description;
  
  int	 	 _mounted;
  int	 	 _i2c_port;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  double	 _min_intensity;
  double	 _max_intensity;
  double	 _intesity_increment;
 
  signal_table signal_table;
  
  void (*_error_handler) (char *error_string, Lamp* lamp);  
};


Lamp* lamp_new(char *name, char *description, size_t size);

int  send_lamp_error_text (Lamp* lamp, char fmt[], ...);

void lamp_set_error_handler(Lamp* lamp, void (*handler) (char *error_string, Lamp *lamp));

int  lamp_set_i2c_port(Lamp* lamp, int port);
int  lamp_set_description(Lamp* lamp, const char* description);
int  lamp_get_description(Lamp* lamp, char *description);
int  lamp_set_name(Lamp* lamp, char* name);
int  lamp_get_name(Lamp* lamp, char *name);

int  lamp_destroy(Lamp* lamp);
int  lamp_off(Lamp* lamp);
int  lamp_on(Lamp* lamp);
int  lamp_status(Lamp* lamp, int *status);
int  lamp_set_intensity(Lamp* lamp, double intensity);
int  lamp_get_intensity(Lamp* lamp, double *intensity);
int  lamp_set_intensity_range(Lamp *lamp, double min, double max, double increment);
int  lamp_get_intensity_range(Lamp *lamp, double *min, double *max);

void lamp_disable_timer(Lamp* lamp);
void lamp_enable_timer(Lamp* lamp);
void lamp_on_change(Lamp* lamp); 

int  lamp_display_main_ui (Lamp* lamp);
int  lamp_hide_main_ui (Lamp* lamp);
int  lamp_is_main_ui_visible (Lamp* lamp);


// Signals
typedef void (*LAMP_EVENT_HANDLER) (Lamp* lamp, void *data); 

int lamp_signal_hide_handler_connect (Lamp* lamp, LAMP_EVENT_HANDLER handler, void *callback_data);
int lamp_changed_handler_connect(Lamp* lamp, LAMP_EVENT_HANDLER handler, void *data );

#endif

 
