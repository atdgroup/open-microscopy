#include "iscope90i.h"
#include "90i_optical_path.h"

#include <utility.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for optical path control.
////////////////////////////////////////////////////////////////////////////

static void OnOpticalPathManagerClose(OpticalPathManager* optical_path_manager, void *data)
{
	optical_path_manager_destroy(optical_path_manager);
	
	QuitUserInterface(0);	
}

static void OnOpticalPathManagerChanged(OpticalPathManager* optical_path_manager, void *data)
{
	OpticalPath optical_path;
	int pos;
	
	if (optical_path_manager_get_current_optical_path(optical_path_manager, &optical_path) == OPTICAL_PATH_MANAGER_SUCCESS)
		printf("Optical Path changed to pos %d a.\n", optical_path.position);
	else if (optical_path_manager_get_current_optical_path_position(optical_path_manager, &pos) == OPTICAL_PATH_MANAGER_SUCCESS)
		printf("Optical Path changed to %d.\n", pos);
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
	OpticalPathManager *optical_path_manager;

  	if (InitCVIRTE (hInstance, 0, 0) == 0) return -1;    /* out of memory */

	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	GetProjectDir (path);
	strcat(path, "\\Microscope Data\\OpticalPathData.xml");
	if( (optical_path_manager = Nikon90i_optical_path_manager_new(hNikon90i, path)) == NULL)
		return -1;

	optical_path_manager_signal_close_handler_connect (optical_path_manager, OnOpticalPathManagerClose, optical_path_manager); 
	optical_path_manager_signal_optical_path_changed_handler_connect(optical_path_manager, OnOpticalPathManagerChanged, NULL);

	optical_path_manager_display_main_ui(optical_path_manager); 
	
	RunUserInterface();

  	return 0;
}
