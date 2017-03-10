#include "dummy_laser.h"
#include "gci_utils.h"

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include <utility.h>
#include "string_utils.h"

#include "fortify.h"

int om_laser_destroy (Laser* laser)
{
	return  LASER_SUCCESS;  
}

static int om_laser_get_info (HardwareDevice* device, char* info)
{
	DummyLaser* om_laser = (DummyLaser*) device;

	if(info != NULL) {
		sprintf(info, "Dummy Laser Version %f", 1.0);
	}

	return LASER_SUCCESS;
}

static int om_laser_get_power(Laser* laser, float *power)
{
	int val = 0;

	*power = 0.0f;

	return LASER_SUCCESS;
}

Laser* om_laser_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	DummyLaser* om_laser = (DummyLaser*) malloc(sizeof(DummyLaser));  
	Laser *laser = (Laser*) om_laser;

	memset(om_laser, 0, sizeof(DummyLaser));
    
	laser_constructor(laser, name, description, data_dir);

	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(laser), hardware_getinfo) = om_laser_get_info; 
	LASER_VTABLE_PTR(laser, destroy) = om_laser_destroy; 
	LASER_VTABLE_PTR(laser, get_power) = om_laser_get_power; 

	return laser;
}
