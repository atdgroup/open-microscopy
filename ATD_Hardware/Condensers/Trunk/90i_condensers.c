#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

#include "90i_condensers.h"
#include "condensers_ui.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>
#include <cviauto.h>

#define DONT_PROFILE
#include "profile.h"

int Nikon90i_condenser_manager_destroy (CondenserManager* condenser_manager)
{
	CondenserManager90i * condenser_manager90i = (CondenserManager90i *) condenser_manager; 
	
	CA_DiscardObjHandle(condenser_manager90i->hCondenser);        
	
	return CONDENSER_MANAGER_SUCCESS;
}


static HRESULT CondenserCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	CondenserManager * condenser_manager = (CondenserManager *) caCallbackData;   
	int pos = 0;
	
  	PROFILE_START ("CondenserCallback") ;
  	
  	condenser_manager_get_current_condenser_position(condenser_manager, &pos);
  	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(condenser_manager), "CondenserChanged", GCI_VOID_POINTER, condenser_manager, GCI_INT, pos); 		
	
	PROFILE_STOP ("CondenserCallback") ;

	return 0;
}


static int Nikon90i_move_to_condenser_position(CondenserManager* condenser_manager, int position)
{
	ERRORINFO errInfo;
	int err;

	CondenserManager90i * condenser_manager90i = (CondenserManager90i *) condenser_manager; 
	
	//Move filter cassette to specified position, (1 to CONDENSER_TURRET_SIZE).

	if (condenser_manager == NULL)
		return CONDENSER_MANAGER_ERROR;
	
	if ((position < 1) || ( position > CONDENSER_TURRET_SIZE))
		return CONDENSER_MANAGER_ERROR;
	
	err = ISCOPELib_ICondenserCassetteSet_Position (condenser_manager90i->hCondenser, &errInfo, position);

	if (err) {

		logger_log(UIMODULE_LOGGER(condenser_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(condenser_manager90i), errInfo.description);  

		return CONDENSER_MANAGER_ERROR;
	}

	return CONDENSER_MANAGER_SUCCESS;
}

static int Nikon90i_get_current_condenser_position (CondenserManager* condenser_manager, int *position)
{
	ERRORINFO errInfo;
	int val, err;
    
	CondenserManager90i * condenser_manager90i = (CondenserManager90i *) condenser_manager; 
	
	//Read current condenser position, (1 to CONDENSER_TURRET_SIZE).

	if (condenser_manager == NULL)
		return CONDENSER_MANAGER_ERROR;

	err = ISCOPELib_ICondenserCassetteGet_Position (condenser_manager90i->hCondenser, &errInfo, &val);
	if (err) {

		logger_log(UIMODULE_LOGGER(condenser_manager90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(condenser_manager90i), errInfo.description);  

		return CONDENSER_MANAGER_ERROR;
	}

	*position = val;
	if ((*position < 1) || ( *position > CONDENSER_TURRET_SIZE)) return CONDENSER_MANAGER_ERROR;
	
	return CONDENSER_MANAGER_SUCCESS;
}

int	Nikon90i_condenser_hardware_init (CondenserManager* condenser_manager)
{

	return CONDENSER_MANAGER_SUCCESS;
}

CondenserManager* Nikon90i_condenser_manager_new(CAObjHandle hNikon90i, const char *name, const char *description, const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType;
	
	CondenserManager* condenser_manager = condenser_manager_new(name, description, data_dir, filepath, sizeof(CondenserManager90i));
	CondenserManager90i * condenser_manager90i = (CondenserManager90i *) condenser_manager;
	
	ui_module_set_error_handler(UIMODULE_CAST(condenser_manager), error_handler, data); 
	
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, hardware_init) = Nikon90i_condenser_hardware_init;
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, destroy) = Nikon90i_condenser_manager_destroy;
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, move_to_condenser_position) = Nikon90i_move_to_condenser_position; 
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, get_current_condenser_position) = Nikon90i_get_current_condenser_position; 

	if(err = ISCOPELib_INikon90iGetCondenserCassette(hNikon90i, &errInfo, &(condenser_manager90i->hCondenser)))
		goto Error;
	
	if(err = ISCOPELib_ICondenserCassetteGet_IsMounted (condenser_manager90i->hCondenser, &errInfo, &mounted))
		goto Error;
	
	#ifndef ENABLE_CONDENSER_STATUS_POLLING 
	
	if(err = ISCOPELib_ICondenserCassetteGetPosition (condenser_manager90i->hCondenser, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, CondenserCallback, condenser_manager, 1, NULL))
		goto Error;

	#endif
	
	return condenser_manager;
	
	Error:
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_condenser_error_text (condenser_manager, "%s", error_str);     
	
	return NULL;
}
