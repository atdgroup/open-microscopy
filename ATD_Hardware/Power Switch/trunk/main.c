#include "ATD_PowerSwitch_A.h"
#include "gci_utils.h"

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

int main()
{
    PowerSwitch* ps = NULL;
	DeviceInfo info;
	int stress = 0;

 	//if (InitCVIRTE (hInstance, 0, 0) == 0)
  	//	return -1;    /* out of memory */

	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"ATD_PowerSwitch_A", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "ATD_PowerSwitch_A") == 0)
	{
		ps = atd_power_switch_a_new(info.name, "ATD_PowerSwitch_A",
                    error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(ps), OnCloseOrHideEvent, NULL);

	power_switch_initialise(ps);

	ui_module_display_main_panel(UIMODULE_CAST(ps));

	RunUserInterface();
	
	power_switch_destroy(ps);
	
  	return 0;
}
