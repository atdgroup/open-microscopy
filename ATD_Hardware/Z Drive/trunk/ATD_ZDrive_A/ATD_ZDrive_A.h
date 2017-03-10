#ifndef __ATD_ZDRVIE_A_Z_DRIVE__
#define __ATD_ZDRVIE_A_Z_DRIVE__

#include "ZDrive.h"
#include "FTDI_Utils.h"

typedef struct
{
	Z_Drive parent;
	
	FTDIController*		_controller;

	int		 _panel_id;
	int	 	 _com_port;
	int		 _i2c_chip_address;
	int 	 _i2c_bus;
	int 	 _i2c_chip_type;
  	int 	 _lock;
	int		 _fast_byte;

	int      _sampling_rate;
	int      _sampling_scale_factor;
	float    _adc_read_wait_time;
	
	int 	 _has_adc;
	double	 _adc_closedloop_min;
	double	 _adc_closedloop_max;
	int		 _adc_timer;
	int		 _adc_monitor_timer;
	
	volatile int  _prevent_adc_update;

  	int		 _autofocus_i2c_chip_type;

  	int 	_autofocus_lock;
  	int   	_autofocus_ui_pnl;
  	int 	_autofocus_input0;
  	int 	_autofocus_input1;
  	int 	_autofocus_laser_I;
  	int 	_autofocus_errorRange;
  	int 	_autofocus_offsetCoarse;
  	int 	_autofocus_offsetFine;
  	int 	_autofocus_lowSignalLimit;
  	int 	_autofocus_differentialGain;  

	// 1 is manual
	int		_last_mode;
	
} ATD_ZDRIVE_A;

Z_Drive* atd_zdrive_a_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

// AUTOFOCUS Stuff

#define DAC0	0	   //Define DAC number
#define DAC1	1 
#define DAC2	2 
#define DAC3	3 
#define DAC4	4 
#define DAC5	5 
#define DAC6	6 
#define DAC7	7 

int  autofocus_display_main_ui(ATD_ZDRIVE_A* atd_zdrive_a_zd);
int  autofocus_hide_main_ui(ATD_ZDRIVE_A* atd_zdrive_a_zd);
int  autofocus_send_vals(ATD_ZDRIVE_A* atd_zdrive_a_zd);

int atd_zdrive_a_out_byte_max521_multiport (int port, int bus, unsigned char address, unsigned char DAC, unsigned char patt );
void atd_zdrive_a_set_alternate_adc_sampling (Z_Drive* zd);

int CVICALLBACK cb_autofocus_sendall_setup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int  CVICALLBACK cb_autofocus_abattn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_differentialgain(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_input_0(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_input_1(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_lasercurrent(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_lowsignalllimit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_offsetcoarse(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_offsetfine(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_autofocus_sendall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
