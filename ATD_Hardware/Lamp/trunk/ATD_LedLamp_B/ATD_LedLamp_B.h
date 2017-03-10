#ifndef __ATD_B_LED__
#define __ATD_B_LED__

#include "lamp.h"

typedef enum {ATD_B_LED_OFF = 0, ATD_B_LED_ON=1} ATD_B_LED_MODE;
typedef enum {ATD_B_LED_DISABLE = 0, ATD_B_LED_ENABLE=1} ATD_B_LED_ENABLE_STATUS;

typedef struct _ATD_LAMP_B ATD_LAMP_B;

struct _ATD_LAMP_B {
 
  Lamp       parent;    
	
  int		 _i2c_chip_type;
  int	 	 _com_port;
  int	 	 _i2c_bus;
  int	 	 _i2c_chip_address;
  int	 	 _i2c_ADaddress;
  int 	 	 _lock;
  int	 	 _main_ui_panel;
  double	 _min_intensity;
  double	 _max_intensity;
  double	 _intensity_increment;
  double   	 _intensity;
  
  ATD_B_LED_MODE		_led_mode;
  ATD_B_LED_ENABLE_STATUS _enabled_status;
};

Lamp* adt_b_led_lamp_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

#endif
