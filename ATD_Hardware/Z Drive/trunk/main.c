#include "ATD_ZDrive_Dummy.h"
#include "ATD_ZDrive_A.h"
#include "ATD_ZDrive_B.h"
#include "Marzhauser_ZDrive_LStep.h"
#include "prior_piezo_focus.h"
#include "gci_utils.h"

#include "icsviewer_window.h"

#include "profile.h"
#include "config.h"

static int error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	MessagePopup("Error", error_string);
	
	return UIMODULE_ERROR_NONE;
}

static void OnCloseOrHideEvent (UIModule *module, void *data)
{
	QuitUserInterface(0);
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszzddLine, int nzddShow)
{
	int stress=0;
	DeviceInfo info;
    Z_Drive* zd = NULL;
	XYStage* ls_stage = NULL;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"ATD_ZDrive_Dummy;ATD_ZDrive_A;ATD_ZDrive_B;LStep_ZDrive;PRIOR_ZDrive_NanoScanZ", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "ATD_ZDrive_Dummy") == 0)
	{
		zd = manual_zdrive_new(info.name, "ZDrive", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "ATD_ZDrive_A") == 0)
	{
		zd = atd_zdrive_a_new(info.name, "ZDrive", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "ATD_ZDrive_B") == 0)
	{
		zd = atd_zdrive_b_new(info.name, "ZDrive", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "LStep_ZDrive") == 0)
	{
		XYStage* ls_stage = lstep_xy_stage_new("LStepStage", "XYStage", error_handler, NULL, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");

		zd = marzhauser_zdrive_lstep_new(ls_stage, "LStepZDrive_1", "ZDrive", error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "PRIOR_ZDrive_NanoScanZ") == 0)
	{
		zd = prior_zdrive_new(info.name, "ZDrive", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(zd), OnCloseOrHideEvent, NULL);

	z_drive_hardware_initialise(zd);
	z_drive_initialise(zd, 0);

	hardware_load_state_from_file(HARDWARE_DEVICE_CAST(zd), TOPLEVEL_SOURCE_DIRECTORY "\\DataDir\\ATD_CoarseZDrive_A2.ini");

	ui_module_display_main_panel(UIMODULE_CAST(zd));
	
	RunUserInterface();
	
	z_drive_destroy(zd);
	
	if(ls_stage != NULL)
		stage_destroy(ls_stage);

  	return 0;
}
