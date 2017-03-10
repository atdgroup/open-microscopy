// Do NOT include inherited stages here all implementations should conform to this interface.

#include "HardWareTypes.h" 

#include "StagePlate.h"
#include "stage\stage.h"
#include "profile.h"
#include "stage\stage_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"

#include "iniparser.h"
#include <utility.h>

#include "toolbox.h"
#include "asynctmr.h"
#include <ansi_c.h> 

//Functions to calculate how long the move should take
void stage_calc_move_time_from_vars(double dist, double speed, double accel, double *t)
{
	register double s, d, d_2;
	
	d = fabs(dist);
	d_2 = d/2;

	s = speed*speed/(2*accel);		//distance to reach full speed from rest
	if (s > d_2) 
		*t = 2*sqrt(2*d_2/accel);	//accelerate then decellerate at same rate
	else {
		*t = 2*(speed/accel);		//Time to reach full speed and decellerate
		*t += (d - (2*s))/speed;	//Time to move remaining distance
	}
}

void stage_calc_move_time(XYStage* stage, double x_dist, double y_dist, double *t)
{
	double tx, ty, tmax;
	static double xspeed = 0.0, yspeed = 0.0, xaccel = 0.0, yaccel = 0.0;

	if (x_dist == 0.0 && y_dist == 0.0) {
		*t = 0.0;
		return;
	}

	if(xspeed == 0.0) {

		stage_get_speed(stage, XAXIS, &xspeed);
		xspeed *= 1000.0;

		if (xspeed == 0.0) {
			*t = 10.0;	//Can't calculate it. Assume max move time of 10 seconds
			return;
		}
	}

	if(yspeed == 0.0) {
		stage_get_speed(stage, YAXIS, &yspeed);  
		yspeed *= 1000.0;

		if (yspeed == 0.0) {
			*t = 10.0;	//Can't calculate it. Assume max move time of 10 seconds
			return;
		}
	}

	if(xaccel == 0.0) {
		stage_get_acceleration(stage, XAXIS, &xaccel);   
		xaccel *= 1000.0;

		if (xaccel == 0.0) {
			*t = 10.0;	//Can't calculate it. Assume max move time of 10 seconds
			return;
		}
	}

	if(yaccel == 0.0) {
		stage_get_acceleration(stage, YAXIS, &yaccel);
		yaccel *= 1000.0;

		if (yaccel == 0.0) {
			*t = 10.0;	//Can't calculate it. Assume max move time of 10 seconds
			return;
		}
	}

	stage_calc_move_time_from_vars(x_dist, xspeed, xaccel, &tx);  //mm to um conversion
	stage_calc_move_time_from_vars(y_dist, yspeed, yaccel, &ty);
	tmax = max(tx, ty);
	*t = tmax;
}

static void emit_changed_signal(XYStage* stage)
{
	if(!stage->_prevent_change_signal_emit)
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(stage), "StageChanged", GCI_VOID_POINTER, stage);
}

static void emit_xy_changed_signal(XYStage* stage, double x, double y)
{
	if(!stage->_prevent_change_signal_emit)
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(stage), "XY_Changed", GCI_VOID_POINTER, stage, GCI_DOUBLE, x, GCI_DOUBLE, y);
}

void stage_prevent_changed_signal_emission(XYStage* stage)
{
	stage->_prevent_change_signal_emit = 1;
}

void stage_allow_changed_signal_emission(XYStage* stage)
{
	stage->_prevent_change_signal_emit = 0;
}

int  stage_log_errors(XYStage* stage, int val)
{
	stage->_log_errors = val;
	
	return STAGE_SUCCESS;
}

int send_stage_error_text (XYStage* stage, char fmt[], ...)
{
	va_list ap;
	va_start(ap, fmt);     
	
	ui_module_send_valist_error(UIMODULE_CAST(stage), "Stage Error", fmt, ap);
	
	va_end(ap);  
	
	return STAGE_SUCCESS;
}

int stage_signal_change_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage), "StageChanged", handler, callback_data) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Can not connect signal handler for StageChanged signal");
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
}

int stage_signal_xy_changed_handler_connect (XYStage* stage, STAGE_XY_EVENT_HANDLER handler, void *callback_data)
{
	int id;

	if((id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage), "XY_Changed", handler, callback_data)) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Can not connect signal handler for XY_Changed signal");
		return STAGE_ERROR;
	}

	return id;
}

int stage_signal_xy_changed_handler_disconnect  (XYStage* stage, int id)
{
	if( GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(stage), "XY_Changed", id) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(stage), "Stage", "Can not disconnect signal handler for XY_Changed signal");
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
}


int stage_signal_wait_for_stop_completion_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data)
{
	int id;

	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage), "StageStoppedMoving", handler, callback_data)) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage", "Can not connect signal handler for StageStoppedMoving signal");
		return STAGE_ERROR;
	}

	return id;
}

void stage_signal_wait_for_stop_completion_handler_disconnect (XYStage* stage, int id)
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(stage), "StageStoppedMoving", id);   
}

int stage_signal_initialise_extents_start_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage), "XYInitExtentsStart", handler, callback_data) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Can not connect signal handler for XYInitExtentsStart signal");
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
}

int stage_signal_initialise_extents_end_handler_connect (XYStage* stage, STAGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage), "XYInitExtentsEnd", handler, callback_data) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Can not connect signal handler for XYInitExtentsEnd signal");
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
}

static int STAGE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (XYStage*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (XYStage *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int STAGE_PTR_DOUBLE_DOUBLE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (XYStage*, double, double, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (XYStage *) args[0].void_ptr_data, (double) args[1].double_data, (double) args[2].double_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int stage_graph_display_current_position(XYStage* stage, double x, double y)
{

    SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_POS, x); 
    SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_POS, y);

	DeleteGraphPlot (stage->_main_ui_panel, STAGE_PNL_XY_DISP, -1, VAL_IMMEDIATE_DRAW);

	if (stage->_safe_region_enabled){
		Roi safe_roi = stage->_safe_region.roi;
		if (stage->_safe_region.shape == STAGE_SHAPE_RECTANGLE){
			PlotRectangle (stage->_main_ui_panel, STAGE_PNL_XY_DISP, safe_roi.min_x, safe_roi.min_y,
				safe_roi.max_x, safe_roi.max_y, VAL_RED, VAL_TRANSPARENT);
		}
		else if (stage->_safe_region.shape == STAGE_SHAPE_CIRCLE){
			double x1 = stage->_safe_region.center.x - stage->_safe_region.radius;
			double y1 = stage->_safe_region.center.y - stage->_safe_region.radius;
			double x2 = stage->_safe_region.center.x + stage->_safe_region.radius;
			double y2 = stage->_safe_region.center.y + stage->_safe_region.radius;
			PlotOval (stage->_main_ui_panel, STAGE_PNL_XY_DISP, x1, y1, x2, y2, VAL_RED, VAL_TRANSPARENT);
		}
	}

	PlotPoint (stage->_main_ui_panel, STAGE_PNL_XY_DISP, 0.0, 0.0, VAL_SMALL_SOLID_SQUARE, VAL_BLUE);
	PlotPoint (stage->_main_ui_panel, STAGE_PNL_XY_DISP, x, y, VAL_SMALL_SOLID_SQUARE, VAL_RED);

	return STAGE_SUCCESS;
}


int stage_is_hardware_initialised(XYStage *stage)
{
	return stage->_hw_initialised;
}

int stage_is_initialised(XYStage *stage)
{
	return (stage->_initialised && stage->_hw_initialised);
}

int CVICALLBACK OnStageTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
		{
			XYStage* stage = (XYStage *) callbackData;
			//double x, y;
			double x, y, z;
			int status;
	
			stage_get_xyz_position (stage, &x, &y, &z);    
			stage_graph_display_current_position(stage, x, y);		

			stage_is_moving (stage, &status);  
			SetCtrlVal(stage->_main_ui_panel, STAGE_PNL_ACTIVE, status);
			
			if(x != stage->_current_pos.x || y != stage->_current_pos.y || z != stage->_current_pos.z) {
				stage->_current_pos.x = x;
				stage->_current_pos.y = y;
				stage->_current_pos.z = z;
				emit_changed_signal(stage); 
				emit_xy_changed_signal(stage,stage->_current_pos.x, stage->_current_pos.y);
			}

            break;
		}
    }
    
    return 0;
}

void stage_plate_changed (StagePlateModule* stage_plate_module, int pos, void *data) 
{
	XYStage* stage = (XYStage *) data;
	StagePlate plate;
	Roi roi;

	// No sensible safe region has been enter for the current plate
	if(!stage_current_plate_has_valid_safe_region(stage_plate_module)) {
	
		stage_set_safe_region_rectangle(stage, stage->_limits);
		return;
	}

	stage_plate_get_current_plate(stage_plate_module, &plate);

	roi.min_x = plate.safe_left_top.x;
	roi.min_y = plate.safe_left_top.y;
	roi.max_x = plate.safe_right_bottom.x;
	roi.max_y = plate.safe_right_bottom.y;
	roi.min_z = -DBL_MAX;
	roi.max_z = DBL_MAX;

	if(plate.type == PLATE_DISH) {

		Coord center;
		double radius = roi.max_x - roi.min_x;

		center.x = plate.x_offset;
		center.y = plate.y_offset;
		center.z = 0.0;

		// Set a circular safe region
		stage_set_safe_region_circle(stage, radius, center);
	}
	else {

		stage_set_safe_region_rectangle(stage, roi);
	}

}

void stage_constructor(XYStage *stage, const char* name)
{
	memset(stage, 0, sizeof(XYStage));

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(stage), name); 

	stage_allow_changed_signal_emission(stage);

	stage->_stage_plate_signal_plate_changed_handler_signal_id = -1;
	stage->_stage_plate_module = NULL;
	stage->_closed_loop_enabled = 0;
	stage->_do_not_initialise = 0;
	stage->_log_errors = 1;
	stage->_params_ui_panel = -1;
	stage->_initialised = 0;  
	stage->_hw_initialised = 0;
	stage_set_axis_direction(stage, XAXIS, STAGE_NEGATIVE_TO_POSITIVE);        
	stage_set_axis_direction(stage, YAXIS, STAGE_NEGATIVE_TO_POSITIVE);

	stage->_limits.min_z = -DBL_MAX;
	stage->_limits.max_z = DBL_MAX;

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(stage), "XYInitExtentsStart", STAGE_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(stage), "XYInitExtentsEnd", STAGE_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(stage), "StageChanged", STAGE_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(stage), "XY_Changed", STAGE_PTR_DOUBLE_DOUBLE_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(stage), "StageStoppedMoving", STAGE_PTR_MARSHALLER); 
}

void stage_set_stage_plate_module(XYStage *stage, StagePlateModule* stage_plate_module)
{
	stage->_stage_plate_module = stage_plate_module;

	if(stage->_stage_plate_module != NULL) {
		stage->_stage_plate_signal_plate_changed_handler_signal_id = stage_plate_signal_plate_changed_handler_connect(stage->_stage_plate_module,
																		 stage_plate_changed, stage); // Signal: StagePlateChanged
	}
}

void stage_adjust_position_relative_to_top_left(XYStage *stage, double *x, double *y)
{
	*x = fabs(stage->_limits.min_x) + *x;
	*y = fabs(stage->_limits.min_y) + *y;
}

static int stage_setup_graph(XYStage* stage)
{
	if(stage->_main_ui_panel == 0)
		return STAGE_ERROR;
	
	// Set up graph axes, correcting for direction of travel
	DeleteGraphPlot (stage->_main_ui_panel, STAGE_PNL_XY_DISP, -1, VAL_IMMEDIATE_DRAW);

	SetAxisScalingMode (stage->_main_ui_panel, STAGE_PNL_XY_DISP, VAL_BOTTOM_XAXIS, VAL_MANUAL, stage->_limits.min_x, stage->_limits.max_x);
	SetAxisScalingMode (stage->_main_ui_panel, STAGE_PNL_XY_DISP, VAL_TOP_XAXIS, VAL_MANUAL, stage->_limits.min_x, stage->_limits.max_x);
	
	SetAxisScalingMode (stage->_main_ui_panel, STAGE_PNL_XY_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, stage->_limits.min_y, stage->_limits.max_y);
	SetAxisScalingMode (stage->_main_ui_panel, STAGE_PNL_XY_DISP, VAL_RIGHT_YAXIS, VAL_MANUAL, stage->_limits.min_y, stage->_limits.max_y);
	
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_XY_DISP, ATTR_YREVERSE, 1);

	return STAGE_SUCCESS;
}


int  stage_hardware_init (XYStage* stage)
{
	if(STAGE_VTABLE(stage, hw_init) == NULL) {
    
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Hardware Init operation not implemented for stage");

    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, hw_init)(stage) == STAGE_ERROR ) {
	
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "Stage", "Hardware Init operation failed for stage");  
	
		return STAGE_ERROR;
  	}
	
	stage->_hw_initialised = 1; 
		
	return STAGE_SUCCESS;
}

int stage_init (XYStage* stage)
{
	stage->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(stage), "stage_ui.uir", STAGE_PNL, 1);   
	stage->_init_ui_panel = ui_module_add_panel(UIMODULE_CAST(stage), "stage_ui.uir", INIT_PNL, 0);   
	
	ui_module_set_main_panel_title (UIMODULE_CAST(stage));

	#ifdef SINGLE_THREADED_POLLING
		stage->_timer = NewCtrl(stage->_main_ui_panel, CTRL_TIMER, "", 0, 0);
		
		InstallCtrlCallback (stage->_main_ui_panel, stage->_timer, OnStageTimerTick, stage);
	
		SetCtrlAttribute(stage->_main_ui_panel, stage->_timer, ATTR_INTERVAL, 1.0);  
		SetCtrlAttribute(stage->_main_ui_panel, stage->_timer, ATTR_ENABLED, 0);
	#else
		stage->_timer = NewAsyncTimer (1.0, -1, 0, OnStageTimerTick, stage);
		SetAsyncTimerName(stage->_timer, "Stage");
		SetAsyncTimerAttribute (stage->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
	
 	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_ABORT_MOVE, OnStageAbortMove, stage);
    InstallCtrlCallback (stage->_init_ui_panel, INIT_PNL_ABORT_INIT, OnStageAbortInit, stage);
    InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_QUIT, OnStageQuit, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_ADVANCED, OnStageAdvancedButton, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_JOYSTICK_XY_SPEED, OnStageJoystickSpeed, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_JOYSTICK_ENABLE, OnStageJoystickToggled, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_ABOUT, OnAbout, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_SHOW_SETTINGS, OnSettings, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_INITIALISE, OnReinit, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_XY, MoveByXY, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_XY, MoveToXY, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_SET_DATUM_XY, OnSetXYDatum, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_GOTO_XY_DATUM, OnGotoXYDatum, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_TOP_LEFT, MoveTopLeft, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_TOP_RIGHT, MoveTopRight, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_BOTTOM_LEFT, MoveBottomLeft, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_BOTTOM_RIGHT, MoveBottomRight, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_LEFT, MoveLeft, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_RIGHT, MoveRight, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_UP, MoveUp, stage);
	InstallCtrlCallback (stage->_main_ui_panel, STAGE_PNL_DOWN, MoveDown, stage);
	
	// We dont want to show the full controls ie the advanced panel
//	SetPanelAttribute(stage->_main_ui_panel, ATTR_HEIGHT, 100);    // advanced option always on, has been disabled
	
	if(STAGE_VTABLE(stage, init) != NULL) {
    
    	if( STAGE_VCALL(stage, init)(stage) == STAGE_ERROR ) {
	
			ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Init operation failed for stage");
			return STAGE_ERROR;
  		}
  	}
	
	stage->_initialised = 1; 
	
	return STAGE_SUCCESS;
}


void stage_set_axis_direction(XYStage* stage, Axis axis, StageDirection dir)
{
	// dir should be -1 or +1
	if (dir<=0) dir = STAGE_POSITIVE_TO_NEGATIVE;
	else if (dir>0) dir = STAGE_NEGATIVE_TO_POSITIVE;

	if(axis == XAXIS)
		stage->_axis_dir[XAXIS] = dir;
	
	if(axis == YAXIS)
		stage->_axis_dir[YAXIS] = dir;

	if(axis == ZAXIS)
		stage->_axis_dir[ZAXIS] = dir;
}


StageDirection stage_get_axis_dir(XYStage* stage, Axis axis)
{
	if(axis == XAXIS)
		return 	stage->_axis_dir[XAXIS];
	
	if(axis == YAXIS)
		return stage->_axis_dir[YAXIS];

	if(axis == ZAXIS)
		return stage->_axis_dir[ZAXIS];
	
	// Error 0 not valid StageDirection
	return 0;
}


int stage_check_user_has_aborted(XYStage *stage)
{
	ProcessSystemEvents ();     // Force CVI to look for events, in this case an abort init
			
	return stage->_abort_move;
}

int stage_set_timeout(XYStage *stage, double timeout)
{
	stage->_timeout = timeout;
	
	if(STAGE_VTABLE(stage, set_timeout) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Set timeout not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if(STAGE_VCALL(stage, set_timeout)(stage, timeout) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to set timeout");
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;    
}
	
static int CVICALLBACK check_stage_is_stopped(void *callback)
{	// return 0 if not moving, ie has stopped
	XYStage *stage = (XYStage*) callback;
	int is_moving = 0;
	char time_str[256];

	stage->_abort_move = 0;

	while(1) {
			
#ifdef VERBOSE_DEBUG
		get_time_string(time_str);
		printf("%s: check_stage_is_stopped call stage_is_moving\n", time_str);
#endif
		if(stage_is_moving (stage, &is_moving) == STAGE_ERROR)
			return STAGE_ERROR;	
		
		ProcessSystemEvents();

#ifdef VERBOSE_DEBUG
		get_time_string(time_str);
		printf("%s: check_stage_is_stopped stage_is_moving done\n", time_str);
#endif

		if(!is_moving)
			//break;
			return 0;
	
		Delay(0.1);  //0.05);

#ifdef VERBOSE_DEBUG
		get_time_string(time_str);
		printf("%s: Waiting for xy stage\n", time_str);
#endif

		if (stage->_abort_move)
			//break;
			return is_moving;
	}

	// unreachable I think
	return is_moving;   
}


int stage_wait_for_stop_moving(XYStage *stage)
{
	int thread_id, return_value;
    char time_str[256];
	
	PROFILE_START("stage_wait_for_stop_moving");       
	
	// no need for thread here
//	get_time_string(time_str);
//	printf("%s: stage_wait_for_stop_moving: check_stage_is_stopped scheduled\n", time_str);
//	CmtScheduleThreadPoolFunction (gci_thread_pool(), check_stage_is_stopped, stage, &thread_id);
//	get_time_string(time_str);
//	printf("%s: waiting\n", time_str);
//	CmtWaitForThreadPoolFunctionCompletion(gci_thread_pool(),  thread_id, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
//	get_time_string(time_str);
//	printf("%s: check_stage_is_stopped finished ...\n", time_str);
//	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_RETURN_VALUE, &return_value);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: stage_wait_for_stop_moving: check_stage_is_stopped, wait\n", time_str);
#endif
	return_value = check_stage_is_stopped(stage);
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: check_stage_is_stopped finished ...\n", time_str);
#endif

	if(return_value < 0) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_WARNING, "stage_wait_for_stop_moving failed");   
	}

	PROFILE_STOP("stage_wait_for_stop_moving");    
	
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: stage_wait_for_stop_moving ProcessSystemEvents", time_str);
#endif
	ProcessSystemEvents();

#ifdef VERBOSE_DEBUG
	printf(" GCI_Signal_Emit\n");
#endif
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(stage), "StageStoppedMoving", GCI_VOID_POINTER, stage);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: Finished stage_wait_for_stop_moving\n", time_str);
#endif

	return STAGE_SUCCESS; 
}


/*
int stage_wait_for_stop_moving(XYStage *stage)
{
	int is_moving;
	
	PROFILE_START("stage_wait_for_stop_moving");       
	
	stage->_abort_move = 0;
			   
	while(1) {
		
		if(stage_is_moving (stage, &is_moving) == STAGE_ERROR)
			return STAGE_ERROR;	
		
		if(!is_moving)
			break;
	
		ProcessSystemEvents (); // Force CVI to look for events, in this case an abort init 
		Delay(0.1);

		if (stage->_abort_move)
			break;
	}

	PROFILE_STOP("stage_wait_for_stop_moving");    
	
	return STAGE_SUCCESS; 
}
*/

static void swap(double* a, double* b)
{
	register double tmp = *b;
	
	*b = *a;
	*a = tmp;
}


int stage_is_within_safe_xyz_region(XYStage* stage, double x, double y, double z)
{
    // We haven't got a safe region so return TRUE
	if (!stage->_safe_region_enabled)
		return 1;
	
	if (stage->_safe_region.shape == STAGE_SHAPE_RECTANGLE) {
		
		if ((x < stage->_safe_region.roi.min_x) || (x > (stage->_safe_region.roi.max_x)))
			return 0;
		
		if ((y < stage->_safe_region.roi.min_y) || (y > (stage->_safe_region.roi.max_y)))
			return 0;
	}
	else if (stage->_safe_region.shape == STAGE_SHAPE_CIRCLE) { 
	
		double xdiff = x - stage->_safe_region.center.x;

		double ydiff = y - stage->_safe_region.center.y;
			
		if (sqrt((xdiff*xdiff)+(ydiff*ydiff)) > stage->_safe_region.radius)
			return 0;
	}

	if ((z < stage->_safe_region.roi.min_z) || (z > (stage->_safe_region.roi.max_z)))
		return 0;

	return 1;
}

int stage_position_is_within_safe_xy_limits(XYStage* stage, double x, double y)
{
    // We haven't got a safe region so return TRUE
	if (!stage->_safe_region_enabled)
		return 1;
	
	if (stage->_safe_region.shape == STAGE_SHAPE_RECTANGLE) {
		
		if ((x < stage->_safe_region.roi.min_x) || (x > (stage->_safe_region.roi.max_x)))
			return 0;
		
		if ((y < stage->_safe_region.roi.min_y) || (y > (stage->_safe_region.roi.max_y)))
			return 0;
	}
	else if (stage->_safe_region.shape == STAGE_SHAPE_CIRCLE) { 
	
		double xdiff = x - stage->_safe_region.center.x;

		double ydiff = y - stage->_safe_region.center.y;
			
		if (sqrt((xdiff*xdiff)+(ydiff*ydiff)) > stage->_safe_region.radius)
			return 0;
	}

	return 1;
}

// This is only here so we can call this in the initialise extents function.
// At that point we have not done any axis reversal so we don't want to correct
// for it.
static int async_goto_xyz_position_no_orientation_correction (XYStage* stage, double raw_x, double raw_y, double raw_z)
{  
	double x = raw_x, y = raw_y, z = raw_z;
	STAGE_CORRECT_VALS_FOR_XYZ_ORIENTATION(stage, x, y, z);

	if(!stage_is_position_is_within_xyz_limits(stage, x, y, z)) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_INFORMATIONAL, "The requested co-ordinates are outside the stage area");
		return STAGE_ERROR; 
	}
	
	if(!stage_is_within_safe_xyz_region(stage, x, y, z)) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_INFORMATIONAL, "The requested co-ordinates are outside the stage safe area");   
		return STAGE_ERROR; 
	}

  	if(STAGE_VTABLE(stage, async_goto_xyz_position) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","async_goto_xyz_position not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, async_goto_xyz_position)(stage, raw_x, raw_y, raw_z) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","async_goto_xyz_position failed");
		return STAGE_ERROR;
	}

	emit_changed_signal(stage);    
	emit_xy_changed_signal(stage, x, y);

	return STAGE_SUCCESS;
}


int stage_check_is_within_xy_limits_and_safe_region(XYStage *stage, double x, double y)
{
	if(!stage_position_is_within_xy_limits(stage,  x, y)) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_WARNING,
			"%s coord %.1f, %.1f is beyond limit", UIMODULE_GET_DESCRIPTION(stage), x, y);
		return 0; 
	}

	if(!stage_position_is_within_safe_xy_limits(stage, x, y)) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_WARNING,
			"%s coord %.1f, %.1f is beyond safe limit", UIMODULE_GET_DESCRIPTION(stage), x, y);
		return 0; 
	}

	return 1;
}

static int async_goto_xy_position_no_orientation_correction (XYStage* stage, double raw_x, double raw_y)
{  
	double x = raw_x, y = raw_y;
	STAGE_CORRECT_VALS_FOR_XY_ORIENTATION(stage, x, y);

	if(!stage_check_is_within_xy_limits_and_safe_region(stage, x, y)) {
		return STAGE_ERROR; 
	}

  	if(STAGE_VTABLE(stage, async_goto_xy_position) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","async_goto_xy_position not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, async_goto_xy_position)(stage, raw_x, raw_y) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","async_goto_xy_position failed");
		return STAGE_ERROR;
	}

	emit_changed_signal(stage);    
	emit_xy_changed_signal(stage, x, y);

	return STAGE_SUCCESS;
}

// This is only here so we can call this in the initialise extents function.
// At that point we have not done any axis reversal so we don't want to correct
// for it and we cannot check the limits. Do not emit signals.
static int async_goto_xy_position_no_orientation_correction_no_check (XYStage* stage, double raw_x, double raw_y)
{  
	double x = raw_x, y = raw_y;
	STAGE_CORRECT_VALS_FOR_XY_ORIENTATION(stage, x, y);

  	if(STAGE_VTABLE(stage, async_goto_xy_position) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","async_goto_xy_position not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, async_goto_xy_position)(stage, raw_x, raw_y) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","async_goto_xy_position failed");
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
}

// This Initialises the extents and finds the original origin
// do not worry about raw and values corrected for stage orientation as limits are set to be symetrical about the datum
int stage_find_initialise_extents (XYStage* stage, int full)
{
	//int xDir, yDir;	
	double x, y, a, b, c, d;
	int waitCursor = 0;
	Roi old_limits;


	SetWaitCursor(1);
	stage_stop_timer(stage);
	stage->_abort_move = 0;
	stage->_init_aborted = 0;
	ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE); 

	// The init abort panel does not work, stage does not stop, the init does not end!
//	ui_module_display_panel(UIMODULE_CAST(stage), stage->_init_ui_panel);
//	ui_module_enable_panel(stage->_init_ui_panel, 0);  

	if(stage->_do_not_initialise)
		return STAGE_ERROR;

	// Emit Signal to any interested party.
	if (full) { 
		
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(stage), "XYInitExtentsStart", GCI_VOID_POINTER, stage);

		// keep old limits for comparison later
		old_limits.min_x = stage->_limits.min_x;
		old_limits.max_x = stage->_limits.max_x;    
		old_limits.min_y = stage->_limits.min_y;  
		old_limits.max_y = stage->_limits.max_y; 

		// Initialise the stage controller - also determines the limits if full is true.
    	// It is intended to get the left, top, right, bottom left and top should be less than the right, bottom.
    	// After this we set the center to be the origin so the left, top are towarss the negative
    	// and right bottom towards the positive.
		if(stage_calibrate_extents (stage, &(stage->_limits.min_x), &(stage->_limits.min_y),
								&(stage->_limits.max_x), &(stage->_limits.max_y)) == STAGE_ERROR) {
			goto Error;
		}
		
		// The stage must have returned sensible extents
		assert(stage->_limits.max_x  >  stage->_limits.min_x);
		assert(stage->_limits.max_y  >  stage->_limits.min_y); 
	
		// Set software limits 2mm inside the hardware limits 
		stage->_limits.min_x += stage->_software_limit;
		stage->_limits.max_x -= stage->_software_limit;    
		stage->_limits.min_y += stage->_software_limit;  
		stage->_limits.max_y -= stage->_software_limit; 

		// Find the center of the stage extents
		stage->_datum.x = stage->_limits.min_x + (stage->_limits.max_x - stage->_limits.min_x) / 2.0;
		stage->_datum.y = stage->_limits.min_y + (stage->_limits.max_y - stage->_limits.min_y) / 2.0;
		
		// Move to the centre
		// Dont check for safe region at this point as we have not set datum and
		// the safe region calculations depend upon it.
		if (async_goto_xy_position_no_orientation_correction_no_check (stage, stage->_datum.x, stage->_datum.y) == STAGE_ERROR)
			goto Error; // could not move to the centre for some reason, not safe?   

		Delay(1.0);

    	// Now the center has been set we have a stage with coord 
		// stage limits are centred here:
    	//left = -x, top = -y
		//right = x, bootom = y with the center being 0.
		// Also saves these settings to the config file
		stage_set_xy_datum (stage, stage->_datum.x, stage->_datum.y);  

		// Check new limits against old (old are loaded from file on startup), check against some tolerance
		a = fabs(old_limits.min_x - stage->_limits.min_x);
		b = fabs(old_limits.max_x - stage->_limits.max_x);
		c = fabs(old_limits.min_y - stage->_limits.min_y);
		d = fabs(old_limits.max_y - stage->_limits.max_y);

		if ((a > 500.0) ||
			(b > 500.0) ||
			(c > 500.0) ||
			(d > 500.0)) {

				char message[1024];

				sprintf(message, "Stage limits are significantly different to previous session.\nStage Plate definitions may be incorrect.\ndx = %.0g %.0g\ndy = %.0g %.0g",
					a, b, c, d);

				waitCursor = GetWaitCursorState();
				if (waitCursor) 
					SetWaitCursor(0);
				
				GCI_MessagePopup("Stage Warning", message);
				logger_log(UIMODULE_LOGGER(stage), LOGGER_INFORMATIONAL, "%s limits are significantly different to previous session. %.0g %.0g %.0g %.0g", UIMODULE_GET_DESCRIPTION(stage),
					a, b, c, d);

				if (waitCursor) 
					SetWaitCursor(1);
		}

		// Emit Signal to any interested party.
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(stage), "XYInitExtentsEnd", GCI_VOID_POINTER, stage);
	}						   
	else {
		
		stage_load_default_settings(stage);	

		// Check we have sensible extents loaded from the config    
		assert(stage->_limits.max_x  >  stage->_limits.min_x);
		assert(stage->_limits.max_y  >  stage->_limits.min_y); 

		// Find the center of the stage extents
		//stage->_datum.x = stage->_limits.min_x + ((stage->_limits.max_x - stage->_limits.min_x) / 2.0);
		//stage->_datum.y = stage->_limits.min_y + ((stage->_limits.max_y - stage->_limits.min_y) / 2.0);
	
		//stage_set_xy_datum (stage, stage->_datum.x, stage->_datum.y);   
	
		//Tell stage controller that current xy position is to become zero
		//if( STAGE_VCALL(stage, set_xy_datum)(stage, 0.0, 0.0) == STAGE_ERROR ) {
		//	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","set_xy_datum failed");
		//	return STAGE_ERROR;
		//}

		stage_set_xy_datum (stage, stage->_x_pos_at_last_shutdown, stage->_y_pos_at_last_shutdown);   

		stage_setup_graph(stage);
		//stage_save_settings_as_default(stage);
	}
	
	stage_setup_graph(stage);     
	
	// catch strange conditions were safe region is enabled but not defined.
	// default to a large rectanglular safe region.
	if(stage->_safe_region_enabled) {
		if(stage->_safe_region.shape == STAGE_SHAPE_RECTANGLE){
			if (stage->_safe_region.roi.min_x == 0.0 && stage->_safe_region.roi.max_x == 0.0)
				stage_set_safe_region_rectangle_percentage_smaller_than_limits(stage, 0.0);
		}
		else if(stage->_safe_region.shape == STAGE_SHAPE_CIRCLE){
			if (stage->_safe_region.radius == 0.0)
				stage_set_safe_region_rectangle_percentage_smaller_than_limits(stage, 0.0);
		}
		else
			stage_set_safe_region_rectangle_percentage_smaller_than_limits(stage, 0.0);
	}

	//Avoid accidental Z drive calamities
	stage_get_xy_position(stage, &x, &y);	
	stage_start_timer(stage);
	
	stage_set_joystick_on (stage);  
	
	ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0);  
	ui_module_hide_panel(UIMODULE_CAST(stage), stage->_init_ui_panel);
	SetWaitCursor(0);
    
	return STAGE_SUCCESS;
	
Error:

	ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0);  
	ui_module_hide_panel(UIMODULE_CAST(stage), stage->_init_ui_panel);
	SetWaitCursor(0);
	return STAGE_ERROR; 
}

Roi stage_get_limits(XYStage* stage)
{
	return stage->_limits;
}

void stage_stop_timer(XYStage* stage)
{
	#ifdef SINGLE_THREADED_POLLING
	if(stage->_main_ui_panel > 0)  
		SetCtrlAttribute(stage->_main_ui_panel, stage->_timer, ATTR_ENABLED, 0); 
	#else
	SetAsyncTimerAttribute (stage->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void stage_start_timer(XYStage* stage)
{
	#ifdef SINGLE_THREADED_POLLING
	if(stage->_main_ui_panel > 0) 
		SetCtrlAttribute(stage->_main_ui_panel, stage->_timer, ATTR_ENABLED, 1); 
	#else
	SetAsyncTimerAttribute (stage->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}


int stage_destroy(XYStage* stage)
{
	stage_stop_timer(stage);

	// Some stage ie LSTEP need manual mode ir joystick off to move to absolute pos ?
	stage_set_joystick_off(stage);

	if (GCI_ConfirmPopup("Reset Stage", IDI_WARNING, "Do you want to move stage to centre position\nready for the next session?")) {
		// Move stage to origin for next start.
		stage_goto_xy_position(stage, 0.0, 0.0);
	}

	stage_set_joystick_on(stage);
	
	#ifndef SINGLE_THREADED_POLLING   
	DiscardAsyncTimer(stage->_timer);
	#endif

	if( STAGE_VCALL(stage, destroy)(stage) == STAGE_ERROR ) {
	
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Destroy operation failed for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
		return STAGE_ERROR;
	}

	stage_destroy_params_ui(stage);

	if (stage->_stage_plate_signal_plate_changed_handler_signal_id >= 0)
		stage_plate_signal_plate_changed_handler_disconnect(stage->_stage_plate_module, stage->_stage_plate_signal_plate_changed_handler_signal_id);
	
	ui_module_destroy(UIMODULE_CAST(stage)); 

  	free(stage);
  	
  	return STAGE_SUCCESS;
}


int stage_set_pitch(XYStage* stage, Axis axis, double pitch)
{
  	if(STAGE_VTABLE(stage, set_pitch) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Set pitch not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if(STAGE_VCALL(stage, set_pitch)(stage, axis, pitch) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to set pitch");
		return STAGE_ERROR;
	}
	
//	stage->_pitch[axis] = pitch;
	
//	if (axis == ALL_AXIS) {
//		stage->_pitch[XAXIS] = pitch;
//		stage->_pitch[YAXIS] = pitch;
//	}
	
	return STAGE_SUCCESS;
}

int stage_get_pitch(XYStage* stage, Axis axis, double *pitch)
{
	if(STAGE_VTABLE(stage, get_pitch) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Get pitch not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if(STAGE_VCALL(stage, get_pitch)(stage, axis, pitch) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to get pitch");
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;   
}

int stage_set_speed(XYStage* stage, Axis axis, double speed)
{
  	if(STAGE_VTABLE(stage, set_speed) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Set speed not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, set_speed)(stage, axis, speed) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to set speed");
		return STAGE_ERROR;
	}
	
	if (axis == ALL_AXIS) 
	{
		stage->_speed[XAXIS] = speed;
		stage->_speed[YAXIS] = speed;
	}
	else
		stage->_speed[axis] = speed;
	
	return STAGE_SUCCESS;
}

int  stage_get_speed (XYStage* stage, Axis axis, double *speed)
{
  	if(STAGE_VTABLE(stage, get_speed) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Get speed not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, get_speed)(stage, axis, speed) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to get speed");
		return STAGE_ERROR;
	}
	
	if (axis == ALL_AXIS) 
	{
		stage->_speed[XAXIS] = *speed;
		stage->_speed[YAXIS] = *speed;
	}
	else
		stage->_speed[axis] = *speed;
	
	return STAGE_SUCCESS;
}

int  stage_set_joystick_on (XYStage* stage)
{
	if(STAGE_VTABLE(stage, set_joystick_on) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","set_joystick_on not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, set_joystick_on)(stage) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to set joystick on");
		return STAGE_ERROR;
	}
	
	stage->_joystick_status = 1;
	
	SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOYSTICK_ENABLE, stage->_joystick_status);
	SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOY_ON, stage->_joystick_status);
	SetCtrlAttribute(stage->_main_ui_panel, STAGE_PNL_JOYSTICK_XY_SPEED, ATTR_DIMMED, 0);

	emit_changed_signal(stage);    

	return STAGE_SUCCESS;
}

int stage_set_joystick_off (XYStage* stage)
{
	if(STAGE_VTABLE(stage, set_joystick_off) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","set_joystick_off not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}
  	
	if( STAGE_VCALL(stage, set_joystick_off)(stage) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to set joystick off");
		return STAGE_ERROR;
	}
	
	stage->_joystick_status = 0;

	SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOYSTICK_ENABLE, stage->_joystick_status);
	SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOY_ON, stage->_joystick_status);
	SetCtrlAttribute(stage->_main_ui_panel, STAGE_PNL_JOYSTICK_XY_SPEED, ATTR_DIMMED, 1);
	
	emit_changed_signal(stage);  

	return STAGE_SUCCESS;
}

int  stage_get_joystick_status (XYStage* stage)
{
	return stage->_joystick_status; 
}

int stage_get_joystick_speed(XYStage* stage, double *speed)
{
	if(STAGE_VTABLE(stage, get_joystick_speed) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Get joystick speed not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, get_joystick_speed)(stage, speed) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to get joystick speed");
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
}

int stage_set_joystick_speed(XYStage* stage, double speed)
{
  	if(STAGE_VTABLE(stage, set_joystick_speed) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Set joystick speed not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, set_joystick_speed)(stage, speed) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to set joystick speed");
		return STAGE_ERROR;
	}
	
	stage->_joystick_speed = speed;
	SetCtrlVal(stage->_main_ui_panel, STAGE_PNL_JOYSTICK_XY_SPEED, speed);

	return STAGE_SUCCESS;
}

int  stage_abort_move (XYStage* stage)
{
	if(STAGE_VTABLE(stage, abort_move) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","abort_move not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}
  	
	if( STAGE_VCALL(stage, abort_move)(stage) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to abort the move");
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;
}

int stage_calibrate_extents (XYStage* stage, double *min_x, double *min_y, double *max_x, double *max_y)
{
	if(STAGE_VTABLE(stage, calibrate_extents) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","calibrate_extents not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, calibrate_extents)(stage, min_x, min_y, max_x, max_y) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","calibrate_extents failed");
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;
}

int stage_set_acceleration(XYStage* stage, Axis axis, double acceleration)
{
  	if(STAGE_VTABLE(stage, set_acceleration) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Set acceleration not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, set_acceleration)(stage, axis, acceleration) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to set acceleration");
		return STAGE_ERROR;
	}
	
	if (axis == ALL_AXIS) 
	{
		stage->_acceleration[XAXIS] = acceleration;
		stage->_acceleration[YAXIS] = acceleration;
	}
	else
		stage->_acceleration[axis] = acceleration;
	
	
	return STAGE_SUCCESS;
}

int stage_get_acceleration(XYStage* stage, Axis axis, double *acceleration)
{
  	if(STAGE_VTABLE(stage, get_acceleration) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Get acceleration not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, get_acceleration)(stage, axis, acceleration) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to get acceleration");
		return STAGE_ERROR;
	}
	
	if (axis == ALL_AXIS) 
	{
		stage->_acceleration[XAXIS] = *acceleration;
		stage->_acceleration[YAXIS] = *acceleration;
	}
	else
		stage->_acceleration[axis] = *acceleration;

	return STAGE_SUCCESS;
}

int stage_get_x_position (XYStage* stage, double *x)
{
	double tmp_y;
	
	if(stage_get_xy_position(stage, x, &tmp_y) == STAGE_ERROR)
		return STAGE_ERROR;
		
	return STAGE_SUCCESS;
}

int stage_get_y_position (XYStage* stage, double *y)
{
	double tmp_x;
	
	if(stage_get_xy_position(stage, &tmp_x, y) == STAGE_ERROR)
		return STAGE_ERROR;
		
	return STAGE_SUCCESS;
}

int stage_get_z_position (XYStage* stage, double *z)
{
	double tmp_x, tmp_y;
	
	if(stage_get_xyz_position(stage, &tmp_x, &tmp_y, z) == STAGE_ERROR)
		return STAGE_ERROR;
		
	return STAGE_SUCCESS;
}

int stage_goto_x_position (XYStage* stage, double x)
{
	double xCur, yCur;
	
	if(stage_get_xy_position(stage, &xCur, &yCur) == STAGE_ERROR)
		return STAGE_ERROR;	

	return stage_goto_xy_position (stage, x, yCur);
}


int stage_goto_y_position (XYStage* stage, double y)
{
	double xCur, yCur;

	if(stage_get_xy_position(stage, &xCur, &yCur) == STAGE_ERROR)
		return STAGE_ERROR;	

	return stage_goto_xy_position (stage, xCur, y);
}

int  stage_goto_z_position (XYStage* stage, double z)
{
	double xCur, yCur;

	if(stage_get_xy_position(stage, &xCur, &yCur) == STAGE_ERROR)
		return STAGE_ERROR;	

	return stage_goto_xyz_position (stage, xCur, yCur, z);
}

int stage_set_controller_safe_region (XYStage* stage, Roi roi)
{
	Roi raw_roi; // raw values sent to the stage must account for any stage orientation

	if(STAGE_VTABLE(stage, set_controller_safe_region) == NULL) {
   // 	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","set_xy_datum not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_SUCCESS;  // It may not be implemented for all devices
  	}
	
	raw_roi.min_x=roi.min_x;
	raw_roi.max_x=roi.max_x; 
	raw_roi.min_y=roi.min_y; 
	raw_roi.max_y=roi.max_y;
	raw_roi.min_z=roi.min_z; 
	raw_roi.max_z=roi.max_z;

	STAGE_CORRECT_VALS_FOR_XY_ORIENTATION(stage, raw_roi.min_x, raw_roi.min_y);
	STAGE_CORRECT_VALS_FOR_XY_ORIENTATION(stage, raw_roi.max_x, raw_roi.max_y);

	// check that max is still max etc.
	if (raw_roi.min_x > raw_roi.max_x) 
		swap(&raw_roi.min_x, &raw_roi.max_x);
	if (raw_roi.min_y > raw_roi.max_y) 
		swap(&raw_roi.min_y, &raw_roi.max_y);
	
	if( STAGE_VCALL(stage, set_controller_safe_region)(stage, raw_roi) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","set_controller_safe_region failed");
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
}

void stage_set_safe_region_circle(XYStage* stage, double radius, Coord center)
{
	Roi roi;
	SafeRegion sr;
    sr.shape = STAGE_SHAPE_CIRCLE;
    sr.radius = radius;
    sr.center = center;
    memset((void *)&(sr.roi), 0, sizeof(Roi));
    stage->_safe_region = sr;

	roi.min_x = center.x - radius;
	roi.min_y = center.y - radius;
	roi.max_x = center.x + radius;
	roi.max_y = center.y + radius;
	roi.min_z = -DBL_MAX;
	roi.max_z = DBL_MAX;

	stage_set_controller_safe_region(stage, roi);

	stage->_safe_region_enabled = 1;
}

void stage_set_safe_region_rectangle(XYStage* stage, Roi roi)
{
    SafeRegion sr;
    sr.shape = STAGE_SHAPE_RECTANGLE;
    sr.radius = 0.0;
    sr.center.x = 0.0;
	sr.center.y = 0.0;   
    sr.roi = roi;
    stage->_safe_region = sr;

	stage_set_controller_safe_region(stage, roi);

	stage->_safe_region_enabled = 1;
}

void stage_set_safe_region_rectangle_percentage_smaller_than_limits(XYStage* stage, double percentage)
{
	Roi roi;

	double x_percent = ((stage->_limits.max_x - stage->_limits.min_x) / 100.0) * percentage; 
	double y_percent = ((stage->_limits.max_y - stage->_limits.min_y) / 100.0) * percentage; 
	double z_percent = ((stage->_limits.max_z - stage->_limits.min_z) / 100.0) * percentage; 

	roi.min_x = stage->_limits.min_x +  x_percent;
	roi.max_x = stage->_limits.max_x -  x_percent;

	roi.min_y = stage->_limits.min_y +  y_percent;
	roi.max_y = stage->_limits.max_y -  y_percent;

	roi.min_z = stage->_limits.min_z +  z_percent;
	roi.max_z = stage->_limits.max_z -  z_percent;

	stage_set_safe_region_rectangle(stage, roi);
}

int stage_position_is_within_xy_limits(XYStage *stage, double x, double y)
{
	//Check to see if the requested move will go outside the stage area
	if (x < stage->_limits.min_x || x > stage->_limits.max_x)
		return 0;

	if (y < stage->_limits.min_y || y > stage->_limits.max_y) 
		return 0;
	
	return 1;  
}

int stage_is_position_is_within_xyz_limits(XYStage *stage, double x, double y, double z)
{
	//Check to see if the requested move will go outside the stage area
	if (x < stage->_limits.min_x || x > stage->_limits.max_x)
		return 0;

	if (y < stage->_limits.min_y || y > stage->_limits.max_y) 
		return 0;

	if (z < stage->_limits.min_z || z > stage->_limits.max_z) 
		return 0;

	return 1;  
}


int stage_goto_xyz_position (XYStage* stage, double x, double y, double z)
{
	if(STAGE_VTABLE(stage, goto_xyz_position) != NULL) {
	
		double raw_x=x, raw_y=y, raw_z=z;
		STAGE_CORRECT_VALS_FOR_XYZ_ORIENTATION(stage, raw_x, raw_y, raw_z);

		if( STAGE_VCALL(stage, goto_xyz_position)(stage, raw_x, raw_y, raw_z) == STAGE_ERROR ) {
			ui_module_send_error(UIMODULE_CAST(stage),  "Stage", "goto_xyz_position failed");
			return STAGE_ERROR;
		}

		emit_changed_signal(stage);    
		emit_xy_changed_signal(stage, x, y);

		return STAGE_SUCCESS;
	}

	return STAGE_ERROR;
}


int stage_goto_xy_position (XYStage* stage, double x, double y)
{
	// If a specific stage has implemented the syncronous stage move function then call that.
	// If not then call the asyncronous method and then call wait for stop moving.
	if(STAGE_VTABLE(stage, goto_xy_position) != NULL) {
		double raw_x=x, raw_y=y;
		STAGE_CORRECT_VALS_FOR_XY_ORIENTATION(stage, raw_x, raw_y);
	
		stage_graph_display_current_position(stage, x, y);  

		if(!stage_check_is_within_xy_limits_and_safe_region(stage, x, y)) {
			return STAGE_ERROR; 
		}

		logger_log(UIMODULE_LOGGER(stage), LOGGER_INFORMATIONAL, "%s goto (x,y) position (%.2f,%.2f)", UIMODULE_GET_DESCRIPTION(stage), x, y);
	
		if( STAGE_VCALL(stage, goto_xy_position)(stage, raw_x, raw_y) == STAGE_ERROR ) {
			ui_module_send_error(UIMODULE_CAST(stage),  "Stage", "goto_xy_position failed");
			return STAGE_ERROR;
		}
		
		stage->_current_pos.x = x;
		stage->_current_pos.y = y;

		emit_changed_signal(stage);    	
		emit_xy_changed_signal(stage, x, y);

		return STAGE_SUCCESS;
	}
	
	if(stage_async_goto_xy_position (stage, x, y) == STAGE_ERROR)
		return STAGE_ERROR;

	emit_changed_signal(stage);    
	emit_xy_changed_signal(stage, x, y);

	return stage_wait_for_stop_moving(stage);	
}


int stage_enable_speed_over_accuracy (XYStage* stage, int enable)
{
	if(STAGE_VTABLE(stage, enable_speed_over_accuracy) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","enable_speed_over_accuracy not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, enable_speed_over_accuracy)(stage, enable) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","enable_speed_over_accuracy failed");
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;	
}

int stage_async_goto_x_position (XYStage* stage, double x)
{
	double xCur, yCur;

	if(stage_get_xy_position(stage, &xCur, &yCur) == STAGE_ERROR)
		return STAGE_ERROR;	

	stage_async_goto_xy_position (stage, x, yCur);
	
	return STAGE_SUCCESS;
}

int stage_async_goto_y_position (XYStage* stage, double y)
{
	double xCur, yCur;     

	if(stage_get_xy_position(stage, &xCur, &yCur) == STAGE_ERROR)
		return STAGE_ERROR;	

	stage_async_goto_xy_position (stage, xCur, y);
	
	return STAGE_SUCCESS;
}


int stage_async_goto_xy_position (XYStage* stage, double x, double y)
{  
	int ret;
	double raw_x=x, raw_y=y;
	
	STAGE_CORRECT_VALS_FOR_XY_ORIENTATION(stage, raw_x, raw_y);
	
	stage_graph_display_current_position(stage, x, y);  

    ret = async_goto_xy_position_no_orientation_correction (stage, raw_x, raw_y);  
	 
	if(ret == STAGE_ERROR)
		return STAGE_ERROR;
	
	logger_log(UIMODULE_LOGGER(stage), LOGGER_INFORMATIONAL, "%s async goto (%.2f,%.2f)", UIMODULE_GET_DESCRIPTION(stage), x, y);
	
	stage->_current_pos.x = x;
	stage->_current_pos.y = y;

	return ret;
}

int stage_async_goto_xyz_position (XYStage* stage, double x, double y, double z)
{  
	int ret;
	double raw_x=x, raw_y=y, raw_z=z;
	
	stage_graph_display_current_position(stage, x, y);  
	
	STAGE_CORRECT_VALS_FOR_XYZ_ORIENTATION(stage, raw_x, raw_y, raw_z);

    ret = async_goto_xyz_position_no_orientation_correction (stage, raw_x, raw_y, raw_z);  
	 
	if(ret == STAGE_ERROR)
		return STAGE_ERROR;

	logger_log(UIMODULE_LOGGER(stage), LOGGER_INFORMATIONAL, "%s async goto (%.2f,%.2f,%.2f)", UIMODULE_GET_DESCRIPTION(stage), x, y, z);
	
	stage->_current_pos.x = x;
	stage->_current_pos.y = y;
	stage->_current_pos.z = z;

	return ret;
}

int stage_get_xyz_position_without_orientation_correction (XYStage* stage, double *raw_x, double *raw_y, double *raw_z)
{
  	if(STAGE_VTABLE(stage, get_xyz_position) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Get xy position not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, get_xyz_position)(stage, raw_x, raw_y, raw_z) == STAGE_ERROR ) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "Stage", "get_xyz_position failed");  
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;
}

int stage_get_xy_position_without_orientation_correction (XYStage* stage, double *raw_x, double *raw_y)
{
	double z;
	
  	if(STAGE_VTABLE(stage, get_xyz_position) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Get xy position not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, get_xyz_position)(stage, raw_x, raw_y, &z) == STAGE_ERROR ) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "Stage", "stage_get_xy_position_without_orientation_correction failed");  
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;
}

void stage_use_cached_data_for_read(XYStage *stage, int val)
{
	stage->_use_cached_position = val;
}

int stage_get_xyz_position (XYStage* stage, double *x, double *y, double *z)
{
	double raw_x=0.0, raw_y=0.0, raw_z=0.0;
	
	if(stage->_use_cached_position) {
		*x = stage->_current_pos.x;
		*y = stage->_current_pos.y;
		*z = stage->_current_pos.z;
	
		return STAGE_SUCCESS;
	}

	if(stage_get_xyz_position_without_orientation_correction(stage, &raw_x, &raw_y, &raw_z)== STAGE_ERROR) {
		*x = 0.0;
		*y = 0.0;
		*z = 0.0;
		return STAGE_ERROR;
	}

	*x = raw_x;
	*y = raw_y;
	*z = raw_z;
	STAGE_CORRECT_VALS_FOR_XYZ_ORIENTATION(stage, *x, *y, *z);     
	
	return STAGE_SUCCESS;
}

int stage_get_xy_position (XYStage* stage, double *x, double *y)
{
	double z;

	if(stage_get_xyz_position (stage, x, y, &z) == STAGE_ERROR) {
		*x = 0.0;
		*y = 0.0;
		return STAGE_ERROR;
	}

	return STAGE_SUCCESS;
}


int stage_async_rel_move_by (XYStage* stage, double x, double y)
{
	//This will send xyz to the controller. If there's no z no problem.
	double old_x, old_y, new_x, new_y, raw_x=x, raw_y=y;
	
	STAGE_CORRECT_VALS_FOR_XY_ORIENTATION(stage, raw_x, raw_y);
	
	if(stage_get_xy_position (stage, &old_x, &old_y) == STAGE_ERROR)
		return STAGE_ERROR;

	if(!stage_check_is_within_xy_limits_and_safe_region(stage, old_x + x, old_y + y)) {
			return STAGE_ERROR; 
	}

	if(STAGE_VTABLE(stage, async_rel_move_by) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","async_rel_move_by not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, async_rel_move_by)(stage, raw_x, raw_y) == STAGE_ERROR ) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "Stage", "async_rel_move_by failed");  
		return STAGE_ERROR;
	}
	
	if(stage_get_xy_position (stage, &new_x, &new_y) == STAGE_ERROR) {
		return STAGE_ERROR;
	}
	
	stage_graph_display_current_position(stage, new_x, new_y);	
	
	return STAGE_SUCCESS;          
}

int stage_rel_move_by (XYStage* stage, double x, double y)
{
	if(stage_async_rel_move_by ( stage, x, y)  == STAGE_ERROR)
		return STAGE_ERROR;	

	return stage_wait_for_stop_moving(stage);
}

int stage_set_xy_datum (XYStage* stage, double x, double y)
{
	double cur_x, cur_y;
	
	if(STAGE_VTABLE(stage, set_xy_datum) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","set_xy_datum not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}
	
	if(stage_get_xy_position(stage, &cur_x, &cur_y) == STAGE_ERROR)
		return STAGE_ERROR;

	//Set new values of X and Y limits
	stage->_limits.min_x -= x;
	stage->_limits.max_x -= x;  
	stage->_limits.min_y -= y;  
	stage->_limits.max_y -= y; 
	
	stage_setup_graph(stage);

	//Tell stage controller that current xy position is to become zero
	if( STAGE_VCALL(stage, set_xy_datum)(stage, 0.0, 0.0) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","set_xy_datum failed");
		return STAGE_ERROR;
	}
	
	stage->_datum.x = x; 
	stage->_datum.y = y; 

	// just save the user action dependant settings here, limit etc. not all the stage settings
	stage_save_user_settings_as_default(stage);

	return STAGE_SUCCESS;
}

/*
int  stage_set_current_x_position_as_x_datum (XYStage* stage)
{
	double x;
	
	stage_get_x_position (stage, &x);           
	
	return stage_set_x_datum (stage, x);
}

int  stage_set_current_y_position_as_y_datum (XYStage* stage)
{
	double y;
	
	stage_get_y_position (stage, &y);           
	
	return stage_set_y_datum (stage, y);
}
*/

int  stage_set_current_xy_position_as_xy_datum (XYStage* stage)       
{
	double x, y;
	
	stage_get_xy_position (stage, &x, &y);
	
	return stage_set_xy_datum (stage, x, y);   
}

int stage_get_xy_datum (XYStage* stage, double *x, double *y)
{
	*x = stage->_datum.x;
	*y = stage->_datum.y;

	return STAGE_SUCCESS;
}

int stage_is_moving (XYStage* stage, int *status)
{
	if(STAGE_VTABLE(stage, is_moving) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","is_moving not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, is_moving)(stage, status) == STAGE_ERROR ) {
		logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "Stage", "stage_is_moving failed");  
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;

}

int stage_get_info(XYStage* stage, char *info)
{
  	if(STAGE_VTABLE(stage, get_info) == NULL) {
    	ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Get info not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

	if( STAGE_VCALL(stage, get_info)(stage, info) == STAGE_ERROR ) {
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Failed to get info");
		return STAGE_ERROR;
	}
	
	return STAGE_SUCCESS;
}


int stage_save_settings(XYStage* stage, const char *filename)
{
	if(filename == NULL)
    	return STAGE_ERROR;

	if(STAGE_VTABLE(stage, save_settings) == NULL) {
    	send_stage_error_text(stage,"Save settings not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

  	if( STAGE_VCALL(stage, save_settings)(stage, filename)  == STAGE_ERROR) {
  		send_stage_error_text(stage,"Can not save settings for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
  		return STAGE_ERROR;
  	}
	
	return STAGE_SUCCESS;
}
int stage_save_user_settings(XYStage* stage, const char *filename)
{
	FILE *fd=NULL;
	dictionary *d = dictionary_new(20);
	char temp_dir[GCI_MAX_PATHNAME_LEN] = "", temp_filepath[GCI_MAX_PATHNAME_LEN] = "";

	if(filename == NULL)
    	return STAGE_ERROR;

	// Get temp dir
	if(!GetEnvironmentVariable("Temp", temp_dir, 500))
		ui_module_send_error(UIMODULE_CAST(stage), "Stage Error", "Can not get temporary directory");  

	sprintf(temp_filepath, "%s\\%s", temp_dir, "\\stage_user_temp.ini");	

	fd = fopen(temp_filepath, "w");

	if (fd==NULL)
		return STAGE_ERROR;

	// get user action dependant settings
	stage_save_user_data_to_dictionary(stage, d);
	
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	if(CopyFile (temp_filepath, filename) < 0)
		return STAGE_ERROR;

	return STAGE_SUCCESS;	
}

int stage_load_settings(XYStage* stage, const char *filename)
{
	if(filename == NULL)
    	return STAGE_ERROR;

	if(STAGE_VTABLE(stage, load_settings) == NULL) {
    	send_stage_error_text(stage,"Load settings not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(stage));
    	return STAGE_ERROR;
  	}

  	if( STAGE_VCALL(stage, load_settings)(stage, filename)  == STAGE_ERROR)
  		return STAGE_ERROR;
	
	return STAGE_SUCCESS;
}

int stage_load_user_settings(XYStage* stage, const char *filepath)
{
	dictionary* d = NULL;
	int file_size;

	if(filepath == NULL)
    	return STAGE_ERROR;
	
	if(!FileExists(filepath, &file_size)) {
	// use some default values
		stage->_x_pos_at_last_shutdown = 0.0;
		stage->_y_pos_at_last_shutdown = 0.0;

		stage->_limits.min_x = -DBL_MAX;
		stage->_limits.max_x = DBL_MAX;
		stage->_limits.min_y = -DBL_MAX;
		stage->_limits.max_y = DBL_MAX;
		stage->_limits.min_z = -DBL_MAX;
		stage->_limits.max_z = DBL_MAX;

		stage->_safe_region_enabled = 0;
		stage->_safe_region.shape   = STAGE_SHAPE_RECTANGLE;
		stage->_safe_region.roi.min_x = 0.0;
		stage->_safe_region.roi.max_x = 0.0;
		stage->_safe_region.roi.min_y = 0.0;
		stage->_safe_region.roi.max_y = 0.0;
		stage->_safe_region.roi.min_z = 0.0;
		stage->_safe_region.roi.max_z = 0.0;
		stage->_safe_region.center.x = 0.0;
		stage->_safe_region.center.y = 0.0;
		stage->_safe_region.radius = 0.0;

		stage->_software_limit = 2000.0; 
		
		stage_set_controller_safe_region(stage, stage->_safe_region.roi);
		stage_setup_graph(stage);

		return STAGE_SUCCESS;
	}
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		stage_load_user_data_from_dictionary(stage, d); 
		dictionary_del(d);
	}

	return STAGE_SUCCESS;
}

int stage_save_settings_as_default(XYStage* stage)
{
	char path[GCI_MAX_PATHNAME_LEN];

    sprintf(path, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(stage), UIMODULE_GET_NAME(stage), DEFAULT_STAGE_FILENAME_SUFFIX);
	if(stage_save_settings(stage, path) == STAGE_ERROR)
		return STAGE_ERROR;

    sprintf(path, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(stage), UIMODULE_GET_NAME(stage), DEFAULT_STAGE_USER_FILENAME_SUFFIX);
	if (stage_save_user_settings(stage, path) == STAGE_ERROR)
		return STAGE_ERROR;
	
	return STAGE_SUCCESS;
}

int stage_save_user_settings_as_default(XYStage* stage)
{
	// just save the user action dependant settings
	char path[GCI_MAX_PATHNAME_LEN];

	sprintf(path, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(stage), UIMODULE_GET_NAME(stage), DEFAULT_STAGE_USER_FILENAME_SUFFIX);
	if (stage_save_user_settings(stage, path) == STAGE_ERROR)
		return STAGE_ERROR;
	
	return STAGE_SUCCESS;
}


int stage_load_default_settings(XYStage* stage)
{
	char path[GCI_MAX_PATHNAME_LEN];

    sprintf(path, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(stage), UIMODULE_GET_NAME(stage), DEFAULT_STAGE_FILENAME_SUFFIX);
	if(stage_load_settings(stage, path) == STAGE_ERROR)
		return STAGE_ERROR;

	sprintf(path, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(stage), UIMODULE_GET_NAME(stage), DEFAULT_STAGE_USER_FILENAME_SUFFIX);
	if(stage_load_user_settings(stage, path) == STAGE_ERROR)
		return STAGE_ERROR;
		
	return STAGE_SUCCESS;
}

int stage_save_data_to_dictionary(XYStage *stage, dictionary *d)
{	// Settings that should not change
	double speed, acceleration;      
	
	dictionary_set(d, "Stage", NULL);

	stage_get_speed(stage, XAXIS, &speed);
	stage_get_acceleration(stage, XAXIS, &acceleration);
	
	if(acceleration == 0.0 || speed == 0.0) {
		GCI_MessagePopup("Stage Error", "Acceleration or speed is going to be saved as 0");
	}
	
	if(acceleration < 0.0 || speed < 0.0) {
		GCI_MessagePopup("Stage Error", "Acceleration or speed is negative!");
	}
	
	if(acceleration > 1.0e4 || speed  > 1.0e4) {
		GCI_MessagePopup("Stage Warning", "Acceleration or speed is very large");
	}

	dictionary_setint(d, "Closed Loop Enabled", stage->_closed_loop_enabled);

	dictionary_setint(d, "X Axis Enable", stage->_enabled_axis[XAXIS]);
    dictionary_setint(d, "X Axis Direction",stage->_axis_dir[XAXIS]);
    dictionary_setdouble(d, "X Axis Speed", speed);
    dictionary_setdouble(d, "X Axis Acceleration", acceleration);
	
	stage_get_speed(stage, YAXIS, &speed);
	stage_get_acceleration(stage, YAXIS, &acceleration);
	
	if(acceleration == 0.0 || speed == 0.0) {
		GCI_MessagePopup("Stage Error", "Acceleration or speed is going to be saved as 0");
	}

    dictionary_setint(d, "Y Axis Enable", stage->_enabled_axis[YAXIS]);
    dictionary_setint(d, "Y Axis Direction",stage->_axis_dir[YAXIS]);
    dictionary_setdouble(d, "Y Axis Speed", speed);
    dictionary_setdouble(d, "Y Axis Acceleration", acceleration);
    
	if (stage->_speed[ZAXIS] > 0.0){  // must have valid information to store this value
		dictionary_setint(d, "Z Axis Direction",stage->_axis_dir[ZAXIS]);
		dictionary_setint(d, "Z Axis Speed",stage->_speed[ZAXIS]);
	}

	return STAGE_SUCCESS;
}


int stage_save_user_data_to_dictionary(XYStage *stage, dictionary *d)
{	// Settings that may change with user action
	double x, y;      
	
	dictionary_set(d, "Stage", NULL);

	stage_get_xy_position(stage, &x, &y);

	dictionary_setdouble(d, "Last x position", x);
    dictionary_setdouble(d, "Last y position", y);

    // Save the limits
    dictionary_setdouble(d, "Limit XMin", stage->_limits.min_x);
    dictionary_setdouble(d, "Limit XMax", stage->_limits.max_x);
    dictionary_setdouble(d, "Limit YMin", stage->_limits.min_y);
    dictionary_setdouble(d, "Limit YMax", stage->_limits.max_y);
	dictionary_setdouble(d, "Limit ZMin", stage->_limits.min_z);
	dictionary_setdouble(d, "Limit ZMax", stage->_limits.max_z);

    // Save the Safe Region
    dictionary_setint(d, "Safe Region Enabled", stage->_safe_region_enabled);
	dictionary_setint(d, "Safe Region Shape",   stage->_safe_region.shape);
    dictionary_setdouble(d, "Safe Region XMin", stage->_safe_region.roi.min_x);
    dictionary_setdouble(d, "Safe Region XMax", stage->_safe_region.roi.max_x);
    dictionary_setdouble(d, "Safe Region YMin", stage->_safe_region.roi.min_y);
    dictionary_setdouble(d, "Safe Region YMax", stage->_safe_region.roi.max_y);
    dictionary_setdouble(d, "Safe Region Center X", stage->_safe_region.center.x);
    dictionary_setdouble(d, "Safe Region Center Y", stage->_safe_region.center.y);
    dictionary_setdouble(d, "Safe Region Radius", stage->_safe_region.radius);
	dictionary_setdouble(d, "Safe Distance from Limit",stage->_software_limit); 
	
	stage->_safe_region.roi.min_z = -DBL_MAX;
	stage->_safe_region.roi.max_z = DBL_MAX;

    return STAGE_SUCCESS;
}

int stage_load_data_from_dictionary(XYStage *stage, dictionary *d)
{	// settings that should not change
	double speed, acceleration; 
	StageDirection direction;  
	
	stage->_closed_loop_enabled = dictionary_getint(d, "Stage:closed loop enabled", 0);
    
	// do z first as is not enabled on some systems and bad data sent will be overwritten by good xy data below, should never happen, these stages should not have z speed entry in ini
    stage->_enabled_axis[ZAXIS] = dictionary_getint(d, "Stage:Z Axis Enable", 0);
	direction = dictionary_getint(d, "Stage:Z Axis Direction", STAGE_NEGATIVE_TO_POSITIVE);
    speed = dictionary_getdouble(d, "Stage:Z Axis Speed", -1.0);
	if(stage->_enabled_axis[ZAXIS] && speed > 0.0) {  // z speed must be a valid value in ini file for the combined xyz stages.
		stage_set_axis_direction(stage, ZAXIS, direction);
		stage_set_speed(stage, ZAXIS, speed); 
	}

    stage->_enabled_axis[XAXIS] = dictionary_getint(d, "Stage:X Axis Enable", 1);
    direction = dictionary_getint(d, "Stage:X Axis Direction", STAGE_NEGATIVE_TO_POSITIVE);
	speed = dictionary_getdouble(d, "Stage:X Axis Speed", 10.0);
    acceleration = dictionary_getdouble(d, "Stage:X Axis Acceleration", 500.0);
	
	stage_set_axis_direction(stage, XAXIS, direction);
	stage_set_speed(stage, XAXIS, speed); 
	stage_set_acceleration(stage, XAXIS, acceleration);      
	
    stage->_enabled_axis[YAXIS] = dictionary_getint(d, "Stage:Y Axis Enable", 1);
    direction = dictionary_getint(d, "Stage:Y Axis Direction", STAGE_NEGATIVE_TO_POSITIVE);
    speed = dictionary_getdouble(d, "Stage:Y Axis Speed", 10.0);
    acceleration = dictionary_getdouble(d, "Stage:Y Axis Acceleration", 500.0);

	stage_set_axis_direction(stage, YAXIS, direction);
	stage_set_speed(stage, YAXIS, speed); 
	stage_set_acceleration(stage, YAXIS, acceleration);      
	
    return STAGE_SUCCESS;
}

int stage_load_user_data_from_dictionary(XYStage *stage, dictionary *d)
{   // only user action dependant settings
	
	stage->_x_pos_at_last_shutdown = dictionary_getdouble(d, "Stage:Last x position", 0.0);
    stage->_y_pos_at_last_shutdown = dictionary_getdouble(d, "Stage:Last y position", 0.0);

    stage->_limits.min_x = dictionary_getdouble(d, "Stage:Limit XMin", -DBL_MAX);
    stage->_limits.max_x = dictionary_getdouble(d, "Stage:Limit XMax", DBL_MAX);
    stage->_limits.min_y = dictionary_getdouble(d, "Stage:Limit YMin", -DBL_MAX);
    stage->_limits.max_y = dictionary_getdouble(d, "Stage:Limit YMax", DBL_MAX);
	stage->_limits.min_z = dictionary_getdouble(d, "Stage:Limit ZMin", -DBL_MAX);
    stage->_limits.max_z = dictionary_getdouble(d, "Stage:Limit ZMax", DBL_MAX);

    stage->_safe_region_enabled = dictionary_getint(d, "Stage:Safe Region Enabled", 0);
    stage->_safe_region.shape   = dictionary_getint(d, "Stage:Safe Region Shape", STAGE_SHAPE_RECTANGLE);
    stage->_safe_region.roi.min_x = dictionary_getdouble(d, "Stage:Safe Region XMin", 0.0);
    stage->_safe_region.roi.max_x = dictionary_getdouble(d, "Stage:Safe Region XMax", 0.0);
    stage->_safe_region.roi.min_y = dictionary_getdouble(d, "Stage:Safe Region YMin", 0.0);
    stage->_safe_region.roi.max_y = dictionary_getdouble(d, "Stage:Safe Region YMax", 0.0);
	stage->_safe_region.roi.min_z = dictionary_getdouble(d, "Stage:Safe Region ZMin", -DBL_MAX);
    stage->_safe_region.roi.max_z = dictionary_getdouble(d, "Stage:Safe Region ZMax", DBL_MAX);
    stage->_safe_region.center.x = dictionary_getdouble(d, "Stage:Safe Region Center X", 0.0);
    stage->_safe_region.center.y = dictionary_getdouble(d, "Stage:Safe Region Center Y", 0.0);
    stage->_safe_region.radius = dictionary_getdouble(d, "Stage:Safe Region Radius",0.0);

	stage->_software_limit = dictionary_getdouble(d, "Stage:Safe Distance from Limit", 2000.0); 
	
	stage_set_controller_safe_region(stage, stage->_safe_region.roi);
	stage_setup_graph(stage);
	
    return STAGE_SUCCESS;
}

int  stage_display_params_ui(XYStage* stage)
{
	if(stage->_params_ui_panel != -1) {
		
		//Restore last used panel positions
		ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(stage), stage->_params_ui_panel, 0);  
		
		GCI_ShowPasswordProtectedPanel(stage->_params_ui_panel, stage->_main_ui_panel);
	}

	return STAGE_SUCCESS;
}

int  stage_hide_params_ui(XYStage* stage)
{
  	if(stage->_params_ui_panel != -1) {
  		ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(stage), stage->_params_ui_panel, 1);
		HidePanel(stage->_params_ui_panel);
	}
		
	return STAGE_SUCCESS;
}

int  stage_destroy_params_ui(XYStage* stage)
{
  	if(stage->_params_ui_panel != -1) {
		ui_module_destroy_panel(UIMODULE_CAST(stage), stage->_params_ui_panel);
  	}
  	
  	stage->_params_ui_panel = -1;
  	
  	return STAGE_SUCCESS;
}

