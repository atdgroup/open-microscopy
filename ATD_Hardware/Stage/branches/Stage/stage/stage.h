#ifndef __GCI_STAGE__
#define __GCI_STAGE__

#include "signals.h"
#include "focus_drive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <userint.h>

/////////////////////////////////////////////////////////////////////////////
// XYZ stage module for Marzhauser stage - GP & RJL Jan 2006
//
/////////////////////////////////////////////////////////////////////////////

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//#define XY_ONLY		//define if separate focus drive

#define STAGE_SUCCESS 0
#define STAGE_ERROR -1

//1 rev of the z axis is 100um. However we set z pitch to 1mm otherwise it can't
//move as fast for some reason. This means we have to adjust z moves and readings. 
#define F_FACTOR 10.0
//#define F_FACTOR 1.0
#define Z_STAGE_MIN -25000.0
#define Z_STAGE_MAX 25000.0

#define DEFAULT_SOFTWARE_LIMIT 2000.0

#define CAL_VEL_TOWARDS_XY	10.0		//Initial velocity for xy calibration rev/sec towards limits
#define CAL_VEL_AWAY_XY		2.0			//Initial velocity for xy calibration rev/sec away from limits

typedef struct {
	double min_x;
	double min_y;
	double min_z;
	double max_x;
	double max_y;
	double max_z;
} Roi;


typedef struct {
	double x;
	double y;
	double z;
} Coord;

//Define region of safe travel
typedef struct {
	int coerce;
	int shape;
	double ox;
	double oy;
	double gx;
	double gy;
	double cx;
	double cy;
	double radius;
} safeRegion;

typedef enum {XAXIS=1, YAXIS, ZAXIS, ALL_AXIS} Axis;
typedef enum {SAFE=1, FAST} MoveType;
typedef enum {RECTANGLE2, CIRCLE} shape;

typedef struct _Stage Stage;

struct _Stage {

  struct stage_operations *_cops;
 
  int 	 _lock;
  int	 _abort_move;
  int    _timer;
  int	 _baud_rate;
  int	 _move_type;
  int	 _advanced_view;
  double _software_limit;
  double _cal_speed_1;
  double _cal_speed_2;
  double _speed[5];
  double _default_speed[5];
  double _acceleration[5];
  double _backlash[5];
  double _pitch[5];
  double _motor_current[5];
  int	 _current_reduction[5];
  double _joystick_speed;
  char	*_name;
  char	*_description;
  char	*_data_dir;
  int	 _port;
  char	 _port_string[10];
  int	 _show_errors;
  int	 _no_errors;
  int 	 _joystick_status;
  int	 _main_ui_panel;
  int	 _params_ui_panel;
  int	 _powered_up;
  int	 _enabled_axis[5];
  int	 _dir[5];
  int 	 _init_aborted;
  Roi	 _limits;
  Roi	 _roi;
  Point  _original_origin;
  Coord	 _datum;
  Coord	 _current_pos;
  double _focus_offset;
  safeRegion  _safe_region;
  
  FocusDrive* focus_drive;
  
  signal_table signal_table;
  
  void (*_error_handler) (char *error_string, Stage* stage);  
};


/* Function pointers used as virtual functions */
struct stage_operations
{
	int (*power_up) (Stage* stage);
	int (*power_down) (Stage* stage);
	
	// Don't put stage functions in init (error_handler is not yet set)
	int (*init) (Stage* stage);
	int (*destroy) (Stage* stage);
	int (*reset) (Stage* stage);
	int (*set_baud_rate) (Stage* stage, int baud_rate);
	int (*self_test) (Stage* stage);
	int (*enable_axis) (Stage* stage, Axis axis);
	int (*disable_axis) (Stage* stage, Axis axis);
	int (*set_pitch) (Stage* stage, Axis axis, double pitch);
	int (*set_speed) (Stage* stage, Axis axis, double speed);
	int (*get_speed) (Stage* stage, Axis axis, double *speed);
	int (*calibrate_extents) (Stage* stage, int full, double *min_x, double *min_y, double *max_x, double *max_y); 
	int (*set_timeout) (Stage* stage, double timeout);
	int (*set_joystick_on) (Stage* stage);
	int (*set_joystick_off) (Stage* stage);
	int (*set_joystick_speed) (Stage* stage, double speed);
	int (*set_acceleration) (Stage* stage, Axis axis, double acceleration);
	int (*get_acceleration) (Stage* stage, Axis axis, double *acceleration);
	int (*get_xyz_position) (Stage* stage, double *x, double *y, double *z);
	int (*async_goto_xyz_position) (Stage* stage, double x, double y, double z);
	int (*async_goto_xy_position) (Stage* stage, double x, double y);
	int (*async_goto_x_position) (Stage* stage, double x);
	int (*async_goto_y_position) (Stage* stage, double y);
	int (*async_goto_z_position) (Stage* stage, double z);
	int (*async_rel_move_by) (Stage* stage, double x, double y, double z);
	int (*abort_move) (Stage* stage);
	int (*send_command) (Stage* stage, char* command);
	int (*get_response) (Stage* stage, char* response);
	int (*set_xyz_datum) (Stage* stage, double x, double y, double z);
	int (*set_xy_datum) (Stage* stage, double x, double y);
	int (*is_moving) (Stage* stage, int *status);
	int (*get_info) (Stage* stage, char *text);
	int (*save_settings) (Stage* stage, const char *filepath);
	int (*load_settings) (Stage* stage, const char *filepath);
	int (*set_port_timeout) (Stage* stage, double timeout);
};


/* Functions which operate upon seperate stage instances */
Stage* stage_new(void);

void stage_dim_controls(Stage *stage, int dim);
int  send_stage_error_text (Stage* stage, char fmt[], ...);
void stage_set_operations(Stage* stage, struct stage_operations* cops);
void stage_set_error_handler( Stage* stage, void (*handler) (char *error_string, Stage *stage) );
int  stage_check_user_has_aborted(Stage *stage);
void stage_set_move_type(Stage* stage, int type);
void stage_get_move_type(Stage* stage, int *type);
int  stage_reset(Stage* stage);
void stage_stop_timer(Stage* stage);
void stage_start_timer(Stage* stage);
int  stage_destroy(Stage* stage);
int  stage_set_port_timeout(Stage* stage, double timeout);
int  stage_show_errors(Stage* stage, int val);

int stage_enable_focus_drive_callback(Stage* stage, int enable);

int  stage_self_test(Stage* stage);
int  stage_init (Stage* stage);
int  stage_find_initialise_extents (Stage* stage, int full);
int  stage_set_timeout (Stage* stage, double timeout);
int  stage_enable_axis (Stage* stage, Axis axis);
int	 stage_disable_axis (Stage* stage, Axis axis);
int	 stage_is_axis_enabled (Stage* stage, Axis axis);

int  stage_set_min_z(Stage* stage, double z);
int  stage_set_max_z(Stage* stage, double z);
int  stage_set_port (Stage* stage, int port);

int  stage_wait_for_stop_moving(Stage *stage, double timeout);
int  stage_get_port_string (Stage* stage, char *port_str);
int  stage_set_baud_rate (Stage* stage, int baud_rate);
int  stage_get_baud_rate (Stage* stage, int *baud_rate);
int  stage_set_data_dir(Stage* stage, const char *dir);
int  stage_get_data_dir(Stage* stage, char* dir); 
int  stage_set_description(Stage* stage, const char* description);
int  stage_get_description(Stage* stage, char *description);
int  stage_set_name(Stage* stage, char* name);
int  stage_get_name(Stage* stage, char *name);

int  stage_power_up(Stage* stage);
int  stage_is_powered_up(Stage* stage);
int  stage_power_down(Stage* stage);
int  stage_calibrate_extents (Stage* stage, int full, double *min_x, double *min_y, double *max_x, double *max_y);
int  stage_get_speed (Stage* stage, Axis axis, double *speed);
int  stage_set_speed (Stage* stage, Axis axis, double speed);
int  stage_set_default_speed (Stage* stage);
int  stage_set_pitch (Stage* stage, Axis axis, double pitch);
void stage_get_pitch(Stage* stage, Axis axis, double *pitch);
int  stage_get_joystick_speed (Stage* stage, double *speed);
int  stage_set_joystick_speed (Stage* stage, double speed);
int  stage_get_acceleration (Stage* stage, Axis axis, double *acceleration);
int  stage_set_acceleration (Stage* stage, Axis axis, double acceleration);
void stage_set_z_tolerance (Stage* stage, int tolerance);
void stage_get_z_tolerance (Stage* stage, int *tolerance);
void stage_set_z_speed (Stage* stage, int speed);
void stage_get_z_speed (Stage* stage, int *speed);

int  stage_async_goto_x_position (Stage* stage, double x);
int  stage_async_goto_y_position (Stage* stage, double y);
int  stage_async_goto_z_position (Stage* stage, double z);
int  stage_async_goto_xy_position (Stage* stage, double x, double y);
int  stage_async_goto_xyz_position (Stage* stage, double x, double y, double z);
int  stage_goto_x_position (Stage* stage, double x);
int  stage_goto_y_position (Stage* stage, double y);
int  stage_goto_z_position (Stage* stage, double z);
int  stage_goto_xy_position (Stage* stage, double x, double y);
int  stage_goto_xyz_position (Stage* stage, double x, double y, double z, double xCur, double yCur, double zCur);
int  stage_get_x_position (Stage* stage, double *x);
int  stage_get_y_position (Stage* stage, double *y);
int  stage_get_z_position (Stage* stage, double *z);
int  stage_get_xy_position (Stage* stage, double *x, double *y);
int  stage_get_xyz_position (Stage* stage, double *x, double *y, double *z);
int  stage_fast_get_xyz_position(Stage* stage, double *x, double *y, double *z);

//int  stage_set_x_datum (Stage* stage, double x);
//int  stage_set_y_datum (Stage* stage, double y);
int  stage_goto_z_datum (Stage* stage);
int  stage_set_z_datum (Stage* stage);
int  stage_set_xy_datum (Stage* stage);
int  stage_set_xyz_datum (Stage* stage);
int  stage_get_x_datum (Stage* stage, double *x);
int  stage_get_y_datum (Stage* stage, double *y);
int  stage_get_z_datum (Stage* stage, double *z);
int  stage_get_xyz_datum (Stage* stage, double *x, double *y, double *z);

void  stage_set_z_offset (Stage* stage, double z);

int  stage_async_rel_move_by (Stage* stage, double x, double y, double z);
int  stage_rel_move_by (Stage* stage, double x, double y, double z);
int  stage_rel_move_by_z (Stage* stage, double z);
int  stage_is_moving (Stage* stage, int *status);

int  stage_set_current_position(Stage* stage, double x, double y, double z);
int  stage_plot_current_position(Stage* stage);
int  stage_get_roi (Stage* stage, Roi *roi);
int  stage_set_roi (Stage* stage, Roi roi);

int  stage_set_joystick_on (Stage* stage);
int  stage_set_joystick_off (Stage* stage);
int  stage_get_joystick_status (Stage* stage);

int  stage_abort_move (Stage* stage);
int  stage_send_command (Stage* stage, char* command);
int  stage_get_response (Stage* stage, char* response);
int  stage_get_info (Stage* stage, char *text); 
int  stage_save_limits (Stage* stage);
int  stage_load_limits (Stage* stage);
int  stage_display_main_ui(Stage* stage);
int  stage_hide_main_ui(Stage* stage);
int  stage_display_params_ui(Stage* stage);
int  stage_hide_params_ui(Stage* stage);
int  stage_destroy_params_ui(Stage* stage);
int  stage_hide_ui(Stage* stage);
int  stage_save_settings(Stage* stage, const char *filepath);
int  stage_load_settings(Stage* stage, const char *filepath);
int  stage_load_default_settings(Stage* stage);
int  stage_save_settings_as_default(Stage* stage);
int  stage_send_all_params(Stage *stage);
int  stage_read_all_params(Stage *stage);

void stage_set_safe_region(Stage *stage, safeRegion safe_region);

int stage_save_parameters(const char *filename, Stage *stage);
int stage_load_parameters(const char *filename, Stage *stage);
void stage_load_default_parameters(Stage *stage);

/* Signal Connection */
typedef void (*STAGE_EVENT_HANDLER) (Stage* stage, void *data);

int stage_signal_close_handler_connect (Stage* stage, STAGE_EVENT_HANDLER handler, void *callback_data);
int stage_signal_XYZ_change_handler_connect (Stage* stage, STAGE_EVENT_HANDLER handler, void *callback_data);
int stage_signal_initialise_extents_start_handler_connect (Stage* stage, STAGE_EVENT_HANDLER handler, void *callback_data); 
int stage_signal_initialise_extents_end_handler_connect (Stage* stage, STAGE_EVENT_HANDLER handler, void *callback_data); 

#endif
