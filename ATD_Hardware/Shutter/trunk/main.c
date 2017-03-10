#include "ATD_Shutter_Dummy.h"
#include "ATD_Shutter_A.h"
#include "OfflineImager_Shutter.h"
#include "gci_utils.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

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
                       LPSTR lpszshutterdLine, int nshutterdShow)
{
	int stress;
	DeviceInfo info;
    Shutter* shutter = NULL;
	
	HANDLE handle_out, handle_in;
	FILE* hf_out, *hf_in;
	int hCrt;
	HWND hwnd;
	UINT myTimer;
	BYTE val = 0;

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

  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"ATD_Shutter_Dummy;ATD_Shutter_A;OfflineImager_Shutter", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "ATD_Shutter_Dummy") == 0)
	{
		shutter = manual_shutter_new(info.name, "Shutter", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "ATD_Shutter_A") == 0)
	{
		shutter = atd_shutter_a_new(info.name, "Shutter", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "OfflineImager_Shutter") == 0)
	{
		shutter = offline_imager_shutter_new(info.name, "Shutter", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(shutter), OnCloseOrHideEvent, NULL);

	shutter_hardware_initialise(shutter);
	shutter_initialise(shutter, 0);

	ui_module_display_main_panel(UIMODULE_CAST(shutter));

	shutter_enable_timer(shutter);

	RunUserInterface();
	
	shutter_destroy(shutter);
	
  	return 0;
}
