#include "iscope90i.h"
#include "90i_analyzer.h"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for analyzer control.
////////////////////////////////////////////////////////////////////////////

static void OnAnalyzerClose(Analyzer* analyzer, void *data)
{
	analyzer_destroy(analyzer);
	
	QuitUserInterface(0);	
}

static void OnAnalyzerChanged(Analyzer* analyzer, void *data)
{
	int status;
	
	if (analyzer_status(analyzer, &status) == ANALYZER_SUCCESS)
		printf("Analyzer changed. status = %d.\n", status);
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err;
	Analyzer *analyzer;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	if( (analyzer = Nikon90i_analyzer_new(hNikon90i)) == NULL)
		return -1;
	
	analyzer_signal_hide_handler_connect (analyzer, OnAnalyzerClose, analyzer); 
	analyzer_changed_handler_connect (analyzer, OnAnalyzerChanged, analyzer); 

	analyzer_display_main_ui(analyzer); 
	analyzer_out(analyzer);
	
	RunUserInterface();

  	return 0;
}
