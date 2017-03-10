#include <cviauto.h>

#include "iscope90i.h"
#include "mipparam90i.h"

#include "90i_optical_path.h"
#include "optical_path_ui.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Optical path control.
////////////////////////////////////////////////////////////////////////////

int Nikon90i_optical_path_manager_destroy (OpticalPathManager* optical_path_manager)
{
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager;   
	
	CA_DiscardObjHandle(optical_path_manager90i->hOpticalPath);  
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


static HRESULT OpticalPathCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	OpticalPathManager *optical_path_manager =  (OpticalPathManager *) caCallbackData;   
	
  	PROFILE_START ("OpticalPathCallback") ;
	optical_path_manager_on_change(optical_path_manager);
	PROFILE_STOP ("OpticalPathCallback") ;
	//PROFILE_PRINT () ;
	
	return 0;
}

	
static int Nikon90i_move_to_optical_path_position(OpticalPathManager* optical_path_manager, int position)
{
	ERRORINFO errInfo;
	int err;
	
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager;   

	//Move filter cassette to specified position, (1 to OPTICAL_PATH_TURRET_SIZE).

	if (optical_path_manager == NULL) return OPTICAL_PATH_MANAGER_ERROR;
	if (optical_path_manager->_mounted != 1) return OPTICAL_PATH_MANAGER_ERROR;
	
	if ((position < 1) || ( position > OPTICAL_PATH_TURRET_SIZE)) return OPTICAL_PATH_MANAGER_ERROR;
	
	err = ISCOPELib_ILightPathDriveSet_LightPath (optical_path_manager90i->hOpticalPath, &errInfo, position);
	if (err) {
		err = CA_DisplayErrorInfo (optical_path_manager90i->hOpticalPath, NULL, err, &errInfo);
		return OPTICAL_PATH_MANAGER_ERROR;
	}

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int Nikon90i_get_current_optical_path_position (OpticalPathManager* optical_path_manager, int *position)
{
	enum ISCOPELibEnum_EnumLightPath val;
	ERRORINFO errInfo;
	int err;
	
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager;   
    
	//Read current optical path position, (1 to OPTICAL_PATH_TURRET_SIZE).

	if (optical_path_manager == NULL) return OPTICAL_PATH_MANAGER_ERROR;
	if (optical_path_manager->_mounted != 1) return OPTICAL_PATH_MANAGER_ERROR;
	
	err = ISCOPELib_ILightPathDriveGet_LightPath (optical_path_manager90i->hOpticalPath, &errInfo, &val);
	if (err) {
		err = CA_DisplayErrorInfo (optical_path_manager90i->hOpticalPath, NULL, err, &errInfo);
		return OPTICAL_PATH_MANAGER_ERROR;
	}

	*position = val;
	if ((*position < 1) || ( *position > OPTICAL_PATH_TURRET_SIZE)) return OPTICAL_PATH_MANAGER_ERROR;
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


OpticalPathManager* Nikon90i_optical_path_manager_new(CAObjHandle hNikon90i, const char *filepath)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType;
	
	OpticalPathManager* optical_path_manager = optical_path_manager_new("Optical Path", "Optical Path", filepath, sizeof(OpticalPath90i));
	
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager; 
	
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, destroy) = Nikon90i_optical_path_manager_destroy;
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, move_to_optical_path_position) = Nikon90i_move_to_optical_path_position; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, get_current_optical_path_position) = Nikon90i_get_current_optical_path_position; 

	if(err = ISCOPELib_INikon90iGetLightPathDrive(hNikon90i, &errInfo, &(optical_path_manager90i->hOpticalPath)))
		goto Error;

	if(err = ISCOPELib_ILightPathDriveGet_IsMounted (optical_path_manager90i->hOpticalPath, &errInfo, &mounted))
		goto Error;
	
	//With interlock enabled the field stops change with the optical path
	//if(err = ISCOPELib_ILightPathDriveDisableInterlock (optical_path_manager90i->hOpticalPath, &errInfo))
	//	goto Error;
	
	optical_path_manager->_mounted = mounted;
	SetCtrlAttribute (optical_path_manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, ATTR_DIMMED, !optical_path_manager->_mounted);

	
	// Check that Polling is disabled
	#ifndef ENABLE_OPTICAL_PATH_POLLING
	
	if(err = ISCOPELib_ILightPathDriveGetLightPath (optical_path_manager90i->hOpticalPath, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, OpticalPathCallback, optical_path_manager, 1, NULL))
		goto Error;
	
	optical_path_manager_on_change(optical_path_manager); //display current position

	#endif
	
	return optical_path_manager;
	
	Error:
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_optical_path_error_text (optical_path_manager, "%s", error_str);     
	
	return NULL;
}
