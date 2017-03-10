
//#include <stdio.h>
#include <precisExcite.h>

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Standalone test for COOLLED precisExcite control.
////////////////////////////////////////////////////////////////////////////

static void OnPrecisExciteClose(precisExcite* precise_excite, void *data)
{
	precise_excite_destroy(precise_excite);
	
	QuitUserInterface(0);	
}

static void OnPrecisExciteChange(precisExcite* precise_excite, void *data)
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
	precisExcite *precise_excite;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Create precisExcite instance
	if( (precise_excite = precise_excite_new("precisExcite", "COOLLED Light Source")) == NULL)
		return -1;
	
	precise_excite_signal_hide_handler_connect (precise_excite, OnPrecisExciteClose, precise_excite); 
	precise_excite_changed_handler (precise_excite, OnPrecisExciteChange, NULL); 

	precise_excite_display_main_ui(precise_excite); 
	
	RunUserInterface();

  	return 0;
}
