#include "ATD_CubeSlider_A.h"
#include "CubeSliderUI.h"

#include "gci_utils.h"
#include "ATD_UsbInterface_A.h"

#include "asynctmr.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>
#include <cviauto.h>

////////////////////////////////////////////////////////////////////////////
//GP/RJL/RGN April 2007
//HTS Microscope system. 
//Fluorescent cube control.
////////////////////////////////////////////////////////////////////////////

//#define DEBUG

//#define CUBE_PIC		0x64  	//Address programmed into cube slider PIC;
//#define CUBE_BUS 		2		//Set to required bus(on MPTR system) else set to 2
//#define CUBE_ADDRESS 	0		//Programable address of PIC

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

int atd_cubeslider_a_fluo_manager_hardware_init (FluoCubeManager* cube_manager)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;  

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	atd_cubeslider_a_cube_manager->_controller = ftdi_controller_new();

//	ftdi_controller_set_debugging(atd_cubeslider_a_cube_manager->_controller, 1);
	ftdi_controller_set_error_handler(atd_cubeslider_a_cube_manager->_controller, ftdi_error_handler, NULL);
	ftdi_controller_open(atd_cubeslider_a_cube_manager->_controller, atd_cubeslider_a_cube_manager->_device_sn);

	#else

	initialise_comport(atd_cubeslider_a_cube_manager->_com_port, 9600);
	
	#endif

	return CUBE_MANAGER_SUCCESS;
}

int atd_cubeslider_a_fluo_manager_destroy (FluoCubeManager* cube_manager)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;   
	
	#ifndef SINGLE_THREADED_POLLING   
	DiscardAsyncTimer(cube_manager->_timer);
	#endif

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(atd_cubeslider_a_cube_manager->_controller);
	#else
	close_comport(atd_cubeslider_a_cube_manager->_com_port);
	#endif

	return CUBE_MANAGER_SUCCESS;
}


static int read_cube_position(FluoCubeManager* cube_manager, int *position)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;   
	byte data[10];   
	int mode, err, number_of_cubes;

	mode = 255; 			 //Set mode for reading  

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	ftdi_controller_get_lock(atd_cubeslider_a_cube_manager->_controller);
	
	memset(data, 0, 10);     
	
	//NB Cube PIC expects 6 or more bytes even if they aren't relevant
	data[0] = mode;

	if(ftdi_controller_i2c_write_bytes(atd_cubeslider_a_cube_manager->_controller, atd_cubeslider_a_cube_manager->_i2c_chip_address, 6, data) != FT_OK) {
		ftdi_controller_release_lock(atd_cubeslider_a_cube_manager->_controller);
		return CUBE_MANAGER_ERROR;
	}

	if(ftdi_controller_i2c_read_bytes(atd_cubeslider_a_cube_manager->_controller, atd_cubeslider_a_cube_manager->_i2c_chip_address, 1, data) != FT_OK) {
		ftdi_controller_release_lock(atd_cubeslider_a_cube_manager->_controller);
		return CUBE_MANAGER_ERROR;
	}
   		
	*position = data[0] & 0xff; 
	
	ftdi_controller_release_lock(atd_cubeslider_a_cube_manager->_controller);  
	
	#else

	GetI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube read_cube_position");  
	
	memset(data, 0, 10);     
	
	//NB Cube PIC expects 6 or more bytes even if they aren't relevant
	
	data[0] = atd_cubeslider_a_cube_manager->_i2c_chip_type | (atd_cubeslider_a_cube_manager->_i2c_chip_address <<1);
	data[1] = mode;
	err = GCI_writeI2C_multiPort(atd_cubeslider_a_cube_manager->_com_port, 6, data, atd_cubeslider_a_cube_manager->_i2c_bus, "Cube read_cube_position");  	
	
	if (err) {
		ReleaseI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube read_cube_position"); 
		logger_log(UIMODULE_LOGGER(cube_manager), LOGGER_ERROR, "Cube Error", "read_cube_position failed - GCI_writeI2C_multiPort failed");

		return CUBE_MANAGER_ERROR;
	}

	data[0] = atd_cubeslider_a_cube_manager->_i2c_chip_type | (atd_cubeslider_a_cube_manager->_i2c_chip_address <<1) | 0x01;
		
	err = GCI_readI2C_multiPort(atd_cubeslider_a_cube_manager->_com_port, 1, data, atd_cubeslider_a_cube_manager->_i2c_bus, "Cube read_cube_position");
	
	if (err) {
		ReleaseI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube read_cube_position");   
		logger_log(UIMODULE_LOGGER(cube_manager), LOGGER_ERROR, "Cube Error", "read_cube_position failed - GCI_readI2C_multiPort failed");

		return CUBE_MANAGER_ERROR;
	}

	*position = data[0] & 0xff; 
	
	ReleaseI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube read_cube_position");    
	
	#endif

	cube_manager_get_number_of_cubes(cube_manager, &number_of_cubes);

	if ((*position < 1) || ( *position > number_of_cubes))
		return CUBE_MANAGER_ERROR;
	
	return CUBE_MANAGER_SUCCESS;  
}

static int atd_cubeslider_a_move_to_cube_position(FluoCubeManager* cube_manager, int position)
{
	int mode, err, number_of_cubes;
	byte data[10];
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;   
	
	memset(data, 0, 10);
	
	//Move filter cassette to specified position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;
	
	cube_manager_get_number_of_cubes(cube_manager, &number_of_cubes);
	
	if ((position < 1) || ( position > number_of_cubes)) {		
		return CUBE_MANAGER_ERROR;
	}

	mode = 0; 		//Set position mode

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	//NB Cube PIC expects 6 or more bytes even if they aren't relevant
	data[0] = mode;
	data[1] = position;

	if(ftdi_controller_i2c_write_bytes(atd_cubeslider_a_cube_manager->_controller, atd_cubeslider_a_cube_manager->_i2c_chip_address, 6, data) != FT_OK) {
		ftdi_controller_release_lock(atd_cubeslider_a_cube_manager->_controller);
		return CUBE_MANAGER_ERROR;
	}

	#else

	GetI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube atd_cubeslider_a_move_to_cube_position");    
	
	//NB Cube PIC expects 6 or more bytes even if they aren't relevant
	data[0] = mode;
	data[1] = position;
	err = GCI_Out_PIC_multiPort (atd_cubeslider_a_cube_manager->_com_port, atd_cubeslider_a_cube_manager->_i2c_bus,
		atd_cubeslider_a_cube_manager->_i2c_chip_type, atd_cubeslider_a_cube_manager->_i2c_chip_address, 6, data);

	ReleaseI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube atd_cubeslider_a_move_to_cube_position");  
	
	if (err) {
		printf("Err", "GCI_Out_PIC_multiPort returned error %d\n", err);	
		return CUBE_MANAGER_ERROR;
	}

	#endif

	return CUBE_MANAGER_SUCCESS;
}


static int atd_cubeslider_a_get_current_cube_position (FluoCubeManager* cube_manager, int *position)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;   
	int error_count = 0;
    
	//Read current cube position, (1 to CUBE_TURRET_SIZE).
	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;

	do {
		
		if(read_cube_position(cube_manager, position) == CUBE_MANAGER_SUCCESS)
			goto SUCCESS;
		
		// Don't cram to much on the i2c bus
		Delay(0.3);
	}
	while(error_count++ < 5);

	return CUBE_MANAGER_ERROR;
	
	SUCCESS:

	return CUBE_MANAGER_SUCCESS;
}

FluoCubeManager* atd_cubeslider_a_new(const char *name, const char *description, const char *data_dir, const char *file, UI_MODULE_ERROR_HANDLER error_handler, void *data)
{
	FluoCubeManager* cube_manager = cube_manager_new(name, description, data_dir, file, sizeof(ATD_CUBESLIDER_A));
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;
	
	ui_module_set_error_handler(UIMODULE_CAST(cube_manager), error_handler, data); 
	
	CUBE_MANAGER_VTABLE_PTR(cube_manager, hardware_init) = atd_cubeslider_a_fluo_manager_hardware_init;   
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = atd_cubeslider_a_fluo_manager_destroy;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = atd_cubeslider_a_move_to_cube_position; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = atd_cubeslider_a_get_current_cube_position; 
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	get_device_string_param_from_ini_file(name, "FTDI_SN", atd_cubeslider_a_cube_manager->_device_sn);  
	get_device_int_param_from_ini_file   (name, "I2C_DEVICE_BUS", &(atd_cubeslider_a_cube_manager->_i2c_bus));
	get_device_int_param_from_ini_file   (name, "I2C_DEVICE_ADDRESS", &(atd_cubeslider_a_cube_manager->_i2c_chip_address));  
	
	atd_cubeslider_a_cube_manager->_controller = ftdi_controller_new();

	#else

	get_device_param_from_ini_file(name, "i2c_Bus", &(atd_cubeslider_a_cube_manager->_i2c_bus));
	get_device_param_from_ini_file(name, "i2c_ChipAddress", &(atd_cubeslider_a_cube_manager->_i2c_chip_address));  
	get_device_param_from_ini_file(name, "i2c_ChipType", &(atd_cubeslider_a_cube_manager->_i2c_chip_type));  
	get_device_param_from_ini_file(name, "COM_Port", &(atd_cubeslider_a_cube_manager->_com_port));  
	
	#endif

	return cube_manager;
}
