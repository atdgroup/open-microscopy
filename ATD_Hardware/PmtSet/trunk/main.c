#include "PmtSet_Dummy.h"
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
                       LPSTR lpszopdLine, int nopdShow)
{
	int stress=0;
	DeviceInfo info;
    PmtSet* op = NULL;
	
  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"PmtSet_Dummy;ATD_PmtSet_A;ATD_PmtSet_B", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "PmtSet_Dummy") == 0)
	{
		op = manual_pmtset_new(info.name, "PmtSet_Dummy_1",
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", "PmtSet_Dummy_1.ini", error_handler, NULL);
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

	pmtset_initialise(op, 0);
	pmtset_hardware_initialise(op);

	ui_module_display_main_panel(UIMODULE_CAST(op));

	RunUserInterface();
	
	pmtset_destroy(op);
	
  	return 0;
}
