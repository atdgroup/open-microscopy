#include <cviauto.h>

#include "ATD_OpticalPath_Dummy.h"
#include "OpticalPathUI.h"
#include "gci_utils.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

int manual_optical_path_manager_hw_init (OpticalPathManager* optical_path_manager, int move_to_default)
{
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int manual_optical_path_manager_destroy (OpticalPathManager* optical_path_manager)
{
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


static int manual_move_to_optical_path_position(OpticalPathManager* optical_path_manager, int position)
{
	ManualOpticalPath *optical_path_manager_manual =  (ManualOpticalPath *) optical_path_manager;   
	char info[UIMODULE_NAME_LEN];
	
	if (position < 1)
		return OPTICAL_PATH_MANAGER_ERROR;

	if (optical_path_manager_manual->manual_position == position)  // already at the position
		return OPTICAL_PATH_MANAGER_SUCCESS;

	optical_path_manager_manual->manual_position = position;

	hardware_device_get_current_value_text((HardwareDevice *)optical_path_manager, info);
	GCI_MessagePopup("Manual Optical Path", "Please change the optical path on the microscope to %s.", info);

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int manual_get_current_optical_path_position (OpticalPathManager* optical_path_manager, int *position)
{
	ManualOpticalPath *optical_path_manager_manual =  (ManualOpticalPath *) optical_path_manager;   
    
	//Get current optical path position, (1 to OPTICAL_PATH_TURRET_SIZE).
	*position = optical_path_manager_manual->manual_position;

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int manual_optical_path_setup (OpticalPathManager* optical_path_manager)
{
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int manual_hide_optical_path_calib (OpticalPathManager* optical_path_manager)
{
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

OpticalPathManager* manual_optical_path_new(const char *name, const char *description, const char* data_dir, const char *filepath)
{
	OpticalPathManager* optical_path_manager = optical_path_manager_new(name, description, data_dir, filepath, sizeof(ManualOpticalPath));
	
	ManualOpticalPath *optical_path_manager_manual =  (ManualOpticalPath *) optical_path_manager; 
	
	optical_path_manager_manual->manual_position = 1;
	
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, hw_init) = manual_optical_path_manager_hw_init; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, destroy) = manual_optical_path_manager_destroy;
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, move_to_optical_path_position) = manual_move_to_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, get_current_optical_path_position) = manual_get_current_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, setup_optical_path) = manual_optical_path_setup; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, hide_optical_path_calib) = manual_hide_optical_path_calib; 

	hardware_device_set_as_manual(HARDWARE_DEVICE_CAST(optical_path_manager));

	return optical_path_manager;
}
