#include "FilterSet_Dummy.h"
#include "gci_utils.h"

#include "icsviewer_window.h"

#include <utility.h>

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
                       LPSTR lpszCmdLine, int nCmdShow)
{
	int stress=1;
	DeviceInfo info;
    FluoCubeManager* cm = NULL;
	
  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"FilterSet_Dummy;", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "FilterSet_Dummy") == 0)
	{
		cm = manual_filterset_new(info.name, "DummyFilterSet1",
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", "DummyFilterSet1.ini", error_handler, NULL);
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(cm), OnCloseOrHideEvent, NULL);

	filterset_initialise(cm, 0);
	filterset_hardware_initialise(cm);

	ui_module_display_main_panel(UIMODULE_CAST(cm));

	RunUserInterface();
	
	filterset_destroy(cm);
	
  	return 0;
}
