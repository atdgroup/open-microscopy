#include <cviauto.h>

#include "ATD_CubeSlider_Dummy.h"
#include "CubeSliderUI.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

#define MANUAL_CUBE_POSITIONS 5

int Manual_fluo_manager_hardware_init (FluoCubeManager* cube_manager)
{
	return CUBE_MANAGER_SUCCESS;
}

int Manual_fluo_manager_destroy (FluoCubeManager* cube_manager)
{
	FluoCubeManagerManual * cube_manager_manual = (FluoCubeManagerManual *) cube_manager;
	
	return CUBE_MANAGER_SUCCESS;
}

static int Manual_move_to_cube_position(FluoCubeManager* cube_manager, int position)
{
	FluoCubeManagerManual * cube_manager_manual = (FluoCubeManagerManual *) cube_manager;
	
	//Move filter cassette to specified position, (1 to MANUAL_CUBE_POSITIONS).
	if ((position < 1) || ( position > MANUAL_CUBE_POSITIONS))
		return CUBE_MANAGER_ERROR;
	
	cube_manager_manual->stored_position = position;

	return CUBE_MANAGER_SUCCESS;
}

static int Manual_get_current_cube_position (FluoCubeManager* cube_manager, int *position)
{
	FluoCubeManagerManual * cube_manager_manual = (FluoCubeManagerManual *) cube_manager;
	
	//Read current cube position, (1 to MANUAL_CUBE_POSITIONS).
	*position = cube_manager_manual->stored_position;
	
	if ((*position < 1) || ( *position > MANUAL_CUBE_POSITIONS))
		return CUBE_MANAGER_ERROR;
	
	return CUBE_MANAGER_SUCCESS;
}


FluoCubeManager* Manual_fluo_cube_manager_new(const char *name, const char *description, const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data)
{
	int mounted = 1;
	
	FluoCubeManager* cube_manager = cube_manager_new(name, description, data_dir, filepath, sizeof(FluoCubeManagerManual));
	
	FluoCubeManagerManual * cube_manager_manual = (FluoCubeManagerManual *) cube_manager; 
	
	cube_manager_manual->stored_position = 1;
	
	ui_module_set_error_handler(UIMODULE_CAST(cube_manager_manual), error_handler, data); 
	
	CUBE_MANAGER_VTABLE_PTR(cube_manager, hardware_init) = Manual_fluo_manager_hardware_init;  
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = Manual_fluo_manager_destroy;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = Manual_move_to_cube_position; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = Manual_get_current_cube_position; 

	hardware_device_set_as_manual(HARDWARE_DEVICE_CAST(cube_manager));

	return cube_manager;
}
