#include "laser.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "iniparser.h"  

#include "GL_CVIRegistry.h"

#include <utility.h>
#include "toolbox.h"

#include "profile.h"

#include "asynctmr.h"

#include <ansi_c.h> 

int  send_laser_error_text (Laser* laser, char fmt[], ...)
{
	int ret=0;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(laser), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(laser), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(laser), "Laser Error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}

void laser_constructor(Laser* laser, const char *name, const char *description, const char *data_dir)
{
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(laser), name);
	ui_module_set_description(UIMODULE_CAST(laser), description);       
	ui_module_set_data_dir(UIMODULE_CAST(laser), data_dir);      
}

int laser_destroy(Laser* laser)
{
	CHECK_LASER_VTABLE_PTR(laser, destroy) 
  	
	CALL_LASER_VTABLE_PTR(laser, destroy) 
	
	ui_module_destroy(UIMODULE_CAST(laser));  
  	
  	free(laser);
  	
  	return LASER_SUCCESS;
}

int laser_get_power(Laser* laser, float *power)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_LASER_VTABLE_PTR(laser, get_power)  
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( LASER_VTABLE(laser, get_power)(laser, power) == LASER_ERROR ) {
			status = send_laser_error_text(laser, "laser_get_power failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return LASER_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
  	return LASER_SUCCESS;
}

