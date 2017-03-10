#include "ATD_Lamp_Dummy.h"
#include "ATD_LedLamp_A.h"
#include "ATD_LedLamp_B.h"
#include "ATD_LedLighting_A.h"
#include "OfflineImager_Lamp.h"
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
                       LPSTR lpszlampdLine, int nlampdShow)
{
	int stress=0;
	DeviceInfo info;
    Lamp* lamp = NULL;
	
  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"ATD_LedLamp_Dummy;ATD_LedLamp_A;ATD_LedLamp_B;ATD_LedLighting_A;OfflineImager", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "ATD_LedLamp_Dummy") == 0)
	{
		lamp = manual_lamp_new(info.name, "Lamp", error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else if(strcmp(info.type, "ATD_LedLamp_A") == 0)
	{
		lamp = adt_a_led_lamp_new(info.name, "Lamp", error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");		
	}
	else if(strcmp(info.type, "ATD_LedLamp_B") == 0)
	{
		lamp = adt_b_led_lamp_new(info.name, "Lamp", error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");		
	}
	else if(strcmp(info.type, "ATD_LedLighting_A") == 0)
	{
		lamp = atd_led_lighting_a_new(info.name, "Lamp", error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");		
	}
	else if(strcmp(info.type, "OfflineImager") == 0)
	{
		lamp = offline_imager_lamp_new(info.name, "Lamp", error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");		
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(lamp), OnCloseOrHideEvent, NULL);

	lamp_initialise(lamp, 0);
	lamp_enable_timer(lamp);

	ui_module_display_main_panel(UIMODULE_CAST(lamp));

	RunUserInterface();
	
	lamp_destroy(lamp);
	
  	return 0;
}
