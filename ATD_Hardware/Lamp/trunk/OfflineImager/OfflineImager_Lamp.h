#ifndef __OFFLINE_IMAGER_LAMP__
#define __OFFLINE_IMAGER_LAMP__

#include "lamp.h"

typedef enum {OFFLINE_IMAGER_LAMP_OFF = 1, OFFLINE_IMAGER_LAMP_ON=2} OFFLINE_IMAGER_LAMP_MODE;
typedef enum {OFFLINE_IMAGER_LAMP_DISABLE = 0, OFFLINE_IMAGER_LAMP_ENABLE=1} OFFLINE_IMAGER_LAMP_STATUS;

typedef struct _OfflineImager_Lamp OfflineImager_Lamp;

struct _OfflineImager_Lamp {
 
  Lamp       parent;    
	
  int		 _i2c_chip_type;
  int	 	 _com_port;
  int	 	 _i2c_bus;
  int	 	 _i2c_chip_address;
  int	 	 _i2c_ADaddress;
  int 	 	 _lock;
  int	 	 _main_ui_panel;
  int	 	 _settings_ui_panel;
  int		 _program_flag;
  double	 _min_intensity;
  double	 _max_intensity;
  double	 _intensity_increment; 
  double   	 _intensity;

  
  OFFLINE_IMAGER_LAMP_MODE		_led_mode;
  OFFLINE_IMAGER_LAMP_STATUS _enabled_status;
};


Lamp* offline_imager_lamp_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

int offline_imager_lamp_select_output_sequence(Lamp* lamp, int sequence);
int offline_imager_lamp_set_intensity (Lamp* lamp, double intensity);
int offline_imager_lamp_get_intensity (Lamp* lamp, double *intensity);
int offline_imager_lamp_display_settings_ui(Lamp* lamp);
int offline_imager_lamp_hide_settings_ui(Lamp* lamp);
int offline_imager_lamp_save_settings (Lamp* lamp, const char *filepath, const char *flags);
int offline_imager_lamp_load_settings (Lamp* lamp, const char *filepath);
int offline_imager_lamp_set_mode(Lamp *lamp, OFFLINE_IMAGER_LAMP_MODE mode);
int offline_imager_lamp_disable (Lamp* lamp);
int offline_imager_lamp_enable (Lamp* lamp);

int  CVICALLBACK offline_imager_lamp_intensity(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK offline_imager_lamp_ledmode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK offline_imager_lamp_quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#endif
