#include "HardWareTypes.h" 
#include "HardwareDevice.h"

#include "microscope.h"
#include "MicroscopeCommon.h"
#include "MicroscopeUI.h"

#include "CubeSlider.h"
#include "lamp.h"
#include "Shutter.h"
#include "PowerSwitch.h"
#include "OpticalPath.h"

#include "gci_menu_utils.h"

#ifdef BUILD_MODULE_PMT
#include "PmtSet.h"
#endif

#ifdef BUILD_MODULE_FILTER
#include "FilterSet.h"
#endif

void microscope_add_common_module_menu_entries (Microscope* microscope)
{
	int i;
	int menu_bar = GetPanelMenuBar (microscope->_main_ui_panel); 

	microscope_add_menuitem(microscope, "Components//Log", 0, OnMicroscopeMenuItemLogClicked, microscope->_ri);

	microscope_add_menuitem(microscope, "Configure//Load and save settings//Reset all modes to default settings",
		0, OnMicroscopeResetToDefaultSettingsClicked, microscope);

	microscope_add_menuitem(microscope, "Configure//Load and save settings//Save settings as ...",
		0, OnMicroscopeSaveSettingsAsClicked, microscope);

	microscope_add_menuitem(microscope, "Configure//Load and save settings//Load settings from file ...",
		0, OnMicroscopeLoadSettingsFromFileClicked, microscope);
	
	microscope_add_menuitem(microscope, "Configure//Load and save settings//Save settings as default for this mode",
		0, OnMicroscopeSaveSettingsAsDefaultForModeClicked, microscope);

	microscope_add_menuitem(microscope, "Configure//Objective pixel calibration",
		0, OnMicroscopeMenuItemClicked,  microscope->_optical_cal);

	microscope_add_menuitem(microscope, "Configure//Background correction",
		0, OnMicroscopeMenuItemClicked, microscope->_ri);

	i = microscope_add_menuitem(microscope, "Configure//Automatic snap",
		0, OnMicroscopeMenuItemAutoSnapClicked, microscope);
	SetMenuBarAttribute (menu_bar, i, ATTR_CHECKED, microscope->_autosnap); 

	microscope_add_menuitem(microscope, "Configure//Reinitialise Devices",
		0, OnMicroscopeMenuItemReInitialiseDevicesClicked, microscope);
	
	// Help Menu
	microscope_add_menuitem(microscope, "Help//Program Help",
		0, OnMicroscopeMenuHelpHelp, microscope);
	
	microscope_add_menuitem(microscope, "Help//About",
		0, OnMicroscopeMenuHelpAbout, microscope);

	// Non hardware components

	microscope_add_menuitem(microscope, "Components//Software autofocus", 0, OnMicroscopeMenuItemClicked, microscope->_sw_af);
	
	microscope_add_menuitem(microscope, "Components//Realtime overview", 0, OnMicroscopeRealTimeOverviewClicked, microscope);
	
	microscope_add_menuitem(microscope, "Components//Wellplate definer", 0, OnMicroscopeMenuItemClicked, microscope->_wpd);
}

void microscope_disable_failed_devices(Microscope *microscope)
{
	int menu_bar = GetPanelMenuBar (microscope->_main_ui_panel); 
	
	// Disable ui elements for hardware that has not initialised correctly.
	if(MICROSCOPE_MASTER_CAMERA(microscope) == NULL || !gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope))) {
		
		#ifdef MICROSCOPE_LIVE
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LIVE, ATTR_DIMMED, 1);
		#endif

		#ifdef MICROSCOPE_SNAP
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_SNAP, ATTR_DIMMED, 1);
		#endif

		#ifdef MICROSCOPE_EXPOSURE
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_EXPOSURE, ATTR_DIMMED, 1);
		#endif

		#ifdef MICROSCOPE_GAIN
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_GAIN, ATTR_DIMMED, 1);
		#endif

		#ifdef MICROSCOPE_BINNING
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_BINNING, ATTR_DIMMED, 1);
		#endif

		DimMenuPathItem(menu_bar, "Components//Hardware//Camera");
		MICROSCOPE_MASTER_CAMERA(microscope) = NULL;  
	}
		
	if(microscope->_power_switch == NULL || !power_switch_is_initialised (microscope->_power_switch)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Power switch");
		microscope->_power_switch = NULL;
	}

	if(microscope->_stage == NULL || !stage_is_initialised (microscope->_stage)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Stage");
		
		#ifdef MICROSCOPE_DATUM_XY
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_DATUM_XY, ATTR_DIMMED, 1);
		#endif

		#ifdef MICROSCOPE_GOTO_XY_DATUM
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_GOTO_XY_DATUM, ATTR_DIMMED, 1); 
		#endif

		#ifdef MICROSCOPE_JOY_ENABLE
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_JOY_ENABLE, ATTR_DIMMED, 1);  
		#endif

		#ifdef MICROSCOPE_X_POS
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_X_POS, ATTR_DIMMED, 1);  
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_Y_POS, ATTR_DIMMED, 1); 
		#endif

		microscope->_stage = NULL;
	}
	
	if(microscope->_optical_path == NULL || !optical_path_is_initialised (microscope->_optical_path)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Optical path");
		microscope->_optical_path = NULL;  
	}
	
	#ifdef BUILD_MODULE_FILTER
    if(microscope->_filter_set == NULL || !filterset_is_initialised (microscope->_filter_set)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Filters");
		microscope->_filter_set = NULL;  
	}
	#endif

	#ifdef BUILD_MODULE_PMT
    if(microscope->_pmt_set == NULL || !pmtset_is_initialised (microscope->_pmt_set)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Pmts");
		microscope->_pmt_set = NULL;  
	}
	#endif

	if(microscope->_fluor_cube == NULL || !cube_manager_is_initialised (microscope->_fluor_cube)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Fluor cubes");
		microscope->_fluor_cube = NULL;  
	}
	
	if(microscope->_shutter == NULL || !shutter_is_initialised (microscope->_shutter)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Shutter");
		microscope->_shutter = NULL;
	}
	
	if(microscope->_lamp == NULL || !lamp_hardware_is_initialised (microscope->_lamp)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Lamp");

		#ifdef MICROSCOPE_LEDSTATE
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LEDSTATE, ATTR_DIMMED, 1);
		#endif

		#ifdef MICROSCOPE_LEDINTENSITY
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LEDINTENSITY, ATTR_DIMMED, 1); 
		#endif

		microscope->_lamp = NULL;
	}
	
	if(microscope->_z_drive == NULL || !z_drive_is_initialised (microscope->_z_drive)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Z Drive");
		microscope->_z_drive = NULL;
	}

	if(microscope->_laser_power_monitor == NULL || !hardware_device_hardware_is_initialised (HARDWARE_DEVICE_CAST( microscope->_laser_power_monitor))) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Laser Power");
		microscope->_laser_power_monitor = NULL;
	}
}

int microscope_disable_all_common_module_panels(Microscope* microscope, int disable)
{
	int i, menu_bar = GetPanelMenuBar (microscope->_main_ui_panel); 
	
	UIModule *module = NULL;

	microscope_dim_menubar(microscope, disable);

	//SetPanelAttribute (microscope->_main_ui_panel, ATTR_DIMMED, disable);
	
	ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope),  disable);

	// ref images ie background correction.
	if (microscope->_ri != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_ri),  disable);

	if (microscope->_objective != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_objective),  disable);

	if (microscope->_fluor_cube != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_fluor_cube),  disable);

	if (microscope->_optical_path != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_optical_path),  disable);

    if (microscope->_filter_set != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_filter_set),  disable);

    if (microscope->_pmt_set != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_pmt_set),  disable);

	if (microscope->_stage != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_stage),  disable);

	if (microscope->_lamp != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_lamp),  disable);

	if (microscope->_scanner != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_scanner),  disable);

	if (microscope->_shutter != NULL) 
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_shutter),  disable);

	if (microscope->_z_drive != NULL)
		ui_module_disable_all_panel_controls(UIMODULE_CAST(microscope->_z_drive),  disable);

	if (microscope->_spc != NULL) {
		ui_module_disable_all_panels_and_controls(UIMODULE_CAST(microscope->_spc), disable);
	}

    if (microscope->_laser != NULL) {
		ui_module_disable_all_panels_and_controls(UIMODULE_CAST(microscope->_laser), disable);
	} 

	// Set all cameras to dimmied or undimmed
	for(i=0; i < microscope->_number_of_cameras; i++)
	{
		GciCamera *cam = microscope->_cameras[i];
		
		ui_module_disable_all_panels_and_controls(UIMODULE_CAST(cam), disable);
	}

	return MICROSCOPE_SUCCESS;
}