#include "ATD_Objectives_Dummy.h"
#include "gci_ui_module.h"
#include "ObjectivesUI.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

// This module is for an object that only has 1 objective at a time.

typedef struct
{
	ObjectiveManager parent;
	
	int pos;
	
} DummyObjectiveManager;


static int Dummy_destroy (ObjectiveManager* objective_manager)
{
	return OBJECTIVE_MANAGER_SUCCESS;
}


static int Dummy_move_to_turret_position(ObjectiveManager* objective_manager, int position)
{
	DummyObjectiveManager* om_om = (DummyObjectiveManager*) objective_manager;
	char info[UIMODULE_NAME_LEN];
	
	if (position < 1)
		return OBJECTIVE_MANAGER_ERROR;

	if (om_om->pos == position)  // already at the position
		return OBJECTIVE_MANAGER_SUCCESS;

	om_om->pos = position;

	// We are a manual objective so we can't automatically move.
	hardware_device_get_current_value_text((HardwareDevice *)objective_manager, info);
	GCI_MessagePopup("Manual Objective", "Please change the objective on the microscope to %s.\n(or correct the setting in the software)", info);

	return OBJECTIVE_MANAGER_SUCCESS;
}

static int Dummy_get_current_turret_position(ObjectiveManager* objective_manager, int *position)
{
	DummyObjectiveManager* om_om = (DummyObjectiveManager*) objective_manager;
	
	*position = om_om->pos;
	
	return OBJECTIVE_MANAGER_SUCCESS;
}

static int Dummy_hw_init (ObjectiveManager* objective_manager)
{
	return OBJECTIVE_MANAGER_SUCCESS;        	
}

ObjectiveManager* Dummy_objective_manager_new(const char *name, const char *description, const char *data_dir, const char *filepath)
{
	ObjectiveManager* objective_manager = objective_manager_new(name, description, data_dir, filepath, sizeof(DummyObjectiveManager));
	DummyObjectiveManager* om_om = (DummyObjectiveManager*) objective_manager;
	
	if(objective_manager == NULL)
		return NULL;
	
	om_om->pos = 1;
	
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, hw_init) = Dummy_hw_init;   
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, destroy) = Dummy_destroy;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, move_to_turret_position) = Dummy_move_to_turret_position;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, get_current_turret_position) = Dummy_get_current_turret_position;

	hardware_device_set_as_manual(HARDWARE_DEVICE_CAST(objective_manager));

	return objective_manager;
}
