#include "ATD_CoarseZDrive_A.h"
#include "gci_utils.h"

#include "icsviewer_window.h"

#include "profile.h"
#include "config.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

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

	#ifdef DEVELOPMENTAL_FEATURES 

	HANDLE handle_out, handle_in;
	FILE* hf_out, *hf_in;
	int hCrt;
	HWND hwnd;
	UINT myTimer;

	AllocConsole();
	
    handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
    hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;

	#endif

  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"ATD_CoarseZDrive_A1", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "ATD_CoarseZDrive_A") == 0)
	{
		zd = atd_coarse_zdrive_a_new(info.name, "ATD_CoarseZDrive_A5", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", "ATD_CoarseZDrive_A5.ini");
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(zd), OnCloseOrHideEvent, NULL);

	coarse_z_drive_hardware_initialise(zd);
	coarse_z_drive_initialise(zd);
	coarse_zdrive_disable_timer(zd);
	//coarse_zdrive_enable_timer(zd);

	ui_module_display_main_panel(UIMODULE_CAST(zd));
	
	RunUserInterface();
	
	coarse_z_drive_destroy(zd);
	
  	return 0;
}
