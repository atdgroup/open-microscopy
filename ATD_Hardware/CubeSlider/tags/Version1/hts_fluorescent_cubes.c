#include <cviauto.h>
#include <rs232.h>

#include "hts_fluorescent_cubes.h"
#include "fluorescent_cubes_ui.h"

#include "IO_interface_v2.h" 
#include "usbconverter_v2.h"
#include "DeviceFinder.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

////////////////////////////////////////////////////////////////////////////
//GP/RJL/RGN April 2007
//HTS Microscope system. 
//Fluorescent cube control.
////////////////////////////////////////////////////////////////////////////

#define DEBUG

#define CUBE_PIC		0x64  	//Address programmed into cube slider PIC;
#define CUBE_BUS 		2		//Set to required bus(on MPTR system) else set to 2
#define CUBE_ADDRESS 	0		//Programable address of PIC


int HTS_fluo_manager_destroy (FluoCubeManager* cube_manager)
{
	FluoCubeManagerHts * cube_managerHts = (FluoCubeManagerHts *) cube_manager;
	
	GCI_closeI2C_multiPort(cube_manager->_i2c_port); 

	return CUBE_MANAGER_SUCCESS;
}

static int HTS_move_to_cube_position(FluoCubeManager* cube_manager, int position)
{
	int mode, err;
	byte data[2];
	FluoCubeManagerHts * cube_managerHts = (FluoCubeManagerHts *) cube_manager;
	
	//Move filter cassette to specified position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;
	
	if (cube_manager->_mounted != 1)
		return CUBE_MANAGER_ERROR;
	
	if ((position < 1) || ( position > CUBE_TURRET_SIZE)) return CUBE_MANAGER_ERROR;
	
	mode = 0; 		//Set position mode
	data[0] = mode;
	data[1] = position;
	err = GCI_Out_PIC_multiPort (cube_manager->_i2c_port, cube_manager->_i2c_bus, CUBE_PIC, cube_manager->_i2c_address, 2, data);

	if (err) {
		send_fluocube_error_text(cube_manager, "Failed to send cube position");
		return CUBE_MANAGER_ERROR;
	}

	return CUBE_MANAGER_SUCCESS;
}

static int HTS_get_current_cube_position (FluoCubeManager* cube_manager, int *position)
{
	byte data[2];
	int mode, err;
    
	FluoCubeManagerHts * cube_managerHts = (FluoCubeManagerHts *) cube_manager;
	
	//Read current cube position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;
	
	if (cube_manager->_mounted != 1)
		return CUBE_MANAGER_ERROR;
	
#ifndef DEBUG
	mode = 255; 			 //Set mode for reading  
	data[0] = CUBEPIC | (cube_manager->_i2c_address <<1);
	data[1] = mode;
	err = GCI_writeI2C_multiPort(cube_manager->_i2c_port, 2, data, cube_manager->_i2c_bus);  	
	if (err) {
		send_fluocube_error_text(cube_manager, "Failed to send command to cube slider");
		return CUBE_MANAGER_ERROR;
	}

	data[0] = CUBEPIC | (cube_manager->_i2c_address <<1) | 0x01;
	err = GCI_readI2C_multiPort(cube_manager->_i2c_port, 2, data, cube_manager->_i2c_bus);
	if (err) {
		send_fluocube_error_text(cube_manager, "Failed to read cube slider");
		return CUBE_MANAGER_ERROR;
	}

	*position = data[0] & 0xff; 
#else
	*position = 1;
#endif

	if ((*position < 1) || ( *position > CUBE_TURRET_SIZE)) return CUBE_MANAGER_ERROR;
	
	return CUBE_MANAGER_SUCCESS;
}

static int getFTDIport(FluoCubeManager* cube_manager, int *port)
{
	char path[MAX_PATHNAME_LEN];
	char msg[100];
	
	//If we are using an FTDI gizmo Device Finder will give us the port number

	GetProjectDir (path);
	strcat(path, "\\Microscope Data\\CubesliderID.txt");
	sprintf(msg, "Select port for  cube slider");
	return selectPortForDevice(path, port, msg); 
}

static int initI2Cport(FluoCubeManager* cube_manager)
{
	int  ans;   
	
	while (getFTDIport(cube_manager, &cube_manager->_i2c_port) != 0) {
		ans = ConfirmPopup ("Cube slider error", "Try plugging USB cable in or do you want to quit?");
		if (ans == 1)    //quit
				return CUBE_MANAGER_ERROR;
	}
			
	while (GCI_initI2C_multiPort(cube_manager->_i2c_port) != 0) {
		ans = ConfirmPopup ("Cube slider error", "Do you want to quit?");
		if (ans == 1)    //quit
				return CUBE_MANAGER_ERROR;
	}
	
	SetComTime (cube_manager->_i2c_port, 1.0);	//Set port time-out to 1 sec
	FlushInQ (cube_manager->_i2c_port);
	FlushOutQ (cube_manager->_i2c_port); 
	
	return CUBE_MANAGER_SUCCESS;
}


FluoCubeManager* HTS_fluo_cube_manager_new(const char *filepath)
{
	FluoCubeManager* cube_manager = cube_manager_new("Fluorescent Cubes", "Fluorescent Cubes", filepath, sizeof(FluoCubeManagerHts));
	
	FluoCubeManagerHts * cube_managerHts = (FluoCubeManagerHts *) cube_manager; 
	
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = HTS_fluo_manager_destroy;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = HTS_move_to_cube_position; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = HTS_get_current_cube_position; 

#ifndef DEBUG
	if (initI2Cport(cube_manager) == CUBE_MANAGER_ERROR)
		goto Error;
#endif

	cube_manager->_i2c_bus = CUBE_BUS;
	cube_manager->_i2c_address = CUBE_ADDRESS;

	cube_manager->_mounted = 1;
	SetCtrlAttribute (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, ATTR_DIMMED, !cube_manager->_mounted);

	cube_manager_on_change(cube_manager);  //display current position
	
	return cube_manager;
	
	Error:
	
	cube_manager->_mounted = 0;
	
	send_fluocube_error_text (cube_manager, "Failed to create cube manager");     
	
	return NULL;
}
