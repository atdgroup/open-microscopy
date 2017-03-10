#include "dummy_stage.h"

/* Function pointers used as virtual functions */
static struct stage_operations dummy_ops;

static int dummy_set_speed (Stage* stage, Axis axis, double speed)
{
	printf("Setting speed for Dummy stage \n");

	return STAGE_SUCCESS;
}


static int dummy_set_acceleration (Stage* stage, Axis axis, double acceleration)
{
	printf("Setting acceleration for Dummy stage \n");

	return STAGE_SUCCESS;
}


static int dummy_get_info (Stage* stage, char *info)
{
	printf("Setting roi for Dummy stage \n");
	strcpy(info, "Dummy Vendor\nDummy Model");

	return STAGE_SUCCESS;
}


static int dummy_init (Stage* stage)
{
	if(stage == NULL)
		return STAGE_ERROR;

	stage_set_name(stage, "Dummy Stage");
	stage_set_description(stage, "Dummy Stage");

	return STAGE_SUCCESS;
}


static int dummy_destroy(Stage* stage)
{
  	printf("Destroying dummy stage\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_self_test (Stage* stage)
{
	printf("Self testing dummy stage\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_enable_axis (Stage* stage, Axis axis)
{
	printf("Enabling axis dummy stage\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_disable_axis (Stage* stage, Axis axis)
{
	printf("Disabling axis dummy stage\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_calibrate_extents (Stage* stage, int full, double *min_x, double *min_y, double *max_x, double *max_y)
{
	printf("dummy_calibrate_extents\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_async_goto_xyz_position (Stage* stage, double x, double y, double z)
{
	printf("dummy_set_xyz_position\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_get_xyz_position (Stage* stage, double *x, double *y, double *z)
{
	printf("dummy_get_xyz_position\n");
  	
  	*x = 20.0; 
  	*y = 60.0;
  	*z = 140.0;
  	
  	return STAGE_SUCCESS;
}

	
static int dummy_async_rel_move_by (Stage* stage, double x, double y, double z)
{
	printf("dummy_move_by\n");
  	
  	return STAGE_SUCCESS;
}

	
static int dummy_set_xyz_datum (Stage* stage, double x, double y, double z)
{
	printf("dummy_set_xyz_datum\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_is_moving (Stage* stage, int *status)
{
	printf("dummy_is_moving\n");
  	
  	return STAGE_SUCCESS;
}	


static int dummy_set_joystick_speed (Stage* stage, double speed)
{
	printf("dummy_set_joystick_speed\n");
  	
  	return STAGE_SUCCESS;
}	

	
static int dummy_set_joystick_on (Stage* stage)
{
	printf("dummy_set_joystick_on\n");
  	
  	return STAGE_SUCCESS;
}	

	
static int dummy_set_joystick_off (Stage* stage)
{
	printf("dummy_set_joystick_off\n");
  	
  	return STAGE_SUCCESS;
}		
	
	
static int dummy_set_timeout (Stage* stage, double timeout)
{
	printf("dummy_set_timeout\n");
  	
  	return STAGE_SUCCESS;
}	


static  int dummy_save_settings (Stage* stage, const char *filepath)
{
	printf("dummy_save_settings\n");
  	
  	return STAGE_SUCCESS;
}	


static int dummy_load_settings (Stage* stage, const char *filepath)
{
	printf("dummy_load_settings\n");
  	
  	return STAGE_SUCCESS;
}	


static int dummy_save_settings_as_default (Stage* stage)
{
	printf("dummy_save_settings_as_default\n");
  	
  	return STAGE_SUCCESS;
}		
	
	
static int dummy_power_up(Stage* stage)
{
	printf("dummy_power_up\n");
  	
  	return STAGE_SUCCESS;
}
	
	
static int dummy_power_down(Stage* stage)
{
	printf("dummy_power_down\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_get_x_datum (Stage* stage, double *x)
{
	printf("dummy_get_x_datum\n");
  	
  	return STAGE_SUCCESS;
}


static int dummy_reset(Stage* stage)
{
	printf("reset\n");
  	
  	return STAGE_SUCCESS;
}



Stage* stage_dummy_new(void)
{
	Stage* stage = stage_new();

	dummy_ops.init = dummy_init;
	dummy_ops.destroy = dummy_destroy;
	dummy_ops.power_up = dummy_power_up;
	dummy_ops.power_down = dummy_power_down;
	dummy_ops.reset = dummy_reset;
	dummy_ops.get_info = dummy_get_info;
	dummy_ops.self_test = dummy_self_test;
	dummy_ops.enable_axis = dummy_enable_axis;
	dummy_ops.disable_axis = dummy_disable_axis;
	dummy_ops.calibrate_extents = dummy_calibrate_extents;
	dummy_ops.set_speed = dummy_set_speed;
	dummy_ops.set_acceleration = dummy_set_acceleration;
	dummy_ops.async_goto_xyz_position = dummy_async_goto_xyz_position;
	dummy_ops.get_xyz_position = dummy_get_xyz_position;
	dummy_ops.async_rel_move_by = dummy_async_rel_move_by;
	dummy_ops.set_xyz_datum = dummy_set_xyz_datum;
	dummy_ops.is_moving = dummy_is_moving;
	dummy_ops.set_joystick_speed = dummy_set_joystick_speed;
	dummy_ops.set_joystick_on = dummy_set_joystick_on;
	dummy_ops.set_joystick_off = dummy_set_joystick_off;
	dummy_ops.set_timeout = dummy_set_timeout;
	dummy_ops.save_settings = dummy_save_settings; 
	dummy_ops.load_settings = dummy_load_settings; 
	
	stage_set_operations(stage, &dummy_ops); 
	
	return stage;	
}
