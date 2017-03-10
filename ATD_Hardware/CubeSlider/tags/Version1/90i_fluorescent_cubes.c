#include <cviauto.h>

#include "90i_fluorescent_cubes.h"
#include "fluorescent_cubes_ui.h"

#include "iscope90i.h"
#include "mipparam90i.h"

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

static HRESULT CubeCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	FluoCubeManager * cube_manager = (FluoCubeManager *)  caCallbackData;   
	
  	PROFILE_START ("CubeCallback") ;
	cube_manager_on_change(cube_manager);
	PROFILE_STOP ("CubeCallback") ;
	//PROFILE_PRINT () ;
	
	return 0;
}

static int Nikon90i_move_to_cube_position(FluoCubeManager* cube_manager, int position)
{
	ERRORINFO errInfo;
	int err;

	FluoCubeManager90i * cube_manager90i = (FluoCubeManager90i *) cube_manager;
	
	//Move filter cassette to specified position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;
	
	if (cube_manager->_mounted != 1)
		return CUBE_MANAGER_ERROR;
	
	if ((position < 1) || ( position > CUBE_TURRET_SIZE)) return CUBE_MANAGER_ERROR;
	
	err = ISCOPELib_IFilterBlockCassetteSet_Position (cube_manager90i->hCube, &errInfo, position);
	
	if (err) {
		err = CA_DisplayErrorInfo (cube_manager90i->hCube, NULL, err, &errInfo);
		return CUBE_MANAGER_ERROR;
	}

	return CUBE_MANAGER_SUCCESS;
}

static int Nikon90i_get_current_cube_position (FluoCubeManager* cube_manager, int *position)
{
	ERRORINFO errInfo;
	int val, err;
    
	FluoCubeManager90i * cube_manager90i = (FluoCubeManager90i *) cube_manager;
	
	//Read current cube position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;
	
	if (cube_manager->_mounted != 1)
		return CUBE_MANAGER_ERROR;
	
	err = ISCOPELib_IFilterBlockCassetteGet_Position (cube_manager90i->hCube, &errInfo, &val);
	if (err) {
		err = CA_DisplayErrorInfo (cube_manager90i->hCube, NULL, err, &errInfo);
		return CUBE_MANAGER_ERROR;
	}

	*position = val;
	if ((*position < 1) || ( *position > CUBE_TURRET_SIZE)) return CUBE_MANAGER_ERROR;
	
	return CUBE_MANAGER_SUCCESS;
}


FluoCubeManager* Nikon90i_fluo_cube_manager_new(CAObjHandle hNikon90i, const char *filepath)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType;
	
	FluoCubeManager* cube_manager = cube_manager_new("Fluorescent Cubes", "Fluorescent Cubes", filepath, sizeof(FluoCubeManager90i));
	
	FluoCubeManager90i * cube_manager90i = (FluoCubeManager90i *) cube_manager; 
	
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = Nikon90i_fluo_manager_destroy;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = Nikon90i_move_to_cube_position; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = Nikon90i_get_current_cube_position; 

	if(err = ISCOPELib_INikon90iGetFilterBlockCassette(hNikon90i, &errInfo, &(cube_manager90i->hCube)))
		goto Error;

	if(err = ISCOPELib_IFilterBlockCassetteGet_IsMounted (cube_manager90i->hCube, &errInfo, &mounted))
		goto Error;
	
	cube_manager->_mounted = mounted;
	SetCtrlAttribute (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, ATTR_DIMMED, !cube_manager->_mounted);

	
	#ifndef ENABLE_CUBE_STATUS_POLLING 
	
	if(err = ISCOPELib_IFilterBlockCassetteGetPosition (cube_manager90i->hCube, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, CubeCallback, cube_manager, 1, NULL))
		goto Error;
	
	cube_manager_on_change(cube_manager);  //display current position
		
	#endif
	
	return cube_manager;
	
	Error:
	
	cube_manager->_mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_fluocube_error_text (cube_manager, "%s", error_str);     
	
	return NULL;
}
