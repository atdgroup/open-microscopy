#include "Nikon_Shutter_90i.h"
#include "ShutterUI.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include <cviauto.h>
#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Optical shutter control.
////////////////////////////////////////////////////////////////////////////

int Nikon90i_shutter_destroy (Shutter* shutter)
{
	Shutter90i* shutter90i = (Shutter90i *) shutter; 
	
	CA_DiscardObjHandle(shutter90i->hShutter);    
	
	return SHUTTER_SUCCESS;
}


static int Nikon90i_shutter_open(Shutter* shutter)
{
	ERRORINFO errInfo;
	int err;
	double t1;
	
	Shutter90i* shutter90i = (Shutter90i *) shutter; 
	
	PROFILE_START ("Nikon90i_shutter_open") ;
	t1 = Timer();
	err = ISCOPELib_IEpiShutterOpen (shutter90i->hShutter, &errInfo);
	//printf("Open shutter %f\n", Timer() - t1);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(shutter90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(shutter90i), errInfo.description); 

		return SHUTTER_ERROR;
	}
	
	shutter_emit_if_shutter_changed(shutter);

	PROFILE_STOP ("Nikon90i_shutter_open") ;
	//PROFILE_PRINT () ;

  	return SHUTTER_SUCCESS;
}

static int Nikon90i_shutter_close(Shutter* shutter)
{
	ERRORINFO errInfo;
	int err;
	double t1;

	Shutter90i* shutter90i = (Shutter90i *) shutter; 
	
	PROFILE_START ("Nikon90i_shutter_close") ;
	if (shutter == NULL)
		return SHUTTER_ERROR;
	
	t1 = Timer();
	err = ISCOPELib_IEpiShutterClose (shutter90i->hShutter, &errInfo);
	//printf("Close shutter %f\n", Timer() - t1);

	if (err) {
		
		logger_log(UIMODULE_LOGGER(shutter90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(shutter90i), errInfo.description); 

		return SHUTTER_ERROR;
	}
	
	PROFILE_STOP ("Nikon90i_shutter_close") ;

  	return SHUTTER_SUCCESS;
}

static int Nikon90i_get_shutter_status (Shutter* shutter, int *status)
{
	enum ISCOPELibEnum_EnumStatus shutter_open;
	ERRORINFO errInfo;
	int err;
    
	Shutter90i* shutter90i = (Shutter90i *) shutter; 
	//Read current shutter status, (1 = on).

	if (shutter == NULL)
		return SHUTTER_ERROR;
	
	err = ISCOPELib_IEpiShutterGet_IsOpened (shutter90i->hShutter, &errInfo, &shutter_open);
	
	if (err) {
		
		logger_log(UIMODULE_LOGGER(shutter90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(shutter90i), errInfo.description); 

		return SHUTTER_ERROR;
	}
	
	*status = shutter_open;
	
	return SHUTTER_SUCCESS;
}

static int Nikon90i_set_shutter_open_time (Shutter* shutter, double open_time)
{
	return SHUTTER_SUCCESS;
}

static int Nikon90i_get_shutter_open_time (Shutter* shutter, double *open_time)
{
	*open_time = 0.0;

	return SHUTTER_SUCCESS;
}

static int Nikon90i_shutter_trigger(Shutter *shutter)
{
	return Nikon90i_shutter_close(shutter);
}


static HRESULT ShutterCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	Shutter* shutter = (Shutter *) caCallbackData;   
	
	shutter_emit_if_shutter_changed(shutter);

	return 0;
}

static int Nikon90i_shutter_get_info (Shutter* shutter, char* info)
{
	if(info != NULL)
		strcpy(info, "Nikon 90i shutter");

	return SHUTTER_SUCCESS;	
}

static int Nikon90i_shutter_init(Shutter* shutter)
{
	return SHUTTER_SUCCESS;
}

static int Nikon90i_shutter_is_inhibited(Shutter* shutter, int *inhibit)
{
	*inhibit = 0;
	
	return  SHUTTER_SUCCESS;  
}

static int Nikon90i_shutter_inhibit(Shutter* shutter, int inhibit_in)
{
	return  SHUTTER_SUCCESS;  
}

int Nikon90i_shutter_set_automatic_control(Shutter* shutter, int computer)
{
	return  SHUTTER_SUCCESS;
}

Shutter* Nikon90i_shutter_new(CAObjHandle hNikon90i,  const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType; 
	
	Shutter* shutter = (Shutter*) malloc(sizeof(Shutter90i));  
	Shutter90i* shutter90i = (Shutter90i *) shutter;    
	
	shutter_constructor(shutter, "90i shutter", "Shutter Control");

    ui_module_set_data_dir( UIMODULE_CAST(shutter90i), data_dir);
    ui_module_set_error_handler(UIMODULE_CAST(shutter90i), error_handler, data); 

	SHUTTER_VTABLE_PTR(shutter, hw_init) = Nikon90i_shutter_init; 
	SHUTTER_VTABLE_PTR(shutter, destroy) = Nikon90i_shutter_destroy; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = Nikon90i_shutter_open; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = Nikon90i_shutter_close; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = Nikon90i_get_shutter_status;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = Nikon90i_set_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = Nikon90i_get_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_info) = Nikon90i_shutter_get_info; 
	SHUTTER_VTABLE_PTR(shutter, shutter_is_inhibited) = Nikon90i_shutter_is_inhibited; 
	SHUTTER_VTABLE_PTR(shutter, shutter_inhibit) = Nikon90i_shutter_inhibit;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_computer_control) = Nikon90i_shutter_set_automatic_control;

	if(err = ISCOPELib_INikon90iGetEpiShutter (hNikon90i, &errInfo, &(shutter90i->hShutter)))
		goto Error;

	if(err = ISCOPELib_IEpiShutterGet_IsMounted (shutter90i->hShutter, &errInfo, &mounted))
		goto Error;
	
	err = SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_INHIBIT, ATTR_VISIBLE, 0);
	SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_COMP_CTRL, ATTR_VISIBLE, 0);

	SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_OPEN, ATTR_DIMMED, !mounted);
	SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_CLOSE, ATTR_DIMMED, !mounted);

	//No trigger controls
	//SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_TRIGGER, ATTR_VISIBLE, 0);
	//SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_OPEN_TIME, ATTR_VISIBLE, 0);
	//SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_DECORATION_10, ATTR_VISIBLE, 0);
	
	//SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_EXIT, ATTR_TOP, 87);
	//SetPanelAttribute (UIMODULE_MAIN_PANEL_ID(shutter), ATTR_HEIGHT, 124);
	
	//Alternatively if trigger implemented
	//SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_EXIT, ATTR_TOP, 187);
	//SetPanelAttribute (UIMODULE_MAIN_PANEL_ID(shutter), ATTR_HEIGHT, 223);
	
	//Hide the test button
	//SetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(shutter), SHUTTER_TEST, ATTR_VISIBLE, 0);

	#ifndef ENABLE_SHUTTER_STATUS_POLLING
	
	if(err = ISCOPELib_IEpiShutterGetIsOpened (shutter90i->hShutter, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, ShutterCallback, shutter, 1, NULL))
		goto Error;
	
	#endif
	
	return shutter;
	
	Error:
	
	mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_shutter_error_text (shutter, "%s", error_str);     
	
	return NULL;
}
