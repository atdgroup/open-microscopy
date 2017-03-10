#include <cviauto.h>

#include "manual_fluorescent_cubes.h"
#include "fluorescent_cubes_ui.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Fluorescent cube control for manual microscope.
////////////////////////////////////////////////////////////////////////////

int Manual_fluo_manager_destroy (FluoCubeManager* cube_manager)
{
	FluoCubeManagerManual * cube_manager_manual = (FluoCubeManagerManual *) cube_manager;
	
	return CUBE_MANAGER_SUCCESS;
}

static int Manual_move_to_cube_position(FluoCubeManager* cube_manager, int position)
{
	FluoCubeManagerManual * cube_manager_manual = (FluoCubeManagerManual *) cube_manager;
	
	//Move filter cassette to specified position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL) return CUBE_MANAGER_ERROR;
	if (cube_manager->_mounted != 1) return CUBE_MANAGER_ERROR;
	
	if ((position < 1) || ( position > CUBE_TURRET_SIZE)) return CUBE_MANAGER_ERROR;
	
	cube_manager->_current_position = position;
	cube_manager_on_change(cube_manager);

	return CUBE_MANAGER_SUCCESS;
}

static int Manual_get_current_cube_position (FluoCubeManager* cube_manager, int *position)
{
	FluoCubeManagerManual * cube_manager_manual = (FluoCubeManagerManual *) cube_manager;
	
	//Read current cube position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL) return CUBE_MANAGER_ERROR;
	if (cube_manager->_mounted != 1) return CUBE_MANAGER_ERROR;
	
	*position = cube_manager->_current_position;
	if ((*position < 1) || ( *position > CUBE_TURRET_SIZE)) return CUBE_MANAGER_ERROR;
	
	return CUBE_MANAGER_SUCCESS;
}


FluoCubeManager* Manual_fluo_cube_manager_new(const char *filepath)
{
	int mounted = 1;
	
	FluoCubeManager* cube_manager = cube_manager_new("Fluorescent Cubes", "Fluorescent Cubes", filepath, sizeof(FluoCubeManagerManual));
	
	FluoCubeManagerManual * cube_manager_manual = (FluoCubeManagerManual *) cube_manager; 
	
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = Manual_fluo_manager_destroy;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = Manual_move_to_cube_position; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = Manual_get_current_cube_position; 

	cube_manager->_mounted = mounted;
	SetCtrlAttribute (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, ATTR_DIMMED, !cube_manager->_mounted);

	cube_manager->_current_position = 1;
	
	return cube_manager;
}
