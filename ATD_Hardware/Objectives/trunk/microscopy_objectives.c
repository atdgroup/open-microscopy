#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "microscopy_objectives.h"
#include "com_port_control.h"
#include "usbconverter_v2.h"
#include "IO_interface_v2.h"


static int microscopy_destroy (ObjectiveManager* objective_manager)
{
	return OBJECTIVE_MANAGER_SUCCESS;
}


static int microscopy_move_to_turret_position(ObjectiveManager* objective_manager, int position, int linkage)
{
	char command[10]="";

	//Rotate nosepiece to specified position, (1 to 6).
	//linkage = 1 gives independent control.
	//Set this to "0" to link the move to the condenser

	if (objective_manager == NULL) return OBJECTIVE_MANAGER_ERROR;
	if (objective_manager->_mounted != 1) return OBJECTIVE_MANAGER_ERROR;
	
	if ((linkage < 0) || ( linkage > 1)) return OBJECTIVE_MANAGER_ERROR;
	if ((position < 1) || ( position > 6)) return OBJECTIVE_MANAGER_ERROR;
	
	sprintf(command, "fRDM%d%d\r", linkage, position);
	if (TE2000_tx_command_ftype(objective_manager->_te2000, command, 10.0) != TE2000_SUCCESS)
		return OBJECTIVE_MANAGER_ERROR;

	return OBJECTIVE_MANAGER_SUCCESS;
}

static int microscopy_get_current_turret_position(ObjectiveManager* objective_manager, int *position)
{
	int val;

	//Read current nosepiece position, (1 to 6).

	if (objective_manager == NULL) return OBJECTIVE_MANAGER_ERROR;
	if (objective_manager->_mounted != 1) return OBJECTIVE_MANAGER_ERROR;
	
	if (TE2000_rx_command_int(objective_manager->_te2000, "rRAR\r", &val) != TE2000_SUCCESS)
		return OBJECTIVE_MANAGER_ERROR;

	*position = val;
	if ((*position < 1) || ( *position > 6)) return OBJECTIVE_MANAGER_ERROR;
	
	return OBJECTIVE_MANAGER_SUCCESS;
}

static int microscopy_nosepiece_mounted(ObjectiveManager* objective_manager)
{
	int val;
	
	//Is objective turret mounted? 0 - no, 1 - yes, 2 - error
	if (objective_manager == NULL) return OBJECTIVE_MANAGER_ERROR;

	if (TE2000_rx_command_int(objective_manager->_te2000, "rRCR\r", &val) != 0)
		return OBJECTIVE_MANAGER_ERROR;
	
	objective_manager->_mounted = val;
	
	return OBJECTIVE_MANAGER_SUCCESS;
}


ObjectiveManager* microscopy_objective_manager_new(char *filepath)
{
	ObjectiveManager* objective_manager = objective_manager_new("Objectives", "Objectives", filepath);

	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, destroy) = microscopy_destroy;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, move_to_turret_position) = microscopy_move_to_turret_position;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, nosepiece_mounted) = microscopy_nosepiece_mounted;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, get_current_turret_position) = microscopy_get_current_turret_position;

	return objective_manager;
}
