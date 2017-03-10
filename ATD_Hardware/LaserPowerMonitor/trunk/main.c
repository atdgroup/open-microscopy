#include "atd_laserpowermonitor_a.h"

#include <stdio.h>
#include "gci_utils.h"

#include <utility.h>

#include "profile.h"
#include "config.h"

void OnLaserPowerMonitorClose (UIModule *module, void *data)
{
	QuitUserInterface(0);	
}

static int error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	MessagePopup("Error", error_string);
	
	return UIMODULE_ERROR_NONE;
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	int err;
	LaserPowerMonitor *lm;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)
        return -1;    /* out of memory */

	lm = atd_laserpowermonitor_a_new("atd_laserpowermonitor_a2", "atd_laserpowermonitor_a",
		error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", NULL);

	if(lm == NULL)
		return -1;
	
	hardware_device_hardware_initialise(HARDWARE_DEVICE_CAST(lm));
	laserpowermonitor_initialise(lm);

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(lm), OnLaserPowerMonitorClose, NULL);

	ui_module_display_main_panel(UIMODULE_CAST(lm));

	//laserpowermonitor_enable_timer(lm);

	RunUserInterface();

	hardware_device_destroy(HARDWARE_DEVICE_CAST(lm));

  	return 0;
}
