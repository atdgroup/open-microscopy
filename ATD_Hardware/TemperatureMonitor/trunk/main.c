#include "ATD_TemperatureMonitor_Dummy.h"
#include "ATD_TemperatureMonitor_A.h"
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
                       LPSTR lpsztmdLine, int ntmdShow)
{
	int stress=0;
	DeviceInfo info;
    TemperatureMonitor* tm = NULL;
	
  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"ATD_TemperatureMonitor_Dummy;ATD_TemperatureMonitor_A", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "ATD_TemperatureMonitor_Dummy") == 0)
	{
		tm = atd_temperature_monitor_dummy_new(info.name, "Temp Monitor", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "ATD_TemperatureMonitor_A") == 0)
	{
		tm = atd_temperature_monitor_a_new(info.name, "Temp Monitor", error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(tm), OnCloseOrHideEvent, NULL);

	temperature_monitor_initialise(tm, 0);
	temperature_monitor_hardware_initialise(tm);
	temperature_monitor_enable_timer(tm);

	ui_module_display_main_panel(UIMODULE_CAST(tm));

	RunUserInterface();
	
	temperature_monitor_destroy(tm);
	
  	return 0;
}
