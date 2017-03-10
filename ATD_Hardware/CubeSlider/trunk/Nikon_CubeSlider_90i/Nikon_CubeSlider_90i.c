#include <cviauto.h>

#include "Nikon_CubeSlider_90i\Nikon_CubeSlider_90i.h"
#include "CubeSliderUI.h"

#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Fluorescent cube control.
////////////////////////////////////////////////////////////////////////////

int Nikon90i_fluo_manager_destroy (FluoCubeManager* cube_manager)
{
	FluoCubeManager90i * cube_manager90i = (FluoCubeManager90i *) cube_manager;
	
	CA_DiscardObjHandle(cube_manager90i->hCube);   
	
	return CUBE_MANAGER_SUCCESS;
}

int Nikon90i_fluo_manager_hardware_init (FluoCubeManager* cube_manager)
{
	FluoCubeManager90i * cube_manager90i = (FluoCubeManager90i *) cube_manager;
	
	return CUBE_MANAGER_SUCCESS;
}

static int Nikon90i_move_to_cube_position(FluoCubeManager* cube_manager, int position)
{
	ERRORINFO errInfo;
	int err, number_of_cubes;

	FluoCubeManager90i * cube_manager90i = (FluoCubeManager90i *) cube_manager;
	
	//Move filter cassette to specified position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;
	
	cube_manager_get_number_of_cubes(cube_manager, &number_of_cubes);	

	if ((position < 1) || ( position > number_of_cubes))
		return CUBE_MANAGER_ERROR;
	
	err = ISCOPELib_IFilterBlockCassetteSet_Position (cube_manager90i->hCube, &errInfo, position);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(cube_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(cube_manager90i), errInfo.description);  

		return CUBE_MANAGER_ERROR;
	}

	return CUBE_MANAGER_SUCCESS;
}

static int Nikon90i_get_current_cube_position (FluoCubeManager* cube_manager, int *position)
{
	ERRORINFO errInfo;
	int val, err, number_of_cubes;
    
	FluoCubeManager90i * cube_manager90i = (FluoCubeManager90i *) cube_manager;
	
	//Read current cube position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;
	
	err = ISCOPELib_IFilterBlockCassetteGet_Position (cube_manager90i->hCube, &errInfo, &val);

	if (err) {

		logger_log(UIMODULE_LOGGER(cube_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(cube_manager90i), errInfo.description);  

		return CUBE_MANAGER_ERROR;
	}

	*position = val;
	
	cube_manager_get_number_of_cubes(cube_manager, &number_of_cubes);	

	if ((*position < 1) || (*position > number_of_cubes))
		return CUBE_MANAGER_ERROR;
	
	return CUBE_MANAGER_SUCCESS;
}

static HRESULT CubeCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	FluoCubeManager * cube_manager = (FluoCubeManager *)  caCallbackData;   
	int pos = 0;
	
    Nikon90i_get_current_cube_position (cube_manager, &pos);

    if(pos != cube_manager->_requested_pos)
	  GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(cube_manager), "FluoCubeChanged", GCI_VOID_POINTER, cube_manager, GCI_INT, pos); 		

	
	return 0;
}

FluoCubeManager* Nikon90i_fluo_cube_manager_new(CAObjHandle hNikon90i, const char *name, const char *description, const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType;
		
	FluoCubeManager* cube_manager = cube_manager_new(name, description, data_dir, filepath, sizeof(FluoCubeManager90i));
	
	FluoCubeManager90i * cube_manager90i = (FluoCubeManager90i *) cube_manager; 
	
	ui_module_set_data_dir( UIMODULE_CAST(cube_manager), data_dir);
	ui_module_set_error_handler(UIMODULE_CAST(cube_manager), error_handler, data); 

	CUBE_MANAGER_VTABLE_PTR(cube_manager, hardware_init) =  Nikon90i_fluo_manager_hardware_init;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = Nikon90i_fluo_manager_destroy;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = Nikon90i_move_to_cube_position; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = Nikon90i_get_current_cube_position; 

	if(err = ISCOPELib_INikon90iGetFilterBlockCassette(hNikon90i, &errInfo, &(cube_manager90i->hCube)))
		goto Error;

	if(err = ISCOPELib_IFilterBlockCassetteGet_IsMounted (cube_manager90i->hCube, &errInfo, &mounted))
		goto Error;
	
	#ifndef ENABLE_CUBE_STATUS_POLLING 
	
	if(err = ISCOPELib_IFilterBlockCassetteGetPosition (cube_manager90i->hCube, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, CubeCallback, cube_manager, 1, NULL))
		goto Error;
	
	#endif
	
	return cube_manager;
	
	Error:
		
	CA_GetAutomationErrorString (err, error_str, 200);
	send_fluocube_error_text (cube_manager, "%s", error_str);     
	
	return NULL;
}
