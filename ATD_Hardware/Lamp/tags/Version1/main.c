#include "iscope90i.h"
#include "90i_lamp.h"

#include <stdio.h>
//#include <ansi_c.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for brightness control.
////////////////////////////////////////////////////////////////////////////

static void OnLampClose(Lamp* lamp, void *data)
{
	lamp_destroy(lamp);
	
	QuitUserInterface(0);	
}

static void OnLampChanged(Lamp* lamp, void *data)
{
	int status;
	double voltage;
	
	if ((lamp_status(lamp, &status) == LAMP_SUCCESS) && (lamp_get_intensity(lamp, &voltage) == LAMP_SUCCESS))
		printf("Lamp changed. status = %d. voltage = %f\n", status, voltage);
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err;
	Lamp *lamp;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	if( (lamp = Nikon90i_lamp_new(hNikon90i)) == NULL)
		return -1;
	
	lamp_signal_hide_handler_connect (lamp, OnLampClose, lamp); 
	lamp_changed_handler_connect (lamp, OnLampChanged, lamp); 

	lamp_display_main_ui(lamp); 
	
	RunUserInterface();

  	return 0;
}
