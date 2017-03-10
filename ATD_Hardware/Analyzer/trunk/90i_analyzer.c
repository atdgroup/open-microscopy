#include <cviauto.h>

#include "90i_analyzer.h"
#include "analyzer_ui.h"

#include "ISCOPELib.h"
#include "MIPPARAMLib.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

int Nikon90i_analyzer_destroy (Analyzer* analyzer)
{
	Analyzer90i * analyzer90i = (Analyzer90i *) analyzer; 
	
	CA_DiscardObjHandle(analyzer90i->hAnalyzer);
	
	return ANALYZER_SUCCESS;
}


static HRESULT AnalyzerCallback (CAObjHandle caServerObjHandle, void *caCallbackData, long *__returnValue)
{
	Analyzer * analyzer = (Analyzer *)  caCallbackData;   
	
  	PROFILE_START ("AnalyzerCallback") ;
	analyzer_on_change(analyzer);
	PROFILE_STOP ("AnalyzerCallback") ;
	//PROFILE_PRINT () ;
	
	return 0;
}

static int Nikon90i_analyzer_out(Analyzer* analyzer)
{
	ERRORINFO errInfo;
	int err;

	Analyzer90i * analyzer90i = (Analyzer90i *) analyzer;
	
	err = ISCOPELib_IAnalyzerExtract (analyzer90i->hAnalyzer, &errInfo);

	if (err) {

		logger_log(UIMODULE_LOGGER(analyzer90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(analyzer90i), errInfo.description);  

		return ANALYZER_ERROR;
	}
	
	analyzer_on_change(analyzer);

  	return ANALYZER_SUCCESS;
}

static int Nikon90i_analyzer_in(Analyzer* analyzer)
{
	ERRORINFO errInfo;
	int err;

	Analyzer90i * analyzer90i = (Analyzer90i *) analyzer;
	
	if (analyzer == NULL)
		return ANALYZER_ERROR;
	
	err = ISCOPELib_IAnalyzerInsert (analyzer90i->hAnalyzer, &errInfo);

	if (err) {

		logger_log(UIMODULE_LOGGER(analyzer90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(analyzer90i), errInfo.description);  

		return ANALYZER_ERROR;
	}
	
	analyzer_on_change(analyzer);

  	return ANALYZER_SUCCESS;
}

static int Nikon90i_get_analyzer_status (Analyzer* analyzer, int *status)
{
	enum ISCOPELibEnum_EnumStatus analyzer_in;
	ERRORINFO errInfo;
	int err;
    
	Analyzer90i * analyzer90i = (Analyzer90i *) analyzer;
	
	//Read current analyzer status, (1 = inserted).

	if (analyzer == NULL)
		return ANALYZER_ERROR;

	err = ISCOPELib_IAnalyzerGet_IsInserted (analyzer90i->hAnalyzer, &errInfo, &analyzer_in);
	
	if (err) {

		logger_log(UIMODULE_LOGGER(analyzer90i), LOGGER_ERROR, "Nikon Error (%s): %s",
			UIMODULE_GET_DESCRIPTION(analyzer90i), errInfo.description);  

		return ANALYZER_ERROR;
	}
	
	*status = analyzer_in;
	
	return ANALYZER_SUCCESS;
}


Analyzer* Nikon90i_analyzer_new(CAObjHandle hNikon90i, const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	enum ISCOPELibEnum_EnumStatus mounted;
	ERRORINFO errInfo;
	int err;
	VARIANT pVal;
	MIPPARAMLibObj_IMipParameter mipParameter;
	char error_str[200];   
	int variantType;
	
	Analyzer* analyzer = analyzer_new("90i analyzer", "Analyzer Control", sizeof(Analyzer90i));
	
	Analyzer90i * analyzer90i = (Analyzer90i *) analyzer;
	
	ui_module_set_data_dir( UIMODULE_CAST(analyzer90i), data_dir);
    ui_module_set_error_handler(UIMODULE_CAST(analyzer90i), error_handler, data); 
    
	ANALYZER_VTABLE_PTR(analyzer, destroy) = Nikon90i_analyzer_destroy; 
	ANALYZER_VTABLE_PTR(analyzer, analyzer_out) = Nikon90i_analyzer_out; 
	ANALYZER_VTABLE_PTR(analyzer, analyzer_in) = Nikon90i_analyzer_in; 
	ANALYZER_VTABLE_PTR(analyzer, analyzer_status) = Nikon90i_get_analyzer_status;

	if(err = ISCOPELib_INikon90iGetAnalyzer (hNikon90i, &errInfo, &(analyzer90i->hAnalyzer)))
		goto Error;
	
	if(err = ISCOPELib_IAnalyzerGet_IsMounted (analyzer90i->hAnalyzer, &errInfo, &mounted))
		goto Error;
	
	#ifndef ENABLE_ANALYZER_STATUS_POLLING 
	
	if(err = ISCOPELib_IAnalyzerGetIsInserted (analyzer90i->hAnalyzer, &errInfo, &pVal))
		goto Error;
	
	variantType = CA_VariantGetType (&pVal);
	
	assert(variantType == CAVT_OBJHANDLE);
	 
	if(err = CA_VariantGetObjHandle (&pVal, &mipParameter))
		goto Error;
	
	
	if(err = MIPPARAMLib_IMipParameterEventsRegOnOnValueChanged (mipParameter, AnalyzerCallback, analyzer, 1, NULL))
		goto Error;
	
	analyzer_on_change(analyzer);  //display initial position
	
	#endif
	
	return analyzer;
	
	Error:
		
	CA_GetAutomationErrorString (err, error_str, 200);
	send_analyzer_error_text (analyzer, "%s", error_str);     
	
	return NULL;
}
