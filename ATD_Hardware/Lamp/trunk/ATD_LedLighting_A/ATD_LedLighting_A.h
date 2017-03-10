#ifndef __ATD_A_LIGHTING__
#define __ATD_A_LIGHTING__

#include "lamp.h"

typedef struct _ATD_LIGHTING_A ATD_LIGHTING_A;

struct _ATD_LIGHTING_A {
 
  Lamp       parent;    
	
  int		 _i2c_chip_type;
  int	 	 _com_port;
  int	 	 _i2c_bus;
  int	 	 _i2c_chip_address;
  int	 	 _i2c_ADaddress;
  int 	 	 _lock;
  int	 	 _main_ui_panel;
  int		 _program_flag;
  int		 _led_mode;
};


Lamp* atd_led_lighting_a_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

int atd_led_lighting_a_display_settings_ui(Lamp* lamp);
int atd_led_lighting_a_hide_settings_ui(Lamp* lamp);

int  CVICALLBACK OnLedLightingStateChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLedLightingQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
