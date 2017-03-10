#ifndef __ATD_ZDRIVE_B_Z_DRIVE__
#define __ATD_ZDRIVE_B_Z_DRIVE__

#include "ZDrive.h"

typedef enum {ATD_ZDRIVE_B_OUTPUT_MODE_AF = 0, ATD_ZDRIVE_B_OUTPUT_MODE_OFF = 1, ATD_ZDRIVE_B_OUTPUT_MODE_DAC = 2} ATD_ZDRIVE_B_OUTPUT_MODE;

typedef enum {ATD_ZDRIVE_B_CONVERSION_MODE_CONTINOUS = 0, ATD_ZDRIVE_B_CONVERSION_MODE_SINGLE = 1} ATD_ZDRIVE_B_CONVERSION_MODE;

typedef enum {ATD_ZDRIVE_B_SAMPLING_VAL_60 = 1, ATD_ZDRIVE_B_SAMPLING_VAL_30 = 2, ATD_ZDRIVE_B_SAMPLING_VAL_15 = 3} ATD_ZDRIVE_B_SAMPLING_VAL;

#define OBJ_DAC 0
#define FOCUS_DAC 1

typedef struct
{
	int		 _read_address;
	int		 _write_address;
	double	 _current_dac_val;
	double	 _min_microns;
	double	 _max_microns;
	double	 _adc_min;
	double	 _adc_max;
	double	 _scale_factor;
	double	 _adc_scale_factor;
	double	 _offset;
	double	 _adc_offset;

} Dac;

typedef struct
{
	Z_Drive parent;
	
	int		 _panel_id;
	int		 _setup_panel_id;
	int	 	 _com_port;
	int		 _pcf8574a_chip_address;
	int 	 _i2c_bus;
	int 	 _i2c_chip_type;
  	int 	 _lock;
	int		 _fast_byte;
	int      _scale_factor;

	Dac		 _dacs[2];
	int		 _current_dac;
	int 	 _has_adc;
	int		 _adc_timer;
	int		 _adc_monitor_timer;
	
  	int		 _autofocus_atd_b_i2c_chip_type;
	int		 _autofocus_atd_b_i2c_chip_address;

  	int 	_autofocus_atd_b_lock;
  	int   	_autofocus_atd_b_ui_pnl;
  	int 	_autofocus_atd_b_input0;
  	int 	_autofocus_atd_b_input1;
  	int 	_autofocus_atd_b_laser_I;
  	int 	_autofocus_atd_b_errorRange;
  	int 	_autofocus_atd_b_offsetCoarse;
  	int 	_autofocus_atd_b_offsetFine;
  	int 	_autofocus_atd_b_lowSignalLimit;
  	int 	_autofocus_atd_b_differentialGain;  

	int		_stage_removed;

	volatile int  _prevent_adc_update;
	
} ATD_ZDRIVE_B;

Z_Drive* atd_zdrive_b_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

// AUTOFOCUS Stuff

#define DAC0	0	   //Define DAC number
#define DAC1	1 
#define DAC2	2 
#define DAC3	3 
#define DAC4	4 
#define DAC5	5 
#define DAC6	6 
#define DAC7	7 

int dac_read_address(ATD_ZDRIVE_B* atd_zdrive_b);
int dac_write_address(ATD_ZDRIVE_B* atd_zdrive_b);
void atd_zdrive_b_initialise(ATD_ZDRIVE_B* atd_zdrive_b, ATD_ZDRIVE_B_CONVERSION_MODE conversion, ATD_ZDRIVE_B_SAMPLING_VAL sampling);
int  atd_zdrive_b_set_dac(ATD_ZDRIVE_B* atd_zdrive_b, int dac, int val, int update_ui_val);
int  atd_zdrive_b_set_focus_dac(ATD_ZDRIVE_B* atd_zdrive_b, int val, int update_ui_val);
int  atd_zdrive_b_set_obj_dac(ATD_ZDRIVE_B* atd_zdrive_b, int val, int update_ui_val);
int  atd_zdrive_b_set_output_mode(ATD_ZDRIVE_B* atd_zdrive_b, ATD_ZDRIVE_B_OUTPUT_MODE mode);
void atd_zdrive_b_read_dac(ATD_ZDRIVE_B* atd_zdrive_b, unsigned char dac_address, int *value);
void atd_zdrive_b_set_dac1_use(ATD_ZDRIVE_B *atd_zdrive_b_zd);
void atd_zdrive_b_set_dac2_use(ATD_ZDRIVE_B *atd_zdrive_b_zd);
void atd_zdrive_b_read_status_bits(ATD_ZDRIVE_B* atd_zdrive_b, int *obj_dac_selected, int *front_panel_enabled, int *focus_dac_selected, int *stage_removed);

int  autofocus_atd_b_display_main_ui(ATD_ZDRIVE_B* atd_zdrive_b_zd);
int  autofocus_atd_b_hide_main_ui(ATD_ZDRIVE_B* atd_zdrive_b_zd);
int  autofocus_atd_b_send_vals(ATD_ZDRIVE_B* atd_zdrive_b_zd);

int atd_zdrive_b_out_byte_max521_multiport (ATD_ZDRIVE_B* atd_zdrive_b, int port, int bus, unsigned char address, unsigned char DAC, unsigned char patt );

int CVICALLBACK cb_autofocus_atd_b_sendall_setup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int  CVICALLBACK cb_atd_b_abattn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_differentialgain(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_input_0(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_input_1(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_lasercurrent(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_lowsignalllimit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_offsetcoarse(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_offsetfine(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_atd_b_sendall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


int CVICALLBACK OnAtdBZdriveSetup_Close (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnAtdBZdriveSetup_ModeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnAtdBZdriveSetup_ConversionControlChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnAtdBZdriveSetup_OnSamplingRateChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnAtdBZdriveSetup_OnDac1Selected (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnAtdBZdriveSetup_OnDac2Selected (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnAtdBZdriveSetup_OnDacValChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnAtdBZdriveSetup_OnDacSettingsChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

#endif
