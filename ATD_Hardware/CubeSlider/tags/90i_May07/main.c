#include "iscope90i.h"
#include "90i_fluorescent_cubes.h"

#include <utility.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for cube control.
////////////////////////////////////////////////////////////////////////////

static void OnFluoCubeManagerClose(FluoCubeManager* cube_manager, void *data)
{
	cube_manager_destroy(cube_manager);
	
	QuitUserInterface(0);	
}

static void OnFluoCubeManagerChanged(FluoCubeManager* cube_manager, void *data)
{
	FluoCube cube;
	int pos;
	
	if (cube_manager_get_current_cube(cube_manager, &cube) == CUBE_MANAGER_SUCCESS)
		printf("Cube changed to pos %d a.\n", cube.position);
	else if (cube_manager_get_current_cube_position(cube_manager, &pos) == CUBE_MANAGER_SUCCESS)
		printf("Cube changed to pos %d b.\n", pos);
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err;
	char path[MAX_PATHNAME_LEN];
	FluoCubeManager *cube_manager;

  	if (InitCVIRTE (hInstance, 0, 0) == 0) return -1;    /* out of memory */

	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	GetProjectDir (path);
	strcat(path, "\\Microscope Data\\CubeData.xml");
	if( (cube_manager = Nikon90i_fluo_cube_manager_new(hNikon90i, path)) == NULL)
		return -1;

	cube_manager_signal_close_handler_connect (cube_manager, OnFluoCubeManagerClose, cube_manager); 
	cube_manager_signal_cube_changed_handler_connect(cube_manager, OnFluoCubeManagerChanged, NULL);

	cube_manager_display_main_ui(cube_manager); 
	
	RunUserInterface();

  	return 0;
}
