#include "white_light_laser.h"
#include "gci_utils.h"

#include "profile.h"
#include "config.h"

#define FORTIFY

#include "fortify.h"



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
    WhiteLightLaser* laser = NULL;
	DeviceInfo info;
	int stress = 0;
 
	//Fortify_EnterScope(); 
	
	if(ShowStandaloneIniDevicesController(CONFIG_INI_FILEPATH,
		"FianiumLaser1", &stress, &info) < 0) {
		return -1;
	}

	if(strcmp(info.type, "FianiumLaser") == 0)
	{
		laser = fianium_whitelight_laser_new("FianiumLaser1", "Fianium Laser", error_handler, TOPLEVEL_SOURCE_DIRECTORY "\\DataDir");
	}
	else
	{
		MessagePopup("Error", "Device does not exist ?");
		return -1;
	}

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(laser), OnCloseOrHideEvent, NULL);

	fianium_whitelight_laser_hardware_init(laser);

	fianium_whitelight_laser_start_display_timer(laser);

	ui_module_display_main_panel(UIMODULE_CAST(laser));

	RunUserInterface();
	
	fianium_whitelight_laser_destroy(laser);

	//Fortify_LeaveScope();

  	return 0;
}
