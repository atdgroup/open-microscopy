#include "iscope90i.h"
#include "90i_field_stop.h"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for field stop control.
////////////////////////////////////////////////////////////////////////////

static void OnFieldStopClose(FieldStop* field_stop, void *data)
{
	field_stop_destroy(field_stop);
	
	QuitUserInterface(0);	
}

static void OnFieldStopChange(FieldStop* field_stop, void *data)
{
	double val;
	
	if (field_stop_get(field_stop, &val) == FIELD_STOP_SUCCESS)
		printf("Field stop changed to %f.\n", val);
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err;
	FieldStop *field_stop;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	if( (field_stop = Nikon90i_field_stop_new(hNikon90i)) == NULL)
		return -1;
	
	field_stop_signal_hide_handler_connect (field_stop, OnFieldStopClose, field_stop); 
	field_stop_changed_handler (field_stop, OnFieldStopChange, NULL); 

	field_stop_display_main_ui(field_stop); 
	
	RunUserInterface();

  	return 0;
}
