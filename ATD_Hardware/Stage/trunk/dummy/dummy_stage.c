#include "dummy_stage.h"
#include "utility.h"

static double _current_pos_x;
static double _current_pos_y;

static int om_set_speed (XYStage* stage, Axis axis, double speed)
{
	return STAGE_SUCCESS;
}


static int om_set_acceleration (XYStage* stage, Axis axis, double acceleration)
{
	return STAGE_SUCCESS;
}


static int om_get_info (XYStage* stage, char *info)
{
	strcpy(info, "Manual Stage");

	return STAGE_SUCCESS;
}


static int om_hw_init (XYStage* stage)
{
	if(stage == NULL)
		return STAGE_ERROR;

	// Hardware initialisation would go here

	stage_load_default_settings(stage);

	return STAGE_SUCCESS;
}

static int om_init (XYStage* stage)
{
			
	return STAGE_SUCCESS;
}


static int om_destroy(XYStage* stage)
{
  	return STAGE_SUCCESS;
}


static int om_self_test (XYStage* stage)
{
  	return STAGE_SUCCESS;
}


static int om_enable_axis (XYStage* stage, Axis axis)
{
  	return STAGE_SUCCESS;
}


static int om_disable_axis (XYStage* stage, Axis axis)
{
  	return STAGE_SUCCESS;
}


static int om_calibrate_extents (XYStage* stage, double *min_x, double *min_y, double *max_x, double *max_y)
{
	// Return some resonable values for stage extents, as if the stage had found it's limit switches
	*min_x = -50000;
	*max_x = 50000;
	*min_y = -50000;
	*max_y = 50000;

  	return STAGE_SUCCESS;
}


static int om_async_goto_xy_position (XYStage* stage, double x, double y)
{
	_current_pos_x = x; 
  	_current_pos_y = y;

  	return STAGE_SUCCESS;
}

static int om_goto_xy_position (XYStage* stage, double x, double y)
{
	_current_pos_x = x; 
  	_current_pos_y = y;

	// 100 ms Delay - Stage moves wouldn't be faster than this in reality.
	Delay(0.1);

  	return STAGE_SUCCESS;
}

static int om_get_xyz_position (XYStage* stage, double *x, double *y, double *z)
{
  	// just return the current pos in the dummy stage
	
	*x = _current_pos_x; 
  	*y = _current_pos_y;
	*z = 0;
  	
  	return STAGE_SUCCESS;
}

	
static int om_async_rel_move_by (XYStage* stage, double x, double y)
{
	_current_pos_x += x; 
  	_current_pos_y += y;

	return STAGE_SUCCESS;
}

	
static int om_set_xy_datum (XYStage* stage, double x, double y)
{
	return STAGE_SUCCESS;
}


static int om_is_moving (XYStage* stage, int *status)
{
  	*status = 0;
	return STAGE_SUCCESS;
}	


static int om_set_joystick_speed (XYStage* stage, double speed)
{
  	return STAGE_SUCCESS;
}	


static int om_get_pitch (XYStage* stage, Axis axis, double *pitch)
{
  	*pitch = 1.0;
	return STAGE_SUCCESS;
}	

	
static int om_set_joystick_on (XYStage* stage)
{
  	return STAGE_SUCCESS;
}	

	
static int om_set_joystick_off (XYStage* stage)
{
  	return STAGE_SUCCESS;
}		
	
	
static int om_set_timeout (XYStage* stage, double timeout)
{
  	return STAGE_SUCCESS;
}	

static  int om_save_settings (XYStage* stage, const char *filepath)
{
	FILE *fd=NULL;
	dictionary *d = dictionary_new(20);

	fd = fopen(filepath, "w");

	if (fd==NULL)
		return STAGE_ERROR;
	
	// get generic settings
	stage_save_data_to_dictionary(stage, d);
	
	// Save any specific settings here

	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return STAGE_SUCCESS;
}	

static int om_load_settings (XYStage* stage, const char *filepath)
{
	dictionary* d = NULL;
	int file_size;
	
	if(!FileExists(filepath, &file_size))
		return STAGE_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(iniparser_getnsec(d) == 0)
	{
		stage->_do_not_initialise = 1;
		GCI_MessagePopup("Error", "Failed to read stage configuration. The file exists but no data was read.\n"
							  "I am not going to initialise the stage extents as this could cause damage.");
		return STAGE_ERROR;
	}

	if(d != NULL) {

		// generic settings
		stage_load_data_from_dictionary(stage, d); 
 
		dictionary_del(d);
	}
	
	return STAGE_SUCCESS;	  
}	

static int om_power_up(XYStage* stage)
{
  	return STAGE_SUCCESS;
}
	
	
static int om_power_down(XYStage* stage)
{
  	return STAGE_SUCCESS;
}


static int om_get_x_datum (XYStage* stage, double *x)
{
	return STAGE_SUCCESS;
}


static int om_reset(XYStage* stage)
{
  	return STAGE_SUCCESS;
}

static int om_get_speed (XYStage* stage, Axis axis, double *speed)
{
	*speed = 40;

	return STAGE_SUCCESS;
}

static int om_get_acceleration (XYStage* stage, Axis axis, double *accel)
{
	*accel = 500;

	return STAGE_SUCCESS;
}

XYStage* stage_dummy_new(const char* name, const char* desription, UI_MODULE_ERROR_HANDLER error_handler, void *data, const char *data_dir)
{
	XYStage* stage = (XYStage*) malloc(sizeof(XYStage));

	stage_constructor(stage, name);   

	stage_set_axis_direction(stage, XAXIS, STAGE_POSITIVE_TO_NEGATIVE);        
	stage_set_axis_direction(stage, YAXIS, STAGE_POSITIVE_TO_NEGATIVE);

	ui_module_set_description(UIMODULE_CAST(stage), desription);	
	ui_module_set_data_dir(UIMODULE_CAST(stage), data_dir);   

	ui_module_set_error_handler(UIMODULE_CAST(stage), error_handler, data); 
	
	STAGE_VTABLE(stage, hw_init) = om_hw_init;
	STAGE_VTABLE(stage, init) = om_init;
	STAGE_VTABLE(stage, destroy) = om_destroy;
	STAGE_VTABLE(stage, get_info) = om_get_info;
	STAGE_VTABLE(stage, calibrate_extents) = om_calibrate_extents;
	STAGE_VTABLE(stage, set_speed) = om_set_speed;
	STAGE_VTABLE(stage, set_acceleration) = om_set_acceleration;
	STAGE_VTABLE(stage, async_goto_xy_position) = om_async_goto_xy_position;
	STAGE_VTABLE(stage, goto_xy_position) = om_goto_xy_position;
	STAGE_VTABLE(stage, get_xyz_position) = om_get_xyz_position;
	STAGE_VTABLE(stage, async_rel_move_by) = om_async_rel_move_by;
	STAGE_VTABLE(stage, set_xy_datum) = om_set_xy_datum;
	STAGE_VTABLE(stage, is_moving) = om_is_moving;
	STAGE_VTABLE(stage, set_joystick_speed) = om_set_joystick_speed;
	STAGE_VTABLE(stage, set_joystick_on) = om_set_joystick_on;
	STAGE_VTABLE(stage, set_joystick_off) = om_set_joystick_off;
	STAGE_VTABLE(stage, save_settings) = om_save_settings; 
	STAGE_VTABLE(stage, load_settings) = om_load_settings; 
	STAGE_VTABLE(stage, get_pitch) = om_get_pitch;
	STAGE_VTABLE(stage, get_speed) = om_get_speed;
	STAGE_VTABLE(stage, get_acceleration) = om_get_acceleration;
	STAGE_VTABLE(stage, set_timeout) = om_set_timeout;
	
	return stage;	
}
