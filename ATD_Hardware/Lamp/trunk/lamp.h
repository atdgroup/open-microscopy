#ifndef __LAMP__
#define __LAMP__

#include "HardwareTypes.h"
#include "HardWareDevice.h" 

#include "gci_ui_module.h"
#include "iniparser.h"
#include "signals.h"

#define LAMP_SUCCESS 0
#define LAMP_ERROR -1

#define LAMP_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define LAMP_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_LAMP_VTABLE_PTR(ob, member) if(LAMP_VTABLE_PTR(ob, member) == NULL) { \
    ui_module_send_error(UIMODULE_CAST(ob), "Lamp Error", "member not implemented"); \
    return LAMP_ERROR; \
}  

#define CALL_LAMP_VTABLE_PTR(ob, member) if( LAMP_VTABLE(ob, member)(ob) == LAMP_ERROR ) { \
	ui_module_send_error(UIMODULE_CAST(ob), "Lamp Error", "member failed");  \
	return LAMP_ERROR; \
}

#define DEFAULT_LAMP_FILENAME_SUFFIX "LampSettings.ini"

typedef enum {LAMP_OFF=0, LAMP_ON=1} LampStatus;

typedef struct
{
	int (*init) (Lamp *lamp);
	int (*hardware_init) (Lamp *lamp);
	int (*destroy) (Lamp* lamp);
	int (*lamp_off) (Lamp* lamp);
	int (*lamp_on) (Lamp* lamp); 
	int (*lamp_off_on_status) (Lamp* lamp, LampStatus *status);
	int (*lamp_set_intensity) (Lamp* lamp, double intensity);
	int (*lamp_get_intensity) (Lamp* lamp, double *intensity);
	int (*lamp_set_intensity_range) (Lamp *lamp, double min, double max, double increment);
	int (*save_settings) (Lamp *lamp, const char *filepath, const char *flags);
	int (*load_settings) (Lamp *lamp, const char *filepath);

} LampVtbl;


struct _Lamp
{ 
    HardwareDevice parent; 
    
    LampVtbl vtable;
 
    int		 _initialised;
    int      _timer;
    int	 	 _main_ui_panel;
};


int lamp_constructor(Lamp *lamp, char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);
int lamp_set_main_panel (Lamp* lamp, int panel_id);

void lamp_set_error_handler(Lamp* lamp, UI_MODULE_ERROR_HANDLER handler);

int  lamp_initialise(Lamp* lamp);
int  lamp_hardware_initialise(Lamp* lamp);
int  lamp_hardware_is_initialised(Lamp* lamp);
int  lamp_destroy(Lamp* lamp);
int  lamp_off(Lamp* lamp);
int  lamp_on(Lamp* lamp);
int  lamp_off_on_status(Lamp* lamp, LampStatus *status);
int  lamp_set_intensity(Lamp* lamp, double intensity);
int  lamp_get_intensity(Lamp* lamp, double *intensity);
int  lamp_set_intensity_range(Lamp *lamp, double min, double max, double increment);
int  lamp_get_intensity_range(Lamp *lamp, double *min, double *max);

void lamp_disable_timer(Lamp* lamp);
void lamp_enable_timer(Lamp* lamp);

int  lamp_display_main_ui (Lamp* lamp);
int  lamp_hide_main_ui (Lamp* lamp);

int lamp_load_settings(Lamp *lamp, const char *filename);
int lamp_save_settings(Lamp *lamp, const char *filename, const char *flags);

/*
int lamp_save_default_settings(Lamp *lamp);
int lamp_load_default_settings(Lamp *lamp);
*/

// Signals
typedef void (*LAMP_EVENT_HANDLER) (Lamp* lamp, void *data); 
typedef void (*LAMP_CHANGE_EVENT_HANDLER) (Lamp* lamp, int status, double intensity, void *data); 

int lamp_changed_handler_connect(Lamp* lamp, LAMP_EVENT_HANDLER handler, void *data );

int  CVICALLBACK OnLampClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLampIntensity(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLampOnOffToggle(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
