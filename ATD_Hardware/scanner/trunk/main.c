#include "ATD_Scanner_A.h"
#include "ATD_Scanner_Dummy.h"
#include "gci_utils.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "icsviewer_window.h"

#include "profile.h"
#include "config.h"

static int error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	//MessagePopup("Error", error_string);
	//printf("Error", error_string);

	return UIMODULE_ERROR_NONE;
}

static void OnCloseOrHideEvent (UIModule *module, void *data)
{
	QuitUserInterface(0);
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
    Scanner* scanner = NULL;
	DeviceInfo info;
	int stress = 0;

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
		"ATD_Scanner_Dummy;ATD_Scanner_A", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "ATD_Scanner_Dummy") == 0)
	{
		//scanner = manual_scanner_new(info.name, "Scanner", error_handler,
		//	TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "ATD_Scanner_A") == 0)
	{
		scanner = atd_scanner_a_new(info.name, "Scanner", 
					TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", "ATD_Scanner_A.ini", error_handler, NULL);
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(scanner), OnCloseOrHideEvent, NULL);

	scanner_hardware_initialise(scanner);
	scanner_initialise(scanner);

	ui_module_display_main_panel(UIMODULE_CAST(scanner));

	scanner_enable_timer(scanner);

	RunUserInterface();
	
	scanner_destroy(scanner);
	
  	return 0;
}
