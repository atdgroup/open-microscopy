#ifndef __GCI_STAGE__
#define __GCI_STAGE__

#include "HardWareTypes.h"
#include "HardWareDevice.h" 

#include "gci_ui_module.h"
#include "iniparser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <userint.h>

/////////////////////////////////////////////////////////////////////////////
// XY stage module GP & RJL Jan 2006
//
/////////////////////////////////////////////////////////////////////////////

#define STAGE_SUCCESS 0
#define STAGE_ERROR -1

#define STAGE_LONG_TIMEOUT	20.0
#define STAGE_NORMAL_TIMEOUT 5.0

// All position and move values are given in um
// The range of the stage is set by variable limit switches
// Backlash (X and Y directions) - always approach from the origin direction, ie increasing values of distance

//1 rev of the z axis is 100um. However we set z pitch to 1mm otherwise it can't
//move as fast for some reason. This means we have to adjust z moves and readings. 
#define F_FACTOR 10.0

#define DEFAULT_SOFTWARE_LIMIT 2000.0

#define CAL_VEL_TOWARDS_XY	10.0		//Initial velocity for xy calibration rev/sec towards limits
#define CAL_VEL_AWAY_XY		2.0			//Initial velocity for xy calibration rev/sec away from limits

#define STAGE_CAST(obj) ((XYStage *) (obj)) 

#define STAGE_VTABLE(obj, member) STAGE_CAST(obj)->vtbl.member
#define STAGE_VCALL(obj, member) (*(STAGE_CAST(obj)->vtbl.member))                     

#define STAGE_CORRECT_VALS_FOR_XY_ORIENTATION(stage, x, y) \
		{ \
		if(((x) != 0)) \
			(x) *= stage_get_axis_dir(stage, XAXIS); \
		if(((y) != 0)) \
			(y) *= stage_get_axis_dir(stage, YAXIS); \
		}

#define STAGE_CORRECT_VALS_FOR_XYZ_ORIENTATION(stage, x, y, z) \
		{ \
		if(((x) != 0)) \
			(x) *= stage_get_axis_dir(stage, XAXIS); \
		if(((y) != 0)) \
			(y) *= stage_get_axis_dir(stage, YAXIS); \
		if(((z) != 0)) \
			(z) *= stage_get_axis_dir(stage, ZAXIS); \
		}


#define STAGE_CORRECT_VAL_FOR_ORIENTATION(stage, v, axis) \
    ((v) *= stage_get_axis_dir(stage, axis))

#define DEFAULT_STAGE_FILENAME_SUFFIX "StageSettings.ini"          
#define DEFAULT_STAGE_USER_FILENAME_SUFFIX "StageUserSettings.ini"          

typedef struct {
	double x;
	double y;
	double z;
} Coord;

typedef struct {
	double min_x;
    double max_x;
	double min_y;
	double max_y;
	double min_z;
	double max_z;
} Roi;

#define AXIS_DATA_ARRAY_SIZE 4	   // needs to be 4 for incase of 3 axes (axies are 1-indexed)

typedef enum {ALL_AXIS=-1, XAXIS=1, YAXIS=2, ZAXIS=3} Axis;
typedef enum {STAGE_POSITIVE_TO_NEGATIVE=-1, STAGE_NEGATIVE_TO_POSITIVE=1} StageDirection; 
typedef enum {STAGE_SHAPE_RECTANGLE, STAGE_SHAPE_CIRCLE} StageShape;

//Define region of safe travel
typedef struct
{
	StageShape shape;
	Roi roi;
	double radius;
    Coord center;
    
} SafeRegion;

typedef struct _XYStage XYStage;

/* Function pointers used as virtual functions */
struct vtable
{
	// Don't put stage functions in init (error_handler is not yet set)
	int (*hw_init) (XYStage* stage); 
	int (*init) (XYStage* stage);
	int (*destroy) (XYStage* stage);
	int (*enable_speed_over_accuracy) (XYStage* stage, Axis axis);      
	int (*set_timeout) (XYStage* stage, double timeout);  
	int (*set_pitch) (XYStage* stage, Axis axis, double pitch);
	int (*get_pitch) (XYStage* stage, Axis axis, double *pitch); 
	int (*set_speed) (XYStage* stage, Axis axis, double speed);
	int (*get_speed) (XYStage* stage, Axis axis, double *speed);
	int (*calibrate_extents) (XYStage* stage, double *min_x, double *min_y,
		double *max_x, double *max_y); 
	int (*set_joystick_on) (XYStage* stage);
	int (*set_joystick_off) (XYStage* stage);
	int (*set_joystick_speed) (XYStage* stage, double speed);
	int (*get_joystick_speed) (XYStage* stage, double *speed);
	int (*set_acceleration) (XYStage* stage, Axis axis, double acceleration);
	int (*get_acceleration) (XYStage* stage, Axis axis, double *acceleration);
	int (*get_xyz_position) (XYStage* stage, double *x, double *y, double *z);
	int (*goto_xy_position) (XYStage* stage, double x, double y);
	int (*goto_xyz_position) (XYStage* stage, double x, double y, double z);
	int (*async_goto_xy_position) (XYStage* stage, double x, double y);
	int (*async_goto_xyz_position) (XYStage* stage, double x, double y, double z);
	int (*async_rel_move_by) (XYStage* stage, double x, double y);
	int (*abort_move) (XYStage* stage);
	int (*set_xy_datum) (XYStage* stage, double x, double y);
    int (*set_controller_safe_region) (XYStage* stage, Roi roi);
	int (*is_moving) (XYStage* stage, int *status);
	int (*get_info) (XYStage* stage, char *text);
    int (*save_settings) (XYStage* stage, const char *filepath);
	int (*load_settings) (XYStage* stage, const char *filepath);
};

struct _XYStage
{
  HardwareDevice parent; 
	
  struct vtable vtbl;
 
  StagePlateModule *_stage_plate_module;

  int	 _stage_plate_signal_plate_changed_handler_signal_id;
  int	 _do_not_initialise;
  int	 _abort_move;
  int	 _log_errors;
  int    _timer;
  int	 _advanced_view;
  int    _safe_region_enabled;
  int	 _enabled_axis[AXIS_DATA_ARRAY_SIZE];
  int	 _prevent_change_signal_emit;
  double _software_limit;
  double _default_speed[AXIS_DATA_ARRAY_SIZE];
  double _speed[AXIS_DATA_ARRAY_SIZE];
  double _acceleration[AXIS_DATA_ARRAY_SIZE];
  double _timeout;
  double _x_pos_at_last_shutdown;
  double _y_pos_at_last_shutdown;
  double _joystick_speed;
  int	 _no_errors;
  int 	 _joystick_status;
  int	 _main_ui_panel;
  int	 _params_ui_panel;
  int	 _init_ui_panel;
  int 	 _init_aborted;
  int 	 _axis_dir[AXIS_DATA_ARRAY_SIZE];
  int	 _initialised;
  int	 _hw_initialised; 
  int	 _closed_loop_enabled;
  int	 _use_cached_position;
  Roi	 _limits;
  Roi	 _roi;
  Point  _original_origin;
  Coord	 _datum;
  Coord	 _current_pos;
  SafeRegion  _safe_region; 
  
};

/* Functions which operate upon seperate stage instances */

void stage_calc_move_time_from_vars(double dist, double speed, double accel, double *t);
void stage_calc_move_time(XYStage* stage, double x_dist, double y_dist, double *t);

void stage_constructor(XYStage *stage, const char *name);
void stage_set_stage_plate_module(XYStage *stage, StagePlateModule *stage_plate_module);
int stage_is_initialised(XYStage *stage);
int stage_is_hardware_initialised(XYStage *stage);
void stage_use_cached_data_for_read(XYStage *stage, int val);
int  stage_log_errors(XYStage* stage, int val); 
int  send_stage_error_text (XYStage* stage, char fmt[], ...); 
void stage_prevent_changed_signal_emission(XYStage* stage);
void stage_allow_changed_signal_emission(XYStage* stage);
int  stage_check_user_has_aborted(XYStage *stage);
void stage_stop_timer(XYStage* stage);
void stage_start_timer(XYStage* stage);
int  stage_destroy(XYStage* stage);
int  stage_self_test(XYStage* stage);
int  stage_hardware_init (XYStage* stage); 
int  stage_init (XYStage* stage);
int  stage_find_initialise_extents (XYStage* stage, int full);
int  stage_position_is_within_safe_xy_limits(XYStage* stage, double x,double y); 
int  stage_is_within_safe_xyz_region(XYStage* stage, double x, double y, double z);
int  stage_position_is_within_xy_limits(XYStage *stage, double x, double y); 
int  stage_is_position_is_within_xyz_limits(XYStage *stage, double x, double y, double z);
int  stage_wait_for_stop_moving(XYStage *stage);
int  stage_calibrate_extents (XYStage* stage, double *min_x, double *min_y, double *max_x, double *max_y);
int  stage_get_speed (XYStage* stage, Axis axis, double *speed);
int  stage_set_speed (XYStage* stage, Axis axis, double speed);
int  stage_set_default_speed (XYStage* stage);
int  stage_set_pitch (XYStage* stage, Axis axis, double pitch);
int  stage_get_pitch(XYStage* stage, Axis axis, double *pitch);
int  stage_get_joystick_speed (XYStage* stage, double *speed);
int  stage_set_joystick_speed (XYStage* stage, double speed);
int  stage_get_acceleration (XYStage* stage, Axis axis, double *acceleration);
int  stage_set_acceleration (XYStage* stage, Axis axis, double acceleration);
int  stage_async_goto_x_position (XYStage* stage, double x);
int  stage_async_goto_y_position (XYStage* stage, double y);
int  stage_async_goto_xy_position (XYStage* stage, double x, double y);
int  stage_async_goto_xyz_position (XYStage* stage, double x, double y, double z);
int  stage_goto_x_position (XYStage* stage, double x);
int  stage_goto_y_position (XYStage* stage, double y);
int  stage_goto_z_position (XYStage* stage, double z);
int  stage_goto_xy_position (XYStage* stage, double x, double y);
int  stage_goto_xyz_position (XYStage* stage, double x, double y, double z);
int  stage_get_x_position (XYStage* stage, double *x);
int  stage_get_y_position (XYStage* stage, double *y);
int  stage_get_z_position (XYStage* stage, double *z);
int  stage_get_xy_position (XYStage* stage, double *x, double *y);
int  stage_get_xyz_position (XYStage* stage, double *x, double *y, double *z);
int  stage_get_xyz_position_without_orientation_correction (XYStage* stage, double *x, double *y, double *z);
int  stage_get_xy_position_without_orientation_correction (XYStage* stage, double *x, double *y);
int  stage_set_xy_datum (XYStage* stage, double x, double y);
int  stage_set_current_xy_position_as_xy_datum (XYStage* stage);  
int  stage_get_xy_datum (XYStage* stage, double *x, double *y);
int  stage_async_rel_move_by (XYStage* stage, double x, double y);
int  stage_rel_move_by (XYStage* stage, double x, double y);
void stage_adjust_position_relative_to_top_left(XYStage *stage, double *x, double *y);
int  stage_is_moving (XYStage* stage, int *status);
int  stage_get_roi (XYStage* stage, Roi *roi);
int  stage_set_roi (XYStage* stage, Roi roi);
int  stage_set_joystick_on (XYStage* stage);
int  stage_set_joystick_off (XYStage* stage);
int  stage_get_joystick_status (XYStage* stage);
int  stage_abort_move (XYStage* stage);
int  stage_send_command (XYStage* stage, char* command);
int  stage_get_response (XYStage* stage, char* response);
int  stage_get_info (XYStage* stage, char *text); 
int  stage_save_limits (XYStage* stage);
int  stage_load_limits (XYStage* stage);
int  stage_display_main_ui(XYStage* stage);
int  stage_hide_main_ui(XYStage* stage);
int  stage_display_params_ui(XYStage* stage);
int  stage_hide_params_ui(XYStage* stage);
int  stage_destroy_params_ui(XYStage* stage);
int  stage_hide_ui(XYStage* stage);
int  stage_set_timeout (XYStage* stage, double timeout);      

void stage_set_axis_direction(XYStage* stage, Axis axis, StageDirection dir);
StageDirection stage_get_axis_dir(XYStage* stage, Axis axis);

int stage_check_is_within_xy_limits_and_safe_region(XYStage *stage, double x, double y);
void stage_set_safe_region_circle(XYStage* stage, double radius, Coord center);
void stage_set_safe_region_rectangle(XYStage* stage, Roi roi);
void stage_set_safe_region_rectangle_percentage_smaller_than_limits(XYStage* stage, double percentage);

Roi stage_get_limits(XYStage* stage);

int stage_save_data_to_dictionary(XYStage *stage, dictionary *d);
int stage_load_data_from_dictionary(XYStage *stage, dictionary *d);
int  stage_save_settings(XYStage* stage, const char *filepath);
int  stage_load_settings(XYStage* stage, const char *filepath);
int stage_save_settings_as_default(XYStage* stage);
int stage_load_default_settings(XYStage* stage);
int stage_save_user_settings_as_default(XYStage* stage);
int stage_save_user_data_to_dictionary(XYStage *stage, dictionary *d);
int stage_load_data_from_dictionary(XYStage *stage, dictionary *d);
int stage_load_user_data_from_dictionary(XYStage *stage, dictionary *d);

/* Signal Connection */
typedef void (*STAGE_EVENT_HANDLER) (XYStage* stage, void *data);
typedef void (*STAGE_XY_EVENT_HANDLER) (XYStage* stage, double x, double y, void *data);

int stage_signal_close_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data);
int stage_signal_change_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data);
int stage_signal_xy_changed_handler_connect (XYStage* stage, STAGE_XY_EVENT_HANDLER handler, void *callback_data);
int stage_signal_xy_changed_handler_disconnect  (XYStage* stage, int id);
int stage_signal_initialise_extents_start_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data); 
int stage_signal_initialise_extents_end_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data); 
int stage_signal_wait_for_stop_completion_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data);
void stage_signal_wait_for_stop_completion_handler_disconnect (XYStage* stage, int id);

int  CVICALLBACK MoveBottomLeft(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveBottomRight(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveByXY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveLeft(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveRight(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveUp(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveTopLeft(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveTopRight(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveToXY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnAbout(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnGotoXYDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnReinit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSettings(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSetXYDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageAbortInit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageAbortMove(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageAdvancedButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageJoystickSpeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageJoystickToggled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#endif
