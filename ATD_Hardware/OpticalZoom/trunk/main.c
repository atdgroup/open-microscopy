#include "iscope90i.h"
#include "90i_optical_zoom.h"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for optical zoom control.
////////////////////////////////////////////////////////////////////////////

static void OnOpticalZoomClose(OpticalZoom* optical_zoom, void *data)
{
	optical_zoom_destroy(optical_zoom);
	
	QuitUserInterface(0);	
}

static void OnOpticalZoomChange(OpticalZoom* optical_zoom, void *data)
{
	double val;
	
	if (optical_zoom_get(optical_zoom, &val) == OPTICAL_ZOOM_SUCCESS)
		printf("Zoom changed to %f.\n", val);
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err;
	OpticalZoom *optical_zoom;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	if( (optical_zoom = Nikon90i_optical_zoom_new(hNikon90i)) == NULL)
		return -1;
	
	optical_zoom_signal_hide_handler_connect (optical_zoom, OnOpticalZoomClose, optical_zoom); 
	optical_zoom_changed_handler (optical_zoom, OnOpticalZoomChange, NULL); 

	optical_zoom_display_main_ui(optical_zoom); 
	
	RunUserInterface();

  	return 0;
}
