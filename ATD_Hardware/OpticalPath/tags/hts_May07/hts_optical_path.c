#include <cviauto.h>


#include "hts_optical_path.h"
#include "optical_path_ui.h"
#include "minipak mirror stepper.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI HTS Microscope system. 
//Optical path control uses Minipak mirror stepper.
////////////////////////////////////////////////////////////////////////////

int hts_optical_path_manager_destroy (OpticalPathManager* optical_path_manager)
{
	OpticalPathHTS *optical_path_manager_hts =  (OpticalPathHTS *) optical_path_manager;   
	
	mirror_stepper_destroy(optical_path_manager_hts->mirror_stepper);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


static int hts_move_to_optical_path_position(OpticalPathManager* optical_path_manager, int position)
{
	int err;
	
	OpticalPathHTS *optical_path_manager_hts =  (OpticalPathHTS *) optical_path_manager;   

	//Move filter cassette to specified position, (1 to OPTICAL_PATH_TURRET_SIZE).

	if (optical_path_manager == NULL) return OPTICAL_PATH_MANAGER_ERROR;
	if (optical_path_manager->_mounted != 1) return OPTICAL_PATH_MANAGER_ERROR;
	
	if ((position < 1) || ( position > OPTICAL_PATH_TURRET_SIZE)) return OPTICAL_PATH_MANAGER_ERROR;
	
	err = mirror_stepper_set_pos(optical_path_manager_hts->mirror_stepper, position);
	if (err) {
		send_optical_path_error_text (optical_path_manager, "Failed to optical path position");     
		return OPTICAL_PATH_MANAGER_ERROR;
	}

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int hts_get_current_optical_path_position (OpticalPathManager* optical_path_manager, int *position)
{
	OpticalPathHTS *optical_path_manager_hts =  (OpticalPathHTS *) optical_path_manager;   
    
	//Get current optical path position, (1 to OPTICAL_PATH_TURRET_SIZE).

	if (optical_path_manager == NULL) return OPTICAL_PATH_MANAGER_ERROR;
	if (optical_path_manager->_mounted != 1) return OPTICAL_PATH_MANAGER_ERROR;
	
	*position = mirror_stepper_get_pos(optical_path_manager_hts->mirror_stepper);

	if ((*position < 1) || ( *position > OPTICAL_PATH_TURRET_SIZE)) return OPTICAL_PATH_MANAGER_ERROR;
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int hts_optical_path_setup (OpticalPathManager* optical_path_manager)
{
	OpticalPathHTS *optical_path_manager_hts =  (OpticalPathHTS *) optical_path_manager;   
    
	//Display the Minipak mirror ui

	if (optical_path_manager == NULL) return OPTICAL_PATH_MANAGER_ERROR;
	if (optical_path_manager->_mounted != 1) return OPTICAL_PATH_MANAGER_ERROR;
	
	mirror_stepper_display_calib_ui(optical_path_manager_hts->mirror_stepper);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int hts_hide_optical_path_calib (OpticalPathManager* optical_path_manager)
{
	OpticalPathHTS *optical_path_manager_hts =  (OpticalPathHTS *) optical_path_manager;   
    
	//Hide the Minipak mirror ui

	if (optical_path_manager == NULL) return OPTICAL_PATH_MANAGER_ERROR;
	if (optical_path_manager->_mounted != 1) return OPTICAL_PATH_MANAGER_ERROR;
	
	mirror_stepper_hide_calib_ui(optical_path_manager_hts->mirror_stepper);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

OpticalPathManager* hts_optical_path_manager_new(const char *filepath)
{
	OpticalPathManager* optical_path_manager = optical_path_manager_new("Optical Path", "Optical Path", filepath, sizeof(OpticalPathHTS));
	
	OpticalPathHTS *optical_path_manager_hts =  (OpticalPathHTS *) optical_path_manager; 
	
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, destroy) = hts_optical_path_manager_destroy;
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, move_to_optical_path_position) = hts_move_to_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, get_current_optical_path_position) = hts_get_current_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, setup_optical_path) = hts_optical_path_setup; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, hide_optical_path_calib) = hts_hide_optical_path_calib; 

	optical_path_manager_hts->mirror_stepper = mirror_stepper_new("1", "Optical path");
	if (optical_path_manager_hts->mirror_stepper == NULL)
		goto Error;

	optical_path_manager->_mounted = 1;
	SetCtrlAttribute (optical_path_manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, ATTR_DIMMED, !optical_path_manager->_mounted);

	optical_path_manager_on_change(optical_path_manager); //display current position

	return optical_path_manager;
	
	Error:
	
	send_optical_path_error_text (optical_path_manager, "Failed to create optical path");     
	
	return NULL;
}
