#include <cviauto.h>
#include "iscope90i.h"
#include "90i_shutter.h"
#include "shutter_ui.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "iscope90i.h"
#include "mipparam90i.h"

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
		err = CA_DisplayErrorInfo (shutter90i->hShutter, NULL, err, &errInfo);
		return SHUTTER_ERROR;
	}
	
	shutter_changed(shutter, 1);
	//shutter_on_change(shutter);
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
	
	if (shutter->_mounted != 1)
		return SHUTTER_ERROR;

	t1 = Timer();
	err = ISCOPELib_IEpiShutterClose (shutter90i->hShutter, &errInfo);
	//printf("Close shutter %f\n", Timer() - t1);

	if (err) {
		err = CA_DisplayErrorInfo (shutter90i->hShutter, NULL, err, &errInfo);
		return SHUTTER_ERROR;
	}
	
	shutter_changed(shutter, 0);
	//shutter_on_change(shutter);
	PROFILE_STOP ("Nikon90i_shutter_close") ;
	//PROFILE_PRINT () ;

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
	
	if (shutter->_mounted != 1)
		return SHUTTER_ERROR;

	err = ISCOPELib_IEpiShutterGet_IsOpened (shutter90i->hShutter, &errInfo, &shutter_open);
	
	if (err) {
		err = CA_DisplayErrorInfo (shutter90i->hShutter, NULL, err, &errInfo);
		return SHUTTER_ERROR;
	}
	
	shutter->_open = shutter_open;
	*status = shutter_open;
	
	return SHUTTER_SUCCESS;
}

static int Nikon90i_set_shutter_open_time (Shutter* shutter, int open_time)
{
	return SHUTTER_SUCCESS;
}

static int Nikon90i_get_shutter_open_time (Shutter* shutter, int *open_time)
{
	return SHUTTER_SUCCESS;
}

static int Nikon90i_shutter_trigger(Shutter *shutter)
{
	return Nikon90i_shutter_close(shutter);
}


static HRESULT ShutterCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	Shutter* shutter = (Shutter *) caCallbackData;   
	
  	PROFILE_START ("ShutterCallback") ;
	shutter_on_change(shutter);
	PROFILE_STOP ("ShutterCallback") ;
	//PROFILE_PRINT () ;
	
	return 0;
}



Shutter* Nikon90i_shutter_new(CAObjHandle hNikon90i)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType; 
	
	Shutter* shutter = shutter_new("90i shutter", "Shutter Control", sizeof(Shutter90i));
	
	Shutter90i* shutter90i = (Shutter90i *) shutter;
	
	SHUTTER_VTABLE_PTR(shutter, destroy) = Nikon90i_shutter_destroy; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = Nikon90i_shutter_open; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = Nikon90i_shutter_close; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = Nikon90i_get_shutter_status;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = Nikon90i_set_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = Nikon90i_get_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_trigger) = Nikon90i_shutter_trigger;

	if(err = ISCOPELib_INikon90iGetEpiShutter (hNikon90i, &errInfo, &(shutter90i->hShutter)))
		goto Error;

	if(err = ISCOPELib_IEpiShutterGet_IsMounted (shutter90i->hShutter, &errInfo, &mounted))
		goto Error;
	
	
	shutter->_mounted = mounted;
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_OPEN, ATTR_DIMMED, !mounted);
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_CLOSE, ATTR_DIMMED, !mounted);

	//No trigger controls
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_TRIGGER, ATTR_VISIBLE, 0);
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_OPEN_TIME, ATTR_VISIBLE, 0);
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_DECORATION_10, ATTR_VISIBLE, 0);
	
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_EXIT, ATTR_TOP, 87);
	SetPanelAttribute (shutter->_main_ui_panel, ATTR_HEIGHT, 124);
	
	//Alternatively if trigger implemented
	//SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_EXIT, ATTR_TOP, 187);
	//SetPanelAttribute (shutter->_main_ui_panel, ATTR_HEIGHT, 223);
	
	//Hide the test button
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_TEST, ATTR_VISIBLE, 0);

	#ifndef ENABLE_SHUTTER_STATUS_POLLING
	
	if(err = ISCOPELib_IEpiShutterGetIsOpened (shutter90i->hShutter, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, ShutterCallback, shutter, 1, NULL))
		goto Error;
	
	shutter_on_change(shutter);  //display initial value
	
	#endif
	
	return shutter;
	
	Error:
	
	mounted = 0;
	
	CA_GetAutomationErrorString (err, error_str, 200);
	send_shutter_error_text (shutter, "%s", error_str);     
	
	return NULL;
}
