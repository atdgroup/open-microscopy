#ifndef __SCANNER__
#define __SCANNER__

#include "HardWareTypes.h"   
#include "HardWareDevice.h" 

#include "gci_ui_module.h"
#include "signals.h"
#include "dictionary.h"

////////////////////////////////////////////////////////////////////////////
//RJL/RGN April/May 2007
//GCI HTS Microscope system. 
//Scanner control.
////////////////////////////////////////////////////////////////////////////

#define ENABLE_SCANNER_STATUS_POLLING

#define SCANNER_SUCCESS 0
#define SCANNER_ERROR -1

#define DEFAULT_SHIFT 2047
#define DEFAULT_HYST_OFFSET 64

#define SCANNER_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define SCANNER_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_SCANNER_VTABLE_PTR(ob, member) if(SCANNER_VTABLE_PTR(ob, member) == NULL) { \
    send_scanner_error_text(ob, "member not implemented"); \
    return SCANNER_ERROR; \
}  

#define CALL_SCANNER_VTABLE_PTR(ob, member) if( SCANNER_VTABLE(ob, member)(ob) == SCANNER_ERROR ) { \
	send_scanner_error_text(ob, "member failed");  \
	return SCANNER_ERROR; \
}

enum {SCAN_DIR_REVERSE = 0, SCAN_DIR_NORMAL = 1};

enum {VERY_FAST_SCAN=1, FAST_SCAN, NORMAL_SCAN, SLOW_SCAN, VERY_SLOW_SCAN, SLOWEST_SCAN};

enum {SCANNER_ZOOM_X1=1, SCANNER_ZOOM_X2=2, SCANNER_ZOOM_X5=5,
		SCANNER_ZOOM_X10=10, SCANNER_ZOOM_X20=20, SCANNER_ZOOM_PARK=0};

enum {EXTERNAL_CLOCK, INTERNAL_CLOCK};

typedef struct
{
	int (*hw_init) (Scanner* scanner);    
	int (*destroy) (Scanner* scanner);
	int (*scanner_select_clock) (Scanner* scanner, int pixclk, int lineclk, int frameclk);
	int (*scanner_load_line_data) (Scanner* scanner, int line); 
	int (*scanner_set_rep_rate) (Scanner* scanner, int rep_val,int rep_div_1);
	int (*scanner_set_dwell_time) (Scanner* scanner, int dwell,int clockSelect);
	int (*scanner_set_speed) (Scanner* scanner, int speed, int hyst_offset);
	int (*scanner_set_resolution) (Scanner *scanner, int resolution);
	int (*scanner_set_zoom) (Scanner *scanner, int zoom);
	int (*scanner_set_x_shift) (Scanner *scanner, int x_shift);
	int (*scanner_get_min_max_x_shift) (Scanner *scanner, int *min, int *max);
	int (*scanner_set_y_shift) (Scanner *scanner, int y_shift);
	int (*scanner_get_min_max_y_shift) (Scanner *scanner, int *min, int *max);
	int (*scanner_reverse_scan) (Scanner *scanner, int direction);
	int (*scanner_line_scan) (Scanner *scanner, int enable);
	int (*scanner_set_no_frames) (Scanner *scanner, int frames);
	int (*scanner_get_frame_time) (Scanner *scanner, double *frame_time);
	int (*scanner_get_line_time) (Scanner *scanner, double *time);
	int (*scanner_get_pixel_time) (Scanner *scanner, double *time);
	int (*scanner_get_spc_left_position) (Scanner *scanner, int res, int *pos);
	int (*scanner_start_scan) (Scanner *scanner, int frames);
	int (*scanner_stop_scan) (Scanner *scanner);
	int (*scanner_disable_scanner) (Scanner *scanner, int disable);
	int (*scanner_save_settings_to_EEPROM) (Scanner *scanner);
	int (*scanner_read_error_signal) (Scanner *scanner, int *scannerServoError);    
    int (*load_settings) (Scanner *scanner, const char *filepath);     
	int (*save_settings) (Scanner *scanner, const char *filepath, const char *flags);     
	
} ScannerVtbl;


struct _Scanner {
 
  HardwareDevice parent; 
  
  ScannerVtbl vtable;

  int		 _initialised;
  int	 	 _mounted;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int 		 _cal_ui_panel ;
  int		 _speed;
  int		 _hyst_offset;    // really the interline overscan
  int		 _x_offset;
  int		 _y_offset;
  int		 _resolution;
  int		 _zoom;
  int		 _x_shift;
  int		 _y_shift;
  int		 _pixel_clock;
  int		 _line_clock;
  int		 _frame_clock;
  int		 _reverse_scan;
  int		 _scan_disable;
  int		 _line_scan;
  int		 _frames;
  int		 _scanning;
  int		 _servo_error;
  double	 _scan_time;
  double	 _start_time;

  char		 _global_settings_file[100];
};


Scanner* scanner_new(char *name, char *description, const char *data_dir, const char *data_file, size_t size);

int scanner_initialise(Scanner* scanner);      
int scanner_hardware_initialise(Scanner* scanner);

int scanner_is_hardware_initialised(Scanner* scanner);

int  send_scanner_error_text (Scanner* scanner, char fmt[], ...);

void scanner_set_error_handler(Scanner* scanner, UI_MODULE_ERROR_HANDLER handler, void *callback_data);


int  scanner_set_com_port(Scanner* scanner, int port);
int  scanner_set_description(Scanner* scanner, const char* description);
int  scanner_get_description(Scanner* scanner, char *description);
int  scanner_set_name(Scanner* scanner, char* name);
int  scanner_get_name(Scanner* scanner, char *name);

dictionary* scanner_add_metadata(Scanner* scanner, dictionary *d);

int  scanner_destroy(Scanner* scanner);

int scanner_set_init_values(Scanner* scanner);
int scanner_select_clock(Scanner* scanner, int pixclk, int lineclk, int frameclk);
int scanner_load_line_data(Scanner* scanner, int line); 
int scanner_set_rep_rate(Scanner* scanner, int rep_val,int rep_div_1);
int scanner_set_speed(Scanner* scanner, int speed, int hyst_offset);
int scanner_get_speed(Scanner* scanner, int *speed);
int scanner_get_hyst_offset(Scanner* scanner, int *offset);
int scanner_get_spc_left_position (Scanner *scanner, int res, int *pos);
int scanner_set_resolution(Scanner *scanner, int resolution);
int scanner_get_resolution_index(Scanner* scanner, int *resolution_index);
int scanner_get_resolution(Scanner* scanner);
int scanner_set_zoom(Scanner *scanner, int zoom);
int scanner_get_zoom_index(Scanner* scanner, int *zoom);
int scanner_get_zoom(Scanner* scanner);					 //x1, x2, x5, x10, x20 or 0 = park
int scanner_get_line_scan(Scanner* scanner);			 //1=line scan, 0=xy scan
int scanner_reset_shifts(Scanner *scanner);
int scanner_set_shifts_to_centre(Scanner* scanner);
int scanner_set_x_shift(Scanner *scanner, int x_shift);
int scanner_set_y_shift(Scanner *scanner, int y_shift);
int scanner_get_x_shift(Scanner* scanner);
int scanner_get_y_shift(Scanner* scanner);
int scanner_get_min_max_x_shift (Scanner *scanner, int *min, int *max);
int scanner_get_min_max_y_shift (Scanner *scanner, int *min, int *max);
int scanner_reverse_scan(Scanner *scanner, int direction);
int scanner_line_scan(Scanner *scanner, int enable);
int scanner_set_no_frames(Scanner *scanner, int frames);
int scanner_get_frame_time(Scanner* scanner, double *time);
int scanner_get_line_time(Scanner* scanner, double *time);
int scanner_get_pixel_time(Scanner* scanner, double *time);
int scanner_start_scan(Scanner *scanner, int frames);
int scanner_stop_scan(Scanner *scanner);
int scanner_disable_scanner(Scanner *scanner, int disable);
int scanner_save_settings_to_EEPROM(Scanner *scanner);
int scanner_read_error_signal(Scanner *scanner, int *scannerServoError);

void scanner_disable_timer(Scanner* scanner);
void scanner_enable_timer(Scanner* scanner);
void scanner_on_change(Scanner* scanner, int control); 

int  scanner_display_main_ui (Scanner* scanner);
int  scanner_hide_main_ui (Scanner* scanner);
int  scanner_is_main_ui_visible (Scanner* scanner);

int scanner_save_settings(Scanner* scanner, const char *filename, const char *flags);
int scanner_load_settings(Scanner* scanner, const char *filename);
int scanner_save_settings_as_default(Scanner* scanner);
int scanner_load_default_settings(Scanner* scanner);

// Signals
typedef void (*SCANNER_EVENT_HANDLER) (Scanner* scanner, void *data); 
typedef void (*SCANNER_EVENT_INT_HANDLER) (Scanner* scanner, int value, void *data);
typedef void (*SCANNER_ZOOM_EVENT_HANDLER) (Scanner* scanner, int old_zoom, int new_zoom, void *data);

int scanner_signal_hide_handler_connect (Scanner* scanner, SCANNER_EVENT_HANDLER handler, void *callback_data);
int scanner_changed_handler_connect(Scanner* scanner, SCANNER_EVENT_INT_HANDLER handler, void *data );
int scanner_zoomed_changed_handler_connect(Scanner* scanner, SCANNER_ZOOM_EVENT_HANDLER handler, void *data );
int scanner_zoomed_pre_changed_handler_connect(Scanner* scanner, SCANNER_ZOOM_EVENT_HANDLER handler, void *data );
int scanner_resolution_pre_changed_handler_connect(Scanner* scanner, SCANNER_EVENT_INT_HANDLER handler, void *data );
int scanner_shift_changed_handler_connect(Scanner* scanner, SCANNER_EVENT_HANDLER handler, void *data );

int  CVICALLBACK cbcalclose_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbdefine_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbframe_num(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbhyst_offset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnOffsetChanged (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbline_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbload_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbquit_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbresolution(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbrev_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbsave_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbscan_disable(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbselect_clock(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbshift_reset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbspeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbstart_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbstop_scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbx_shift(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cby_shift(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbzoom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnScannerTimerTick(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
