#include "ATD_OpticalPath_Dummy.h"
#include "ATD_OpticalPath_A.h"
#include "ATD_OpticalPath_B.h"
//#include "Nikon_OpticalPath_90i.h"
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
                       LPSTR lpszopdLine, int nopdShow)
{
	int stress=0;
	DeviceInfo info;
    OpticalPathManager* op = NULL;
	
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
		"ATD_OpticalPath_Dummy;ATD_OpticalPath_A;ATD_OpticalPath_B", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "ATD_OpticalPath_Dummy") == 0)
	{
		op = manual_optical_path_new(info.name, "ATD_OpticalPath_Dummy",
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", "OpticalPathData1.ini", error_handler, NULL);
	}
	else if(strcmp(info.type, "ATD_OpticalPath_A") == 0)
	{
		op = atd_op_a_optical_path_new(info.name, "ATD_OpticalPath_A",
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", "ATD_OpticalPath_A4.ini", error_handler, NULL);
	}
	else if(strcmp(info.type, "ATD_OpticalPath_B") == 0)
	{
		op = atd_op_b_optical_path_new(info.name, "ATD_OpticalPath_B",
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", "OpticalPathData1.ini", error_handler, NULL);
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	if(op == NULL) {

		GCI_MessagePopup("Error", "Failed to create device");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(op), OnCloseOrHideEvent, NULL);
	
	optical_path_hardware_initialise(op);
	optical_path_initialise(op, 0);

	ui_module_display_main_panel(UIMODULE_CAST(op));

	RunUserInterface();
	
	optical_path_destroy(op);
	
  	return 0;
}
