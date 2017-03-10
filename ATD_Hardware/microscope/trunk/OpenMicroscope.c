#include "microscope.h"
#include "OpenMicroscope.h"
#include "OpenMicroscopeUI.h"
#include "MicroscopeUI.h"
#include "MicroscopeCommon.h"
#include "gci_utils.h"
#include "string_utils.h"
#include "gci_menu_utils.h"

#include "camera\gci_camera.h"
#include "ATD_Objectives_Dummy\ATD_Objectives_Dummy.h"
#include "ATD_CubeSlider_Dummy\ATD_CubeSlider_Dummy.h"
#include "ATD_Shutter_Dummy\ATD_Shutter_Dummy.h"   
#include "ATD_OpticalPath_Dummy\ATD_OpticalPath_Dummy.h"
#include "ATD_ZDrive_Dummy\ATD_ZDrive_Dummy.h" 
#include "ATD_Scanner_Dummy\ATD_Scanner_Dummy.h" 
#include "ATD_LedLamp_Dummy\ATD_Lamp_Dummy.h" 
#include "dummy\gci_dummy_camera.h"
#include "PmtSet_Dummy\PmtSet_Dummy.h"
#include "FilterSet_Dummy\FilterSet_Dummy.h"
#include "LaserPowerMonitor.h"
#include "dummy_laser.h"
#include "ATD_LaserPowerMonitor_LOGP.h"

#ifdef BUILD_MODULE_SPC 
#include "spc.h"
#endif

#include "dummy\dummy_stage.h" 
#include "StagePlate.h"
#include "white_light_laser.h"

#include "cell_finding.h"

#include "microscope.h"

#include <cviauto.h>
#include "asynctmr.h"
#include <userint.h>
#include <formatio.h>
#include <utility.h>

static void OnMicroscopeFluorCubeChanged(FluoCubeManager* manager, int pos, void *data)
{
	Microscope* microscope = (Microscope*)data;
	FluoCube cube;

	device_conf_set_active_devices_list_control_to_pos(microscope->_fluor_cube->dc, microscope->_main_ui_panel, MICROSCOPE_CUBE, pos);  

	microscope->_change_camera_palette = 1;

	if(microscope->_laser_power_monitor != NULL) {
		cube_manager_get_current_cube(microscope->_fluor_cube, &cube);
		laserpowermonitor_set_wavelength_range (microscope->_laser_power_monitor, (double)cube.exc_min_nm, (double)cube.exc_max_nm);        
	}
}


static void OnMicroscopeFluorCubeConfigChanged(FluoCubeManager* manager, void *data)
{
	Microscope* microscope = (Microscope*) data;

	cube_manager_load_active_cubes_into_list_control(microscope->_fluor_cube, microscope->_main_ui_panel, MICROSCOPE_CUBE);   
	
	//cube_manager_goto_default_position(microscope->_fluor_cube); 
}

static void OnMicroscopeObjectiveChanged(ObjectiveManager* manager, int pos, void *data)
{
	Microscope* microscope = (Microscope*)data;
	Objective obj;
	double factor = 1.0;
	
	// Update the image window calibration
/*	
	device_conf_set_active_devices_list_control_to_pos(microscope->_objective->dc, microscope->_main_ui_panel, MICROSCOPE_OBJ, pos);  

	optical_calibration_get_calibration_factor_for_current_objective(microscope->_optical_cal, &factor); 
	
	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL && gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope)))
		GCI_ImagingWindow_SetMicronsPerPixelFactor(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window, factor);

	if(microscope->_rto != NULL)
		realtime_overview_update_microns_per_pixel(microscope->_rto);
*/
	microscope_setup_optical_calibration(microscope);

	if(microscope->_laser_power_monitor != NULL) {
		float val;
		
		objective_manager_get_current_objective(microscope->_objective, &obj);
		if (obj._back_aperture != NULL) {
			sscanf(obj._back_aperture, "%f", &val);
			if (val >= 0.0)
				laserpowermonitor_set_aperture (microscope->_laser_power_monitor, (double)val);        
		}
	}
}	

static void OnMicroscopeObjectiveConfigChanged(ObjectiveManager* manager, void *data)
{
	Microscope* microscope = (Microscope*) data;

	objective_manager_load_active_objectives_into_list_control(microscope->_objective, microscope->_main_ui_panel, MICROSCOPE_OBJ);        
}

static void OnMicroscopeLampChanged(Lamp *lamp, void *data)
{
	Microscope* microscope = (Microscope*) data;
	
	// The leds have changed in some way. Make sure the state control on the led panel
	// is in sync with the one on the dummy microscope panel.
	LampStatus status;
	double intensity;
	
	lamp_off_on_status(microscope->_lamp, &status); 
	lamp_get_intensity(microscope->_lamp, &intensity);
	
	// 1 = Off 2 == On
	
	if(status == LAMP_OFF)
		SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_LEDSTATE, 1);    
	else // We are on pulsed or free running
		SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_LEDSTATE, 2); 

	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_LEDINTENSITY, intensity);    
}

static void OnMicroscopeOpticalPathChanged(OpticalPathManager* optical_path_manager, int pos, void *data)
{
	Microscope* microscope = (Microscope*)data;
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	//optical_path_set_list_control_to_pos(microscope->_optical_path, microscope->_optical_path->_main_ui_panel, OPATH_PNL_TURRET_POS, pos); 
	//optical_path_set_list_control_to_pos(microscope->_optical_path, microscope->_main_ui_panel, MICROSCOPE_OPTICAL_PATH, pos); 
	
	device_conf_set_active_devices_list_control_to_pos(microscope->_optical_path->dc, microscope->_main_ui_panel, MICROSCOPE_OPTICAL_PATH, pos);  

	// Has the opticalpath been changed to the colour camera ?
	// If so lets switch that to the MASTER camera.
	// We have designated pos 4 to the colour camera (For this microscope)
	
	if(pos == 3) {
	
		// Normal camera
		microscope_switch_camera_to_master(microscope, open_microscope->_original_master_camera);
		optical_calibration_device_set_device_as_default (OPTICAL_CALIBRATION_DEVICE_CAST(open_microscope->_original_master_camera));	
	}
	else if(pos == 4) {
		if(open_microscope->_colour_camera != NULL) {
			microscope_switch_camera_to_master(microscope, open_microscope->_colour_camera);
			optical_calibration_device_set_device_as_default (OPTICAL_CALIBRATION_DEVICE_CAST(open_microscope->_colour_camera));
		}
	}

	gci_camera_bring_to_front(MICROSCOPE_MASTER_CAMERA(microscope));

	SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_BINNING, ATTR_DIMMED,
		!gci_camera_supports_binning(MICROSCOPE_MASTER_CAMERA(microscope)));

	microscope_setup_optical_calibration(microscope);
}

static void OnMicroscopeOpticalPathConfigChanged(OpticalPathManager* optical_path_manager, void *data)
{
	Microscope* microscope = (Microscope*)data;  
	
	optical_path_load_active_paths_into_list_control(microscope->_optical_path, microscope->_main_ui_panel, MICROSCOPE_OPTICAL_PATH);
}

/*
static void OnMicroscopeStageChanged(XYStage* stage, void *data)
{
	Microscope* microscope = (Microscope*)data;
	double x, y;

	stage_get_xy_position(stage, &x, &y);

	//The stage position has changed
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_X_POS, x);
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_Y_POS, y);

	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_JOY_ON, stage->_joystick_status);
}
*/

static void OnMicroscopeXYStageChanged(XYStage* stage, double x, double y, void *data)
{
	Microscope* microscope = (Microscope*)data;

	//The stage position has changed
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_X_POS, x);
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_Y_POS, y);

	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_JOY_ON, stage->_joystick_status);
}

static void OnMicroscopeZDriveChanged(Z_Drive* z_drive, void *data)
{
	Microscope* microscope = (Microscope*)data;
	
	double z;
	
	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(microscope), &z);
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_Z_POS, z);    
}

static void OnMicroscopeZDriveChangedByUser(Z_Drive* z_drive, int commit, void *data)
{
	Microscope* microscope = (Microscope*)data;

	double z;
	
	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(microscope), &z);
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_Z_POS, z);    

	// This snaps a new image 
	if(commit)
		microscope_update_display(microscope);
	else {
		// wont snap if in snap mode.
		microscope_update_display_if_live(microscope);
	}
}


static void OnMicroscopeStagePlateChanged (StagePlateModule* stage_plate_module, int pos, void *data)
{
}

static void OnMicroscopeStagePlateConfigChanged (StagePlateModule* stage_plate_module, void *data)
{
	Microscope* microscope = (Microscope*)data;  
	
	//stage_plate_load_active_plates_into_list_control(microscope->_stage_plate_module,
	//	microscope->_main_ui_panel, MICROSCOPE_OPTICAL_PATH);
}


/*
static void OnMicroscopeStageStartInit(XYStage* stage, void *data)
{
	//The stage is about to initialise move any equipment if neccessary
	
	//if (commit)
	//	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_Z_POS, z_drive->_current_pos);
}


static void OnMicroscopeStageEndInit(XYStage* stage, void *data)
{
	//Objective *obj;
	//Microscope* microscope = (Microscope*)data;
}
*/

int open_microscope_disable_all_panels(Microscope* microscope, int disable)
{
	microscope_disable_all_common_module_panels(microscope, disable);

	return MICROSCOPE_SUCCESS;
}


static void open_microscope_disable_failed_devices(Microscope *microscope)
{
	OpenMicroscope* open_microscope = (OpenMicroscope*) microscope;	

	int menu_bar = GetPanelMenuBar (microscope->_main_ui_panel); 
	
	// Disable ui elements for hardware that has not initialised correctly.
	if(open_microscope->_colour_camera == NULL || !gci_camera_is_powered_up(open_microscope->_colour_camera)) {
		DimMenuPathItem(menu_bar, "Components//Hardware//Colour Camera");
		open_microscope->_colour_camera = NULL;  
	}

	if(open_microscope->_colour_camera == NULL && open_microscope->_original_master_camera == NULL) {
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LIVE, ATTR_DIMMED, 1);
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_SNAP, ATTR_DIMMED, 1); 
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_EXPOSURE, ATTR_DIMMED, 1);
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_GAIN, ATTR_DIMMED, 1);
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_BINNING, ATTR_DIMMED, 1);
	}

	microscope_disable_failed_devices(microscope);			   
}

int open_microscope_initialise_hardware_user_interfaces (Microscope* microscope)
{
	int menu_bar;
	unsigned int i;
	char text[20];
	double min, max, gain;
	
	OpenMicroscope* open_microscope = (OpenMicroscope*) microscope;	

	microscope->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(microscope), "OpenMicroscopeUI.uir", MICROSCOPE, 1);  

	menu_bar = GetPanelMenuBar (microscope->_main_ui_panel);
	
	microscope_add_common_module_menu_entries (microscope);

	// Components

	microscope_add_menuitem(microscope, "Components//Hardware//Camera", 0, OnMicroscopeMenuItemClicked, open_microscope->_original_master_camera);

	microscope_add_menuitem(microscope, "Components//Hardware//Colour Camera", 0, OnMicroscopeMenuItemClicked, open_microscope->_colour_camera);
		
	microscope_add_menuitem(microscope, "Components//Hardware//Objectives", 0, OnMicroscopeMenuItemClicked, microscope->_objective);

	microscope_add_menuitem(microscope, "Components//Hardware//Fluor cubes", 0, OnMicroscopeMenuItemClicked, microscope->_fluor_cube);

	microscope_add_menuitem(microscope, "Components//Hardware//Optical path", 0, OnMicroscopeMenuItemClicked, microscope->_optical_path);

	microscope_add_menuitem(microscope, "Components//Hardware//Filters", 0, OnMicroscopeMenuItemClicked, microscope->_filter_set);

	microscope_add_menuitem(microscope, "Components//Hardware//Pmts", 0, OnMicroscopeMenuItemClicked, microscope->_pmt_set);

	microscope_add_menuitem(microscope, "Components//Hardware//Lamp", 0, OnMicroscopeMenuItemClicked, microscope->_lamp);
	
	microscope_add_menuitem(microscope, "Components//Hardware//Scanner", 0, OnMicroscopeMenuItemClicked, microscope->_scanner);
	
    microscope_add_menuitem(microscope, "Components//Hardware//Shutter", 0, OnMicroscopeMenuItemClicked, microscope->_shutter);

	microscope_add_menuitem(microscope, "Components//Hardware//Stage", 0, OnMicroscopeMenuItemClicked, microscope->_stage);

	microscope_add_menuitem(microscope, "Components//Hardware//Stage Plates", 0, OnMicroscopeMenuItemClicked, microscope->_stage_plate_module);
	    
	microscope_add_menuitem(microscope, "Components//Hardware//Z-Drive", 0, OnMicroscopeMenuItemClicked, microscope->_z_drive);
	

	// End of menu


	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_OPTICAL_PATH, OnMicroscopeSetOpticalPath, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_JOY_ENABLE, OnMicroscopeJoystickEnable, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_MODE, OnMicroscopeSetIlluminationMode, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_CUBE, OnMicroscopeSetCube, microscope) < 0)
		return MICROSCOPE_ERROR;

	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_OBJ, OnMicroscopeSetObjective, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_SNAP, OnMicroscopeSnap, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_LIVE, OnMicroscopeLive, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_EXPOSURE, OnMicroscopeCameraExposureChanged, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_GAIN, OnMicroscopeCameraGainChanged, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_BINNING, OnMicroscopeCameraBinningChanged, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_LEDSTATE, OnMicroscopeLEDState, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_LEDINTENSITY, OnMicroscopeLEDIntensity, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_GOTO_XY_DATUM, OnMicroscopeGotoXYZDatum, microscope) < 0)
		return MICROSCOPE_ERROR;
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_DATUM_XY, OnMicroscopeSetDatumXYZ, microscope) < 0)
		return MICROSCOPE_ERROR;

	// Attched device change signals
	if(microscope->_stage != NULL) {
		stage_signal_xy_changed_handler_connect (microscope->_stage, OnMicroscopeXYStageChanged, microscope);
		//stage_signal_change_handler_connect (microscope->_stage, OnMicroscopeStageChanged, microscope);
	}

	if(microscope->_z_drive != NULL) {
		z_drive_changed_handler_connect (microscope->_z_drive, OnMicroscopeZDriveChanged, microscope);
		z_drive_changed_by_user_handler_connect (microscope->_z_drive, OnMicroscopeZDriveChangedByUser, microscope);
	}
	
	if(microscope->_objective != NULL) {
		objective_manager_signal_end_change_handler_connect(microscope->_objective, OnMicroscopeObjectiveChanged, microscope);  
		objective_manager_signal_config_changed_handler_connect(microscope->_objective,  OnMicroscopeObjectiveConfigChanged, microscope); 
	}

	// The hardware initialise is done but we have to call general initialise.
	// Sets up ui etc.
	
	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL && gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope))) {        
		gci_camera_initialise(MICROSCOPE_MASTER_CAMERA(microscope));    	
	}
		
	if(open_microscope->_colour_camera != NULL && gci_camera_is_powered_up(open_microscope->_colour_camera)) {        
		gci_camera_initialise(open_microscope->_colour_camera);  

	}

    if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL && gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope))) {
	
		// *** THIS GETS THE MIN AND MAX FROM THE UI
		gci_camera_get_gain_range(MICROSCOPE_MASTER_CAMERA(microscope), &min, &max);
		gci_camera_get_gain(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_CHANNEL1, &gain);
	
		//gci_camera_set_rotation(MICROSCOPE_MASTER_CAMERA(microscope), 90.0);

		// *** THIS SETS THE UI AGAIN !!!!
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_GAIN, ATTR_MIN_VALUE, min);
		SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_GAIN, ATTR_MAX_VALUE, max);
		SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_GAIN, gain);
	
		if (MICROSCOPE_MASTER_CAMERA(microscope)->number_of_binning_modes > 0) {
			ClearListCtrl (microscope->_main_ui_panel, MICROSCOPE_BINNING);
			for(i=0; i < MICROSCOPE_MASTER_CAMERA(microscope)->number_of_binning_modes; i++) {
		
				if(MICROSCOPE_MASTER_CAMERA(microscope)->supported_binning_modes[i] ==1)
					sprintf (text, "No Binning");  	
				else
					sprintf (text, "Binning by %d", (int) MICROSCOPE_MASTER_CAMERA(microscope)->supported_binning_modes[i]);
		
				InsertListItem (microscope->_main_ui_panel, MICROSCOPE_BINNING, -1, text, (int) MICROSCOPE_MASTER_CAMERA(microscope)->supported_binning_modes[i]);			
			}
		}
	}
    
	if(microscope->_fluor_cube != NULL)       
		cube_manager_initialise(microscope->_fluor_cube, 0); 	
		
    if(microscope->_filter_set != NULL) {
		filterset_initialise(microscope->_filter_set); 
	}

	if(microscope->_pmt_set != NULL) {  
		pmtset_initialise(microscope->_pmt_set);    
	}
    
	if(microscope->_stage_plate_module != NULL) {     
		stage_plate_initialise(microscope->_stage_plate_module); 

		stage_plate_signal_plate_changed_handler_connect(microscope->_stage_plate_module,
			OnMicroscopeStagePlateChanged, microscope);
		
		stage_plate_signal_plate_config_changed_handler_connect(microscope->_stage_plate_module,
			OnMicroscopeStagePlateConfigChanged, microscope);

		stage_set_stage_plate_module(microscope->_stage, microscope->_stage_plate_module);
	}

	if(microscope->_optical_path != NULL)      
		optical_path_initialise(microscope->_optical_path, 0);       
	
	if(microscope->_objective != NULL) {
		objective_manager_initialise(microscope->_objective, 0); 
	}
	
	if(microscope->_shutter != NULL) {
		shutter_initialise (microscope->_shutter);
	}

	if(microscope->_scanner != NULL) {
		scanner_initialise(microscope->_scanner);
	}
	
	if(microscope->_lamp != NULL)
		lamp_initialise(microscope->_lamp);
	
	if(microscope->_z_drive != NULL) {
		z_drive_initialise (microscope->_z_drive);  
		microscope_set_master_zdrive(microscope, microscope->_z_drive);
	}   
			
	#ifdef BUILD_MODULE_SPC 
	if(microscope->_spc != NULL) {
		if(spc_initialise (microscope->_spc) == SPC_ERROR) {
			spc_destroy(microscope->_spc);
			microscope->_spc = NULL;
		}
	}
	#endif

	if(microscope->_stage != NULL) {	
		microscope->_ss	= stage_scan_new();
	}

	// Attached event handlers to devices.
	if(microscope->_lamp != NULL)
		lamp_changed_handler_connect(microscope->_lamp, OnMicroscopeLampChanged, microscope);
	
	if(microscope->_optical_path != NULL) {
		optical_path_signal_changed_handler_connect(microscope->_optical_path, OnMicroscopeOpticalPathChanged, microscope);
		optical_path_signal_config_changed_handler_connect(microscope->_optical_path, OnMicroscopeOpticalPathConfigChanged, microscope);     
	}
	
	if(microscope->_fluor_cube != NULL) {
		
		FluoCube cube;

		cube_manager_signal_cube_changed_handler_connect(microscope->_fluor_cube, OnMicroscopeFluorCubeChanged, microscope);
		cube_manager_signal_cube_config_changed_handler_connect(microscope->_fluor_cube, OnMicroscopeFluorCubeConfigChanged, microscope);
		cube_manager_get_current_cube(microscope->_fluor_cube, &cube);
		GCI_ImagingWindow_SetFalseColourPaletteAsDefault(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window,  cube.emm_min_nm);
	}
	
	// Enter the cubes into the ui ring control.
	if(microscope->_fluor_cube != NULL)
		cube_manager_load_active_cubes_into_list_control(microscope->_fluor_cube, microscope->_main_ui_panel, MICROSCOPE_CUBE);   
	
	// Enter the optical paths into the ui ring control.  
	if(microscope->_optical_path != NULL)
		optical_path_load_active_paths_into_list_control(microscope->_optical_path, microscope->_main_ui_panel, MICROSCOPE_OPTICAL_PATH);
    
	if(microscope->_objective != NULL)
		objective_manager_load_active_objectives_into_list_control(microscope->_objective, microscope->_main_ui_panel, MICROSCOPE_OBJ);
	
	if ( InstallCtrlCallback (microscope->_main_ui_panel, MICROSCOPE_CLOSE, OnMicroscopeClose, microscope) < 0)
		return MICROSCOPE_ERROR;	

	open_microscope_disable_failed_devices(microscope);
	
	return MICROSCOPE_SUCCESS;     
}

int om_save_microscope_settings(Microscope* microscope, const char* path)
{
	int pos;
	char buffer[1000]; 
	
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	// Delete the file then append everything to it
	DeleteFile(path);

	// Camera
	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) {  
		gci_camera_save_settings(MICROSCOPE_MASTER_CAMERA(microscope), path, "a");  
		ui_module_save_settings(UIMODULE_CAST(MICROSCOPE_MASTER_CAMERA(microscope)), path, "a");  	
	}
		
	if(open_microscope->_colour_camera != NULL) {  
		gci_camera_save_settings(open_microscope->_colour_camera, path, "a");  
		ui_module_save_settings(UIMODULE_CAST(open_microscope->_colour_camera), path, "a");  	
	}

	// Shutter
	if(microscope->_shutter != NULL) { 
		shutter_save_settings_in_ini_fmt (microscope->_shutter, path,"a");  // Append to file 
		ui_module_save_settings(UIMODULE_CAST(microscope->_shutter), path, "a"); 
	}
	
	// Cube
	if(microscope->_fluor_cube != NULL) {
		cube_manager_get_current_cube_position(microscope->_fluor_cube, &pos);
		sprintf(buffer, "CurrentPosition=%d\n\n", pos);
		str_change_char(buffer, '\n', '\0'); 
		if(!WritePrivateProfileSection(UIMODULE_GET_NAME(microscope->_fluor_cube), buffer, path))
			return MICROSCOPE_ERROR;
	}
	
	// LEDs
	if(microscope->_lamp != NULL) {
		lamp_save_settings (microscope->_lamp, path, "a") ;  
		ui_module_save_settings(UIMODULE_CAST(microscope->_lamp), path, "a"); 
	}
		
	if(microscope->_optical_path) {
		// Optical Path
		optical_path_get_current_position(microscope->_optical_path, &pos);
		sprintf(buffer, "CurrentPosition=%d\n\n", pos);
		str_change_char(buffer, '\n', '\0'); 
		if(!WritePrivateProfileSection(UIMODULE_GET_NAME(microscope->_optical_path), buffer, path))
			return MICROSCOPE_ERROR;
	}
	
	if(microscope->_objective) {
		// Objective
		objective_manager_get_current_position(microscope->_objective, &pos);
		sprintf(buffer, "CurrentPosition=%d\n\n", pos);
		str_change_char(buffer, '\n', '\0'); 
		if(!WritePrivateProfileSection(UIMODULE_GET_NAME(microscope->_objective), buffer, path))
			return MICROSCOPE_ERROR;
	}

	// Z Drive
	if(microscope->_z_drive != NULL) {
		//z_drive_save_settings_in_ini_fmt (microscope->_z_drive, path); // Append to file 
		ui_module_save_settings(UIMODULE_CAST(microscope->_z_drive), path, "a"); 
	}
    
	// Scanner
	if(microscope->_scanner != NULL) { 
		//scanner_save_settings (microscope->_scanner, path,"a");  // Append to file 
		ui_module_save_settings(UIMODULE_CAST(microscope->_scanner), path, "a"); 
	}

	#ifdef BUILD_MODULE_SPC 
    if(microscope->_spc != NULL) 
		ui_module_save_settings(UIMODULE_CAST(microscope->_spc), path, "a");
	#endif    

	return MICROSCOPE_SUCCESS;   
}

static int CVICALLBACK load_microscope_cube_settings(void *callback)
{
	Microscope* microscope = (Microscope*) callback;
	
	// Cube
	PROFILE_START("load_microscope_settings - cube_manager_move_to_position");

	cube_manager_move_to_position(microscope->_fluor_cube, microscope->_requested_mode_cube_position);

	PROFILE_STOP("load_microscope_settings - cube_manager_move_to_position");

	return MICROSCOPE_SUCCESS;   
}

static int CVICALLBACK load_microscope_op_settings(void *callback)
{
	Microscope* microscope = (Microscope*) callback;

	PROFILE_START("load_microscope_settings - optical_path_move_to_position");

	optical_path_move_to_position(microscope->_optical_path, microscope->_requested_mode_op_position);
	
	PROFILE_STOP("load_microscope_settings - optical_path_move_to_position");

	return MICROSCOPE_SUCCESS;   
}

int om_load_microscope_settings(Microscope* microscope, const char* path)
{
	dictionary* ini = iniparser_load(path);  
	int pos, cube_thread_id, op_thread_id;
	char buffer[64];
	
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	PROFILE_START("load_microscope_settings - cube");

	// Cube stuff in another thread as it takes a long time to move
	if(microscope->_fluor_cube != NULL){
		sprintf(buffer, "%s:CurrentPosition", UIMODULE_GET_NAME(microscope->_fluor_cube));
		pos = iniparser_getint(ini, buffer, -1); 
	
		if(pos != -1) {
			microscope->_requested_mode_cube_position = pos;
			CmtScheduleThreadPoolFunction (gci_thread_pool(), load_microscope_cube_settings, microscope, &cube_thread_id);
		}
	}

	PROFILE_START("load_microscope_settings - op");

	// Optical Path
	if(microscope->_optical_path != NULL){
		sprintf(buffer, "%s:CurrentPosition", UIMODULE_GET_NAME(microscope->_optical_path));
		pos = iniparser_getint(ini, buffer, -1); 
	
		if(pos != -1) {
			microscope->_requested_mode_op_position = pos;
			CmtScheduleThreadPoolFunction (gci_thread_pool(), load_microscope_op_settings, microscope, &op_thread_id);
		}
	}

		// Camera
	PROFILE_START("load_microscope_settings - gci_camera_load_settings");

	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL)
		gci_camera_load_settings (MICROSCOPE_MASTER_CAMERA(microscope), path);      

	if(open_microscope->_colour_camera != NULL)
		gci_camera_load_settings (open_microscope->_colour_camera, path);      	

	PROFILE_STOP("load_microscope_settings - gci_camera_load_settings");

	PROFILE_START("load_microscope_settings - shutter_load_settings_from_ini_fmt");

	// Shutter
	if(microscope->_shutter != NULL)
		shutter_load_settings_from_ini_fmt (microscope->_shutter, path);                     
	
	PROFILE_STOP("load_microscope_settings - shutter_load_settings_from_ini_fmt");

	// LEDs
	if(microscope->_lamp != NULL){
		lamp_load_settings (microscope->_lamp, path);
	}

    /*
        PROFILE_START("load_microscope_settings - objective_manager_move_to_position");

	// Objective
	if(microscope->_objective != NULL){
		sprintf(buffer, "%s:CurrentPosition", UIMODULE_GET_NAME(microscope->_objective));
		pos = iniparser_getint(ini, buffer, -1); 
	
		if(pos != -1)
			objective_manager_move_to_position(microscope->_objective, pos);
	}
	
	PROFILE_STOP("load_microscope_settings - objective_manager_move_to_position");
        */
        
	PROFILE_START("load_microscope_settings - z_drive_load_settings_from_ini_fmt");

    /*
	// Z Drive
	if(microscope->_z_drive != NULL)
		z_drive_load_settings_from_ini_fmt (microscope->_z_drive, path);
	
	PROFILE_STOP("load_microscope_settings - z_drive_load_settings_from_ini_fmt");
    */
    
	/*
	PROFILE_START("load_microscope_settings - scanner_load_settings");

	if(microscope->_scanner != NULL)
		scanner_load_settings (microscope->_scanner, path);
	
	PROFILE_STOP("load_microscope_settings - scanner_load_settings");
	*/

	iniparser_freedict(ini);        
	
	PROFILE_START("load_microscope_settings - ui_module_restore_settings");

	ui_module_restore_settings(UIMODULE_CAST(MICROSCOPE_MASTER_CAMERA(microscope)), path);   
	ui_module_restore_settings(UIMODULE_CAST(microscope->_shutter), path);  
	ui_module_restore_settings(UIMODULE_CAST(microscope->_lamp), path);  
	ui_module_restore_settings(UIMODULE_CAST(microscope->_z_drive), path);  
	ui_module_restore_settings(UIMODULE_CAST(microscope->_scanner), path); 
	
	PROFILE_STOP("load_microscope_settings - ui_module_restore_settings");

	PROFILE_START("load_microscope_settings - spc");

	#ifdef BUILD_MODULE_SPC 
	if(microscope->_spc != NULL) 
		ui_module_restore_settings(UIMODULE_CAST(microscope->_spc), path);
	#endif

	PROFILE_STOP("load_microscope_settings - spc");

	// Wait for cubes and opticalpath
	if(microscope->_fluor_cube != NULL){
		CmtWaitForThreadPoolFunctionCompletion(gci_thread_pool(), cube_thread_id, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
		CmtReleaseThreadPoolFunctionID(gci_thread_pool(),  cube_thread_id);
	}

	PROFILE_STOP("load_microscope_settings - cube");

	if(microscope->_optical_path != NULL){
		CmtWaitForThreadPoolFunctionCompletion(gci_thread_pool(), op_thread_id, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
		CmtReleaseThreadPoolFunctionID(gci_thread_pool(),  op_thread_id);
	}

	PROFILE_STOP("load_microscope_settings - op");

	return MICROSCOPE_SUCCESS;   
}


int open_microscope_set_shutter_open_time (Microscope* microscope)
{   // set shutter open time to exposure time + 5 ms
	double exposure = gci_camera_get_exposure_time(MICROSCOPE_MASTER_CAMERA(microscope));
	shutter_set_open_time(microscope->_shutter, exposure + 5.0);
	return MICROSCOPE_SUCCESS;  
}


int open_microscope_switch_illumination_mode (Microscope* microscope, MicroscopeIlluminationMode mode,
	MicroscopeIlluminationMode old_mode, int save_old_settings)
{
	char path[GCI_MAX_PATHNAME_LEN] = "";
	int fsize=0, settings_exist = 0, d=0; 
	
	#ifdef BUILD_MODULE_SPC 
    if(microscope->_spc != NULL)
		spc_stop(microscope->_spc);
	#endif    

	if(save_old_settings && old_mode >= 0) {
		
		// Get the filepath of the old illumination settings
		microscope_get_filepath_for_illumination_settings(microscope, old_mode, path); 
	
		// Save the old settings
		microscope_save_settings(microscope, path); 
	}
		
	microscope_get_filepath_for_illumination_settings(microscope, mode, path);         
	
	settings_exist = FileExists (path, &fsize);      
	
	if(mode == MICROSCOPE_FLUORESCENCE || mode == MICROSCOPE_FLUOR_NO_SHUTTER) 
	{
		open_microscope_set_shutter_open_time (microscope);  
		
		if(!settings_exist) {
			if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL && gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope))) {
				gci_camera_set_exposure_time(MICROSCOPE_MASTER_CAMERA(microscope), 200 ); //	200ms 
				gci_camera_set_gain(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_ALL_CHANNELS, 128.0 );		//	half max
			}
		}
	}

	// Dim camera controls for certain modes
	if(mode == MICROSCOPE_LASER_SCANNING)
		d=1;
	else
		d=0;

	if(!settings_exist) {
			microscope_save_settings(microscope, path);
	}
	else {
			microscope_load_settings(microscope, path);
	}
	
	SetCtrlIndex(microscope->_main_ui_panel, MICROSCOPE_MODE, mode);
	
	return MICROSCOPE_SUCCESS;
}	

void open_microscope_start_all_timers (Microscope* microscope)
{
	if (microscope->_stage != NULL) stage_start_timer(microscope->_stage); 
	if (microscope->_objective != NULL) objective_manager_start_timer(microscope->_objective);
	if (microscope->_fluor_cube != NULL) cube_manager_start_timer(microscope->_fluor_cube);
	if (microscope->_optical_path != NULL) optical_path_start_timer(microscope->_optical_path);
	if (microscope->_scanner != NULL) scanner_enable_timer(microscope->_scanner); 	
	if (microscope->_shutter != NULL) shutter_enable_timer(microscope->_shutter); 

//	if (microscope->_laser_power_monitor!= NULL && ui_module_main_panel_is_visible(UIMODULE_CAST(microscope->_laser_power_monitor)))
//		laserpowermonitor_enable_timer(microscope->_laser_power_monitor);
}

void open_microscope_stop_all_timers (Microscope* microscope)
{
	if (microscope->_stage != NULL) stage_stop_timer(microscope->_stage); 
	if (microscope->_objective != NULL) objective_manager_stop_timer(microscope->_objective);
	if (microscope->_fluor_cube != NULL) cube_manager_stop_timer(microscope->_fluor_cube);
	if (microscope->_optical_path != NULL) optical_path_stop_timer(microscope->_optical_path);
	if (microscope->_scanner != NULL) scanner_disable_timer(microscope->_scanner);
	if (microscope->_shutter != NULL) shutter_disable_timer(microscope->_shutter); 

	if (microscope->_laser_power_monitor!= NULL)
		laserpowermonitor_disable_timer(microscope->_laser_power_monitor);
}
	

int open_microscope_destroy(Microscope* microscope)
{
	microscope_stop_all_timers(microscope);

	#ifdef BUILD_MODULE_SPC 
	if (microscope->_spc != NULL) { 
		spc_destroy(microscope->_spc);
		microscope->_spc = NULL;
	}
	#endif

	// Destoy lstep zstage before xy stage as the
	// xy stage disconnects from the controller.
	if (microscope->_z_drive != NULL)  
		z_drive_destroy(microscope->_z_drive);
	
	if (microscope->_stage != NULL)
		stage_destroy(microscope->_stage);
	
	if (microscope->_objective != NULL)
		objective_manager_destroy(microscope->_objective);
	
	if (microscope->_fluor_cube != NULL)
		cube_manager_destroy(microscope->_fluor_cube);
	
	if (microscope->_optical_path != NULL)
		optical_path_destroy(microscope->_optical_path);
	
	if (microscope->_lamp != NULL)
		lamp_destroy(microscope->_lamp);
	
	if (microscope->_scanner != NULL)
		scanner_destroy(microscope->_scanner);
	
	if (microscope->_shutter != NULL)
		shutter_destroy(microscope->_shutter);
	
	if(microscope->_laser_power_monitor != NULL)
		laserpowermonitor_destroy(microscope->_laser_power_monitor);
	if (microscope->_focus != NULL)
		focus_destroy(microscope->_focus);
	
	if (microscope->_cf != NULL)  
		cell_finder_destroy(microscope->_cf);  

  	return MICROSCOPE_SUCCESS;
}
	
	
int CVICALLBACK thread_optical_path_init(void *callback)
{
	Microscope *microscope = (Microscope *) callback;
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	if(optical_path_hardware_initialise (microscope->_optical_path) == OPTICAL_PATH_MANAGER_ERROR)
		return MICROSCOPE_ERROR;

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_optical_path), MICROSCOPE_INIT_STATUS_SUCCESS);
	open_microscope->_opt_path_thread_completed = 1;

	return MICROSCOPE_SUCCESS;

//Error:

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_optical_path), MICROSCOPE_INIT_STATUS_FAILED);
	open_microscope->_opt_path_thread_completed = 1;

	return MICROSCOPE_ERROR;
}


int CVICALLBACK thread_cube_init(void *callback)
{
	Microscope *microscope = (Microscope *) callback;
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	if(cube_manager_hardware_initialise (microscope->_fluor_cube) == CUBE_MANAGER_ERROR)
		goto Error;

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_fluor_cube), MICROSCOPE_INIT_STATUS_SUCCESS);
	open_microscope->_cube_thread_completed = 1;

	return MICROSCOPE_SUCCESS;

Error:

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_fluor_cube), MICROSCOPE_INIT_STATUS_FAILED);
	open_microscope->_cube_thread_completed = 1;

	return MICROSCOPE_ERROR;
}

int CVICALLBACK thread_stage_init(void *callback)
{
    Microscope *microscope = (Microscope *) callback;
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	if(stage_hardware_init(microscope->_stage) == STAGE_ERROR) {

		if(GCI_ConfirmPopup("Critical Error", IDI_ERROR, "Stage cannot be commuicated with.\n"
														 "Do you wish the program to be shutdown ?")) {
			exit(-1);
		}
		else {
			goto Error;
		}
	}	

	if(stage_find_initialise_extents (microscope->_stage, microscope->_perform_full_init) == STAGE_ERROR) {
		goto Error;
	}

	if(microscope->_stage_plate_module != NULL)
		stage_plate_set_stage(microscope->_stage_plate_module, microscope->_stage);

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_stage), MICROSCOPE_INIT_STATUS_SUCCESS);
	
	open_microscope->_stage_thread_completed = 1;

	return MICROSCOPE_SUCCESS;

Error:

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_stage), MICROSCOPE_INIT_STATUS_FAILED);
	open_microscope->_stage_thread_completed = 1;
	return MICROSCOPE_ERROR;
}


int CVICALLBACK thread_powerup_camera(void *callback)
{
	Microscope *microscope = (Microscope *) callback;
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	if(gci_camera_power_up(MICROSCOPE_MASTER_CAMERA(microscope)) == CAMERA_ERROR) {

		if(GCI_ConfirmPopup("Critical Error", IDI_ERROR, "Camera cannot be commuicated with.\n"
														 "Do you wish the program to be shutdown ?")) {
			exit(-1);
		}
		else {
			goto Error;
		}
	}	

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(MICROSCOPE_MASTER_CAMERA(microscope)), MICROSCOPE_INIT_STATUS_SUCCESS);
	open_microscope->_camera_thread_completed = 1;

	return MICROSCOPE_SUCCESS;

	Error:

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(MICROSCOPE_MASTER_CAMERA(microscope)), MICROSCOPE_INIT_STATUS_FAILED);
	open_microscope->_camera_thread_completed = 1;

	return MICROSCOPE_ERROR;
}

int CVICALLBACK thread_powerup_colour_camera(void *callback)
{
	Microscope *microscope = (Microscope *) callback;
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	if(open_microscope->_colour_camera == NULL)
		goto Error;

	if(gci_camera_power_up(open_microscope->_colour_camera) == CAMERA_ERROR) {
		goto Error;
	}

	microscope_add_init_device_status(microscope,
		UIMODULE_GET_DESCRIPTION(open_microscope->_colour_camera), MICROSCOPE_INIT_STATUS_SUCCESS);

	open_microscope->_colour_camera_thread_completed = 1;

	return MICROSCOPE_SUCCESS;

	Error:

	microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(MICROSCOPE_MASTER_CAMERA(microscope)), MICROSCOPE_INIT_STATUS_FAILED);
	open_microscope->_colour_camera_thread_completed = 1;

	return MICROSCOPE_ERROR;
}

// Return a string of devices that failed to init. Separated by \n
static int failed_init_devices(Microscope* microscope, char *buffer)  
{
	char name[100];
	int failed = 0;
	
	memset(buffer, 0, 1);
	
	if(MICROSCOPE_MASTER_CAMERA(microscope) == NULL || !gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope))) {
		failed = 1;
		if(MICROSCOPE_MASTER_CAMERA(microscope)){
			gci_camera_get_name(MICROSCOPE_MASTER_CAMERA(microscope), name);
			gci_camera_destroy(MICROSCOPE_MASTER_CAMERA(microscope));
		}
		else strcpy(name, "Camera");
		sprintf(buffer, "- %s\n", name);
		MICROSCOPE_MASTER_CAMERA(microscope) = NULL;
	}
	
	if(microscope->_stage == NULL || !stage_is_initialised (microscope->_stage)) {
		failed = 1;
		if(microscope->_stage) stage_destroy(microscope->_stage);
		strcat(buffer, "- Stage\n"); 
		microscope->_stage = NULL;
	}
	
	if(microscope->_optical_path == NULL || !optical_path_is_initialised (microscope->_optical_path)) {
		failed = 1;
		if(microscope->_optical_path) optical_path_destroy(microscope->_optical_path);
		strcat(buffer, "- Optical Path\n"); 
		microscope->_optical_path = NULL;
	}
	
	if(microscope->_fluor_cube == NULL || !cube_manager_is_initialised (microscope->_fluor_cube)) {
		failed = 1;
		if(microscope->_fluor_cube) cube_manager_destroy(microscope->_fluor_cube);
		strcat(buffer, "- Fluorescent Cubes\n"); 
		microscope->_fluor_cube = NULL;
	}
	
	if(microscope->_scanner == NULL || !scanner_is_hardware_initialised (microscope->_scanner)) {
		failed = 1;
		strcat(buffer, "- Scanner\n");
		if(microscope->_scanner) scanner_destroy(microscope->_scanner);
		microscope->_scanner = NULL;
	}
	
	if(microscope->_shutter == NULL || !shutter_is_initialised (microscope->_shutter)) {
		failed = 1;
		strcat(buffer, "- Shutter\n");
		if(microscope->_shutter) shutter_destroy(microscope->_shutter);   
		microscope->_shutter = NULL;
	}
	
	if(microscope->_lamp == NULL || !lamp_hardware_is_initialised (microscope->_lamp)) {
		failed = 1;
		strcat(buffer, "- Lamp\n"); 
		if(microscope->_lamp) lamp_destroy(microscope->_lamp);
		microscope->_lamp = NULL;
	}
	
	if(microscope->_z_drive == NULL || !z_drive_is_initialised (microscope->_z_drive)) {
		failed = 1;
		strcat(buffer, "- Z_Drive\n"); 
		if(microscope->_z_drive) z_drive_destroy(microscope->_z_drive);
		microscope->_z_drive = NULL;
	}

	if(microscope->_laser_power_monitor == NULL || !hardware_device_hardware_is_initialised (HARDWARE_DEVICE_CAST(microscope->_laser_power_monitor))) {
		failed = 1;
		strcat(buffer, "- Laser Power Monitor\n"); 
		if(microscope->_laser_power_monitor) laserpowermonitor_destroy(microscope->_laser_power_monitor);
		microscope->_laser_power_monitor = NULL;
	}


	return failed;
}

int open_microscope_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	return default_microscope_error_handler (module, title, error_string, callback_data);  
}

static int have_all_threads_completed(Microscope* microscope)
{
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	if(microscope->_stage == NULL)
		open_microscope->_stage_thread_completed = 1;

	if(open_microscope->_colour_camera == NULL)
		open_microscope->_colour_camera_thread_completed = 1;

	if(open_microscope->_stage_thread_completed == 0 ||
		open_microscope->_camera_thread_completed == 0 ||
		open_microscope->_colour_camera_thread_completed == 0 ||
		open_microscope->_opt_path_thread_completed == 0 ||
		open_microscope->_cube_thread_completed == 0)
		return 0;

	return 1;
}

static int open_microscope_initialise_hardware(Microscope* microscope)
{
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	int  op_thread_id = -1, stage_thread_id = -1;
	int  camera_thread_id = -1, colour_camera_thread_id = -1, cube_thread_id = -1, status = 0;
	char path[GCI_MAX_PATHNAME_LEN];
	double start_time;

    memset(path, 0, 500);
    
	microscope_add_init_device(microscope, UIMODULE_GET_DESCRIPTION(MICROSCOPE_MASTER_CAMERA(microscope)));
	microscope_add_init_device(microscope, UIMODULE_GET_DESCRIPTION(microscope->_stage));
	microscope_add_init_device(microscope, UIMODULE_GET_DESCRIPTION(microscope->_fluor_cube));
	microscope_add_init_device(microscope, UIMODULE_GET_DESCRIPTION(microscope->_optical_path));
	microscope_add_init_device(microscope, UIMODULE_GET_DESCRIPTION(microscope->_shutter));
	microscope_add_init_device(microscope, UIMODULE_GET_DESCRIPTION(microscope->_scanner));
	
	if(microscope->_laser_power_monitor != NULL)
		microscope_add_init_device(microscope, UIMODULE_GET_DESCRIPTION(microscope->_laser_power_monitor));
	ui_module_set_description(UIMODULE_CAST(microscope), "Dummy Microscope");
    ui_module_set_name(UIMODULE_CAST(microscope), "Dummy Microscope");

	// Ask whether stage should do full init. ie find the extents by moving from corner to corner.
	microscope->_perform_full_init = GCI_ConfirmPopup ("Attention", IDI_WARNING, "OK to initialise XY stage?"); 

	// Remove flicker as this panel sits upon another
	ProcessDrawEvents();
	if(microscope->_laser_power_monitor != NULL) {

		hardware_device_hardware_initialise(HARDWARE_DEVICE_CAST(microscope->_laser_power_monitor));
		laserpowermonitor_initialise(microscope->_laser_power_monitor);

		if(hardware_device_hardware_is_initialised(HARDWARE_DEVICE_CAST(microscope->_laser_power_monitor))) {
			microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_laser_power_monitor), MICROSCOPE_INIT_STATUS_SUCCESS);
		}
		else {
			microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_laser_power_monitor), MICROSCOPE_INIT_STATUS_FAILED);
			microscope->_laser_power_monitor = NULL;
		}
	}
	// Redirect errors to microscope error handler.
	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL)
		ui_module_set_error_handler(UIMODULE_CAST(MICROSCOPE_MASTER_CAMERA(microscope)), open_microscope_error_handler, microscope);
	
	if(microscope->_optical_path != NULL)
		ui_module_set_error_handler(UIMODULE_CAST(microscope->_optical_path), open_microscope_error_handler, microscope);  
	
	if(microscope->_fluor_cube != NULL)
		ui_module_set_error_handler(UIMODULE_CAST(microscope->_fluor_cube), open_microscope_error_handler, microscope); 
	
	if(microscope->_shutter != NULL)
		ui_module_set_error_handler(UIMODULE_CAST(microscope->_shutter), open_microscope_error_handler, microscope);  
	
	if(microscope->_scanner != NULL)
		ui_module_set_error_handler(UIMODULE_CAST(microscope->_scanner), open_microscope_error_handler, microscope);  
	
	if(microscope->_stage != NULL)
		ui_module_set_error_handler(UIMODULE_CAST(microscope->_stage), open_microscope_error_handler, microscope); 
	
	// Sets up UI stuff not hardware init.
	if(microscope->_stage != NULL)
		stage_init(microscope->_stage);

	open_microscope->_stage_thread_completed = 0;
	open_microscope->_camera_thread_completed = 0;
	open_microscope->_colour_camera_thread_completed = 0;
	open_microscope->_opt_path_thread_completed = 0;
	open_microscope->_cube_thread_completed = 0;

	#ifdef SINGLE_THREAD_INIT 
	
	// Initialise the devices
	
	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL)
		thread_powerup_camera(microscope);

	if(open_microscope->_colour_camera != NULL)
		thread_powerup_camera(microscope);
	
	if(microscope->_optical_path != NULL)
		thread_optical_path_init(microscope);    

	if(microscope->_fluor_cube != NULL)
		thread_cube_init(microscope); 

	if(microscope->_stage != NULL)
		thread_stage_init(microscope);

	#else 
	
	if(microscope->_stage != NULL) 
		CmtScheduleThreadPoolFunction (gci_thread_pool(), thread_stage_init, microscope, &stage_thread_id); 

	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) 
		CmtScheduleThreadPoolFunction (gci_thread_pool(), thread_powerup_camera, microscope, &camera_thread_id);

	if(open_microscope->_colour_camera != NULL) 
		CmtScheduleThreadPoolFunction (gci_thread_pool(), thread_powerup_colour_camera, microscope, &colour_camera_thread_id);

	if(microscope->_optical_path != NULL)
		CmtScheduleThreadPoolFunction (gci_thread_pool(), thread_optical_path_init, microscope, &op_thread_id);   

	if(microscope->_fluor_cube != NULL)
		CmtScheduleThreadPoolFunction (gci_thread_pool(), thread_cube_init, microscope, &cube_thread_id);  
	
	start_time = Timer();

	while(((Timer() - start_time) < microscope->_init_timeout)) {

		if(have_all_threads_completed(microscope) == 1)
			break;

		ProcessSystemEvents();
		Delay(0.1);
	}
	
    CmtReleaseThreadPoolFunctionID (gci_thread_pool(), stage_thread_id);
	CmtReleaseThreadPoolFunctionID (gci_thread_pool(), camera_thread_id);
	CmtReleaseThreadPoolFunctionID (gci_thread_pool(), op_thread_id);
	CmtReleaseThreadPoolFunctionID (gci_thread_pool(), cube_thread_id);
    
	#endif	
	
	// The hardware initialise is done but we have to call general initialise.
	// Sets up ui etc.

    if(microscope->_filter_set != NULL) {
		filterset_hardware_initialise(microscope->_filter_set); 
	}

	if(microscope->_pmt_set != NULL) {  
		pmtset_hardware_initialise(microscope->_pmt_set);    
	}
    
	if(microscope->_objective != NULL) {
		objective_manager_hardware_initialise(microscope->_objective); 
	}
	
	if(microscope->_shutter != NULL) {
		if(shutter_hardware_initialise (microscope->_shutter) == SHUTTER_SUCCESS)   
			microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_shutter), MICROSCOPE_INIT_STATUS_SUCCESS);
		else
			microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_shutter), MICROSCOPE_INIT_STATUS_FAILED);
	}

	if(microscope->_scanner != NULL) {
		if(scanner_hardware_initialise(microscope->_scanner) == SCANNER_SUCCESS)
			microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_scanner), MICROSCOPE_INIT_STATUS_SUCCESS);
		else
			microscope_add_init_device_status(microscope, UIMODULE_GET_DESCRIPTION(microscope->_scanner), MICROSCOPE_INIT_STATUS_FAILED);
	}
	
	if(microscope->_lamp != NULL)
		lamp_hardware_initialise(microscope->_lamp);
	
	if(microscope->_z_drive != NULL) {
		z_drive_hardware_initialise (microscope->_z_drive);
	}   
			
	return MICROSCOPE_SUCCESS;
}


static int open_microscope_set_focusing_mode (Microscope* microscope)
{
	double exposure;
	BinningMode binning = NO_BINNING;
	MicroscopeIlluminationMode mode;

	//long exposure makes focussing difficult
	
	binning = gci_camera_get_binning_mode(MICROSCOPE_MASTER_CAMERA(microscope));
	
	if (binning > NO_BINNING)
		return 0;
	
	exposure = gci_camera_get_exposure_time(MICROSCOPE_MASTER_CAMERA(microscope));
	
	if (exposure <= 100) 	// Short exposure don't need to change anything
		return 0;

	mode = microscope_get_illumination_mode(microscope);

	//if( mode == MICROSCOPE_FLUORESCENCE)
	//	microscope_switch_illumination_mode (microscope, MICROSCOPE_FLUOR_NO_SHUTTER, 1);

	if ( gci_camera_is_live_mode(MICROSCOPE_MASTER_CAMERA(microscope)) )
		gci_camera_set_snap_mode(MICROSCOPE_MASTER_CAMERA(microscope));

	exposure = exposure / pow( 2.0 / binning, 2.0);
	
	if (exposure < 1)
		return -1;
	
	gci_camera_set_exposure_time(MICROSCOPE_MASTER_CAMERA(microscope), exposure);
	gci_camera_set_binning_mode(MICROSCOPE_MASTER_CAMERA(microscope), BINNING2X2);

	//if( mode == MICROSCOPE_FLUORESCENCE)   
	//	microscope_switch_illumination_mode (microscope, MICROSCOPE_FLUORESCENCE, 1);  

	return 0;	
}
	
static int open_microscope_set_hi_resolution_mode (Microscope* microscope)
{
	double exposure;
	BinningMode binning = NO_BINNING;

	binning = gci_camera_get_binning_mode(MICROSCOPE_MASTER_CAMERA(microscope));
	exposure = gci_camera_get_exposure_time(MICROSCOPE_MASTER_CAMERA(microscope));
	
	if (binning == NO_BINNING)
		return 0;
	
	//mode = microscope_get_illumination_mode(microscope);     
	
	//if( mode == MICROSCOPE_FLUORESCENCE)
	//	microscope_switch_illumination_mode (microscope, MICROSCOPE_FLUOR_NO_SHUTTER, 1);

	// Adjust exposure to keep intensity constant
	exposure =  exposure * pow(binning, 2.0);
	
	gci_camera_set_exposure_time(MICROSCOPE_MASTER_CAMERA(microscope), exposure);
	
	// Put camera into non binned mode for best resolution 
	gci_camera_set_binning_mode(MICROSCOPE_MASTER_CAMERA(microscope), NO_BINNING);

	//if( mode == MICROSCOPE_FLUORESCENCE)   
	//	microscope_switch_illumination_mode (microscope, MICROSCOPE_FLUORESCENCE, 1);  
	
	return 0;	
}


// SIGNAL: CameraExposureChanged
static int open_microscope_on_camera_exposure_changed (Microscope* microscope)
{
	int live = 0;
	double exposure = 0.0;
	
	exposure = gci_camera_get_exposure_time(MICROSCOPE_MASTER_CAMERA(microscope));
	
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_EXPOSURE, exposure);
	
	open_microscope_set_shutter_open_time (microscope);
	
	live = gci_camera_is_live_mode(MICROSCOPE_MASTER_CAMERA(microscope));      

	//if (!live)
	//	gci_camera_set_trigger_mode(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_EXTERNAL_TRIG);
	
	return 0;		
}

// SIGNAL: CameraGainChanged
static int open_microscope_on_camera_gain_changed (Microscope* microscope)
{
	double gain = 0.0;
	
	gci_camera_get_gain(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_CHANNEL1, &gain);
	
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_GAIN, gain);  
	
	return 0;
}

// SIGNAL: BinningModeChanged
static int open_microscope_on_camera_binning_changed (Microscope* microscope)
{
	BinningMode mode;
	
	mode = gci_camera_get_binning_mode(MICROSCOPE_MASTER_CAMERA(microscope));
	
	SetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_BINNING, mode);  
	
	return 0;
}

// This function is called the instance before the camera grabs an image (starts integrating)
// SIGNAL: PreCapture
static int open_microscope_on_camera_pre_capture (Microscope* microscope)
{
	return 0;		
}

// This function is called when the external trigger needs to be activated
// SIGNAL: TriggerNow
static int open_microscope_on_camera_trigger_now (Microscope* microscope)
{
	//MicroscopeIlluminationMode mode = microscope_get_illumination_mode(microscope);    
	
	// We don't need to do anything when live. 	
	if(gci_camera_is_live_mode(MICROSCOPE_MASTER_CAMERA(microscope)))
		return 0;		
	
	// Ok here we trigger the shutter
	shutter_open(microscope->_shutter);
	
	if(microscope->_laser_power_monitor != NULL)
		laserpowermonitor_get_and_display (microscope->_laser_power_monitor);

	return 0;		
}

// This method is called when after the camera has integrated and we have turned the data into an image   
// SIGNAL: PostCapture
static int open_microscope_on_camera_post_capture (Microscope* microscope)
{
	return 0;
}


// This is called when someone or the code has entered snap mode. 
// SIGNAL: EnterSnapMode, EnterSnapSequenceMode
static int open_microscope_enter_snap_sequence_or_snapmode (Microscope* microscope)
{
	//MicroscopeIlluminationMode mode = microscope_get_illumination_mode(microscope);    
	
	// We don't need to do anything when not in fluorescence mode.
//	if( mode != MICROSCOPE_FLUORESCENCE)
//		return 0;	
	
	// Ok If the shutter is open perhaps something as gone wrong previously
	// or we were in live mode (? PB)
	// This shouldn't be neccessary but we we close the shutter
//	printf("Enter Snap or Sequence Mode We Are Closing Shutter As Ros Does This To Be Safe ?\n");
	shutter_close(microscope->_shutter);
	open_microscope_set_shutter_open_time (microscope);
	
	//gci_camera_set_trigger_mode(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_EXTERNAL_TRIG);       
	//gci_camera_set_trigger_mode(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_INTERNAL_TRIG);       
	
	return 0;  
}

// This is called when someone or the code has entered live mode.
// SIGNAL: EnterLiveMode
static int open_microscope_on_camera_live_enter (Microscope* microscope)      
{
	int mode = microscope_get_illumination_mode(microscope);

	SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LIVE, ATTR_DIMMED, 1); 
	
	if(microscope->_laser_power_monitor != NULL)
		laserpowermonitor_enable_timer(microscope->_laser_power_monitor);

	gci_camera_set_trigger_mode(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_NO_TRIG);

	if( mode == MICROSCOPE_FLUORESCENCE) { 

		if(microscope->_shutter != NULL){
			shutter_set_open_time(microscope->_shutter, 0);   // open permenantly
			shutter_open(microscope->_shutter);
		}
	}
	
	return 0;   
}

// This is called when someone or the code has exits live mode.
// SIGNAL: ExitLiveMode
static int open_microscope_on_camera_live_exit (Microscope* microscope)      
{
	int mode = microscope_get_illumination_mode(microscope);
	
	if(microscope->_laser_power_monitor != NULL)
		laserpowermonitor_disable_timer(microscope->_laser_power_monitor);

	SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LIVE, ATTR_DIMMED, 0); 
	
	if( mode == MICROSCOPE_FLUORESCENCE) {
//		printf("Exit Live Closing Shutter\n");
		if(microscope->_shutter != NULL)
			shutter_close(microscope->_shutter); 
	}

	return 0;   
}

static int om_on_camera_exit_get_image  (Microscope* microscope, FIBITMAP **dib)      
{
	if(!ref_images_should_process(microscope->_ri))
		return MICROSCOPE_SUCCESS;
	
	if(!gci_camera_is_live_mode(MICROSCOPE_MASTER_CAMERA(microscope)))
		logger_log(UIMODULE_LOGGER(microscope), LOGGER_INFORMATIONAL, "Performing background correction");  
	
	if(!microscope_is_automatic_background_correction_disabled(microscope)) {
		if(ref_images_in_place_process(microscope->_ri, dib) > 0)
			ref_images_disable(microscope->_ri); 
	}
		
	return MICROSCOPE_SUCCESS;          
}

static int microscope_get_detection_device_count (Microscope* microscope)
{
	// For the dummy scope we have two dummy camera and the spc module
	return 3;
}

static int microscope_get_detection_device_names (Microscope* microscope, const char** path)
{
	path[0] = "Camera";
	path[1] = "Colour camera";
	path[2] = "Laser scanning device";

	return MICROSCOPE_SUCCESS;   
}

static int open_microscope_post_initialise (Microscope* microscope)
{
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

//#define DEBUG_CAMERA
//#define DEBUG_REGIONSCAN

#ifdef DEBUG_CAMERA
	//gci_camera_display_main_ui(open_microscope->_original_master_camera);
	//gci_camera_display_extra_ui(open_microscope->_original_master_camera);
//	gci_dummy_set_display_file_path(open_microscope->_original_master_camera, "c:\\Microscope Data\\paul\\Test_nofinding1.ics");
	gci_dummy_set_display_file_path(open_microscope->_original_master_camera, "C:\\Microscope Data\\paul\\CHARM test\\twocells 81.ics");
	gci_camera_snap_image(open_microscope->_original_master_camera);
#endif

#ifdef DEBUG_REGIONSCAN
	region_scan_display(microscope->_rs);
#endif

	return MICROSCOPE_SUCCESS;          
}

Microscope* microscope_new(HINSTANCE hInstance)
{
	//Create Dummy microscope instance
	char path[500];

	Microscope *microscope = (Microscope*) malloc(sizeof(OpenMicroscope));
	OpenMicroscope *open_microscope = (OpenMicroscope*) microscope;

	memset(open_microscope, 0, sizeof(OpenMicroscope));

	microscope_constructor(microscope, "Dummy microscope", "Dummy Microscope");
	
	microscope_set_error_handler(microscope, open_microscope_error_handler);
	
	MICROSCOPE_VTABLE_PTR(microscope, destroy) = open_microscope_destroy; 
	MICROSCOPE_VTABLE_PTR(microscope, initialise_hardware) = open_microscope_initialise_hardware;    
	
	MICROSCOPE_VTABLE_PTR(microscope, initialise_hardware_user_interfaces) = open_microscope_initialise_hardware_user_interfaces;
	MICROSCOPE_VTABLE_PTR(microscope, microscope_post_initialise) = open_microscope_post_initialise;
	
	MICROSCOPE_VTABLE_PTR(microscope, microscope_get_detection_device_count) = microscope_get_detection_device_count; 
	MICROSCOPE_VTABLE_PTR(microscope, microscope_get_detection_device_names) = microscope_get_detection_device_names; 

	MICROSCOPE_VTABLE_PTR(microscope, set_focusing_mode) = open_microscope_set_focusing_mode; 
	MICROSCOPE_VTABLE_PTR(microscope, set_hi_resolution_mode) = open_microscope_set_hi_resolution_mode; 
	MICROSCOPE_VTABLE_PTR(microscope, microscope_save_settings) = om_save_microscope_settings;
	MICROSCOPE_VTABLE_PTR(microscope, microscope_load_settings) = om_load_microscope_settings;
	MICROSCOPE_VTABLE_PTR(microscope, switch_illumination_mode) = open_microscope_switch_illumination_mode;  
	
	MICROSCOPE_VTABLE_PTR(microscope, start_all_timers) = open_microscope_start_all_timers;  
	MICROSCOPE_VTABLE_PTR(microscope, stop_all_timers) = open_microscope_stop_all_timers;  
	MICROSCOPE_VTABLE_PTR(microscope, disable_all_panels) = open_microscope_disable_all_panels;     
	
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_exposure_changed) = open_microscope_on_camera_exposure_changed;  
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_gain_changed) = open_microscope_on_camera_gain_changed;  
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_binning_changed) = open_microscope_on_camera_binning_changed;
	
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_pre_capture) = open_microscope_on_camera_pre_capture;
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_post_capture) = open_microscope_on_camera_post_capture;  
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_trigger_now) = open_microscope_on_camera_trigger_now;
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_enter_snap_sequence_or_snapmode) = open_microscope_enter_snap_sequence_or_snapmode;
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_live_enter) = open_microscope_on_camera_live_enter; 
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_live_exit) = open_microscope_on_camera_live_exit; 
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_exit_get_image) = om_on_camera_exit_get_image;  		

	MICROSCOPE_MASTER_CAMERA(microscope) = open_microscope->_original_master_camera =
		gci_dummy_camera_new("DummyCamera", "Dummy Camera");

	microscope_add_camera(microscope, MICROSCOPE_MASTER_CAMERA(microscope));

	open_microscope->_colour_camera = gci_dummy_camera_new("DummyColourCamera", "Dummy Colour Camera");
	sprintf(path, "%s\\ColourCameraImages\\fedora.jpg", microscope->_data_dir);
	gci_dummy_set_display_file_path((DummyCamera*) open_microscope->_colour_camera, path);
	microscope_add_camera(microscope, open_microscope->_colour_camera);
	
	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) {
		gci_dummy_set_dummy_file_directory((DummyCamera*)MICROSCOPE_MASTER_CAMERA(microscope), microscope->_data_dir);	
	}

	microscope->_fluor_cube = Manual_fluo_cube_manager_new("ATD_CubeSlider_Dummy_1", "FL Cube",  microscope->_data_dir, "CubeData1.ini", open_microscope_error_handler, microscope);
	
	microscope->_objective = Dummy_objective_manager_new("ATD_Objective_Dummy_1", "Objective Lens", microscope->_data_dir, "ObjectiveData1.ini");
	
	microscope->_optical_path = manual_optical_path_new("ATD_OpticalPath_Dummy_1", "Optical Path", microscope->_data_dir, "OpticalPathData1.ini");
	
	microscope->_filter_set = manual_filterset_new("FilterSet_1", "Filter Set", microscope->_data_dir, "FilterSet_1.ini", open_microscope_error_handler, microscope);

	microscope->_pmt_set = manual_pmtset_new("PMT_1", "PMT", microscope->_data_dir, "PMT_1.ini");

	microscope->_shutter = manual_shutter_new("ATD_Shutter_Dummy_1", "Manual Shutter", open_microscope_error_handler);

	microscope->_lamp    = manual_lamp_new("ATD_LedLamp_Dummy_1", "Manual Lamp", open_microscope_error_handler, microscope->_data_dir);	
	
	microscope->_z_drive = manual_zdrive_new("ATD_ZDrive_Dummy_1", "Manual_Z_Drive", open_microscope_error_handler, microscope->_data_dir);
	
	microscope->_scanner = manual_scanner_new("ATD_Scanner_Dummy_1", "Manual Scanner", microscope->_data_dir, "ScannerSettings1.ini", open_microscope_error_handler, microscope->_data_dir);

	microscope->_stage_plate_module = stage_plate_new("StagePlate", "Stage Plate", microscope->_data_dir, "StagePlateData.ini");

	microscope->_stage = stage_dummy_new("DummyStage", "Dummy Stage", open_microscope_error_handler, microscope, microscope->_data_dir);

	#ifdef BUILD_MODULE_SPC 
	microscope->_spc = spc_new(microscope, "SPC", "Time Resolve Acquisition", open_microscope_error_handler, microscope->_data_dir); 
	#else
	microscope->_spc = NULL;
	#endif
	
	// make sure this is initialised
	microscope->_master_z_drive = microscope->_z_drive;
	
	microscope->_laser = om_laser_new("DummyLaser", "Dummy Laser", open_microscope_error_handler, microscope->_data_dir);

	return microscope;
}
