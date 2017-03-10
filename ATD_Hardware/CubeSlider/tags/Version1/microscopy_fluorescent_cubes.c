#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "com_port_control.h"
#include "usbconverter_v2.h"
#include "IO_interface_v2.h"

#include "microscopy_fluorescent_cubes.h"


int microscopy_fluo_manager_destroy (FluoCubeManager* cube_manager)
{
	return CUBE_MANAGER_SUCCESS;
}


int microscopy_fluo_manager_get_current_cube_position (FluoCubeManager* cube_manager)
{
    int err;
    int cube_position;
    char address4_val;
    
    err = GCI_In_Byte_multiPort(cube_manager->_i2c_port, GCI_I2C_SINGLE_BUS, PCF8574A, 4, &address4_val);

	if(err) {
	
		MessagePopup("Fluor Cube Error", "Failed to read the Fluor cube position."); 
		return CUBE_MANAGER_ERROR;
	}
	
    cube_position = (address4_val & 7);
 
    if (cube_position > 0 && cube_position < 5)
    	cube_position = 5 - cube_position;   //cube positions are reversed
    else
    	return CUBE_MANAGER_ERROR;
    	
    return cube_position;
  
	return CUBE_MANAGER_SUCCESS;
}


FluoCubeManager* microscopy_fluo_cube_manager_new(const char *filepath)
{
	FluoCubeManager* cube_manager = cube_manager_new("Fluorescent Cubes", "Fluorescent Cubes", filepath);
	
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = microscopy_fluo_manager_destroy;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = microscopy_fluo_manager_get_current_cube_position; 

	return cube_manager;
}
