#include "hts_fluorescent_cubes.h"

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
	char path[MAX_PATHNAME_LEN];
	FluoCubeManager *cube_manager;

  	if (InitCVIRTE (hInstance, 0, 0) == 0) return -1;    /* out of memory */

	GetProjectDir (path);
	strcat(path, "\\Microscope Data\\CubeData.xml");
	if( (cube_manager = HTS_fluo_cube_manager_new(path)) == NULL)
		return -1;

	cube_manager_signal_close_handler_connect (cube_manager, OnFluoCubeManagerClose, cube_manager); 
	cube_manager_signal_cube_changed_handler_connect(cube_manager, OnFluoCubeManagerChanged, NULL);

	cube_manager_display_main_ui(cube_manager); 
	
	RunUserInterface();

  	return 0;
}
