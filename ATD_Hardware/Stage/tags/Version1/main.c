#include <shlobj.h>

#include "com_port_control.h"
#include "stage.h"
#include "Corvus_stage.h"

#include "iscope90i.h"
#include "90i_focus_drive.h"
#include <utility.h>


/////////////////////////////////////////////////////////////////////////////
// Test program for XYZ stage module for Corvus with Marzhauser stage - GP & RJL Jan 2006
//
/////////////////////////////////////////////////////////////////////////////
// RJL - May 2006
// Add Z drive for Nikon Eclipse 90i microscope
/////////////////////////////////////////////////////////////////////////////

static void OnStageClose(Stage* stage, void *data)
{
	focus_drive_destroy(stage->focus_drive);
	stage_destroy(stage);
	
	QuitUserInterface(0);	
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err;
	Stage *stage;
	FocusDrive *focus_drive;
	int full;
	char path[MAX_PATHNAME_LEN];
	
	if (!SUCCEEDED(CoInitialize(NULL))) {
		printf("Unable to initialize COM");
		return -1;
	}

  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	//Create Nikon 90i instance
	//NB. If parameter 2, (Support Multi-threading), is 1 it all goes horribly wrong
	ISCOPELib_NewINikon90i (NULL, 0, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iDeviceSetOverlapped (hNikon90i, &errInfo, 1);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	if( (focus_drive = Nikon90i_focus_drive_new(hNikon90i)) == NULL)
		return -1;
	
	//Use Glen's COM port allocation module
	GCI_ComPortControlInit();		 //Initialise Com port table
	GetProjectDir(path);
	strcat(path, "\\Microscope Data\\");
	GCI_ComPortControlSetDataDir(path);


	if ( (stage = Corvus_stage_new()) == NULL)
		return -1;
	stage->focus_drive = focus_drive;
	
	if ( (stage_init(stage)) == STAGE_ERROR)
		return -1;

	stage_signal_close_handler_connect (stage, OnStageClose, stage);

	stage_display_main_ui(stage); 
	
	full = ConfirmPopup("", "OK to initialise XY stage?");

	if( (stage_find_initialise_extents (stage, full)) == STAGE_ERROR)
		return -1;
	
	RunUserInterface();
	
	CoUninitialize(); 

  	return 0;
}
