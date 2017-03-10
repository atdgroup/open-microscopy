#ifndef __ATD_A_LED__
#define __ATD_A_LED__

#include "lamp.h"

typedef enum {ATD_A_LED_OFF = 1, ATD_A_LED_ON=2, ATD_A_LED_PULSED=3, ATD_A_LED_FREE_RUN_MODE=4} ATD_A_LED_MODE;
typedef enum {ATD_A_LED_DISABLE = 0, ATD_A_LED_ENABLE=1} ATD_A_LED_ENABLE_STATUS;

typedef struct _ATD_LAMP_A ATD_LAMP_A;

struct _ATD_LAMP_A {
 
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
  int        _auxout;
  int        _input_trigger;
  int        _seq_time;
  int        _selected_sequence;
  int        _last_pulse_selected_sequence; // This is the last sequence selected (only pulse sequence ie not 6 or 7 (on / off)
  
  char		 _last_uploaded_sequence_filepath[GCI_MAX_PATHNAME_LEN];
  int		 _number_sequence_points[20];
  
  ATD_A_LED_MODE		_led_mode;
  ATD_A_LED_ENABLE_STATUS _enabled_status;
};


Lamp* adt_a_led_lamp_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

int adt_a_led_lamp_select_output_sequence(Lamp* lamp, int sequence);
int adt_a_led_lamp_set_led_mode (Lamp *lamp, ATD_A_LED_MODE mode);
int adt_a_led_lamp_set_input_trigger (Lamp* lamp, int trigger);
int adt_a_led_lamp_set_aux_out (Lamp* lamp, int aux_out);
int adt_a_led_lamp_set_intensity (Lamp* lamp, double intensity);
int adt_a_led_lamp_get_intensity (Lamp* lamp, double *intensity);
int adt_a_led_lamp_display_settings_ui(Lamp* lamp);
int adt_a_led_lamp_hide_settings_ui(Lamp* lamp);
int adt_a_led_lamp_save_settings (Lamp* lamp, const char *filepath, const char *flags);
int adt_a_led_lamp_load_settings (Lamp* lamp, const char *filepath);
int adt_a_led_lamp_save_sequence_data (Lamp* lamp, const char *filepath);
int adt_a_led_lamp_load_sequence_data (Lamp* lamp, const char *filepath);
int adt_a_led_lamp_set_pw_rep_rate(Lamp* lamp);
int adt_a_led_lamp_set_numtimeslots(Lamp* lamp, int numtimeslots, double period);
int adt_a_led_lamp_set_sequence_time(Lamp* lamp, double sequencetime, int numtimeslots);
int adt_a_led_lamp_set_mode(Lamp *lamp, ATD_A_LED_MODE mode, int sequence);
int adt_a_led_lamp_disable (Lamp* lamp);
int adt_a_led_lamp_enable (Lamp* lamp);

int  CVICALLBACK cb_htsleds_auxoutput(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_htsleds_intensity(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_htsleds_ledenable(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_htsleds_ledstate(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_htsleds_loaddata(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_htsleds_numtimeslots(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_htsleds_quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_htsleds_selectoutput(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_htsleds_sequencetime(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbset_htsleds_slottime(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
