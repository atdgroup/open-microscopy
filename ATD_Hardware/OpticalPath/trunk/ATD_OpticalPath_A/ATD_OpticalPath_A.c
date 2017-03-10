#include <cviauto.h>

#include "ATD_OpticalPath_A.h"
#include "OpticalPathUI.h"
#include "ATD_StepperMotor_A\ATD_StepperMotor_A.h"
#include "gci_utils.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

int atd_op_a_optical_path_manager_hw_init (OpticalPathManager* optical_path_manager, int move_to_default)
{
	int left, top;
	ATD_OP_A *atd_op_a_manager =  (ATD_OP_A *) optical_path_manager;   
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	ftdi_controller_open(atd_op_a_manager->_controller, atd_op_a_manager->device_sn);

	#else

	initialise_comport(atd_op_a_manager->_com_port, 9600);    

	#endif

	if (mirror_stepper_initialise(atd_op_a_manager->mirror_stepper, move_to_default) == MIRROR_STEPPER_ERROR)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	// Redirect errors
	mirror_stepper_set_error_handler(atd_op_a_manager->mirror_stepper, UIMODULE_CAST(optical_path_manager)->_error_handler); 

	GetPanelAttribute(UIMODULE_MAIN_PANEL_ID(optical_path_manager), ATTR_LEFT, &left);
	GetPanelAttribute(UIMODULE_MAIN_PANEL_ID(optical_path_manager), ATTR_TOP, &top);
	SetPanelPos (UIMODULE_MAIN_PANEL_ID(atd_op_a_manager->mirror_stepper), top, left);

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int atd_op_a_optical_path_manager_destroy (OpticalPathManager* optical_path_manager)
{
	ATD_OP_A *atd_op_a_manager =  (ATD_OP_A *) optical_path_manager;   
	
	mirror_stepper_destroy(atd_op_a_manager->mirror_stepper);
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(atd_op_a_manager->_controller);
	#else
	close_comport(atd_op_a_manager->_com_port);
	#endif

	return OPTICAL_PATH_MANAGER_SUCCESS;
}


static int atd_op_a_move_to_optical_path_position(OpticalPathManager* optical_path_manager, int position)
{
	int err;
	
	ATD_OP_A *atd_op_a_manager =  (ATD_OP_A *) optical_path_manager;   

	//Move filter cassette to specified position, (1 to OPTICAL_PATH_TURRET_SIZE).

	if (optical_path_manager == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	err = mirror_stepper_set_pos(atd_op_a_manager->mirror_stepper, position);
	
	if (err)
		return OPTICAL_PATH_MANAGER_ERROR;

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int atd_op_a_get_current_optical_path_position (OpticalPathManager* optical_path_manager, int *position)
{
	ATD_OP_A *atd_op_a_manager =  (ATD_OP_A *) optical_path_manager;   
    
	//Get current optical path position, (1 to OPTICAL_PATH_TURRET_SIZE).
	if (optical_path_manager == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	*position = mirror_stepper_get_pos(atd_op_a_manager->mirror_stepper);

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int atd_op_a_optical_path_setup (OpticalPathManager* optical_path_manager)
{
	ATD_OP_A *atd_op_a_manager =  (ATD_OP_A *) optical_path_manager;   
    
	//Display the Minipak mirror ui

	if (optical_path_manager == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	mirror_stepper_display_calib_ui(atd_op_a_manager->mirror_stepper, optical_path_manager->_main_ui_panel);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int atd_op_a_hide_optical_path_calib (OpticalPathManager* optical_path_manager)
{
	ATD_OP_A *atd_op_a_manager =  (ATD_OP_A *) optical_path_manager;   
    
	//Hide the Minipak mirror ui

	if (optical_path_manager == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	mirror_stepper_hide_calib_ui(atd_op_a_manager->mirror_stepper);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

OpticalPathManager* atd_op_a_optical_path_new(const char *name, const char *description, const char* data_dir, const char *file)
{
	int i2c_bus, i2c_address, i2c_pic;
	char mirror_stepper_name[500] = "";
	char mirror_stepper_description[500] = "";
	
	OpticalPathManager* optical_path_manager = optical_path_manager_new(name, description, data_dir, file, sizeof(ATD_OP_A));
	
	ATD_OP_A *atd_op_a_manager =  (ATD_OP_A *) optical_path_manager; 
	
	optical_path_manager->_initialised = 0;
	
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, hw_init) = atd_op_a_optical_path_manager_hw_init; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, destroy) = atd_op_a_optical_path_manager_destroy;
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, move_to_optical_path_position) = atd_op_a_move_to_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, get_current_optical_path_position) = atd_op_a_get_current_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, setup_optical_path) = atd_op_a_optical_path_setup; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, hide_optical_path_calib) = atd_op_a_hide_optical_path_calib; 

	sprintf(mirror_stepper_name, "%s_MirrorStepper_Settings", name);
	sprintf(mirror_stepper_description, "%s MirrorStepper", description);

	atd_op_a_manager->mirror_stepper = mirror_stepper_new(mirror_stepper_name, mirror_stepper_description);

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	atd_op_a_manager->_controller = ftdi_controller_new();

	ftdi_controller_get_lock(atd_op_a_manager->_controller);
	
	atd_op_a_manager->_com_port = 0;
	get_device_string_param_from_ini_file(name, "FTDI_SN", atd_op_a_manager->device_sn);  
	get_device_int_param_from_ini_file   (name, "I2C_DEVICE_BUS", &i2c_bus);
	get_device_int_param_from_ini_file   (name, "I2C_DEVICE_ADDRESS", &i2c_address);  

//	ftdi_controller_set_debugging(atd_op_a_manager->_controller, 1);
	ftdi_controller_set_error_handler(atd_op_a_manager->_controller, ftdi_error_handler, NULL);

	mirror_stepper_set_ftdi_controller(atd_op_a_manager->mirror_stepper , atd_op_a_manager->_controller, 0, i2c_bus, i2c_address);

	#else

	get_device_param_from_ini_file(name, "i2c_Bus", &i2c_bus);
	get_device_param_from_ini_file(name, "i2c_ChipAddress", &i2c_address);  
	get_device_param_from_ini_file(name, "i2c_ChipType", &i2c_pic);  
	get_device_param_from_ini_file(name, "COM_Port", &(atd_op_a_manager->_com_port));  
	
	mirror_stepper_set_com_port_i2c_details(atd_op_a_manager->mirror_stepper, atd_op_a_manager->_com_port, 
		i2c_pic, i2c_bus, i2c_address);

	#endif

	
	
	mirror_stepper_set_data_dir(atd_op_a_manager->mirror_stepper, data_dir);    
	
	if (mirror_stepper_init (atd_op_a_manager->mirror_stepper) == MIRROR_STEPPER_ERROR)
		goto Error;
	
	// Redirect mirror stepper error handler to optical path handler
	mirror_stepper_set_error_handler(atd_op_a_manager->mirror_stepper, NULL);
	
	if (atd_op_a_manager->mirror_stepper == NULL)
		goto Error;

	SetCtrlAttribute (optical_path_manager->_main_ui_panel, OPATH_PNL_CALIBRATE, ATTR_VISIBLE, 1);

	return optical_path_manager;
	
	Error:
	
	return NULL;
}
