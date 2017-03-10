#ifndef __INTENSILIGHT__
#define __INTENSILIGHT__

#include "HardwareTypes.h"
#include "HardWareDevice.h" 

#include "gci_ui_module.h"
#include "iniparser.h"
#include "signals.h"

#define INTENSILIGHT_SUCCESS 0
#define INTENSILIGHT_ERROR -1

typedef enum {INTENSILIGHT_OPEN, INTENSILIGHT_CLOSED} IntensilightStatus;

typedef struct _Intensilight Intensilight;

struct _Intensilight
{ 
  HardwareDevice parent;

  unsigned int 	_port;
  char	 		_port_string[10];
  int	 	 	_connected;
  int 	 	 	_lock;
  int     	 	_timer;
  int	 	 	_main_ui_panel;
 				
  int			_nd_filter;
  int			_shutter_status;
  int			_shutter_open_time;
  int			_signal_delay;
  int			_bounce_delay;

  int			_remote_enabled;
  int			_remote_led; 

  int			_initialised;
};


Intensilight* intensilight_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

int intensilight_hardware_is_initialised(Intensilight* intensilight);
int intensilight_hardware_initialise (Intensilight* intensilight);
int intensilight_is_connected(Intensilight* intensilight);
int intensilight_initialise (Intensilight* intensilight);

int  send_intensilight_error_text (Intensilight* intensilight, char fmt[], ...);
void intensilight_set_error_handler(Intensilight* intensilight, void (*handler) (char *error_string, Intensilight* precise_excite));
void intensilight_stop_timer(Intensilight* intensilight);
void intensilight_start_timer(Intensilight* intensilight);

int  intensilight_destroy(Intensilight* intensilight);

void intensilight_on_change(Intensilight* intensilight);

int  intensilight_display_main_ui (Intensilight* intensilight);
int  intensilight_hide_main_ui (Intensilight* intensilight);
int  intensilight_is_main_ui_visible (Intensilight* intensilight);

int intensilight_connect(Intensilight* intensilight);

int intensilight_set_nd_filter(Intensilight* intensilight, int nd_filter);
int intensilight_get_nd_filter(Intensilight* intensilight, int *nd_filter);
int intensilight_open_shutter(Intensilight* intensilight);
int intensilight_close_shutter(Intensilight* intensilight);
int intensilight_set_shutter_open_time(Intensilight* intensilight, int ms);
int intensilight_trigger_shutter(Intensilight* intensilight);
int intensilight_read_shutter(Intensilight* intensilight, int *status);
int intensilight_set_remote_enable(Intensilight* intensilight, int enable);
int intensilight_get_remote_enable(Intensilight* intensilight, int *enable);
int intensilight_set_remote_led(Intensilight* precise_excite, int led);
int intensilight_get_remote_led(Intensilight* precise_excite, int *led);

// Signals
typedef void (*INTENSILIGHT_EVENT_HANDLER) (Intensilight* precise_excite, void *data); 
typedef void (*INTENSILIGHT_CHANGE_EVENT_HANDLER) (Intensilight* precise_excite, int nd, int shutter_status, void *data); 

int intensilight_signal_hide_handler_connect (Intensilight* intensilight, INTENSILIGHT_EVENT_HANDLER handler, void *callback_data);
int intensilight_changed_handler_connect(Intensilight* intensilight, INTENSILIGHT_CHANGE_EVENT_HANDLER handler, void *data );



int CVICALLBACK cbIntensilightClose (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightSetNdfilter (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightShutterOpen (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightShutterClose (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightShutterTrigger (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightShutterOpenTime (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightShutterTest (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightRemoteEnable (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightRemotebrightness (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cb_IntensilightBounceDelay (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cb_IntensilightSignalDelay (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cb_IntensilightExposure (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int CVICALLBACK cbIntensilightExpTest (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
