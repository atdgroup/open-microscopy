#include <utility.h>

//#include <stdio.h>
#include "com_port_control.h"
#include <intensilight.h>

////////////////////////////////////////////////////////////////////////////
//RJL January 2007
//Standalone test for Nikon Intensilight control.
////////////////////////////////////////////////////////////////////////////

static void OnIntensilightClose(Intensilight* intensilight, void *data)
{
	intensilight_destroy(intensilight);
	
	QuitUserInterface(0);	
}

static void OnIntensilightChange(Intensilight* intensilight, void *data)
{
	double val;
	
	//if (precise_excite_get(precise_excite, &val) == PRECISEXCITE_SUCCESS)
	//	printf("precisExcite changed to %f.\n", val);
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	int err;
	char path[MAX_PATHNAME_LEN];
	Intensilight* intensilight;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Use Glen's COM port allocation module
	GCI_ComPortControlInit();		 //Initialise Com port table
	GetProjectDir(path);
	strcat(path, "\\Data\\");
	GCI_ComPortControlSetDataDir(path);

	//Create precisExcite instance
	if( (intensilight = intensilight_new("Intensilight", "Nikon Light Source")) == NULL)
		return -1;
	
	intensilight_signal_hide_handler_connect (intensilight, OnIntensilightClose, intensilight); 
	intensilight_changed_handler (intensilight, OnIntensilightChange, NULL); 

	intensilight_display_main_ui(intensilight); 
	
	RunUserInterface();

  	return 0;
}
