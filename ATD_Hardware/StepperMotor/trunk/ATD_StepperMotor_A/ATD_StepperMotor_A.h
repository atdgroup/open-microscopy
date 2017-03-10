#ifndef __MIRROR_STEPPER__
#define __MIRROR_STEPPER__

#include "signals.h"
#include "gci_ui_module.h"
#include "FTDI_Utils.h"

#include <ansi_c.h>

////////////////////////////////////////////////////////////////////////////
//RJL/RGN April 2007
//GCI HTP Microscope system. 
//Mirror stepper control.
////////////////////////////////////////////////////////////////////////////

#define MIRROR_STEPPER_SUCCESS 0
#define MIRROR_STEPPER_ERROR -1

//#define MIRROR_STEPPER_PIC	 0xc4   //address set of MIRROR_STEPPER_ PIC

//#define MIRROR_STEPPER_BUS 2			//Set to required bus(on MPTR system) else set to 2
//#define MIRROR_STEPPER_ADDRESS 0		//Programable address of PIC

// Contains no UI functions
#define DEFAULT_HOLD_CURRENT 10
#define DEFAULT_RUN_CURRENT 10
#define DEFAULT_MAX_VELOCITY 8
#define DEFAULT_MIN_VELOCITY 15
#define DEFAULT_ACC 2
#define DEFAULT_SHAFT 0
#define DEFAULT_RESOLUTION 3
#define DEFAULT_ACC_SHAPE 0
#define DEFAULT_TARGET1 3056
#define DEFAULT_TARGET2 0
#define DEFAULT_TARGET3 3040
#define DEFAULT_TARGET4 0
#define DEFAULT_POSITION_OFFSET 10
	

typedef struct _MirrorStepper MirrorStepper;

struct _MirrorStepper {
 
  UIModule parent;    
  
  FTDIController*		_controller;

  char  	 _name;
  char  	 _description;
  
  int	 	 _mounted;
  int	 	 _com_port;
  int	 	 _i2c_bus;
  int		 _i2c_chip_type;
  int	 	 _i2c_chip_address;
  int	 	 _main_ui_panel;
  int	 	 _calib_ui_panel;

  int		 _init_holdcurrent;
  int		 _init_runcurrent;
  int		 _init_maxvelocity;
  int		 _init_minvelocity;
  int		 _init_acc;
  int		 _init_accshape;
  int		 _init_shaft;
  int		 _init_resolution;

  int		 _target_1;
  int		 _target_2;
  int		 _target_3;
  int		 _target_4;
  int		 _default_pos;
  int		 _posoffsets[5];		 // 0 is default offset applied to all positions, 1,2,3,4 are added to theose positions also
   
  int		 _init_mirror_pos_flag;

  volatile	 int _current_pos;
  volatile	 int _requested_pos;
  volatile   int _motion;
  volatile   int _move_abort;
  volatile	 int _set_pos_thread_id;
};


MirrorStepper* mirror_stepper_new(const char *name, const char *description);

int mirror_stepper_init (MirrorStepper* mirror_stepper);

int mirror_stepper_save_data(MirrorStepper* mirror_stepper);

int  send_mirror_stepper_error_text (MirrorStepper* mirror_stepper, char fmt[], ...);

int  mirror_stepper_set_data_dir(MirrorStepper* mirror_stepper, const char *dir);

void mirror_stepper_set_error_handler(MirrorStepper* mirror_stepper, UI_MODULE_ERROR_HANDLER handler);

int  mirror_stepper_set_description(MirrorStepper* mirror_stepper, const char* description);
int  mirror_stepper_get_description(MirrorStepper* mirror_stepper, char *description);
int  mirror_stepper_set_name(MirrorStepper* mirror_stepper, const char* name);
int  mirror_stepper_get_name(MirrorStepper* mirror_stepper, char *name);

int  mirror_stepper_destroy(MirrorStepper* mirror_stepper);

void mirror_stepper_on_change(MirrorStepper* mirror_stepper); 

int  mirror_stepper_display_main_ui (MirrorStepper* mirror_stepper);
int  mirror_stepper_hide_main_ui (MirrorStepper* mirror_stepper);
int  mirror_stepper_is_main_ui_visible (MirrorStepper* mirror_stepper);
int  mirror_stepper_display_calib_ui(MirrorStepper* mirror_stepper, int parent_panel);
int  mirror_stepper_hide_calib_ui (MirrorStepper* mirror_stepper);

int mirror_stepper_initialise(MirrorStepper* mirror_stepper, int move_to_default);
int mirror_stepper_setparam(MirrorStepper* mirror_stepper);

int mirror_stepper_set_offset(MirrorStepper* mirror_stepper, int mirrorpos, int offset);
int mirror_stepper_set_pos(MirrorStepper* mirror_stepper, int mirrorpos);

int mirror_stepper_set_com_port_i2c_details(MirrorStepper* mirror_stepper, int port, int pic, int bus, int address);
int mirror_stepper_set_ftdi_controller(MirrorStepper* mirror_stepper, FTDIController* controller, int pic, int bus, int address);

int mirror_stepper_get_pos(MirrorStepper* mirror_stepper);

// Signals
typedef void (*MIRROR_STEPPER_EVENT_HANDLER) (MirrorStepper* mirror_stepper, void *data); 

int mirror_stepper_signal_hide_handler_connect (MirrorStepper* mirror_stepper, MIRROR_STEPPER_EVENT_HANDLER handler, void *callback_data);
int mirror_stepper_changed_handler_connect(MirrorStepper* mirror_stepper, MIRROR_STEPPER_EVENT_HANDLER handler, void *data );


int  CVICALLBACK cb_mirror_stepper_cal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_close_cal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_init(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_save(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_posoffset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_posoffset1(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_posoffset2(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_posoffset3(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_posoffset4(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_set_default(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_setparam(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_mirror_stepper_setpos(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbaddress(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbaddressok(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#endif

 
