#include "ATD_ZDrive_Dummy.h"
#include "ZDriveUI.h" 
#include "gci_utils.h"

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include <utility.h>

static int manual_zdrive_get_position(Z_Drive* pf, double *focus_microns)
{
	Z_DriveManual* manual_pf = (Z_DriveManual *) pf;     

	*focus_microns = manual_pf->_focus_microns;
	
	return Z_DRIVE_SUCCESS;
}


static int manual_zdrive_set_position(Z_Drive* pf, double focus_microns)
{
	Z_DriveManual* manual_pf = (Z_DriveManual *) pf;     

	manual_pf->_focus_microns = focus_microns;
	pf->_current_pos = focus_microns; 
	
	pf->_busy = 0;

//	z_drive_set_message(pf, "set position");

	return Z_DRIVE_SUCCESS;
}

static int manual_zdrive_init(Z_Drive* zd)
{
	char device[UIMODULE_NAME_LEN];
	
	ui_module_get_name(UIMODULE_CAST(zd), device);

	get_device_double_param_from_ini_file(device, "StepsPerMicron", &(zd->_steps_per_micron));
	get_device_double_param_from_ini_file(device, "Min Microns", &(zd->_min_microns));
	get_device_double_param_from_ini_file(device, "Max Microns", &(zd->_max_microns));
	get_device_double_param_from_ini_file(device, "Spped", &(zd->_speed));

//	z_drive_reveal_message_controls(zd);

	return Z_DRIVE_SUCCESS;  
}


int manual_zdrive_destroy (Z_Drive* pf)
{
	Z_DriveManual* manual_pf = (Z_DriveManual *) pf;    
	
	return Z_DRIVE_SUCCESS;  
}

static int manual_z_drive_enable_disable_timers(Z_Drive* zd, int enable)
{
	Z_DriveManual* manual_pf = (Z_DriveManual *) zd; 

	return Z_DRIVE_SUCCESS; 
}


Z_Drive* manual_zdrive_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	Z_Drive* pf = (Z_Drive*) malloc(sizeof(Z_DriveManual));  
	Z_DriveManual* manual_pf = (Z_DriveManual *) pf;    
	
	z_drive_constructor(pf, name, description, data_dir);

	manual_pf->_focus_microns = 0.0;

	Z_DRIVE_VTABLE_PTR(pf, hw_initialise) = manual_zdrive_init; 
	Z_DRIVE_VTABLE_PTR(pf, destroy) = manual_zdrive_destroy; 
	Z_DRIVE_VTABLE_PTR(pf, z_drive_set_position) = manual_zdrive_set_position; 
	Z_DRIVE_VTABLE_PTR(pf, z_drive_get_position) = manual_zdrive_get_position;  
	Z_DRIVE_VTABLE_PTR(pf, z_drive_enable_disable_timers) = manual_z_drive_enable_disable_timers;

	return pf;
}
