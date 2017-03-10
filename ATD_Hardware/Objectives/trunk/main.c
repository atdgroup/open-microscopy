#include "iscope90i.h"
#include "90i_objectives.h"

#include <stdio.h>
#include <shlobj.h> 
#include <utility.h>

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Standalone test for objective control.
////////////////////////////////////////////////////////////////////////////

static void OnObjectiveManagerClose(ObjectiveManager* manager, void *data)
{
	objective_manager_destroy(manager);
	
	QuitUserInterface(0);	
}

static void OnObjectiveStartChange(ObjectiveManager* manager, void *data)
{
	printf("Start objective change.\n");
}

static void OnObjectiveEndChange(ObjectiveManager* manager, void *data)
{
	Objective obj;
	int pos;
	
	if (objective_manager_get_current_objective(manager, &obj) == OBJECTIVE_MANAGER_SUCCESS)
		printf("End objective change to pos %d a.\n", obj._turret_position);
	else if (objective_manager_get_current_turret_position(manager, &pos) == OBJECTIVE_MANAGER_SUCCESS)
		printf("End objective change to pos %d b.\n", pos);
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char *SysName;
	char *version;
	CAObjHandle hNikon90i;
	ERRORINFO errInfo;
	int err;
	char path[GCI_MAX_PATHNAME_LEN];
	ObjectiveManager *objective_manager;

	if (!SUCCEEDED(CoInitialize(NULL)))
	{
		printf("Unable to initialize COM");
		return -1;
	}
	
  	if (InitCVIRTE (hInstance, 0, 0) == 0) return -1;    /* out of memory */


	//Create Nikon 90i instance
	ISCOPELib_NewINikon90i (NULL, 1, LOCALE_NEUTRAL, 0, &hNikon90i);
	err = ISCOPELib_INikon90iGetSystemName (hNikon90i, &errInfo, &SysName);
	err = ISCOPELib_INikon90iReadVersion (hNikon90i, &errInfo, ISCOPELibConst_Nikon90iController, &version);
	if (err) 
		err = CA_DisplayErrorInfo (hNikon90i, NULL, err, &errInfo);

	GetProjectDir (path);
	strcat(path, "\\Microscope Data\\ObjectiveData.xml");
	if( (objective_manager = Nikon90i_objective_manager_new(hNikon90i, path)) == NULL) {
		return -1;
	}

	objective_manager_nosepiece_mounted(objective_manager);
	
	objective_manager_signal_close_handler_connect (objective_manager, OnObjectiveManagerClose, NULL);
	objective_manager_signal_start_change_handler_connect (objective_manager, OnObjectiveStartChange, NULL);
	objective_manager_signal_end_change_handler_connect (objective_manager, OnObjectiveEndChange, objective_manager);

	objective_manager_display_main_ui(objective_manager); 
	
	RunUserInterface();
	
	CoUninitialize();  

  	return 0;
}
