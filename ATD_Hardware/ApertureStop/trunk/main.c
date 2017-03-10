#include "iscope90i.h"
#include "90i_aperture_stop.h"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for aperture stop control.
////////////////////////////////////////////////////////////////////////////

static void OnApertureStopClose(ApertureStop* aperture_stop, void *data)
{
	aperture_stop_destroy(aperture_stop);
	
	QuitUserInterface(0);	
}

static void OnApertureStopChange(ApertureStop* aperture_stop, void *data)
{
	double val;
	
	if (aperture_stop_get(aperture_stop, &val) == APERTURE_STOP_SUCCESS)
		printf("Aperture stop changed to %f.\n", val);
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err;
	ApertureStop *aperture_stop;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	if( (aperture_stop = Nikon90i_aperture_stop_new(hNikon90i)) == NULL)
		return -1;
	
	aperture_stop_signal_hide_handler_connect (aperture_stop, OnApertureStopClose, aperture_stop); 
	aperture_stop_changed_handler (aperture_stop, OnApertureStopChange, NULL); 

	aperture_stop_display_main_ui(aperture_stop); 
	
	RunUserInterface();

  	return 0;
}
