/*



static int Nikoni90_set_focus_dial_sensitivity (ZDrive90i* zdrive_90i, int sensitivity)
{
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive_90i;    
	
	//Set focus dial sensitivity, (fine = 25um/rev, medium = 100um/rev, coarse = 10-2500um/rev).

	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
	if (zdrive_90i->_mounted != 1)
		return Z_DRIVE_ERROR;

	err = ISCOPELib_IZDriveSet_DialSensitivity (zdrive_90i->hZDrive90i, &errInfo, sensitivity);
	
	if (err) {
		err = CA_DisplayErrorInfo (zdrive_90i->hZDrive90i, NULL, err, &errInfo);
		return Z_DRIVE_ERROR;
	}
	
	return Z_DRIVE_SUCCESS;
}

static int Nikoni90_get_focus_dial_sensitivity (ZDrive90i* zdrive_90i, int *sensitivity)
{
	enum ISCOPELibEnum_EnumSensitivity isensitivity;
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive_90i;    
	
	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
	if (zdrive_90i->_mounted != 1)
		return Z_DRIVE_ERROR;

	err = ISCOPELib_IZDriveGet_DialSensitivity (zdrive_90i->hZDrive90i, &errInfo, &isensitivity);
	
	if (err) {
		err = CA_DisplayErrorInfo (zdrive_90i->hZDrive90i, NULL, err, &errInfo);
		return Z_DRIVE_ERROR;
	}
	
	*sensitivity = isensitivity;
	
	return Z_DRIVE_SUCCESS;
}

static int Nikoni90_focus_abort (ZDrive90i* zdrive_90i)
{
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive_90i;    
	
	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
	if (zdrive_90i->_mounted != 1)
		return Z_DRIVE_ERROR;

	err = ISCOPELib_IZDriveAbortZ (zdrive_90i->hZDrive90i, &errInfo);
	
	if (err) {
		err = CA_DisplayErrorInfo (zdrive_90i->hZDrive90i, NULL, err, &errInfo);
		return Z_DRIVE_ERROR;
	}
	
	return Z_DRIVE_SUCCESS;
}
*/



#include "90i_z_drive.h"
#include "ZDriveUI.h" 
#include "gci_utils.h"

#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

#include "profile.h"

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include <utility.h>

#define STEPS_PER_MICRON 20.0

static int Nikoni90_goto_focus_pos (Z_Drive* zdrive, double val)
{
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    

	if(val < zdrive->_min_microns)
		val = zdrive->_min_microns;
			
	if(val > zdrive->_max_microns)
		val = zdrive->_max_microns;

  	PROFILE_START ("Nikoni90_goto_focus_pos") ;
	//err = ISCOPELib_IZDriveSet_ZPosition (zdrive_90i->hZDrive90i, &errInfo, 1000);
	
	err = ISCOPELib_IZDriveMoveAbsolute (zdrive_90i->hZDrive90i, &errInfo, (long) (val * STEPS_PER_MICRON));
	//err = ISCOPELib_IZDriveMoveAbsolute (zdrive_90i->hZDrive90i, &errInfo, -100);

	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);  
	
		return Z_DRIVE_ERROR;
	}
	PROFILE_STOP ("Nikoni90_goto_focus_pos") ;

	return Z_DRIVE_SUCCESS;
}


static int Nikoni90_set_focus_range(Z_Drive* zdrive, double min, double max, double increment)
{
	//We don't set hardware limits.
	//Range is -15000 to +2000 always. (Only alternative is -25000 to +2000)
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    

	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;

	err = ISCOPELib_IZDriveSet_LowerLimit (zdrive_90i->hZDrive90i, &errInfo, ISCOPELibConst_LowerLimit1);
	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description); 

		return Z_DRIVE_ERROR;
	}

	//if (max > 0) {  // SDK gives a spurious error so we must implement it ourselves
	//	err = ISCOPELib_IZDriveSet_UpperLimit (zdrive_90i->hZDrive90i, &errInfo, 20000);
	//	if (err) {
	//		err = CA_DisplayErrorInfo (zdrive_90i->hZDrive90i, NULL, err, &errInfo);
	//		return Z_DRIVE_ERROR;
	//	}
	//}
	
	return Z_DRIVE_SUCCESS;
}


static int Nikoni90_get_focus_range(Z_Drive* zdrive, double *min, double *max, double *increment)
{
	enum ISCOPELibEnum_EnumLowerLimit imin;
	ERRORINFO errInfo;
	int err;
	long lmax = -1;
	char buffer[500] = "";
	char error_str[500] = "";

	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    
	
	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;

	/*
	ISCOPELib_IZDriveGetLowerLimit (zdrive_90i->hZDrive90i, &errInfo, &pVal);

	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	//if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
	//	return Z_DRIVE_ERROR;
	
	if(err = MIPPARAMLib_IMipParameterConvertRawValueToDisplayString (zdrive_90i->hZDrive90imipParameter, &errInfo,
                                                                         pVal,
                                                                         buffer))
	{
		return Z_DRIVE_ERROR;
	}
*/

	err = ISCOPELib_IZDriveGet_LowerLimit (zdrive_90i->hZDrive90i, &errInfo, &imin);

	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}
	if (imin == 1)
		*min = -25000;
	else
		*min = -15000;
		

	//err = ISCOPELib_IZDriveGetUpperLimit (zdrive_90i->hZDrive90i, &errInfo, &pVal);

	//variantType = CA_VariantGetType (&pVal);
	
	//assert(variantType == CAVT_OBJHANDLE);
	 
	//if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
	//	return Z_DRIVE_ERROR;

	/*
	if((err = CA_VariantGetLong(&pVal, &lmax)) < 0) {

		CA_GetAutomationErrorString (err, error_str, 200);
		send_z_drive_error_text (zdrive_90i, "%s", error_str);  

		return Z_DRIVE_ERROR;
	}
*/

	//MIPPARAMLib_IMipParameterGetDisplayString(mipParameter, &errInfo, buffer);

	
	//Upper hardware limit is +2000 um always
//	err = ISCOPELib_IZDriveGet_UpperLimit (zdrive_90i->hZDrive90i, &errInfo, &lmax);
//	if (err) {
//		err = CA_DisplayErrorInfo (zdrive_90i->hZDrive90i, NULL, err, &errInfo);
//		return Z_DRIVE_ERROR;
//	}


	*max = 2000;

	*increment = 1;
	
	return Z_DRIVE_SUCCESS;
}


static int Nikoni90_set_focus_speed (Z_Drive* zdrive, enum ISCOPELibEnum_EnumZSpeed speed)
{
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    
	//Set focus drive, (val = 1 to 9).

	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
	err = ISCOPELib_IZDriveSet_Speed (zdrive_90i->hZDrive90i, &errInfo, speed);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}
	
	return Z_DRIVE_SUCCESS;
}

static int Nikoni90_get_focus_speed (Z_Drive* zdrive, double *speed)
{
	enum ISCOPELibEnum_EnumZSpeed ispeed;
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;   
		
	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
	err = ISCOPELib_IZDriveGet_Speed (zdrive_90i->hZDrive90i, &errInfo, &ispeed);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}
	*speed = ispeed;
	
	return Z_DRIVE_SUCCESS;
}

static int Nikoni90_set_focus_tolerance (Z_Drive* zdrive, int tolerance)
{
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    
	
	//Set focus drive, (val = 0 to 9).

	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
  	PROFILE_START ("Nikoni90_set_focus_tolerance") ;
	err = ISCOPELib_IZDriveSet_Tolerance (zdrive_90i->hZDrive90i, &errInfo, tolerance);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}
	PROFILE_STOP ("Nikoni90_set_focus_tolerance") ;
	//PROFILE_PRINT () ;
	

	return Z_DRIVE_SUCCESS;
}

static int Nikoni90_get_focus_tolerance (Z_Drive* zdrive, int *tolerance)
{
	enum ISCOPELibEnum_EnumZTolerance itolerance;
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    
	
	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
  	PROFILE_START ("Nikoni90_get_focus_tolerance") ;
	err = ISCOPELib_IZDriveGet_Tolerance (zdrive_90i->hZDrive90i, &errInfo, &itolerance);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}
	*tolerance = itolerance;
	PROFILE_STOP ("Nikoni90_get_focus_tolerance") ;
	//PROFILE_PRINT () ;
	
	return Z_DRIVE_SUCCESS;
}

static int i90_zdrive_get_position(Z_Drive* zdrive, double *focus_microns)
{
	ERRORINFO errInfo;
	int err, ival;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    
	
	//Get focus drive, (val = -15000 to 2000).

	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;

  	PROFILE_START ("Nikoni90_get_focus_pos") ;
	err = ISCOPELib_IZDriveGet_ZPosition (zdrive_90i->hZDrive90i, &errInfo, &ival);
	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}
	
	*focus_microns = (double)ival / STEPS_PER_MICRON;
	PROFILE_STOP ("Nikoni90_get_focus_pos") ;

	return Z_DRIVE_SUCCESS;
}

static int Nikoni90_optimal_move (Z_Drive* zdrive, double val)
{
	//Achieve fastest move with minimal settling time
	double position;

	if (zdrive == NULL)
		return Z_DRIVE_ERROR;
	
	//Set max speed
	//if (Nikoni90_set_focus_speed (zdrive_90i, 1) == Z_DRIVE_ERROR) return Z_DRIVE_ERROR;
	
	i90_zdrive_get_position(zdrive, &position);

	SetCtrlAttribute(zdrive->_panel_id, FOCUS_FOCUS, ATTR_DIMMED, 1);
	ProcessDrawEvents();

	if (fabs(position - val) > 200.0) {
		//Set min tolerance to get near the target position in the least posible time
		if (Nikoni90_set_focus_tolerance (zdrive, 9) == Z_DRIVE_ERROR)
			goto FAIL;

		if (Nikoni90_goto_focus_pos (zdrive, val) == Z_DRIVE_ERROR)
			goto FAIL;
	}
	
	//Set moderate tolerance to get to the target position in <= 0.6 seconds
	if (Nikoni90_set_focus_tolerance (zdrive, 8) == Z_DRIVE_ERROR)
		goto FAIL;

	if (Nikoni90_goto_focus_pos (zdrive, val) == Z_DRIVE_ERROR)
		goto FAIL;

	SetCtrlAttribute(zdrive->_panel_id, FOCUS_FOCUS, ATTR_DIMMED, 0);
	ProcessDrawEvents();

	return Z_DRIVE_SUCCESS;

FAIL:
	SetCtrlAttribute(zdrive->_panel_id, FOCUS_FOCUS, ATTR_DIMMED, 0);
	ProcessDrawEvents();

	return Z_DRIVE_ERROR;
}

int Nikoni90_fast_inaccurate_move (Z_Drive* zdrive, double val)
{
	//Achieve fastest move with minimal settling time
	double position, speed;
	int tolerance;

	if (zdrive == NULL)
		return Z_DRIVE_ERROR;
	
	Nikoni90_get_focus_speed (zdrive, &speed);
	Nikoni90_get_focus_tolerance (zdrive, &tolerance);

	//Set max speed
	if (Nikoni90_set_focus_speed (zdrive, ISCOPELibConst_ZSpeed1) == Z_DRIVE_ERROR)
		return Z_DRIVE_ERROR;
	
	//Set min tolerance to get near the target position in the least posible time
	if (Nikoni90_set_focus_tolerance (zdrive, 9) == Z_DRIVE_ERROR)
		return Z_DRIVE_ERROR;
	
	SetCtrlAttribute(zdrive->_panel_id, FOCUS_FOCUS, ATTR_DIMMED, 1);
	ProcessDrawEvents();

	if (Nikoni90_goto_focus_pos (zdrive, val) == Z_DRIVE_ERROR)
		goto FAIL;

	// Revert original settings
	if (Nikoni90_set_focus_speed (zdrive, (enum ISCOPELibEnum_EnumZSpeed) speed) == Z_DRIVE_ERROR)
		goto FAIL;
	
	//Set min tolerance to get near the target position in the least posible time
	if (Nikoni90_set_focus_tolerance (zdrive, tolerance) == Z_DRIVE_ERROR)
		goto FAIL;

	SetCtrlAttribute(zdrive->_panel_id, FOCUS_FOCUS, ATTR_DIMMED, 0);
	ProcessDrawEvents();

	return Z_DRIVE_SUCCESS;

FAIL:
	SetCtrlAttribute(zdrive->_panel_id, FOCUS_FOCUS, ATTR_DIMMED, 0);
	ProcessDrawEvents();

	return Z_DRIVE_ERROR;
}

static int i90_zdrive_set_position(Z_Drive* zdrive, double focus_microns)
{
	ERRORINFO errInfo;
	int err;
    double t1;
    enum ISCOPELibEnum_EnumZTolerance tolerance;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    
	
	//Set focus drive, (val = -15000 to 2000).

	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
  	PROFILE_START ("Nikoni90_set_focus_pos") ;

	zdrive->_busy = 1;

	err = ISCOPELib_IZDriveGet_Tolerance (zdrive_90i->hZDrive90i, &errInfo, &tolerance);

	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}

	if (tolerance == 0)  // Auto tolerance 
		return Nikoni90_optimal_move(zdrive, focus_microns);

	t1 = Timer();

	ui_module_display_panel(UIMODULE_CAST(zdrive), zdrive->wait_panel_id);
	//display_panel_without_activation(zdrive->wait_panel_id);
	SetCtrlAttribute(zdrive->_panel_id, FOCUS_FOCUS, ATTR_DIMMED, 1);
	ProcessDrawEvents();

	Nikoni90_goto_focus_pos (zdrive, focus_microns);

	ui_module_hide_panel(UIMODULE_CAST(zdrive), zdrive->wait_panel_id);
	SetCtrlAttribute(zdrive->_panel_id, FOCUS_FOCUS, ATTR_DIMMED, 0);
	ProcessDrawEvents();

	zdrive->_busy = 0;

	PROFILE_STOP ("Nikoni90_set_focus_pos") ;

	return Z_DRIVE_SUCCESS;
}

static int i90_zdrive_init(Z_Drive* zd)
{
	char device[UIMODULE_NAME_LEN];
	
	ui_module_get_name(UIMODULE_CAST(zd), device);

	get_device_double_param_from_ini_file(device, "StepsPerMicron", &(zd->_steps_per_micron));
	get_device_double_param_from_ini_file(device, "Min Microns", &(zd->_min_microns));
	get_device_double_param_from_ini_file(device, "Max Microns", &(zd->_max_microns));
	get_device_double_param_from_ini_file(device, "Speed", &(zd->_speed));

//	z_drive_reveal_message_controls(zd);

	return Z_DRIVE_SUCCESS;  
}


int i90_zdrive_destroy (Z_Drive* zdrive)
{
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    
	
	CA_DiscardObjHandle(zdrive_90i->hZDrive90i);  
	zdrive_90i->hZDrive90i = 0;
	
	return Z_DRIVE_SUCCESS;
}

static int i90_z_drive_enable_disable_timers(Z_Drive* zd, int enable)
{
	//Z_DriveManual* i90_pf = (Z_DriveManual *) zd; 

	return Z_DRIVE_SUCCESS; 
}


static HRESULT FocusCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	ZDrive90i *zdrive_90i = (ZDrive90i *) caCallbackData;
	Z_Drive *zdrive = (Z_Drive *) zdrive_90i;    
	double microns;

	PROFILE_START ("FocusCallback") ;
	
	if (zdrive_90i->hZDrive90i == 0)
		return 0;

	i90_zdrive_get_position(zdrive, &microns);

	if(microns < zdrive->_min_microns)
		microns = zdrive->_min_microns;
			
	if(microns > zdrive->_max_microns)
		microns = zdrive->_max_microns;
			
	if(zdrive->_panel_id > 0) {
		SetCtrlVal(zdrive->_panel_id, FOCUS_FOCUS, microns); 	
		SetCtrlVal(zdrive->_panel_id, FOCUS_FOCUS_2, microns); 	
	}
		
	zdrive->_current_pos = microns;

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zdrive), "Z_DriveChanged", GCI_VOID_POINTER, zdrive); 

	PROFILE_STOP ("FocusCallback") ;

	return 0;
}

static int Nikoni90_set_focus_remote (Z_Drive* zdrive, int remote)
{
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive;    
	
	//Set focus control, (none, dial, PC or dial and PC).
	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
	err = ISCOPELib_IZDriveSet_RemoteControl (zdrive_90i->hZDrive90i, &errInfo, remote);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}
	
	return Z_DRIVE_SUCCESS;
}

static int Nikoni90_get_focus_remote (Z_Drive* zdrive, int *remote)
{
	enum ISCOPELibEnum_EnumRemoteControl iremote;
	ERRORINFO errInfo;
	int err;
    
	ZDrive90i *zdrive_90i = (ZDrive90i *) zdrive_90i;    
	
	if (zdrive_90i == NULL)
		return Z_DRIVE_ERROR;
	
	err = ISCOPELib_IZDriveGet_RemoteControl (zdrive_90i->hZDrive90i, &errInfo, &iremote);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(zdrive_90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(zdrive_90i), errInfo.description);

		return Z_DRIVE_ERROR;
	}
	
	*remote = iremote;
	
	return Z_DRIVE_SUCCESS;
}

Z_Drive* Nikoni90_z_drive_new(CAObjHandle hNikon90i, const char *name, const char *description, 
								 UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	int err, variantType;
	char error_str[200] = "";
	double min, max, increment;
	VARIANT pVal;
	ERRORINFO errInfo;
	MIPPARAMLibObj_IMipParameter mipParameter;
	
	ZDrive90i* zdrive_90i = (ZDrive90i*) malloc(sizeof(ZDrive90i));  
	Z_Drive* zdrive = (Z_Drive *) zdrive_90i;    
	
	z_drive_constructor(zdrive, name, description, data_dir);

	Z_DRIVE_VTABLE_PTR(zdrive, hw_initialise) = i90_zdrive_init; 
	Z_DRIVE_VTABLE_PTR(zdrive, destroy) = i90_zdrive_destroy; 
	Z_DRIVE_VTABLE_PTR(zdrive, z_drive_set_position) = i90_zdrive_set_position; 
	Z_DRIVE_VTABLE_PTR(zdrive, z_drive_get_position) = i90_zdrive_get_position;  
	Z_DRIVE_VTABLE_PTR(zdrive, z_drive_enable_disable_timers) = i90_z_drive_enable_disable_timers;
	Z_DRIVE_VTABLE_PTR(zdrive, z_drive_get_speed) = Nikoni90_get_focus_speed; 

	zdrive_dont_respond_to_event_val_changed(zdrive);
	z_drive_hide_autofocus_controls(zdrive);

	if(err = ISCOPELib_INikon90iGetZDrive (hNikon90i, &errInfo, &(zdrive_90i->hZDrive90i)))
		goto Error;

	Nikoni90_set_focus_remote (zdrive, ISCOPELibConst_RemoteControlDialAndScope);

	Nikoni90_set_focus_tolerance (zdrive, 0);

	Nikoni90_set_focus_speed (zdrive, ISCOPELibConst_ZSpeed1);

	if(err = Nikoni90_get_focus_range(zdrive, &min, &max, &increment))
		goto Error;

	if (err = ISCOPELib_IZDriveGetZPosition (zdrive_90i->hZDrive90i, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, FocusCallback, zdrive_90i, 1, NULL))
		goto Error;
	
	return zdrive;

	Error:
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_z_drive_error_text (zdrive, "%s", error_str);     
	
	return NULL;
}
