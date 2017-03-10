#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

#include "Nikon_OpticalPath_90i\Nikon_OpticalPath_90i.h"
#include "OpticalPathUI.h"

#include <cviauto.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

static int callbackId=0;

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

int Nikon90i_optical_path_manager_hw_init (OpticalPathManager* optical_path_manager, int move_to_default)
{
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager;   
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}


static int Nikon90i_move_to_optical_path_position(OpticalPathManager* optical_path_manager, int position)
{
	ERRORINFO errInfo;
	int err, number_of_paths;
	
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager;   

	//Move filter cassette to specified position, (1 to OPTICAL_PATH_TURRET_SIZE).

	if (optical_path_manager == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	optical_path_get_number_of_positions(optical_path_manager, &number_of_paths);	

	if ((position < 1) || ( position > number_of_paths))
		return OPTICAL_PATH_MANAGER_ERROR;
	
	err = ISCOPELib_ILightPathDriveSet_LightPath (optical_path_manager90i->hOpticalPath, &errInfo, position);

	if (err) {

		logger_log(UIMODULE_LOGGER(optical_path_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(optical_path_manager90i), errInfo.description); 

		return OPTICAL_PATH_MANAGER_ERROR;
	}

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static int Nikon90i_get_current_optical_path_position (OpticalPathManager* optical_path_manager, int *position)
{
	enum ISCOPELibEnum_EnumLightPath val;
	ERRORINFO errInfo;
	int err, number_of_paths;
	
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager;   
    
	//Read current optical path position, (1 to OPTICAL_PATH_TURRET_SIZE).

	if (optical_path_manager == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;
		
	err = ISCOPELib_ILightPathDriveGet_LightPath (optical_path_manager90i->hOpticalPath, &errInfo, &val);

	if (err) {

		logger_log(UIMODULE_LOGGER(optical_path_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(optical_path_manager90i), errInfo.description); 

		return OPTICAL_PATH_MANAGER_ERROR;
	}

	*position = val;
	
	optical_path_get_number_of_positions(optical_path_manager, &number_of_paths);	

	if ((*position < 1) || (*position > number_of_paths))
		return OPTICAL_PATH_MANAGER_ERROR;

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

static HRESULT OpticalPathCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	OpticalPathManager *optical_path_manager =  (OpticalPathManager *) caCallbackData;   
	int pos;
	
  	PROFILE_START ("OpticalPathCallback") ;
	//optical_path_manager_on_change(optical_path_manager);
	
	Nikon90i_get_current_optical_path_position (optical_path_manager, &pos);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(optical_path_manager), "OpticalPathManagerChanged", GCI_VOID_POINTER, optical_path_manager, GCI_INT, pos); 		
	
	PROFILE_STOP ("OpticalPathCallback") ;
	//PROFILE_PRINT () ;
	
	return 0;
}

int Nikon90i_optical_path_enable_Nikon_callback (OpticalPathManager* optical_path_manager)
{
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	int err;
	char error_str[200];   
	ERRORINFO errInfo;
	int variantType;
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager;   

#ifndef ENABLE_OPTICAL_PATH_POLLING
	
	if(err = ISCOPELib_ILightPathDriveGetLightPath (optical_path_manager90i->hOpticalPath, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;

	if (err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, OpticalPathCallback, optical_path_manager, 1, &callbackId))
		goto Error;
#endif
			
	return OPTICAL_PATH_MANAGER_SUCCESS;
	
	Error:
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_optical_path_error_text (optical_path_manager, "%s", error_str);     
	
	return OPTICAL_PATH_MANAGER_ERROR;
}

static HRESULT DummyCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	return 0;
}

int Nikon90i_optical_path_disable_Nikon_callback (OpticalPathManager* optical_path_manager)
{
//	VARIANT pVal;
//	MIPPARAMLibObj_IMipParameter mipParameter;
	int err;
	char error_str[200];   
//	ERRORINFO errInfo;
//	int variantType;
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager;   
	
//	if(err = ISCOPELib_ILightPathDriveGetLightPath (optical_path_manager90i->hOpticalPath, &errInfo, &pVal))
//		goto Error;
	
//	variantType = CA_VariantGetType (&pVal);
	
//	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
//		goto Error;

//	if (err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, DummyCallback, optical_path_manager, 1, NULL))
///		goto Error;

#ifndef ENABLE_OPTICAL_PATH_POLLING
	
	if (err = CA_UnregisterEventCallback (callbackId))
		goto Error;

#endif

	return OPTICAL_PATH_MANAGER_SUCCESS;
	
	Error:
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_optical_path_error_text (optical_path_manager, "%s", error_str);     
	
	return OPTICAL_PATH_MANAGER_ERROR;
}

OpticalPathManager* Nikon90i_optical_path_manager_new(CAObjHandle hNikon90i, const char *name, const char *description, const char* data_dir, const char *file)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	char error_str[200];   
	MIPPARAMLibObj_IMipParameter mipParameter;
	int variantType;
	
	OpticalPathManager* optical_path_manager = optical_path_manager_new(name, description, data_dir, file, sizeof(OpticalPath90i));
	
	OpticalPath90i *optical_path_manager90i =  (OpticalPath90i *) optical_path_manager; 
	
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, hw_init) = Nikon90i_optical_path_manager_hw_init; 
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
	
	// Check that Polling is disabled
	#ifndef ENABLE_OPTICAL_PATH_POLLING
	
	Nikon90i_optical_path_enable_Nikon_callback (optical_path_manager);
			
	#endif
	
	return optical_path_manager;
	
	Error:
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_optical_path_error_text (optical_path_manager, "%s", error_str);     
	
	return NULL;
}
