#include <cviauto.h>
#include "ATD_Shutter_Dummy.h"
#include "ShutterUI.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Optical shutter control for manual microscope.
////////////////////////////////////////////////////////////////////////////

int Manual_shutter_init (Shutter* shutter)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 
	
	return SHUTTER_SUCCESS;
}

int Manual_shutter_destroy (Shutter* shutter)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 
	
	return SHUTTER_SUCCESS;
}


static int Manual_shutter_open(Shutter* shutter)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 
	
	shutter_manual->opened = 1;
	
  	return SHUTTER_SUCCESS;
}

static int Manual_shutter_close(Shutter* shutter)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 
	
	shutter_manual->opened = 0; 

  	return SHUTTER_SUCCESS;
}

static int Manual_get_shutter_status (Shutter* shutter, int *status)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 

	*status = shutter_manual->opened;
	
	return SHUTTER_SUCCESS;
}

static int Manual_set_shutter_open_time (Shutter* shutter, double open_time)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter;      
	
	shutter_manual->open_time = open_time; 
	
	return SHUTTER_SUCCESS;
}

static int Manual_get_shutter_open_time (Shutter* shutter, double *open_time)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter;      
	
	*open_time = shutter_manual->open_time;
	
	return SHUTTER_SUCCESS;
}


static int om_shutter_inhibit(Shutter* shutter, int inhibit)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter;      
	
	shutter_manual->inhibited = inhibit;
	return  SHUTTER_SUCCESS;  
}

static int om_shutter_is_inhibited(Shutter* shutter, int *inhibit)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter;      
	
	*inhibit = shutter_manual->inhibited;
	return  SHUTTER_SUCCESS;  
}

static int om_shutter_set_computer_control  (Shutter* shutter, int compCtrl)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter;      
	
	shutter_manual->computer_controlled = compCtrl;
	return  SHUTTER_SUCCESS; 
}

static int om_shutter_get_info (Shutter* shutter, char* info)
{
	if(info != NULL)
		strcpy(info, "Manual Shutter");

	return SHUTTER_SUCCESS;	
}

Shutter* manual_shutter_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler)
{
	Shutter* shutter = (Shutter*) malloc(sizeof(ShutterManual));  

	ShutterManual* shutter_manual = (ShutterManual *) shutter;
	
	shutter_constructor(shutter, name, description);

	ui_module_set_error_handler(UIMODULE_CAST(shutter), handler, NULL);   
	
	SHUTTER_VTABLE_PTR(shutter, hw_init) = Manual_shutter_init;    
	SHUTTER_VTABLE_PTR(shutter, destroy) = Manual_shutter_destroy; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = Manual_shutter_open; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = Manual_shutter_close; 
	SHUTTER_VTABLE_PTR(shutter, shutter_inhibit) = om_shutter_inhibit; 
	SHUTTER_VTABLE_PTR(shutter, shutter_is_inhibited) = om_shutter_is_inhibited; 
	SHUTTER_VTABLE_PTR(shutter, shutter_set_computer_control) = om_shutter_set_computer_control; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = Manual_get_shutter_status;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = Manual_set_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = Manual_get_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_info) = om_shutter_get_info;

	// This won't hurt. It is only a dummy module.
	// This prevents errors about controlling when not in computer control. 
	shutter_set_computer_control(shutter, 1);

	return shutter;
}
