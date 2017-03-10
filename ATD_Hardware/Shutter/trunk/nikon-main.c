#include "iscope90i.h"
#include "90i_shutter.h"

#include <stdio.h>
//#include <ansi_c.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for shutter control.
////////////////////////////////////////////////////////////////////////////

static void OnShutterClose(Shutter* shutter, void *data)
{
	shutter_destroy(shutter);
	
	QuitUserInterface(0);	
}

static void OnShutterChanged(Shutter* shutter, void *data)
{
	int status;
	
	if (shutter_status(shutter, &status) == SHUTTER_SUCCESS)
		printf("Shutter changed. status = %d.\n", status);
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	ISCOPELibObj_IDevice device;
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err, overlapped=99;
	Shutter *shutter;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	err = ISCOPELib_INikon90iGet_Device (hNikon90i, &errInfo, &device);
	if (err) err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);
	
	err = ISCOPELib_INikon90iDeviceGetOverlapped (device, &errInfo, &overlapped);
	if (err) err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);
	printf("overlapped %d\n", overlapped);
	
	err = ISCOPELib_INikon90iDeviceSetOverlapped (device, &errInfo, 1);
	if (err) err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);
	
	err = ISCOPELib_INikon90iDeviceGetOverlapped (device, &errInfo, &overlapped);
	if (err) err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);
	printf("overlapped %d\n", overlapped);
	
	if( (shutter = Nikon90i_shutter_new(hNikon90i)) == NULL)
		return -1;
	
	shutter_signal_hide_handler_connect (shutter, OnShutterClose, shutter); 
	shutter_changed_handler_connect (shutter, OnShutterChanged, shutter); 

	shutter_display_main_ui(shutter); 
	
	RunUserInterface();

  	return 0;
}
