#include <cviauto.h>

#include "manual_condensers.h"
#include "condensers_ui.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Condenser control for manual microscope.
////////////////////////////////////////////////////////////////////////////


	
static int Manual_condenser_hw_init (CondenserManager* condenser_manager)
{
	return CONDENSER_MANAGER_SUCCESS;        	
}

static int Manual_condenser_init (CondenserManager* condenser_manager)
{
	int pos=0;
	
	// Use the default pos to start with

	// But this does not work as intended. Why?
	// The load_ui in microscope.c calls to manual devices to load the settings from last session and call to move devices to new positions.
	// That causes the popup for the user to move the manual position, to the position it was in last time! (if not 1, as 1 is hard coded usually in the _new fn)
	// The ui is loaded after that
	// This fn here gets called later, to set the default pos from the config ui

	if(device_conf_get_default_position(condenser_manager->dc, &pos) == DEVICE_CONF_ERROR)
		return CONDENSER_MANAGER_ERROR;
	condenser_manager->_current_pos = pos;

	return CONDENSER_MANAGER_SUCCESS;        	
}

int Manual_condenser_manager_destroy (CondenserManager* condenser_manager)
{
	CondenserManagerManual * condenser_manager_manual = (CondenserManagerManual *) condenser_manager; 
	
	return CONDENSER_MANAGER_SUCCESS;
}

static int Manual_move_to_condenser_position(CondenserManager* condenser_manager, int position)
{
	CondenserManagerManual * condenser_manager_manual = (CondenserManagerManual *) condenser_manager; 
	char info[UIMODULE_NAME_LEN];
	
	//Move filter cassette to specified position, (1 to CONDENSER_TURRET_SIZE).

	if (condenser_manager == NULL) return CONDENSER_MANAGER_ERROR;
	if (condenser_manager->_mounted != 1) return CONDENSER_MANAGER_ERROR;
	
	if ((position < 1) || ( position > CONDENSER_TURRET_SIZE)) return CONDENSER_MANAGER_ERROR;

	if (condenser_manager_manual->_current_position == position)  // already at the position
		return CONDENSER_MANAGER_SUCCESS;
	
	condenser_manager_manual->_current_position = position;

	// We are a manual condenser so we can't automatically move.
	hardware_device_get_current_value_text((HardwareDevice *)condenser_manager, info);
	GCI_MessagePopup("Manual Condenser", "Please change the condenser on the microscope to %s.", info);

//	condenser_manager_on_change(condenser_manager);
	
	return CONDENSER_MANAGER_SUCCESS;
}

static int Manual_get_current_condenser_position (CondenserManager* condenser_manager, int *position)
{
	CondenserManagerManual * condenser_manager_manual = (CondenserManagerManual *) condenser_manager; 
	
	//Read current condenser position, (1 to CONDENSER_TURRET_SIZE).

	if (condenser_manager == NULL) return CONDENSER_MANAGER_ERROR;
	if (condenser_manager->_mounted != 1) return CONDENSER_MANAGER_ERROR;
	
	*position = condenser_manager_manual->_current_position;

	if ((*position < 1) || ( *position > CONDENSER_TURRET_SIZE)) return CONDENSER_MANAGER_ERROR;
	
	return CONDENSER_MANAGER_SUCCESS;
}


CondenserManager* Manual_condenser_manager_new(const char *name, const char *description, const char *data_dir, const char *filepath)
{
	int mounted = 1;
	
	CondenserManager* condenser_manager = condenser_manager_new("Condenser", "Condenser", data_dir, filepath, sizeof(CondenserManagerManual));
	
	CondenserManagerManual * condenser_manager_manual = (CondenserManagerManual *) condenser_manager;
	
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, hardware_init) = Manual_condenser_hw_init;   
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, initialise) = Manual_condenser_init;   
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, destroy) = Manual_condenser_manager_destroy;
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, move_to_condenser_position) = Manual_move_to_condenser_position; 
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, get_current_condenser_position) = Manual_get_current_condenser_position; 

	condenser_manager->_mounted = mounted;
	SetCtrlAttribute (condenser_manager->_main_ui_panel, CONDENSER_TURRET_POS, ATTR_DIMMED, !condenser_manager->_mounted);

	condenser_manager_manual->_current_position = 1;
	
	hardware_device_set_as_manual(HARDWARE_DEVICE_CAST(condenser_manager));

	return condenser_manager;
}
