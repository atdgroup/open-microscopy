#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "Nikon_Objectives_90i\90i_objectives.h"
#include "ObjectivesUI.h"

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Nosepiece control for Nikon Eclipse 90i.
////////////////////////////////////////////////////////////////////////////


static int Nikon90i_destroy (ObjectiveManager* objective_manager)
{
	ObjectiveManager90i *objective_manager90i =  (ObjectiveManager90i *) objective_manager; 
	
	CA_DiscardObjHandle(objective_manager90i->hNosepiece);
	
	return OBJECTIVE_MANAGER_SUCCESS;
}

static HRESULT Nikon90i_ObjectiveChangedCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	int pos;
	ObjectiveManager *objective_manager =  (ObjectiveManager *) caCallbackData;   
	
  	PROFILE_START ("ObjectiveChangedCallback") ;

	objective_manager_get_current_position(objective_manager, &pos);
	objective_manager_move_to_position(objective_manager, pos);
			
	PROFILE_STOP ("ObjectiveChangedCallback") ;

	return 0;
}

static int Nikon90i_hw_init (ObjectiveManager* objective_manager)
{
	ObjectiveManager90i *objective_manager90i =  (ObjectiveManager90i *) objective_manager; 
	ERRORINFO errInfo;
	int err;
	char error_str[200];   
	int variantType;
	MIPPARAMLibObj_IMipParameter mipParameter;
	VARIANT pVal;
		
	#ifndef ENABLE_OBJECTIVE_POLL_STATUS 
		
	if(err = ISCOPELib_INosepieceGetPosition (objective_manager90i->hNosepiece, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, Nikon90i_ObjectiveChangedCallback, objective_manager, 1, NULL))
		goto Error;
		
	#endif
	
	return OBJECTIVE_MANAGER_SUCCESS;
		
	Error:
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_objectives_error_text (objective_manager, "%s", error_str);  
	
	return OBJECTIVE_MANAGER_ERROR;
}

static int Nikon90i_set_interlock(ObjectiveManager* objective_manager, int enable)
{
	ERRORINFO errInfo;
	int err;

	ObjectiveManager90i *objective_manager90i =  (ObjectiveManager90i *) objective_manager; 
	
	//With interlocks enabled the aperture stop and field stop are determined by the 90i firmware
	
	if (objective_manager == NULL)
		return OBJECTIVE_MANAGER_ERROR;
	
	if (enable)
		err = ISCOPELib_INosepieceEnableInterlock (objective_manager90i->hNosepiece, &errInfo);
	else
		err = ISCOPELib_INosepieceDisableInterlock (objective_manager90i->hNosepiece, &errInfo);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(objective_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(objective_manager90i), errInfo.description); 

		return OBJECTIVE_MANAGER_ERROR;
	}

	return OBJECTIVE_MANAGER_SUCCESS;
}

static int Nikon90i_move_to_turret_position(ObjectiveManager* objective_manager, int position)
{
	ERRORINFO errInfo;
	int err;

	ObjectiveManager90i *objective_manager90i =  (ObjectiveManager90i *) objective_manager; 
	
	//Rotate nosepiece to specified position, (1 to TURRET_SIZE).

	if (objective_manager == NULL)
		return OBJECTIVE_MANAGER_ERROR;
	
	err = ISCOPELib_INosepieceSet_Position (objective_manager90i->hNosepiece, &errInfo, position);

	if (err) {

		logger_log(UIMODULE_LOGGER(objective_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(objective_manager90i), errInfo.description); 

		return OBJECTIVE_MANAGER_ERROR;
	}

	return OBJECTIVE_MANAGER_SUCCESS;
}

static int Nikon90i_get_current_turret_position(ObjectiveManager* objective_manager, int *position)
{
	ERRORINFO errInfo;
	int val, err = 0;

	ObjectiveManager90i *objective_manager90i =  (ObjectiveManager90i *) objective_manager; 
	
	//Read current nosepiece position, (1 to TURRET_SIZE).

	if (objective_manager == NULL)
		return OBJECTIVE_MANAGER_ERROR;
	
	if((err = ISCOPELib_INosepieceGet_Position (objective_manager90i->hNosepiece, &errInfo, &val))) {

		logger_log(UIMODULE_LOGGER(objective_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(objective_manager90i), errInfo.description); 

		return OBJECTIVE_MANAGER_ERROR;
	}

	*position = val;
	
	return OBJECTIVE_MANAGER_SUCCESS;
}

static int Nikon90i_nosepiece_mounted(ObjectiveManager* objective_manager)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	
	ObjectiveManager90i *objective_manager90i =  (ObjectiveManager90i *) objective_manager; 
	
	//Is objective turret mounted? 0 - no, 1 - yes
	if (objective_manager == NULL) return OBJECTIVE_MANAGER_ERROR;

	err = ISCOPELib_INosepieceGet_IsMounted (objective_manager90i->hNosepiece, &errInfo, &mounted);
	if (err) {

		logger_log(UIMODULE_LOGGER(objective_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(objective_manager90i), errInfo.description); 

		mounted = 0;
		return OBJECTIVE_MANAGER_ERROR;
	}
	
	return OBJECTIVE_MANAGER_SUCCESS;
}


ObjectiveManager* Nikon90i_objective_manager_new(CAObjHandle hNikon90i, const char *name, const char *description, const char *data_dir, const char *filepath)
{
	ERRORINFO errInfo;
	int err;
	char error_str[200];   
	enum ISCOPELibEnum_EnumStatus mounted;

	ObjectiveManager* objective_manager = objective_manager_new(name, description, data_dir, filepath, sizeof(ObjectiveManager90i));

	ObjectiveManager90i *objective_manager90i =  (ObjectiveManager90i *) objective_manager; 
	
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, hw_init) = Nikon90i_hw_init;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, destroy) = Nikon90i_destroy;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, move_to_turret_position) = Nikon90i_move_to_turret_position;
//	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, nosepiece_mounted) = Nikon90i_nosepiece_mounted;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, get_current_turret_position) = Nikon90i_get_current_turret_position;
//	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, set_interlock) = Nikon90i_set_interlock;

	if(err = ISCOPELib_INikon90iGetNosepiece(hNikon90i, &errInfo, &(objective_manager90i->hNosepiece)))
		goto Error;
	

	if(err = ISCOPELib_INosepieceGet_IsMounted (objective_manager90i->hNosepiece, &errInfo, &mounted))
		goto Error;
	
	//Default to software interlocks - with interlock enabled the field stops change with the objective
	if(err = ISCOPELib_INosepieceDisableInterlock (objective_manager90i->hNosepiece, &errInfo))
		goto Error;



	
	SetCtrlAttribute (objective_manager->_main_ui_panel, OBJ_PANEL_TURRET_POS, ATTR_DIMMED, !mounted);





//	{
//		ISCOPELibObj_IObjectives objectives;
//		char buffer[500];
//		long count = -1;

	//err = ISCOPELib_INosepieceGetObjectives (objective_manager90i->hNosepiece,
   //                                               &errInfo,
   //                                               &objectives);


	//ISCOPELib_IObjectivesGetCount (objectives,
    //                                           &errInfo, &count);

//	if (err) {
//		err = CA_DisplayErrorInfo (objective_manager90i->hNosepiece, NULL, err, &errInfo);
//	}


	//err = ISCOPELib_IObjectivesGetCount (objective_manager90i->hNosepiece,
    //                                              &errInfo,
     //                                             &count);

	
	//err = ISCOPELib_IObjectiveGetDescription (ppVal[0],
   //                                                  &errInfo,
   //                                                 &buffer);
	
	//if (err) {
	//	err = CA_DisplayErrorInfo (objective_manager90i->hNosepiece, NULL, err, &errInfo);
	//}

	//printf("%s\n", buffer);

	/*
	ISCOPELib_IObjectiveSetObjectiveData (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long lObjectiveModel,
                                                      long lObjectiveType,
                                                      double dMagnification,
                                                      double dWorkingDistance,
                                                      double dNumericalAperture);
													  */




//	}

	return objective_manager;
	
	Error:
	
	mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_objectives_error_text (objective_manager, "%s", error_str);  
	
	return NULL;
}
