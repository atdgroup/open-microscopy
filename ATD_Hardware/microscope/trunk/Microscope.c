#include "HardWareTypes.h" 
#include "HardwareDevice.h"

#include "microscope.h"
#include "MicroscopeUI.h"

#include "stage\stage.h"
#include "StagePlate.h"
#include "icsviewer_window.h"
#include "icsviewer_signals.h"    
#include "ObjectivesUI.h"
#include "CubeSlider.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "gci_menu_utils.h"
#include "lamp.h"
#include "Shutter.h"
#include "WellPlateDefiner.h"
#include <cvintwrk.h>

#define MS_BASE_MSG (WM_USER+60) 
#define LOAD_CONFIG (MS_BASE_MSG+1) 

#ifdef BUILD_MODULE_LASER_POWER_MONITOR
#include "LaserPowerMonitor.h"
#endif

#ifdef BUILD_MODULE_PMT
#include "PmtSet.h"
#endif

#ifdef BUILD_MODULE_FILTER
#include "FilterSet.h"
#endif

#ifdef BUILD_MODULE_LASER
#include "laser.h"
#endif

#ifdef BUILD_MODULE_SINGLE_RANGE_HW_DEVICE
#include "single_range_hardware_device.h"
#endif

#ifdef BUILD_MODULE_CONDENSER
#include "condensers.h"
#endif

#ifdef BUILD_MODULE_ANALYZER
#include "analyzer.h"
#endif

#ifdef BUILD_MODULE_SPC
#include "spc.h"
#endif

#ifdef BUILD_ROBOT_INTERFACE
#include "RobotInterface.h"
#endif

#include "libics.h"
#include "FreeImageIcs_MetaData.h"
#include "FreeImageAlgorithms_IO.h"

#include "ATD_UsbInterface_A.h"  
#include "PowerSwitch.h"

#include "ThreadDebug.h"

#include <utility.h>  
#include <rs232.h> 

#include "GL_CVIRegistry.h"
#include "toolbox.h"

#include "asynctmr.h"

#include <ansi_c.h> 

#define DONT_PROFILE
#include "profile.h"

#ifdef LOCATIONS_DEFINED   
#include "Config.h"
#endif

#ifdef USE_WEB_SERVER 
#include "mongoose.h"
#endif

static Microscope *gmicroscope = NULL;

static void OnMicroscopeChanged(Microscope* microscope);

typedef struct _FileSaveDetail
{
	FIBITMAP *fib;
	char path[GCI_MAX_PATHNAME_LEN];
	dictionary* metadata;

} FileSaveDetail;

void microscope_focus_username_panel(Microscope *microscope)
{
	if(microscope->_username_splash_panel > 0) {
		// Re focus username entry panel
		SetActivePanel (microscope->_username_splash_panel);
	}
}

static void log_start_of_microscope(Microscope* ms)
{
	char date[64];
	english_date(date);
	logger_log(UIMODULE_LOGGER(ms), LOGGER_INFORMATIONAL, "%s started by %s on %s",
		UIMODULE_GET_DESCRIPTION(ms),  ms->_user_name, date);
}

static int CVICALLBACK OnUsernameSplashPanelOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char name[500], study[500], experiment[500];

	switch (event)
	{
		case EVENT_COMMIT:
		{
			Microscope* ms = (Microscope*) callbackData;   
			char path[GCI_MAX_PATHNAME_LEN] = "";
			char buffer[GCI_MAX_PATHNAME_LEN] = "";
			
			GetCtrlVal(ms->_username_splash_panel, USERNAME_NAME, name);
			GetCtrlVal(ms->_username_splash_panel, USERNAME_STUDY, study);
			GetCtrlVal(ms->_username_splash_panel, USERNAME_EXPERIMENT, experiment);
			
			// Don't remove default system username if the user
			// neglected to enter anything
			if(strcmp(name, "") != 0) {
				strcpy(ms->_user_name, name);
			}

			strcpy(ms->_study, study);
			strcpy(ms->_experiment, experiment);

			ui_module_destroy_panel(UIMODULE_CAST(ms), ms->_username_splash_panel);
			ms->_username_splash_panel = 0;

			if(strcmp(name, "") != 0) {
				get_device_string_param_from_ini_file("Microscope", "Microscope User Data", path);
				sprintf(buffer, "%s\\%s", path, ms->_user_name);

				microscope_set_user_data_directory(ms, buffer);  		
			}

			microscope_get_user_data_directory(ms, path);
			GCI_ImagingWindow_SetDefaultDirectoryPathForAllWindows(path);

			log_start_of_microscope(ms);

			break;
		}
	}
	
	return 0; 
}

static int CVICALLBACK OnManualDevicesSplashPanelOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Microscope* ms = (Microscope*) callbackData;   
			ui_module_destroy_panel(UIMODULE_CAST(ms), ms->_manual_devices_splash_panel);
			ms->_manual_devices_splash_panel = 0;
			break;
		}
	}
	
	return 0; 
}

int OnSplashPanelEvent(int panel, int event, void *callbackData,
                                              int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LOST_FOCUS:
		{
			SetActivePanel(panel);
			return 1;
		}
	}

	return 0;
}

static int CVICALLBACK OnStudyGenLink (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			char message[512];
			Microscope* ms = (Microscope*) callbackData;
			InstallPanelCallback (ms->_username_splash_panel, NULL, ms);  //  reomve splash panel callback to allow browser to the front.
			sprintf(message,"http://imcore1.rob.ox.ac.uk/DataNameGenerator/GeneralDataNameForm.php");
			InetLaunchDefaultWebBrowser (message);
			break;
		}
	}

	return 0;
}

void usernameSplashPanel(Microscope* ms)
{
	char config_fullpath[GCI_MAX_PATHNAME_LEN];
	char bypass[50] = "0", study_gen[50] = "0";
	
	if(find_resource("config.ini", config_fullpath) < 0)
		return;      
        							
    GetPrivateProfileString("Microscope", "BypassUsername", "0", bypass, 50, config_fullpath);
	
	if (strcmp(bypass, "1")==0){
		log_start_of_microscope(ms);   // with default windows username
		return;
	}

	ms->_username_splash_panel = ui_module_add_panel(UIMODULE_CAST(ms), "MicroscopeUI.uir", USERNAME, 0);  
	InstallCtrlCallback (ms->_username_splash_panel, USERNAME_OK, OnUsernameSplashPanelOk, ms);
	InstallPanelCallback (ms->_username_splash_panel, OnSplashPanelEvent, ms);

	GetPrivateProfileString("Microscope", "EnableStudyGeneratorLink", "0", study_gen, 50, config_fullpath);
	
	if (strcmp(study_gen, "1")==0){
		SetCtrlAttribute(ms->_username_splash_panel, USERNAME_STUDYGENTEXT, ATTR_VISIBLE, 1);
		SetCtrlAttribute(ms->_username_splash_panel, USERNAME_STUDYGENLINK, ATTR_VISIBLE, 1);
		InstallCtrlCallback (ms->_username_splash_panel, USERNAME_STUDYGENLINK, OnStudyGenLink, ms);
	}

	ui_module_display_panel(UIMODULE_CAST(ms), ms->_username_splash_panel);
}

int CVICALLBACK OnManualDeviceTableEvent (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    Point focus;
	ListType *hardware_device_list;
	HardwareDevice *device = NULL; 
    
    switch (event)
    {
        case EVENT_COMMIT:
            
            GetActiveTableCell(panel, control, &focus);
            
			if (focus.x == 3) { // 3rd col contains the buttons
                //MessagePopup("to do", "show device planel now");// Show panel for device focus.y
				hardware_device_list = hardware_device_get_device_list();
				device = *((HardwareDevice **) (ListGetPtrToItem (*hardware_device_list, focus.y)));
				ui_module_display_main_panel(UIMODULE_CAST(device));
			}

            break;
    }
    return 0;
}

void setupManualDevices (Microscope* ms)
// This function Loads the Saved State and shows the SplashPanel for manual devices
{
	int i, j, number_of_devices, file_size;
	char config_fullpath[GCI_MAX_PATHNAME_LEN];
	char data_dir[GCI_MAX_PATHNAME_LEN];
	char state_filepath[GCI_MAX_PATHNAME_LEN];
	char bypass[50] = "", info[UIMODULE_NAME_LEN] = "";
	char description[500] = "";
	ListType *hardware_device_list;
	HardwareDevice *device = NULL; 
	
	if(find_resource("config.ini", config_fullpath) < 0)
		return;      
        							
    GetPrivateProfileString("Microscope", "BypassManualDevices", "0", bypass, 50, config_fullpath);
	
	if (strcmp(bypass, "1")==0){
		return;
	}

	ms->_manual_devices_splash_panel = ui_module_add_panel(UIMODULE_CAST(ms), "MicroscopeUI.uir", DEVICES, 0);  
	InstallCtrlCallback (ms->_manual_devices_splash_panel, DEVICES_OK, OnManualDevicesSplashPanelOk, ms);
	InstallCtrlCallback (ms->_manual_devices_splash_panel, DEVICES_TABLE, OnManualDeviceTableEvent, ms);
//	InstallPanelCallback (ms->_manual_devices_splash_panel, OnSplashPanelEvent, ms);

//	hardware_device_print_all_devices();

	// Fill the table with manual hardware devices
	hardware_device_list = hardware_device_get_device_list();

	number_of_devices = ListNumItems (*hardware_device_list);

	microscope_get_data_directory(ms, data_dir);
	sprintf(state_filepath, "%s\\manual_device_state.ini", data_dir);

	for(i=1,j=1; i <= number_of_devices; i++) { 

		device = hardware_device_at_index(i);

		if(hardware_device_is_manual(device)) {
			if(FileExists(state_filepath, &file_size)) 
				hardware_load_state_from_file (device, state_filepath);
		}
	
		strcpy(description, UIMODULE_GET_DESCRIPTION(device));
		InsertTableRows(ms->_manual_devices_splash_panel, DEVICES_TABLE, -1, 1, VAL_CELL_STRING);
		SetTableCellVal(ms->_manual_devices_splash_panel, DEVICES_TABLE, MakePoint(1,j), description);

		if (hardware_device_is_manual(device)){
			info[0]=0; //reset the string
			hardware_device_get_current_value_text(device, info);
			SetTableCellVal(ms->_manual_devices_splash_panel, DEVICES_TABLE, MakePoint(2,j), info);
			SetTableCellAttribute(ms->_manual_devices_splash_panel, DEVICES_TABLE, MakePoint(3,j), ATTR_CELL_TYPE, VAL_CELL_BUTTON);
			SetTableCellVal(ms->_manual_devices_splash_panel, DEVICES_TABLE, MakePoint(3,j), "Open Panel");
		}
		else {
			SetTableCellRangeAttribute(ms->_manual_devices_splash_panel, DEVICES_TABLE, MakeRect(j, 1, 1, 3), ATTR_CELL_DIMMED, 1);
			SetTableCellVal(ms->_manual_devices_splash_panel, DEVICES_TABLE, MakePoint(2,j), "Computer Controlled");
		}
		j++;
	} 

	ui_module_display_panel(UIMODULE_CAST(ms), ms->_manual_devices_splash_panel);
}

int microscope_add_menuitem(Microscope *microscope, char *path, 
	int shortCutKey, MenuCallbackPtr eventFunction, void *callback_data)
{
	int menu_bar_id, menu_item_id;
	
	menu_bar_id = GetPanelMenuBar (microscope->_main_ui_panel);
	
	menu_item_id = CreateMenuItemPath(menu_bar_id, path, -1, shortCutKey, eventFunction, callback_data);    
	
	ListInsertItem(microscope->_menu_item_list, &menu_item_id, END_OF_LIST);      
	
	return menu_item_id;
}

void microscope_dim_menuitems(Microscope *microscope, int dim)
{
	int menu_bar_id, *menu_item_id, i, number_of_items;
	
	menu_bar_id = GetPanelMenuBar (microscope->_main_ui_panel);
	
	number_of_items = ListNumItems (microscope->_menu_item_list);

	for(i=1; i <= number_of_items; i++) { 

		menu_item_id = ListGetPtrToItem (microscope->_menu_item_list, i);
		SetMenuBarAttribute(menu_bar_id, *menu_item_id, ATTR_DIMMED, dim); 
	}
}


void microscope_dim_menubar(Microscope *microscope, int dim)
{
	int menu_bar_id = GetPanelMenuBar (microscope->_main_ui_panel);

	SetMenuBarAttribute(menu_bar_id, 0, ATTR_DIMMED, dim); 
}

static void AddKeyValueToTree(int panel, int ctrl, const char *key, const char *value)
{
	int index = InsertTreeItem (panel, ctrl, VAL_SIBLING, 0, VAL_LAST, key, 0, 0, 0);

	SetTreeCellAttribute (panel, ctrl, index, 1, ATTR_LABEL_TEXT, key);
	SetTreeCellAttribute (panel, ctrl, index, 2, ATTR_LABEL_TEXT, value);
}

void microscope_add_microscope_metadata(Microscope* ms, dictionary *d)
{
	char buffer1[UIMODULE_NAME_LEN] = "", buffer2[500] = "";
	char config_fullpath[GCI_MAX_PATHNAME_LEN] = "";

	// Microscope Name
	ui_module_get_name(UIMODULE_CAST(ms), buffer1);
	dictionary_set(d, "microscope name", buffer1);

	dictionary_set(d, "metadata format ver", "1.0");  

	if(find_resource("config.ini", config_fullpath) == 0)
		iniparser_load_section_into_dictionary(d, config_fullpath, "MetaData");

	dictionary_set(d, "experimenter", ms->_user_name);  
	dictionary_set(d, "study", ms->_study);  
	dictionary_set(d, "experiment", ms->_experiment);  
	
	// Data
	english_date(buffer1);
    sprintf(buffer2, "%s %s", TimeStr(), buffer1);
	dictionary_set(d, "creation date", buffer2);  

	return;
}
void microscope_add_hardware_metadata(Microscope* ms, dictionary *d)
{
	char buffer1[UIMODULE_NAME_LEN] = "", buffer2[500] = "";
	char config_fullpath[GCI_MAX_PATHNAME_LEN] = "";
	int tmp, mode;
	double dtmp, dtmp2;
	Objective objective;      
	FluoCube cube;  
	StagePlate plate;

	PROFILE_START("microscope_add_hardware_metadata");

	// Mode
	mode = microscope_get_illumination_mode(ms);
	
	if (mode == MICROSCOPE_BRIGHT_FIELD) {
		dictionary_set(d, "illumination mode", "Brightfield");  
	}
	else if ((mode == MICROSCOPE_FLUORESCENCE) || (mode == MICROSCOPE_FLUOR_NO_SHUTTER)) {
		dictionary_set(d, "illumination mode", "Fluorescence");    
	}
	else if (mode == MICROSCOPE_PHASE_CONTRAST) {
		dictionary_set(d, "illumination mode", "PhaseContrast");   
	}
	else if (mode == MICROSCOPE_LASER_SCANNING) {
		dictionary_set(d, "illumination mode", "Laser Scanning");   
	}

	// Stage Type and Position
	if(ms->_stage != NULL) {

		PROFILE_START("microscope_add_hardware_metadata - stage");

		if(find_resource("config.ini", config_fullpath) == 0)
			iniparser_load_section_into_dictionary(d, config_fullpath, "Stage_Metadata");

		stage_get_info(ms->_stage, buffer1);
		dictionary_set(d, "Stage Firmware", buffer1); 

		if(stage_get_xy_position(ms->_stage, &dtmp, &dtmp2) == STAGE_SUCCESS) {
			sprintf(buffer1, "%.2f %.2f", dtmp, dtmp2); 	
			dictionary_set(d, "stage pos", buffer1);  
		}

		dictionary_setdouble(d, "Stage PositionX", dtmp);  
		dictionary_setdouble(d, "Stage PositionY", dtmp2);  

		PROFILE_STOP("microscope_add_hardware_metadata - stage");
	}

	if(ms->_stage_plate_module != NULL) {
		stage_plate_get_current_plate(ms->_stage_plate_module, &plate);
		dictionary_set(d, "Stage Plate", plate.name);  
	}

	if(ms->_master_z_drive != NULL) {

		PROFILE_START("microscope_add_hardware_metadata - zdrive");

		if(z_drive_get_position(ms->_master_z_drive, &dtmp) ==  Z_DRIVE_SUCCESS) {
			dictionary_setdouble(d, "Stage PositionZ", dtmp);
		}

		PROFILE_STOP("microscope_add_hardware_metadata - zdrive");
	}
	
	// Objectives
	objective_manager_get_current_objective(ms->_objective, &objective);
	dictionary_set(d, "objective", objective._objective_name); 
	dictionary_set(d, "objective mag", objective._magnification_str); 
	dictionary_set(d, "objective NA", objective._numerical_aperture); 

	// Cubes
	if(ms->_fluor_cube != NULL) {
		
		PROFILE_START("microscope_add_hardware_metadata - cubes");

		cube_manager_get_current_cube(ms->_fluor_cube, &cube);
		
		dictionary_set(d, "cube", cube.name);  
		
		sprintf(buffer1, "min %d max %d", cube.exc_min_nm, cube.exc_max_nm);
		dictionary_set(d, "cube exc nm",buffer1);      
		
		sprintf(buffer1, "min %d max %d", cube.emm_min_nm, cube.emm_max_nm);
		dictionary_set(d, "cube emm nm",buffer1);    

		PROFILE_STOP("microscope_add_hardware_metadata - cubes");
	}

    #ifdef BUILD_MODULE_CONDENSER
    // Condensers
	if(ms->_condenser != NULL) {
		
		Condenser condenser;
		
		condenser_manager_get_current_condenser(ms->_condenser, &condenser);
		
		dictionary_set(d, "condenser", condenser.name);        
	}
	#endif 
	
	#ifdef BUILD_MODULE_SINGLE_RANGE_HW_DEVICE
	
    if(ms->_optical_zoom != NULL) {
    
        single_range_hardware_device_get( SINGLE_RANGE_HW_DEVICE_CAST(ms->_optical_zoom), &dtmp);
        dictionary_setdouble(d, "optical zoom", dtmp);  
    }
   
    if(ms->_field_stop != NULL) {
    
        single_range_hardware_device_get( SINGLE_RANGE_HW_DEVICE_CAST(ms->_field_stop), &dtmp);
        dictionary_setdouble(d, "field stop", dtmp);  
    }
    
    if(ms->_epi_field_stop != NULL) {
    
        single_range_hardware_device_get( SINGLE_RANGE_HW_DEVICE_CAST(ms->_epi_field_stop), &dtmp);
        dictionary_setdouble(d, "epifield stop", dtmp);  
    }
    
    if(ms->_aperture_stop != NULL) {
    
        single_range_hardware_device_get( SINGLE_RANGE_HW_DEVICE_CAST(ms->_aperture_stop), &dtmp);
        dictionary_setdouble(d, "aperture stop", dtmp);  
    }
    
    #endif 
    
    #ifdef BUILD_MODULE_ANALYZER
    
    if(ms->_analyzer != NULL) {
    
        int analyzer_status_val;
        
        analyzer_status(ms->_analyzer, &analyzer_status_val);
        dictionary_setdouble(d, "analyser", analyzer_status_val);  
    }
    
    #endif 

	#ifdef BUILD_ROBOT_INTERFACE
    if(ms->_robot_interface != NULL) {
    
        double x, y;
        
		robot_interface_get_pickup_point(ms->_robot_interface, &x, &y);

		dictionary_setdouble(d, "robot pu pt x", x);  
		dictionary_setdouble(d, "robot pu pt y", y);  
    }
	#endif
    
	if (mode == MICROSCOPE_LASER_SCANNING) {
		
		#ifdef BUILD_MODULE_PMT
		// PMT
		if(ms->_pmt_set != NULL) {
			pmtset_get_current_position(ms->_pmt_set, &tmp);
			if (tmp>=0) {
				pmtset_get_device_name_for_pos(ms->_pmt_set, buffer1, tmp);
				dictionary_set(d, "PMT", buffer1); 
			}
		}
		#endif

		#ifdef BUILD_MODULE_FILTER

		// Filter Set
		if(ms->_filter_set != NULL) {
			
			FilterSet filterset;

			filterset_get_current_filterset(ms->_filter_set, &filterset);
			
			dictionary_set(d, "filterset", filterset.name);  
			
			dictionary_set(d, "filterset exc name", filterset.exc_name);  
			sprintf(buffer1, "min %d max %d", filterset.exc_min_nm, filterset.exc_max_nm);
			dictionary_set(d, "filterset exc nm", buffer1);      
			
			dictionary_set(d, "filterset dichroic name", filterset.dic_name);  
			dictionary_setint(d, "filterset dichroic nm", filterset.dichroic_nm);  

			dictionary_set(d, "filterset emm name", filterset.emm_name);  
			sprintf(buffer1, "min %d max %d", filterset.emm_min_nm, filterset.emm_max_nm);
			dictionary_set(d, "filterset emm nm", buffer1);      
		}

		#endif

		#ifdef BUILD_MODULE_LASER

		// Laser
		if(ms->_laser != NULL) {

			float power = 0.0;

			if(find_resource("config.ini", config_fullpath) == 0)
				iniparser_load_section_into_dictionary(d, config_fullpath, "Laser_Metadata");

			hardware_device_getinfo(HARDWARE_DEVICE_CAST(ms->_laser), buffer1);
			dictionary_set(d, "laser firmware", buffer1); 

			laser_get_power(ms->_laser, &power);
			dictionary_setdouble(d, "laser power", power);
		}

		#endif

		#ifdef BUILD_MODULE_LASER_POWER_MONITOR
		if(ms->_laser_power_monitor != NULL) {
			hardware_device_get_current_value_text(HARDWARE_DEVICE_CAST(ms->_laser_power_monitor), buffer1);
					dictionary_set(d, "excitation level", buffer1);
		}
		#endif
	}
	else if (mode == MICROSCOPE_FLUORESCENCE) {

		if(ms->_shutter != NULL) {
			shutter_get_info(ms->_shutter, buffer1);
			dictionary_set(d, "shutter type", buffer1); 
		}

		if(find_resource("config.ini", config_fullpath) == 0)
			iniparser_load_section_into_dictionary(d, config_fullpath, "Lamp_Metadata");

		// This is in the specific microscope code
		// I think the cooled should be a lamp ?
		// Actually I think we in the longer term should add devices to a list
		// Then we won't need this pointers in the microscope structure.
		// Maybe a hask table of devices.
		#ifdef BUILD_MODULE_PRECISE_EXCITE
		if(ms->_precise_excite != NULL) {
			hardware_device_get_current_value_text(HARDWARE_DEVICE_CAST(ms->_precise_excite), buffer1);
				dictionary_set(d, "lamp power", buffer1);
		}
		#endif

		#ifdef BUILD_MODULE_INTENSILIGHT
		if(ms->_intensilight != NULL && intensilight_is_connected(ms->_intensilight)) {
			hardware_device_get_current_value_text(HARDWARE_DEVICE_CAST(ms->_intensilight), buffer1);
				dictionary_set(d, "lamp power", buffer1);
		}
		#endif

		#ifdef BUILD_MODULE_LASER_POWER_MONITOR
		if(ms->_laser_power_monitor != NULL) {

			double shutter_time, min_shutter_time;
			
			if(shutter_get_open_time(ms->_shutter, &shutter_time) != SHUTTER_ERROR) {

				laserpowermonitor_get_minimum_allowed_shutter_time (ms->_laser_power_monitor, &min_shutter_time);

				// Shutter 0.0 means it is open
				if(shutter_time > 0.0 && shutter_time < min_shutter_time)
					dictionary_set(d, "excitation level", "invalid - shutter time too small");
				else {
					hardware_device_get_current_value_text(HARDWARE_DEVICE_CAST(ms->_laser_power_monitor), buffer1);
					dictionary_set(d, "excitation level", buffer1);
				}
			}		
		}
		#endif
	}
	else if (mode == MICROSCOPE_BRIGHT_FIELD) {
		// LAMP
		if(ms->_lamp != NULL) {

			double power = 0.0;
			LampStatus status = 0;

			PROFILE_START("microscope_add_hardware_metadata - bf lamp");

			lamp_off_on_status(ms->_lamp, &status);

			if (status == LAMP_ON){
				lamp_get_intensity(ms->_lamp, &power); 
				dictionary_setdouble(d, "lamp power", power);
			}
			else {
				dictionary_setdouble(d, "lamp power", 0.0);
			}

			PROFILE_STOP("microscope_add_hardware_metadata - bf lamp");
		}
	}

	PROFILE_STOP("microscope_add_hardware_metadata");

	return;
}

dictionary* microscope_get_camera_image_metadata(Microscope* ms, IcsViewerWindow* window)
{
	char buffer1[500] = "", vendor[100] = "", model[100] = "";
	int tmp, width, height;
	double dtmp, dtmp2;
	dictionary *d = dictionary_new(100);	
	FIBITMAP *fib = GCI_ImagingWindow_GetDisplayedFIB(window);  
	
	microscope_add_microscope_metadata(ms, d);

	// Image Width x Height
	sprintf(buffer1, "%d %d", FreeImage_GetWidth(fib), FreeImage_GetHeight(fib)); 
	dictionary_set(d, "dimensions", buffer1);
	
	width = FreeImage_GetWidth(fib);
	height = FreeImage_GetHeight(fib);

	// Microns Per Pixel
	dtmp = gci_camera_get_true_microns_per_pixel(MICROSCOPE_MASTER_CAMERA(ms));
	sprintf(buffer1, "%.3e %.3e", width * dtmp * 1e-6, height * dtmp * 1e-6);  
	dictionary_set(d, "extents", buffer1);     

	// Units
	dictionary_set(d, "units", "m m");      

//	dictionary_setint(d, "image type", FreeImage_GetImageType(fib));
	dictionary_setint(d, "image bpp", FreeImage_GetBPP(fib));
	dictionary_set(d, "image bigendian", "false");
	dictionary_setint(d, "image sizex", width);
	dictionary_setint(d, "image sizey", height);

	sprintf(buffer1, "%.3e", width * dtmp * 1e-6); 
	dictionary_set(d, "image physical_sizex", buffer1);

	sprintf(buffer1, "%.3e", height * dtmp * 1e-6); 
	dictionary_set(d, "image physical_sizey", buffer1);

	// other mscope hardware
	microscope_add_hardware_metadata(ms, d);
		
	// Camera Description
	gci_camera_get_description(MICROSCOPE_MASTER_CAMERA(ms), buffer1); 
	dictionary_set(d, "camera",buffer1); 
	gci_camera_get_info(MICROSCOPE_MASTER_CAMERA(ms), vendor, model, NULL, NULL, NULL, NULL);
	dictionary_set(d, "camera manufacturer", vendor);
	dictionary_set(d, "camera model", model);
        
	// Camera Exposure
	dtmp = gci_camera_get_exposure_time(MICROSCOPE_MASTER_CAMERA(ms));
	sprintf(buffer1, "%.0f ms", dtmp);  
	dictionary_set(d, "exposure_time", buffer1); 
	
	// Camera Gain
	if(gci_camera_dual_channel_enabled(MICROSCOPE_MASTER_CAMERA(ms))) {   
		gci_camera_get_gain(MICROSCOPE_MASTER_CAMERA(ms), CAMERA_CHANNEL1, &dtmp);
		gci_camera_get_gain(MICROSCOPE_MASTER_CAMERA(ms), CAMERA_CHANNEL2, &dtmp2);
		sprintf(buffer1, "%.2f %.2f", dtmp, dtmp2);
		dictionary_set(d, "gain", buffer1);   
	}
	else {
		gci_camera_get_gain(MICROSCOPE_MASTER_CAMERA(ms), CAMERA_CHANNEL1, &dtmp);
		sprintf(buffer1, "%.2f", dtmp);
		dictionary_set(d, "gain", buffer1);   	
	}
	
	// Camera BlackLevel
	if(gci_camera_dual_channel_enabled(MICROSCOPE_MASTER_CAMERA(ms))) {   
		gci_camera_get_blacklevel(MICROSCOPE_MASTER_CAMERA(ms), CAMERA_CHANNEL1, &dtmp);
		gci_camera_get_blacklevel(MICROSCOPE_MASTER_CAMERA(ms), CAMERA_CHANNEL2, &dtmp2);
		sprintf(buffer1, "%.2f %.2f", dtmp, dtmp2);
		dictionary_set(d, "black_level", buffer1);    	 
	}
	else {
		gci_camera_get_blacklevel(MICROSCOPE_MASTER_CAMERA(ms), CAMERA_CHANNEL1, &dtmp);
		sprintf(buffer1, "%.2f", dtmp);
		dictionary_set(d, "black_level", buffer1);   
	}
	
	// Camera Binning
	if(gci_camera_supports_binning(MICROSCOPE_MASTER_CAMERA(ms))) {
		tmp = gci_camera_get_binning_mode(MICROSCOPE_MASTER_CAMERA(ms));
		dictionary_setint(d, "binning", tmp); 
	}
	
	if((tmp = gci_camera_get_average_count_of_last_image_displayed(MICROSCOPE_MASTER_CAMERA(ms)))) {
		sprintf(buffer1, "image averaged over %d frames", tmp);
		dictionary_set(d, "average image", buffer1); 
	}

	return d;
}

dictionary* microscope_get_flim_image_metadata(Microscope* ms, IcsViewerWindow* window)
{
	dictionary *d = dictionary_new(100);	
	
	#ifdef BUILD_MODULE_SPC
	spc_get_metadata(ms->_spc, d);
	#endif

	return d;
}

dictionary* microscope_get_metadata(Microscope* ms, IcsViewerWindow* window)
{   
	switch(ms->illumination_mode)
	{
		case MICROSCOPE_FLUORESCENCE:
		case MICROSCOPE_FLUOR_NO_SHUTTER: 	
    	case MICROSCOPE_BRIGHT_FIELD:
    	case MICROSCOPE_PHASE_CONTRAST:
		{
			return microscope_get_camera_image_metadata (ms, window);     
		} 
		
		case MICROSCOPE_LASER_SCANNING:
		{
			return microscope_get_flim_image_metadata (ms, window);     
		} 
	}
	
	
	return microscope_get_camera_image_metadata(ms, window);
}

/*
struct panel_ctrl_data
{
	int panel;
	int ctrl;
};

void dictionary_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	struct panel_ctrl_data * pcd = (struct panel_ctrl_data*) data;
	
	AddKeyValueToTree(pcd->panel, pcd->ctrl, key, val);          	
}

void microscope_on_show_metadata (IcsViewerWindow* window, int panel, int ctrl, void* callback)
{
	Microscope *ms = microscope_get_microscope();    
	dictionary* d = microscope_get_metadata(ms, window);  
		
	struct panel_ctrl_data * pcd = (struct panel_ctrl_data*) malloc(sizeof(struct panel_ctrl_data));

	pcd->panel = panel;
	pcd->ctrl = ctrl;
	
	dictionary_foreach(d, dictionary_keyval_callback, pcd);
	
	free(pcd);
	dictionary_del(d);
}
*/

/*
static void dictionary_ics_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	ICS *ics = (ICS *) data;
	
	FreeImageIcs_IcsAddHistoryString (ics, key, val);         	
}
*/


/*
void microscope_save_metadata_to_text_file(GCIWindow *window, char *filename)
{
	Microscope *ms = microscope_get_microscope();    
	dictionary* d = microscope_get_metadata(ms, window);  
	FILE *fp;
	
	if((fp = fopen(filename, "w")) == NULL)
		return;
	
	dictionary_foreach(d, dictionary_text_keyval_callback, fp);
	
	dictionary_del(d);
	fclose(fp);
	
	return;
}
*/

/*
void microscope_save_metadata_from_dictionary(GCIWindow *window, char *filename, char *extension,
											  dictionary *d, void* callback)
{
	Microscope *ms = microscope_get_microscope();     
	ICS *ics=NULL;
	
	if(strcmp(extension, ".ics"))
		return;
	
	FreeImageIcs_IcsOpen(&ics, filename, "rw");  
	
	FreeImageIcs_IcsSetNativeScale(ics, 0, 0.0, window->microns_per_pixel, "microns");
	FreeImageIcs_IcsSetNativeScale(ics, 1, 0.0, window->microns_per_pixel, "microns");

	dictionary_foreach(d, dictionary_ics_keyval_callback, ics);
	
	FreeImageIcs_IcsClose (ics);   
	
	return;
}
*/

/*
void microscope_save_metadata(GCIWindow *window, char *filename, char *extension, void* callback)
{
	Microscope *ms = microscope_get_microscope();    
	dictionary* d = microscope_get_metadata(ms, window);  
	ICS *ics=NULL;
	
	if(strcmp(extension, ".ics"))
		return;
	
	FreeImageIcs_IcsOpen(&ics, filename, "rw");  
	
	FreeImageIcs_IcsSetNativeScale(ics, 0, 0.0, window->microns_per_pixel, "microns");
	FreeImageIcs_IcsSetNativeScale(ics, 1, 0.0, window->microns_per_pixel, "microns");

	dictionary_foreach(d, dictionary_ics_keyval_callback, ics);
	
	dictionary_del(d);
	FreeImageIcs_IcsClose (ics);   
	
	return;
}
*/

int microscope_saveimage_with_metadata(Microscope* microscope, FIBITMAP *dib, char *filepath, dictionary *d)
{
	char file_ext[10];          
	
	get_file_extension(filepath, file_ext); 
	
	// We have an ics file so we store all the microscope metadata with it.
	if (strcmp(file_ext, ".ics") == 0) {
		
		if(FreeImageIcs_SaveImage (dib, filepath, 1) == FIA_ERROR) {
			send_microscope_error_text (microscope, "Error saving ics file with metadata");     
			return MICROSCOPE_ERROR;
		}
		
		GCI_ImagingWindow_SaveMetaDataDictionaryToIcsFilePath(CAMERA_WINDOW(MICROSCOPE_MASTER_CAMERA(microscope)), d, filepath);
	}
	else {
		
		// Extension is not ics so we dont save the metadata
		if(FIA_SimpleSaveFIBToFile (dib, filepath) == FIA_ERROR) {
			send_microscope_error_text (microscope, "Error saving file"); 
			return MICROSCOPE_ERROR; 
		}
	}
	
	return MICROSCOPE_SUCCESS;		
}

/*
int microscope_saveimage(Microscope* microscope, FIBITMAP *dib, char *filepath)
{
	char file_ext[10];          
	
	get_file_extension(filepath, file_ext); 
	
	// We have an ics file so we store all the microscope metadata with it.
	if (strcmp(file_ext, ".ics") == 0) {
		
		if(FreeImageIcs_SaveImage (dib, filepath, 1) == FIA_ERROR) {
			send_microscope_error_text (microscope, "Error saving ics file with metadata");     
			return MICROSCOPE_ERROR;
		}
		
		microscope_save_metadata(CAMERA_WINDOW(MICROSCOPE_MASTER_CAMERA(microscope)), filepath, file_ext, microscope);
	}
	else {
		
		// Extension is not ics so we dont save the metadata
		if(FIA_SimpleSaveFIBToFile (dib, filepath) == FIA_ERROR) {
			send_microscope_error_text (microscope, "Error saving file"); 
			return MICROSCOPE_ERROR; 
		}
	}
	
	return MICROSCOPE_SUCCESS;		
}
*/

int microscope_saveimage(Microscope* microscope, FIBITMAP *dib, char *filepath)
{
	int items_in_queue, number_flushed;
	FileSaveDetail detail;

	CmtGetTSQAttribute(microscope->_save_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_queue);		

	// For some reason the acquition thread has got to far ahead of the processing
	// thread. So we now delay a little and keep the images created to 50 
	// This is unlikely to happen in a real world setup. Only when using dummy stuff so
	// the acquisition is really fast.
	while(items_in_queue > MICROSCOPE_SAVE_IMAGE_QUEUE_SIZE) 
	{
		CmtGetTSQAttribute(microscope->_save_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_queue);	
		Delay(0.001);
	}

	detail.fib = FreeImage_Clone(dib);
	strcpy(detail.path, filepath);

	detail.metadata = microscope_get_metadata(microscope, CAMERA_WINDOW(MICROSCOPE_MASTER_CAMERA(microscope)));

	CmtWriteTSQData(microscope->_save_queue, &detail, 1, TSQ_INFINITE_TIMEOUT, &number_flushed);

	return MICROSCOPE_SUCCESS;		
}

int send_microscope_error_text (Microscope* microscope, char fmt[], ...)
{
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(microscope), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(microscope), message);  
	
	ui_module_send_valist_error(UIMODULE_CAST(microscope), "Microscope Error", fmt, ap);
	
	va_end(ap);  
	
	return MICROSCOPE_SUCCESS;
}

static void microscope_read_or_write_main_panel_registry_settings(Microscope *microscope, int write)
{
	ui_module_read_or_write_registry_settings(UIMODULE_CAST(microscope), write);    
}


static int microscope_set_startup_mode(Microscope* microscope)
{
	char config_fullpath[GCI_MAX_PATHNAME_LEN];
	char mode[500];
	
	if(find_resource("config.ini", config_fullpath) < 0)
		return MICROSCOPE_ERROR;      
        							
    if(GetPrivateProfileString("StartupOptions", "Mode",
        "Fluorescence", mode, 500, config_fullpath) <= 0)
		return MICROSCOPE_ERROR;
	
	if(strcmp(mode, "Fluorescence") == 0)
		microscope_switch_illumination_mode(microscope, MICROSCOPE_FLUORESCENCE, 0);
	else if(strcmp(mode, "BrightField") == 0) 
		microscope_switch_illumination_mode(microscope, MICROSCOPE_BRIGHT_FIELD, 0);  
	else if(strcmp(mode, "LaserScanning") == 0) 
		microscope_switch_illumination_mode(microscope, MICROSCOPE_LASER_SCANNING, 0);
	else {
		GCI_MessagePopup("Startup error", "The startup mode specified in config.ini was not recognised.\nTry 'Fluorescence' or 'BrightField'\nDefaulting to Fluorescence");
		microscope_switch_illumination_mode(microscope, MICROSCOPE_FLUORESCENCE, 0);
		return MICROSCOPE_SUCCESS;
	}

	return MICROSCOPE_SUCCESS;
}


static int microscope_reset_mode_settings_to_default(Microscope* microscope, MicroscopeIlluminationMode mode)
{
	char default_path[GCI_MAX_PATHNAME_LEN] = "", path[GCI_MAX_PATHNAME_LEN] = "";

	path[0] = '\0';
	default_path[0] = '\0';

	microscope_get_filepath_for_illumination_settings(microscope, mode, path);
	microscope_get_filepath_for_default_illumination_settings(microscope, mode, default_path);

	if(CopyFile (default_path, path) < 0) {
		GCI_MessagePopup("File copy error", "Could not copy default illumination settings file");
		return MICROSCOPE_ERROR;
	}

	return MICROSCOPE_SUCCESS;
}

int microscope_reset_all_mode_settings_to_default(Microscope* microscope)
{
	microscope_reset_mode_settings_to_default(microscope, MICROSCOPE_BRIGHT_FIELD);
	microscope_reset_mode_settings_to_default(microscope, MICROSCOPE_FLUORESCENCE);
	microscope_reset_mode_settings_to_default(microscope, MICROSCOPE_LASER_SCANNING);
	
	return MICROSCOPE_SUCCESS;
}

int microscope_set_optical_path(Microscope* microscope)
{	
	return 0;
}


void microscope_set_image_scale_by_optical_zoom (Microscope* microscope, double new_value)
{
	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL && gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope))) {

//		double microns_per_pixel = gci_camera_get_true_microns_per_pixel(MICROSCOPE_MASTER_CAMERA(microscope));
		// get raw microns per pixel, not incl. binning
		double microns_per_pixel = GCI_ImagingWindow_GetMicronsPerPixelFactor(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window);
		
		// set the new scale with reference to the last optical zoom setting
		GCI_ImagingWindow_SetMicronsPerPixelFactor(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window, microns_per_pixel * microscope->_optical_zoom_value / new_value);

		// store the new optical zoom
		microscope->_optical_zoom_value = new_value;
	}
}

//Access to devices. Any which aren't present will return NULL

TemperatureMonitor* microscope_get_temperature_monitor(Microscope* microscope)
{
	return microscope->_temperature_monitor;
}

OpticalLift* microscope_get_optical_lift(Microscope* microscope)
{
	return microscope->_optical_lift;
}

Analyzer* microscope_get_analyser(Microscope* microscope)
{
	return microscope->_analyzer;
}

ObjectiveManager* microscope_get_objective(Microscope* microscope)
{
	return microscope->_objective;
}

FluoCubeManager* microscope_get_cube_manager(Microscope* microscope)
{
	return microscope->_fluor_cube;
}

CondenserManager* microscope_get_condenser(Microscope* microscope)
{
	return microscope->_condenser;
}

Shutter* microscope_get_shutter(Microscope* microscope)
{
	return microscope->_shutter;
}

SingleRangeHardwareDevice* microscope_get_aperture_stop(Microscope* microscope)
{
	return microscope->_aperture_stop;
}

SingleRangeHardwareDevice* microscope_get_field_stop(Microscope* microscope)
{
	return microscope->_field_stop;
}

SingleRangeHardwareDevice* microscope_get_epi_field_stop(Microscope* microscope)
{
	return microscope->_epi_field_stop;
}

SingleRangeHardwareDevice* microscope_get_optical_zoom(Microscope* microscope)
{
	return microscope->_optical_zoom;
}

OpticalPathManager* microscope_get_optical_path_manager(Microscope* microscope)
{
	return microscope->_optical_path;
}

XYStage* microscope_get_stage(Microscope* microscope)
{
	return microscope->_stage;
}

StagePlateModule* microscope_get_stage_plate_module(Microscope* microscope)
{
	return microscope->_stage_plate_module;
}

Z_Drive* microscope_get_master_zdrive(Microscope* microscope)
{
	return microscope->_master_z_drive;
}

Lamp* microscope_get_lamp(Microscope* microscope)
{
	return microscope->_lamp;
}

BatchCounterA1* microscope_get_batchcounter(Microscope* microscope)
{
	return microscope->_batch_counter;
}

Scanner* microscope_get_scanner(Microscope* microscope)
{
	return microscope->_scanner;
}

FilterSetCollection* microscope_get_filter_set(Microscope* microscope)
{
	return microscope->_filter_set;
}

AutofocusCtrl* microscope_get_autofocus_ctrl(Microscope* microscope)
{
	return microscope->_autofocusCtrl;
}

precisExcite* microscope_get_precise_excite(Microscope* microscope)
{
	return microscope->_precise_excite;
}

Intensilight* microscope_get_intensilight(Microscope* microscope)
{
	return microscope->_intensilight;
}

GciCamera* microscope_get_camera(Microscope* microscope)
{
    return MICROSCOPE_MASTER_CAMERA(microscope);
}

optical_calibration* microscope_get_optical_calibration(Microscope* microscope)
{
	return microscope->_optical_cal;	
}

timelapse* microscope_get_timelapse(Microscope* microscope)
{
	return microscope->_tl;	
}

cell_finder* microscope_get_cellfinder(Microscope* microscope)
{
	return microscope->_cf;	
}
 
ref_images* microscope_get_background_correction(Microscope* microscope)
{
	return microscope->_ri;	
}

region_scan* microscope_get_region_scan(Microscope* microscope)
{
	return microscope->_rs;	
}

realtime_overview* microscope_get_realtime_overview(Microscope* microscope)
{
	return microscope->_rto;	
}

stage_scan* microscope_get_stage_scan(Microscope* microscope) 
{
	return microscope->_ss;	
}

Spc* microscope_get_spc(Microscope* microscope)
{
	return microscope->_spc;	    
}

void microscope_add_camera(Microscope* microscope, GciCamera *camera)
{
	if(microscope->_number_of_cameras >= MAX_ALLOWED_CAMERAS)
		return;

	microscope->_cameras[microscope->_number_of_cameras] = camera;

	microscope->_number_of_cameras++;
}

//void microscope_switch_current_optical_calibration_device(Microscope* microscope, OpticalCalibrationDevice *device)
//{
//	microscope->_current_optical_calibration_device = device;
//    GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeOpticalCalibrationDeviceChanged", GCI_VOID_POINTER, microscope);	
//}

void microscope_switch_camera_to_master(Microscope* microscope, GciCamera *camera)
{
	int i=0;

	if(MICROSCOPE_MASTER_CAMERA(microscope) == camera)
		return;

	// Set all cameras to snap mode.
	for(i=0; i < microscope->_number_of_cameras; i++)
	{
		GciCamera *cam = microscope->_cameras[i];
		gci_camera_set_snap_mode (cam);

		// Dim the panels
		gci_camera_disable_ui(cam, 1);
	}

	// hide old camera UI
	gci_camera_hide_ui (microscope->_master_camera);
	gci_camera_hide_extra_ui (microscope->_master_camera);

	microscope->_master_camera = camera;
	
	// Compression for colour images is very slow in ics so we turn compression off.
	if(gci_camera_get_colour_type(microscope->_master_camera) == RGB_TYPE) {
		FreeImageIcs_SetCompressionLevel(0);
	}
	else {
		FreeImageIcs_SetCompressionLevel(microscope->_ics_compression_level);
	}

	// display new camera UI
	gci_camera_disable_ui(camera, 0);
	gci_camera_display_main_ui(camera);
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeMasterCameraChanged", GCI_VOID_POINTER, microscope);	
}

int microscope_set_master_zdrive(Microscope* microscope, Z_Drive *zd)
{
	microscope->_master_z_drive = zd;
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeMasterZDriveChanged", GCI_VOID_POINTER, microscope);
	
	return MICROSCOPE_SUCCESS;
}

int microscope_set_objectives_zdrive_link(Microscope* microscope, int link)
{
	microscope->_objective_z_link = link; 
	GCI_Signal_Emit(&(microscope->signal_table), "MicroscopeObjZlinkageChanged", GCI_INT, link);
	
	return MICROSCOPE_SUCCESS;
}


int microscope_disable_all_panels(Microscope* microscope, int disable)
{
	if (microscope->_main_ui_panel < 0)
		return 0;
	
	if(microscope->lpVtbl.disable_all_panels != NULL)
		microscope->lpVtbl.disable_all_panels(microscope, disable);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopePanelDimChange", GCI_VOID_POINTER, microscope, GCI_INT, disable);
	
	return 0;
}

void microscope_zaxis_objective_link_on_changed_handler(Microscope* microscope, MICROSCOPE_ZAXIS_OBJECTIVE_LINK_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeObjZlinkageChanged", handler, data) == SIGNAL_ERROR)
		return;
}



int microscope_signal_hide_handler_connect (Microscope* microscope, MICROSCOPE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(microscope), "Hide", handler, callback_data) == SIGNAL_ERROR) {
		send_microscope_error_text(microscope, "Can not connect signal handler for Microscope Hide signal");
		return MICROSCOPE_ERROR;
	}

	return MICROSCOPE_SUCCESS;
}


int microscope_changed_handler_connect(Microscope* microscope, MICROSCOPE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeChanged", handler, data) == SIGNAL_ERROR) {
		send_microscope_error_text(microscope, "Can not connect signal handler for Microscope Change signal");
		return MICROSCOPE_ERROR;
	}

	return MICROSCOPE_SUCCESS;
}

int microscope_dimmed_handler_connect(Microscope* microscope, MICROSCOPE_PANEL_DIMMED_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopePanelDimChange", handler, data) == SIGNAL_ERROR) {
		send_microscope_error_text(microscope, "Can not connect signal handler for MicroscopePanelDimChange signal");
		return MICROSCOPE_ERROR;
	}

	return MICROSCOPE_SUCCESS;
}

int microscope_master_z_drive_changed_handler_connect(Microscope* microscope, void *handler, void *data )
{
	int id;
	if( (id=GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeMasterZDriveChanged", handler, data)) == SIGNAL_ERROR) {
		send_microscope_error_text(microscope, "Can not connect signal handler for Master Z-Drive Change signal");
		return MICROSCOPE_ERROR;
	}

	return id;
}

int microscope_master_z_drive_changed_handler_disconnect(Microscope* microscope, int id )
{
	if( GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeMasterZDriveChanged", id) == SIGNAL_ERROR) {
		send_microscope_error_text(microscope, "Cannot disconnect signal handler for Master Z-Drive Change signal");
		return MICROSCOPE_ERROR;
	}

	return MICROSCOPE_SUCCESS;
}

int microscope_master_camera_changed_handler_connect(Microscope* microscope, MICROSCOPE_EVENT_HANDLER *handler, void *data )
{
	int id;

	if( (id=GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeMasterCameraChanged", handler, data)) == SIGNAL_ERROR) {
		send_microscope_error_text(microscope, "Cannot connect signal handler for Master Camera Change signal");
		return MICROSCOPE_ERROR;
	}

//	printf("%s connected to master camera change signal.\n", UIMODULE_GET_NAME(data));

	return id;
}

int microscope_master_camera_changed_handler_disconnect(Microscope* microscope, int id)
{
	if( GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeMasterCameraChanged", id) == SIGNAL_ERROR) {
		send_microscope_error_text(microscope, "Can not disconnect signal handler for Master Camera Change signal");
		return MICROSCOPE_ERROR;
	}

	return MICROSCOPE_SUCCESS;
}

/*
int microscope_current_optical_calibration_device_changed_handler_connect(Microscope* microscope, void *handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeOpticalCalibrationDeviceChanged", handler, data) == SIGNAL_ERROR) {
		send_microscope_error_text(microscope, "Can not connect signal handler for optical calibration device change signal");
		return MICROSCOPE_ERROR;
	}

	return MICROSCOPE_SUCCESS;
}
*/

static void OnMasterZDriveChanged (Z_Drive* zd, int update, void *data)
{
	Microscope *ms = microscope_get_microscope();

	// This snaps a new image 
	if(update)
		microscope_update_display(ms);
	else {
		// wont snap if in snap mode.
		microscope_update_display_if_live(ms);
	}
}

void microscope_get_filepath_for_illumination_settings(Microscope* microscope, MicroscopeIlluminationMode mode, char *path)
{
	memset(path, 0, 1);
	
	// use microscope data path
	strcpy(path, microscope->_data_dir);
	
	switch(mode)
	{
		case MICROSCOPE_FLUORESCENCE:
		case MICROSCOPE_FLUOR_NO_SHUTTER: 	
		{
			strcat(path, "\\FluorSettings.ini");    
			break;
		}
		
    	case MICROSCOPE_BRIGHT_FIELD:
		{
			strcat(path, "\\BrightFieldSettings.ini");    
			break;
		} 
		
    	case MICROSCOPE_PHASE_CONTRAST:
		{
			strcat(path, "\\PhaseContrastSettings.ini");      
			break;
		} 
		
		case MICROSCOPE_LASER_SCANNING:
		{
			strcat(path, "\\LaserScanningSettings.ini");      
			break;
		} 
	}
}


void microscope_get_filepath_for_default_illumination_settings(Microscope* microscope, MicroscopeIlluminationMode mode, char *path)
{
	memset(path, 0, 1);
	
	// use microscope data path
	strcpy(path, microscope->_data_dir);
	
	switch(mode)
	{
		case MICROSCOPE_FLUORESCENCE:
		case MICROSCOPE_FLUOR_NO_SHUTTER: 	
		{
			strcat(path, "\\DefaultFluorSettings.ini");    
			break;
		}
		
    	case MICROSCOPE_BRIGHT_FIELD:
		{
			strcat(path, "\\DefaultBrightFieldSettings.ini");    
			break;
		} 
		
    	case MICROSCOPE_PHASE_CONTRAST:
		{
			strcat(path, "\\DefaultPhaseContrastSettings.ini");      
			break;
		} 
		
		case MICROSCOPE_LASER_SCANNING:
		{
			strcat(path, "\\DefaultLaserScanningSettings.ini");      
			break;
		} 
	}
}

MicroscopeIlluminationMode microscope_get_illumination_mode (Microscope* microscope)
{
	return microscope->illumination_mode;	
}

int microscope_save_settings (Microscope* microscope, const char* path)
{
	if(microscope->lpVtbl.microscope_save_settings != NULL)
		return microscope->lpVtbl.microscope_save_settings(microscope, path);

	return MICROSCOPE_SUCCESS;
}

int microscope_post_initialise (Microscope* microscope)
{
	if(microscope->lpVtbl.microscope_post_initialise != NULL)
		return microscope->lpVtbl.microscope_post_initialise(microscope);

	return MICROSCOPE_SUCCESS;
}

int microscope_load_settings (Microscope* microscope, const char* path)
{

	if(microscope->lpVtbl.microscope_load_settings != NULL)
		return microscope->lpVtbl.microscope_load_settings(microscope, path);

	return MICROSCOPE_SUCCESS;
}

void microscope_update_display_if_live(Microscope *microscope)
{
	GciCamera *camera = microscope_get_camera(microscope);   

	if(camera == NULL)
		return;
	
	if (gci_camera_is_live_mode(camera) == LIVE)   // live mode
	{
		gci_camera_update_live_image(camera);
		gci_camera_show_window(camera);
	}
}

void microscope_update_display(Microscope *microscope)
{
	GciCamera *camera = microscope_get_camera(microscope);   

	if(camera == NULL)
		return;
	
	if (gci_camera_is_live_mode(camera) == LIVE)   // live mode
	{
		gci_camera_update_live_image(camera);
	}
	else
	{
		if(microscope->_autosnap) {
			if(microscope_get_illumination_mode(microscope) != MICROSCOPE_LASER_SCANNING)
				gci_camera_snap_image(camera);
		}
	}
	
	gci_camera_show_window(camera);
}

int microscope_switch_illumination_mode (Microscope* microscope, MicroscopeIlluminationMode mode, int save_old_settings)
{
	int was_live = 0;
	char name[100] = ""; 
	
	// Already in required mode
	if(microscope->illumination_mode == mode)
		return MICROSCOPE_SUCCESS;   
	
	PROFILE_START("microscope_switch_illumination_mode");

	switch(mode)
	{
		case MICROSCOPE_FLUORESCENCE:
		case MICROSCOPE_FLUOR_NO_SHUTTER: 	
		{
			strcat(name, "Fluorescence");    
			break;
		}
		
    	case MICROSCOPE_BRIGHT_FIELD:
		{
			strcat(name, "BrightField");    
			break;
		} 
		
    	case MICROSCOPE_PHASE_CONTRAST:
		{
			strcat(name, "PhaseContrast");      
			break;
		} 
		
		case MICROSCOPE_LASER_SCANNING:
		{
			strcat(name, "LaserScanning");      
			break;
		} 
	}
	
	logger_log(UIMODULE_LOGGER(microscope), LOGGER_INFORMATIONAL, "Microscope switch to %s mode", name);
	
	PROFILE_START("microscope_switch_illumination_mode - camera");

	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL && gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope))) {
		was_live = gci_camera_is_live_mode(MICROSCOPE_MASTER_CAMERA(microscope));    
		gci_camera_set_snap_mode(MICROSCOPE_MASTER_CAMERA(microscope)); 
	}
	
	PROFILE_STOP("microscope_switch_illumination_mode - camera");

	PROFILE_START("microscope_switch_illumination_mode - virtual");

	microscope_disable_all_panels(microscope, 1);

	if(microscope->lpVtbl.switch_illumination_mode != NULL)
		microscope->lpVtbl.switch_illumination_mode(microscope, mode, microscope->illumination_mode, save_old_settings);
	
	PROFILE_STOP("microscope_switch_illumination_mode - virtual");

	microscope->illumination_mode = mode; 

	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL && gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope)) &&  mode != MICROSCOPE_LASER_SCANNING) {	
		microscope_update_display(microscope);	
	}

	microscope_disable_all_panels(microscope, 0);

	#ifdef BUILD_MODULE_SPC
	if(mode == MICROSCOPE_LASER_SCANNING) {
		spc_display_main_ui(microscope->_spc);
	}
	#endif

	ref_images_recall_images(microscope->_ri);  

	PROFILE_STOP("microscope_switch_illumination_mode");

	return MICROSCOPE_SUCCESS;
}

int  microscope_set_illumination_mode (Microscope* microscope, MicroscopeIlluminationMode mode)
{
	return microscope_switch_illumination_mode (microscope, mode, 1);
}

int microscope_set_focusing_mode (Microscope* microscope)
{
	if(microscope->lpVtbl.set_focusing_mode != NULL)
		microscope->lpVtbl.set_focusing_mode(microscope);
	
	// Enable focus panel
	focus_set_on(microscope->_focus);

	logger_log(UIMODULE_LOGGER(microscope), LOGGER_INFORMATIONAL, "Microscope switching to focusing mode");  
	
	return MICROSCOPE_SUCCESS;
}

int microscope_set_hi_resolution_mode (Microscope* microscope)
{
	if(microscope->lpVtbl.set_hi_resolution_mode != NULL)
		microscope->lpVtbl.set_hi_resolution_mode(microscope);
	
	logger_log(UIMODULE_LOGGER(microscope), LOGGER_INFORMATIONAL, "Microscope switching to hi resolution mode");  
	
	return MICROSCOPE_SUCCESS;   
}


int microscope_set_user_data_directory(Microscope* microscope, const char *directory)
{
	memset(microscope->_user_data_dir, 0, 1);
	
	strcpy(microscope->_user_data_dir, directory);

	if (!FileExists(microscope->_user_data_dir, 0))
		MakeDir (microscope->_user_data_dir);
	
	return MICROSCOPE_SUCCESS;
}

int microscope_set_data_directory(Microscope* microscope, const char *directory)
{
	memset(microscope->_data_dir, 0, 1);
	
	strcpy(microscope->_data_dir, directory);

	if (!FileExists(microscope->_data_dir, 0))
		MakeDir (microscope->_data_dir);

	ui_module_set_data_dir(UIMODULE_CAST(microscope), microscope->_data_dir);   
	
	return MICROSCOPE_SUCCESS;
}

int microscope_get_user_data_directory(Microscope* microscope, char *path)
{
	int error;
	char command[1000] = "";

	memset(path, 0, 1);
	
	// Default data dir
	strncpy(path, microscope->_user_data_dir, 500);
	
	sprintf(path, "%s\\%s\\%s", microscope->_user_data_dir,  microscope->_study, microscope->_experiment);
	
	sprintf(command, "cmd.exe /C mkdir \"%s\"", path);

	if (!FileExists(path, 0)) {
		error = LaunchExecutableEx(command, LE_HIDE, NULL); 
	}

	return MICROSCOPE_SUCCESS;
}

int microscope_get_temp_directory(char *path)
{
	memset(path, 0, 1);
	
	// Get temp dir
	if(!GetEnvironmentVariable("Temp", path, 500)) {
		
		GCI_MessagePopup("Error", "No temporary directory exists!");
	}
	
	return MICROSCOPE_SUCCESS;
}


int microscope_get_data_directory(Microscope* microscope, char *path)
{
	memset(path, 0, 1);
	
	// Default data dir
	strncpy(path, microscope->_data_dir, 500);
	
	return MICROSCOPE_SUCCESS;
}

int microscope_get_data_subdirectory(Microscope* microscope, const char *subdirectory_name, char *path)
{
	microscope_get_data_directory(microscope, path);
	
	strcat(path, "\\");
	strcat(path, subdirectory_name);  
	
	return MICROSCOPE_SUCCESS;
}

int microscope_get_user_data_subdirectory(Microscope* microscope, const char *subdirectory_name, char *path)
{
	microscope_get_user_data_directory(microscope, path);
	
	strcat(path, "\\");
	
	if(subdirectory_name != NULL)
		strcat(path, subdirectory_name);  
	
	return MICROSCOPE_SUCCESS;
}

void microscope_get_safe_z_position(Microscope *microscope, int *pos)
{
	get_device_param_from_ini_file("Microscope", "Safe Z Position", pos);
}

void microscope_prevent_automatic_background_correction(Microscope* microscope)
{
	microscope->_prevent_automatic_background_correction = 1;
}

void microscope_allow_automatic_background_correction(Microscope* microscope)
{
	microscope->_prevent_automatic_background_correction = 0;
}

int microscope_is_automatic_background_correction_disabled(Microscope* microscope)
{
	return microscope->_prevent_automatic_background_correction;
}

FIBITMAP* perform_backround_correction_on_image(Microscope* microscope, FIBITMAP *dib)
{
	logger_log(UIMODULE_LOGGER(microscope), LOGGER_INFORMATIONAL, "Performing background correction");  
	
	return ref_images_process(microscope->_ri, dib);
}


static void microscope_add_python_module_search_path(Microscope* microscope, const char *path)
{
	char command[500] = "";

	sprintf(command, "sys.path.append(\"%s\")", path);
	
	//Lets append to the python module path  
	if(PyRun_SimpleString(command)) {
	
		GCI_MessagePopup("Error", "Failed to setup Python correctly");
	}
}

static void AddPythonIntegerToDict(PyObject* dict, const char *name, int val)
{
	PyObject* v = Py_BuildValue("i", val);
	PyDict_SetItemString(dict, name, v);
	Py_DECREF(v);
}

static void AddPythonStringToDict(PyObject* dict, const char *name, const char* val)
{
	PyObject* obj = PyString_FromString(val);
	PyDict_SetItemString(dict, name, obj);
	Py_DECREF(obj);
}

static int MICROSCOPE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Microscope*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Microscope *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int MICROSCOPE_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Microscope*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Microscope *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}	

static int CVICALLBACK SaveImageThread(void *data)
{
	Microscope *ms = (Microscope *) data;

	int i, items_in_queue = 1, number_in_batch = 1;
    FileSaveDetail files[MICROSCOPE_SAVE_IMAGE_QUEUE_SIZE];
	
	static int count = 0;

	while(1)
	{
		if(ms->_save_thread_running == 0)
			return 0;

		//CmtGetTSQAttribute(ms->_save_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_queue);		

		//if(items_in_queue == 0) {
		//	Delay(0.05);
		//	continue;
		//}
			
		if((items_in_queue = CmtReadTSQData (ms->_save_queue, files, number_in_batch, 200, 0)) <= 0) {
			Delay(0.05);
			continue;
		}

		for(i=0; i < items_in_queue; i++) {

			printf("Save Queue %d of %d\n", i+1, items_in_queue);

			microscope_saveimage_with_metadata(ms, files[i].fib, files[i].path, files[i].metadata);

			dictionary_del(files[i].metadata);
			FreeImage_Unload(files[i].fib);
			files[i].fib = NULL;
		}
	}

    return 0;	
}

void microscope_constructor(Microscope* microscope, char *name, char *description)
{
	PyObject* microscope_module = NULL, *dict = NULL, *obj = NULL;	

	char path[GCI_MAX_PATHNAME_LEN] = "";
	char data_dir[GCI_MAX_PATHNAME_LEN] = "";
	char user_data_dir[GCI_MAX_PATHNAME_LEN] = "";
	char buffer[500] = "";
	char command[500] = "";
	char microscope_python_root[500] = "";
	int size, ics_version=2;

	memset(microscope, 0, sizeof(Microscope));
	
	ui_module_constructor(UIMODULE_CAST(microscope), "Microscope");
	ui_module_set_description(UIMODULE_CAST(microscope), description);
	
	GetCurrentUser(buffer, 500, &size);
	strcpy(microscope->_user_name, buffer);

	microscope->illumination_mode = -1;
	
	microscope->_number_of_cameras = 0;
	microscope->_requested_mode_cube_position = -1;
	microscope->_requested_mode_op_position = -1;
	microscope->_prevent_automatic_background_correction = 0;
	microscope->_autosnap = 1;
	microscope->_timer = -1;
	microscope->_init_timeout = 120.0;
	microscope->_com_port = 1;
	microscope->_overlapped = 0;
	microscope->_objective_z_link = 1;
	microscope->_objective_interlock = SOFTWARE;   //Default to software interlocks
	microscope->_ics_compression_level = 7;

	microscope->_stored_stage_x = 0.0;	   // default stored reference position
	microscope->_stored_stage_y = 0.0;	
	microscope->_stored_focus_z = 0.0;	

	microscope->_optical_zoom_value = 1.0;
	
	microscope->_analyzer = NULL;
	microscope->_aperture_stop = NULL;
	microscope->_condenser = NULL;
	microscope->_epi_field_stop = NULL;
	microscope->_field_stop = NULL;
	microscope->_fluor_cube = NULL;
	microscope->_lamp = NULL;
	microscope->_objective = NULL;
	microscope->_optical_path = NULL;
	microscope->_optical_zoom = NULL;
	microscope->_shutter = NULL;
	microscope->_lamp = NULL;
	microscope->_scanner = NULL;
	microscope->_autofocusCtrl = NULL;
	microscope->_stage = NULL;
	microscope->_precise_excite = NULL;
	microscope->_intensilight = NULL;
    MICROSCOPE_MASTER_CAMERA(microscope) = NULL;
	microscope->_ss = NULL;
	microscope->_power_switch = NULL;
	microscope->_roi = NULL;
	microscope->_z_drive = NULL;
	microscope->_rto = NULL;
	microscope->_save_thread_running = 1;

	microscope->_change_camera_palette = 0;
		
	microscope->_menu_item_list = ListCreate (sizeof(int));  
	 
	strcpy(microscope->_study, "DefaultStudy");
	strcpy(microscope->_experiment, "DefaultExperiment");

	feedback_new(); 
	
	find_resource("Microscope Data", data_dir);
	get_device_string_param_from_ini_file("Microscope", "Microscope User Data", user_data_dir);
	sprintf(buffer, "%s\\DefaultUser", user_data_dir);

	microscope_set_data_directory(microscope, data_dir);  
	microscope_set_user_data_directory(microscope, buffer); 

	get_device_int_param_from_ini_file("Microscope", "ICS File Version", &ics_version);
	if (ics_version == 1) {  // if ics version is set to 1 in config.ini file, change the ics file saving version
		FreeImageIcs_SetICSVersion(1);
		FreeImageIcs_SetCompressionLevel(0);
		microscope->_ics_compression_level = 0;
	}

	// Option to turn off autosnap by default by putting "AutoSnap = 0" in Microscope section of config.ini
	get_device_int_param_from_ini_file("Microscope", "AutoSnap", &(microscope->_autosnap));

	#ifdef MICROSCOPE_PYTHON_AUTOMATION
	
	Py_Initialize();

	if(!Py_IsInitialized())
		return;

	PyRun_SimpleString("import sys");     
	  
	if(get_device_string_param_from_ini_file("PythonSysPath", "Path", microscope_python_root) == 0) {
		microscope_add_python_module_search_path(microscope, microscope_python_root);
		sprintf(command, "PYTHON_MICROSCOPY_AUTOMATION_HOME=\"%s\"", microscope_python_root);
		PyRun_SimpleString(command);
	}

	// Add timelapse script directory to path
	if(get_device_string_param_from_ini_file("PythonSysPath", "TimeLapseDirPath", buffer) == 0) 
		microscope_add_python_module_search_path(microscope, buffer);

	// Add freeimage python wrapper script directory to path
	if(get_device_string_param_from_ini_file("PythonSysPath", "FreeImageWrapperDirPath", buffer) == 0) 
		microscope_add_python_module_search_path(microscope, buffer);
	
	// Lets import the important python files that allow
	// ui manipulation and stdout redirection etc.
	
	Py_InitModule("gci", Gci_Py_Methods);
	   
	microscope_module = Py_InitModule("microscope", Microscope_Py_Methods);

	if(PyRun_SimpleString("import MicroscopeConstants")) {
	
		PyRun_SimpleString("print sys.path");   
		
		GCI_MessagePopup("Error", "Failed to setup Python correctly");
	}
	
	if(PyRun_SimpleString("import StdOutRedirect")) {
	
		PyRun_SimpleString("print sys.path");   
		
		GCI_MessagePopup("Error", "Failed to setup Python correctly");
	}
	 
	dict = PyModule_GetDict(microscope_module);

	AddPythonStringToDict(dict, "PYTHON_MICROSCOPE_AUTOMATION_ROOT", microscope_python_root);

	AddPythonIntegerToDict(dict, "FLUORESCENCE_MODE", MICROSCOPE_FLUORESCENCE);
	AddPythonIntegerToDict(dict, "BRIGHT_FIELD_MODE", MICROSCOPE_BRIGHT_FIELD);
	AddPythonIntegerToDict(dict, "PHASE_CONTRAST_MODE", MICROSCOPE_PHASE_CONTRAST);
	AddPythonIntegerToDict(dict, "FLUOR_NO_SHUTTER_MODE", MICROSCOPE_FLUOR_NO_SHUTTER);

	#endif    

	MICROSCOPE_VTABLE_PTR(microscope, initialise_hardware_user_interfaces) = NULL;
	MICROSCOPE_VTABLE_PTR(microscope, destroy) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, initialise_hardware) = NULL;
	MICROSCOPE_VTABLE_PTR(microscope, set_overlapped) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, wait_for_device) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, set_focusing_mode) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, set_hi_resolution_mode) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, save_image_metadata) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_live_enter) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_enter_snap_sequence_or_snapmode) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_pre_capture) = NULL; 
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_post_capture) = NULL;
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_trigger_now) = NULL;
	MICROSCOPE_VTABLE_PTR(microscope, on_camera_exit_get_image) = NULL;   
	MICROSCOPE_VTABLE_PTR(microscope, on_objective_changed_start) = NULL;  
	MICROSCOPE_VTABLE_PTR(microscope, on_objective_changed_end) = NULL;  
	MICROSCOPE_VTABLE_PTR(microscope, switch_illumination_mode) = NULL;    
	MICROSCOPE_VTABLE_PTR(microscope, start_all_timers) = NULL;  
	MICROSCOPE_VTABLE_PTR(microscope, stop_all_timers) = NULL;  
	MICROSCOPE_VTABLE_PTR(microscope, disable_all_panels) = NULL;
	MICROSCOPE_VTABLE_PTR(microscope, microscope_post_initialise) = NULL;

	GciCmtNewLock ("Microscope", 0, &(microscope->_lock) );  
	
    gmicroscope = microscope;
    
	FreeImage_SetOutputMessage(microscope_freeimage_error_handler);

	usernameSplashPanel(microscope);

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeMasterCameraChanged", MICROSCOPE_PTR_MARSHALLER);
	//GCI_Signal_New(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeOpticalCalibrationDeviceChanged", MICROSCOPE_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopeMasterZDriveChanged", MICROSCOPE_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(microscope), "MicroscopePanelDimChange", MICROSCOPE_PTR_INT_MARSHALLER);

	//SetPanelAttribute(microscope->_init_panel, ATTR_DIMMED, 1);
	
	// Set the data dir for the camera ie for log files and config
	ui_module_set_data_dir(UIMODULE_CAST(microscope), microscope->_data_dir);
	
	// Set the default user directory for images etc.
	microscope_get_user_data_directory(microscope, path);
	
	GCI_ImagingWindow_SetDefaultDirectoryPathForAllWindows(path);

	CmtNewTSQ(MICROSCOPE_SAVE_IMAGE_QUEUE_SIZE, sizeof(FileSaveDetail), OPT_TSQ_DYNAMIC_SIZE, &(microscope->_save_queue));

	CmtScheduleThreadPoolFunction(gci_thread_pool(), SaveImageThread, microscope, &(microscope->_save_thread_id));
    
	return;
}

Microscope* microscope_get_microscope(void)
{
    return gmicroscope;
}

void microscope_disable_metadata_on_snap(Microscope *microscope, int val)
{
	microscope->_disable_metadata_on_snap = val;
}

int microscope_get_detection_device_count (Microscope* microscope)
{
	CHECK_MICROSCOPE_VTABLE_PTR(microscope, microscope_get_detection_device_count);

  	return (*microscope->lpVtbl.microscope_get_detection_device_count)(microscope);
}

int microscope_get_detection_device_names (Microscope* microscope, const char** path)
{
	CHECK_MICROSCOPE_VTABLE_PTR(microscope, microscope_get_detection_device_names);

	return (*microscope->lpVtbl.microscope_get_detection_device_names)(microscope, path);
}

// For live mode and fluorescence mode we need to open the shutter to acquire images.
// SIGNAL: EnterLiveMode
static void on_camera_live_enter (GciCamera* camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;         
	dictionary* d;

	if(microscope->lpVtbl.on_camera_live_enter != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_live_enter)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_live_enter failed");
			return;
		}
	}

	d = microscope_get_camera_image_metadata(microscope, camera->_camera_window);
	GCI_ImagingWindow_SetMetaData(camera->_camera_window, d);
}

// SIGNAL: ExitLiveMode
static void on_camera_live_exit (GciCamera* camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;         

	if(microscope->lpVtbl.on_camera_live_exit != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_live_exit)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_live_exit failed");
			return;
		}
	}
}

// SIGNAL: EnterSnapMode, EnterSnapSequenceMode
static void on_camera_enter_snapsequence_or_snapmode (GciCamera* camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;         
	
	if(microscope->lpVtbl.on_camera_enter_snap_sequence_or_snapmode != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_enter_snap_sequence_or_snapmode)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_enter_snap_sequence_or_snapmode failed");
			return;
		}
	}
}

// SIGNAL: PreCapture
static void on_camera_pre_capture (GciCamera* camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;         

	if(microscope->lpVtbl.on_camera_pre_capture != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_pre_capture)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_pre_capture failed");
			return;
		}
	}
}


// This method is called when after the camera has integrated and we have turned the data into an image
// SIGNAL: ExitGetImage
static void on_camera_exit_get_image (GciCamera* camera, FIBITMAP** dib, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;         

	if(microscope->lpVtbl.on_camera_exit_get_image != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_exit_get_image)(microscope, dib) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_exit_get_image failed");
			return;
		}
	}
}

	
// When in flurescence mode opening the shutter will trigger the camera an
// image capture will begin.
// For live mode the shutter when we enter live mode.
// SIGNAL: PostCapture
static void on_camera_post_capture (GciCamera* camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;         
	
	if(microscope->lpVtbl.on_camera_post_capture != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_post_capture)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_post_capture failed");
			return;
		}
	}
}


static void on_camera_trigger_now (GciCamera* camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;         
	
	if(microscope->lpVtbl.on_camera_trigger_now != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_trigger_now)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_trigger_now failed");
			return;
		}
	}
}

// SIGNAL: Pre DisplayImage
static void on_camera_window_image_pre_displayed (GciCamera* camera, GCIWindow *window, void *data)
{
	static double time=0;
	const double time_before_focus = 0.1; // seconds
	Microscope *microscope = (Microscope *) data;       
	dictionary* d;
	FluoCube cube;

	if(microscope->_disable_metadata_on_snap == 0 && gci_camera_is_snap_mode(camera)) {
		d = microscope_get_camera_image_metadata(microscope, camera->_camera_window);
		GCI_ImagingWindow_SetMetaData(camera->_camera_window, d);
	}

	// Recalculate focus indicator every time_before_focus 
	if(((Timer()-time) > time_before_focus) && focus_is_on(microscope->_focus))
	{
		FIBITMAP *dib = gci_camera_get_displayed_image(camera);
		
		if(dib == NULL) {
			time = Timer();  
			return;
		}
		
		// Get focus for average of last 5 frames
//		focus_update_focus_ui(microscope->_focus, dib, 5);
		focus_update_focus_ui(microscope->_focus, dib, 1);
		time = Timer();
		
		FreeImage_Unload(dib);
	}

	// update false colour palette if required
	if (microscope->_change_camera_palette == 1){
		if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) {
			cube_manager_get_current_cube(microscope->_fluor_cube, &cube);
			GCI_ImageWindow_SetFalseColourWavelength(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window, cube.emm_min_nm);        
			microscope->_change_camera_palette = 0;
		}
	}

	//#ifdef VERBOSE_DEBUG
	//printf("on_camera_window_image_displayed finished\n");
	//#endif
}

// SIGNAL: Post DisplayImage
static void on_camera_window_image_post_displayed (GciCamera* camera, GCIWindow *window, void *data)
{
	/*
	static double time=0;
	const double time_before_focus = 0.1; // seconds
	Microscope *microscope = (Microscope *) data;       
	dictionary* d;
	FluoCube cube;

	//#ifdef VERBOSE_DEBUG
	//printf("on_camera_window_image_displayed started\n");
	//#endif

	if(microscope->_disable_metadata_on_snap == 0 && gci_camera_is_snap_mode(camera)) {
		d = microscope_get_camera_image_metadata(microscope, camera->_camera_window);
		GCI_ImagingWindow_SetMetaData(camera->_camera_window, d);
	}

	// Recalculate focus indicator every time_before_focus 
	if(((Timer()-time) > time_before_focus) && focus_is_on(microscope->_focus))
	{
		FIBITMAP *dib = gci_camera_get_displayed_image(camera);
		
		if(dib == NULL) {
			time = Timer();  
			return;
		}
		
		// Get focus for average of last 5 frames
//		focus_update_focus_ui(microscope->_focus, dib, 5);
		focus_update_focus_ui(microscope->_focus, dib, 1);
		time = Timer();
		
		FreeImage_Unload(dib);
	}

	// update false colour palette if required
	if (microscope->_change_camera_palette == 1){
		if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) {
			cube_manager_get_current_cube(microscope->_fluor_cube, &cube);
			GCI_ImageWindow_SetFalseColourWavelength(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window, cube.emm_min_nm);        
			microscope->_change_camera_palette = 0;
		}
	}

	//#ifdef VERBOSE_DEBUG
	//printf("on_camera_window_image_displayed finished\n");
	//#endif
	*/
}

static void on_camera_imagewindow_close (GciCamera* C9100_camera, GCIWindow *window, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;
	
	ui_module_hide_all_panels(UIMODULE_CAST(microscope->_focus));  
}

static void on_camera_close_handler(GciCamera *current_camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;
	
	//ui_module_hide_all_panels(UIMODULE_CAST(microscope->_focus));      

	//GCI_ImagingWindow_Hide(current_camera->_camera_window);
}


static void on_camera_show_handler(GciCamera *current_camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;
	
	focus_set_camera (microscope->_focus, current_camera);
	focus_show_all_panels(microscope->_focus);
}

static void on_camera_exposure_changed_handler(GciCamera *current_camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;
	
	if(microscope->lpVtbl.on_camera_exposure_changed != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_exposure_changed)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_exposure_changed failed");
			return;
		}
	}
}

static void on_camera_gain_changed_handler(GciCamera *current_camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;
	
	if(microscope->lpVtbl.on_camera_gain_changed != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_gain_changed)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_gain_changed failed");
			return;
		}
	}
}


static void on_camera_binning_changed_handler(GciCamera *current_camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;
	
	if(microscope->lpVtbl.on_camera_binning_changed != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_binning_changed)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_binning_changed failed");
			return;
		}
	}

	if(microscope->_rto != NULL)
		realtime_overview_update_microns_per_pixel(microscope->_rto);
}

static void on_camera_datamode_changed_handler(GciCamera *current_camera, void *callback_data)
{
	Microscope *microscope = (Microscope *) callback_data;
	
	if(microscope->lpVtbl.on_camera_datamode_changed != NULL) {
		
		if( (*microscope->lpVtbl.on_camera_datamode_changed)(microscope) == MICROSCOPE_ERROR ) {
			send_microscope_error_text(microscope, "calling on_camera_datamode_changed failed");
			return;
		}
	}

	if (microscope->_rto != NULL && MICROSCOPE_MASTER_CAMERA(microscope) != NULL)
		realtime_overview_set_image_max_scale_value(microscope->_rto, pow(2.0, gci_camera_get_data_mode(MICROSCOPE_MASTER_CAMERA(microscope))));
}


void microscope_freeimage_error_handler(FREE_IMAGE_FORMAT fif, const char *msg)
{
	Microscope *microscope = microscope_get_microscope();

	logger_log(UIMODULE_LOGGER(microscope), LOGGER_ERROR,
		"Critical Error With FreeImageAlgorithms: %s", msg); 
}

void microscope_setup_optical_calibration(Microscope *microscope)
{
	double factor = 1.0f;
	
	if(microscope->_number_of_cameras < 1)
		return;
	
	if(MICROSCOPE_MASTER_CAMERA(microscope) == NULL || gci_camera_is_powered_up(MICROSCOPE_MASTER_CAMERA(microscope)) == 0)
		return;

	optical_calibration_get_calibration_factor_for_current_objective(microscope->_optical_cal, &factor); 
	
	GCI_ImagingWindow_SetMicronsPerPixelFactor(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window, factor);

	if(microscope->_rto != NULL)
		realtime_overview_update_microns_per_pixel(microscope->_rto);
}

// SIGNAL: CalibrationChanged
void OnOpticalCalibrationChanged (optical_calibration* cal, void *data)      
{
	Microscope* microscope = (Microscope*) data;

	microscope_setup_optical_calibration(microscope);
}

int microscope_add_init_device(Microscope* microscope, char* name)
{
	int err;
	
	if (microscope == NULL)
	  return MICROSCOPE_ERROR;
	  
	if (name == NULL)
	  return MICROSCOPE_ERROR;

	err = InsertTreeItem (microscope->_init_panel, INIT_HW_PL_INIT_MSG, VAL_SIBLING, 0, VAL_NEXT, name, 0, 0, NULL);

	ProcessDrawEvents();

	return MICROSCOPE_SUCCESS;
}

#ifdef USE_WEB_SERVER 

static void __stdcall
test_error(struct mg_connection *conn, const struct mg_request_info *ri,
		void *user_data)
{
	mg_printf(conn, "HTTP/1.1 %d XX\r\nConntection: close\r\n\r\n", ri->status_code);
	mg_printf(conn, "Error: [%d]", ri->status_code);
}

static void __stdcall microscope_get_json_experiment_info(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	int timelapse_active = 0, timelapse_has_run = 0, regionscan_active = 0, regionscan_has_run = 0;
	double uptime, regionscan_percentage_complete = 0.0;
	char microscope_name[UIMODULE_NAME_LEN] = "", time[200] = "";
	char regionscan_start_time[200] = "", regionscan_end_time[200] = "Not completed";
	char timelapse_start_time[200] = "", timelapse_end_time[200] = "Not completed";
	char json_buffer[5000] = "";
	Microscope *ms = microscope_get_microscope();

	ui_module_get_name(UIMODULE_CAST(ms), microscope_name);
	microscope_get_uptime(ms, &uptime);
	seconds_to_friendly_time(uptime, time);

	if (ms->_tl != NULL)
		timelapse_status(ms->_tl, &timelapse_has_run, &timelapse_active, timelapse_start_time, timelapse_end_time);
	if (ms->_rs != NULL)
		regionscan_status(ms->_rs, &regionscan_has_run, &regionscan_active, &regionscan_percentage_complete, regionscan_start_time, regionscan_end_time);

    mg_printf(conn, "HTTP/1.1 200 OK\r\n\r\n");

	sprintf(json_buffer, "{\"microscope_name\":\"%s\",\"microscope_uptime\":\"%s\","
					"\"timelapse\":{\"has_run\":\"%d\",\"active\":\"%d\",\"start_time\":\"%s\",\"end_time\":\"%s\"},"
					"\"regionscan\":{\"has_run\":\"%d\",\"active\":\"%d\",\"percentage_complete\":\"%f\",\"start_time\":\"%s\",\"end_time\":\"%s\"}}"	
					, microscope_name, time,
					timelapse_has_run, timelapse_active, timelapse_start_time, timelapse_end_time,
					regionscan_has_run, regionscan_active, regionscan_percentage_complete, regionscan_start_time, regionscan_end_time);

	mg_printf(conn, json_buffer);
} 

static void WINAPI
microscope_get_json_log_info(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	int file_size = 0, number_of_lines = 20;
	size_t size;
	double uptime;
	char microscope_name[UIMODULE_NAME_LEN] = "", time[200] = "";
	char log_filepath[500] = "";
	char temp_filepath[500] = "";
	char *buffer = NULL, *json_buffer = NULL;
	FILE *fp = NULL;
	Microscope *ms = microscope_get_microscope();

	ui_module_get_name(UIMODULE_CAST(ms), microscope_name);
	microscope_get_uptime(ms, &uptime);
	seconds_to_friendly_time(uptime, time);

    mg_printf(conn, "HTTP/1.1 200 OK\r\n\r\n");

	logger_get_log_filepath(UIMODULE_LOGGER(ms), log_filepath);

	if(!FileExists(log_filepath, &file_size)) 
		return;

	fp = fopen(log_filepath, "r");

	if(fp == NULL)
		return;

	size = sizeof(char) * 1000 * number_of_lines;

	buffer = (char *) malloc(size);
	memset(buffer, 0, size);

	json_buffer = (char *) malloc(size + 100);
	memset(json_buffer, 0, size + 100);

	gettail_into_array(fp, number_of_lines, 1000, buffer);

	str_remove_char(buffer, '\r');
	str_change_char(buffer, '\n', '#');

	sprintf(json_buffer, "{\"microscope_log\":\"%s\"}", buffer);

	mg_printf(conn, json_buffer);

	free(buffer);
	free(json_buffer);
	fclose(fp);

	return;
} 

void start_web_server(Microscope *microscope)
{
	char buffer[500] = "";
	struct mg_context *ctx;

	if ((ctx = mg_start()) == NULL) {
		(void) printf("%s\n", "Cannot initialize Mongoose context");
		exit(EXIT_FAILURE);
	}

	if(get_device_string_param_from_ini_file("WebServer", "Root", buffer) == 0)  
		mg_set_option(ctx, "root", buffer);

	if(get_device_string_param_from_ini_file("WebServer", "Port", buffer) == 0)  
		mg_set_option(ctx, "ports", buffer);

	mg_bind_to_error_code(ctx, 404, &test_error, NULL);
	mg_bind_to_error_code(ctx, 0, &test_error, NULL);

	mg_bind_to_uri(ctx, "/microscope_json_data", microscope_get_json_experiment_info, NULL); 
	mg_bind_to_uri(ctx, "/microscope_json_log_data", microscope_get_json_log_info, NULL); 
}
#endif

int microscope_add_init_device_status(Microscope* microscope, char* name, int status)
{
	int item, err;

	err = GetTreeItemFromLabel (microscope->_init_panel, INIT_HW_PL_INIT_MSG, VAL_SIBLING, 0, 0, VAL_NEXT_PLUS_SELF, 0, name, &item);

    if(item < 0)
      return MICROSCOPE_ERROR;
  
	if(status == MICROSCOPE_INIT_STATUS_SUCCESS) {
		err = SetTreeCellAttribute (microscope->_init_panel, INIT_HW_PL_INIT_MSG, item, 1, ATTR_LABEL_TEXT, "Success");
		SetTreeCellAttribute (microscope->_init_panel, INIT_HW_PL_INIT_MSG, item, 1, ATTR_LABEL_BOLD, 1);
	}
	else {
		err = SetTreeCellAttribute (microscope->_init_panel, INIT_HW_PL_INIT_MSG, item, 1, ATTR_LABEL_TEXT, "Failed");
		SetTreeCellAttribute (microscope->_init_panel, INIT_HW_PL_INIT_MSG, item, 1, ATTR_LABEL_COLOR, VAL_RED);
	}

	ProcessDrawEvents();

	return MICROSCOPE_SUCCESS;
}

static void RealtimeOverview_ImageWindowCloseEventHandler( GCIWindow *win, void* callback_data )
{
	Microscope* microscope = (Microscope*) callback_data;
	int menu_bar = GetPanelMenuBar (microscope->_main_ui_panel); 

	realtime_overview_deactivate(microscope->_rto);
	realtime_overview_hide(microscope->_rto);
	UnCheckMenuPathItem(menu_bar, "Components//Realtime overview");
	
	return;
}

int microscope_initilise_devices(Microscope* microscope)
{
	microscope->_init_panel = ui_module_add_panel(UIMODULE_CAST(microscope), "MicroscopeUI.uir", INIT_HW_PL, 0);  

	ui_module_display_panel_without_activation(UIMODULE_CAST(microscope), microscope->_init_panel);

	// Save the position for next time
	ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(microscope), microscope->_init_panel, 1);

	if(microscope->lpVtbl.initialise_hardware != NULL)
		microscope->lpVtbl.initialise_hardware(microscope);

	microscope->_ri = ref_images_new(microscope);   
	microscope->_optical_cal = optical_calibration_new(microscope);

	#ifdef MICROSCOPE_PYTHON_AUTOMATION
	microscope->_tl = timelapse_new(microscope);
	#endif // MICROSCOPE_PYTHON_AUTOMATION

	microscope->_rs = region_scan_new(); 

	microscope->_sw_af = sw_autofocus_new();    
	microscope->_wpd = well_plate_definer_new(microscope);   
	microscope->_cf = cell_finder_new();
	microscope->_rto = realtime_overview_new();

	realtime_overview_set_close_handler(microscope->_rto, RealtimeOverview_ImageWindowCloseEventHandler, microscope);

	Delay(2.0);	// Give the user a couple of seconds to see the panel.
	ui_module_destroy_panel(UIMODULE_CAST(microscope), microscope->_init_panel);

	return MICROSCOPE_SUCCESS;
}

static int microscope_setup_camera_callbacks(Microscope* microscope)
{
	int i;

	// Set all cameras to snap mode.
	for(i=0; i < microscope->_number_of_cameras; i++)
	{
		GciCamera *cam = microscope->_cameras[i];
		
		if(cam == NULL)
			continue;

		gci_camera_set_data_dir(cam, microscope->_data_dir);  

		if( gci_camera_signal_on_show_handler_connect (cam, on_camera_show_handler, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
	
		if( gci_camera_signal_on_exposure_changed_handler_connect (cam, on_camera_exposure_changed_handler, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}

		if( gci_camera_signal_on_gain_changed_handler_connect (cam, on_camera_gain_changed_handler, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
		
		if( gci_camera_signal_binning_change_handler_connect (cam, on_camera_binning_changed_handler, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
		
		if( gci_camera_signal_datamode_change_handler_connect (cam, on_camera_datamode_changed_handler, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
		
		if( gci_camera_signal_close_handler_connect (cam, on_camera_close_handler, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}

		if( gci_camera_signal_enter_live_handler_connect (cam, on_camera_live_enter, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}

		if( gci_camera_signal_exit_live_handler_connect (cam, on_camera_live_exit, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
	
		if( gci_camera_signal_enter_snap_sequence_handler_connect (cam, on_camera_enter_snapsequence_or_snapmode, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
	
		if( gci_camera_signal_enter_snap_handler_connect (cam, on_camera_enter_snapsequence_or_snapmode, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
	
		if( gci_camera_signal_pre_capture_handler_connect (cam, on_camera_pre_capture, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
	
		if( gci_camera_signal_exit_get_image_handler_connect (cam, on_camera_exit_get_image, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
		
		if( gci_camera_signal_post_capture_handler_connect (cam, on_camera_post_capture, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}

		if( gci_camera_signal_trigger_now_handler_connect (cam, on_camera_trigger_now, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
		
		if( gci_camera_signal_image_pre_display_handler_connect (cam, on_camera_window_image_pre_displayed, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}

		if( gci_camera_signal_image_post_display_handler_connect (cam, on_camera_window_image_post_displayed, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
		
		if( gci_camera_signal_image_window_close_handler_connect (cam, on_camera_imagewindow_close, microscope) == CAMERA_ERROR) {
			return MICROSCOPE_ERROR;
		}
	}

	return MICROSCOPE_SUCCESS;
}


LRESULT CALLBACK MicroscopeWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	Microscope* microscope = (Microscope *) data;
	
	switch(message) {
			
    	case LOAD_CONFIG:
    	{
			char *path = (char *) lParam;

			if(microscope->lpVtbl.microscope_load_settings != NULL)
				return microscope->lpVtbl.microscope_load_settings(microscope, path);

			return 0;
    	}


      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) microscope->_old_wndproc_func_ptr,
							hwnd, message, wParam, lParam);
}


int microscope_load_ui(Microscope* microscope)
{
	if(microscope->lpVtbl.initialise_hardware_user_interfaces != NULL)
		microscope->lpVtbl.initialise_hardware_user_interfaces(microscope);

	setupManualDevices(microscope);
	logger_show(UIMODULE_LOGGER(microscope));  

	microscope_setup_camera_callbacks(microscope);
	
	// Set the routine that save metadata so the window knows which meta data to save
	//GCI_ImagingWindow_SetSaveHandler(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window, microscope_save_metadata, microscope);
	//GCI_ImagingWindow_SetMetaDataProviderCallback(MICROSCOPE_MASTER_CAMERA(microscope)->_camera_window, microscope_on_show_metadata, microscope);

	microscope->_focus = focus_new(MICROSCOPE_MASTER_CAMERA(microscope));
	
	if(microscope->_optical_cal != NULL) {
		optical_calibration_initialise(microscope->_optical_cal);
		
		optical_calibration_signal_calibration_changed_handler_connect (microscope->_optical_cal, OnOpticalCalibrationChanged, microscope);

		// set the calibration for the first time
		OnOpticalCalibrationChanged (microscope->_optical_cal, microscope);
	}

	if(MICROSCOPE_MASTER_ZDRIVE(microscope) != NULL)
		microscope_master_z_drive_changed_handler_connect(microscope, OnMasterZDriveChanged, microscope);

		
	#ifdef BUILD_MODULE_SINGLE_RANGE_HW_DEVICE

	if (microscope->_optical_zoom != NULL)
	{
		double optical_zoom;
		single_range_hardware_device_get(SINGLE_RANGE_HW_DEVICE_CAST(microscope->_optical_zoom), &optical_zoom);
		microscope->_optical_zoom_value = 1.0; // make sure this is 1.0 to start with
		microscope_set_image_scale_by_optical_zoom(microscope, optical_zoom); 
	}

	#endif

	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) {
		if(microscope->_ri != NULL)
			ref_images_recall_images(microscope->_ri);  
	}

	SetPanelAttribute (microscope->_main_ui_panel, ATTR_TITLE, UIMODULE_GET_DESCRIPTION(microscope));     
	
	// Setup a WndProc for doing some functions in the main thread
	microscope->_old_wndproc_func_ptr = ui_module_set_window_proc(UIMODULE_CAST(microscope), microscope->_main_ui_panel, (LONG_PTR) MicroscopeWndProc);

	microscope_start_all_timers(microscope);
	
	microscope_reset_all_mode_settings_to_default(microscope);
	microscope_set_startup_mode(microscope);

	// Make sure the sensitity of the camera is 0 for cameras that support it.
	// Or the camera can be damaged even if turned off.
	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) {
		if(gci_camera_feature_enabled (MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_FEATURE_SENSITIVITY))	 {
			if(gci_camera_set_sensitivity(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_ALL_CHANNELS, 0.0) == CAMERA_ERROR) {

				GCIDialogNoButtons (0, "Camera Warning", IDI_WARNING,
					"Failed to set camera sensitivity to 0. This could damage the camera even if it is off");   
			}
		}
	}

	microscope->_startup_time = Timer();

	#ifdef USE_XML_RPC
	start_xml_rpc_server(microscope);
	#endif

	#ifdef USE_WEB_SERVER 
	start_web_server(microscope);
	#endif

	return MICROSCOPE_SUCCESS;
}

int microscope_initialise(Microscope* microscope)  
{
	microscope_initilise_devices(microscope);

	microscope_load_ui(microscope);
	
	microscope_post_initialise (microscope);

	return MICROSCOPE_ERROR;
}

void microscope_get_uptime(Microscope *microscope, double *uptime)
{
	*uptime = Timer() - microscope->_startup_time;
}

region_of_interest* microscope_get_region_of_interest(Microscope* microscope)
{
	if(microscope->_roi != NULL)
		return microscope->_roi;
	
	microscope->_roi = region_of_interest_selection_new(MICROSCOPE_MASTER_CAMERA(microscope));
	
	return microscope->_roi;
}

void microscope_configure_background_correction(Microscope* microscope)
{
	ui_module_display_all_panels(UIMODULE_CAST(microscope->_ri)); 	
}

void microscope_start_all_timers(Microscope* microscope)
{
	if(microscope->lpVtbl.start_all_timers != NULL)
		microscope->lpVtbl.start_all_timers(microscope);
}

void microscope_stop_all_timers(Microscope* microscope)
{
	if(microscope->lpVtbl.stop_all_timers != NULL)
		microscope->lpVtbl.stop_all_timers(microscope);
}

int microscope_destroy(Microscope* microscope)
{
	// Save current mode settings
	int i, number_of_devices;
	char path[GCI_MAX_PATHNAME_LEN] = "";
	char data_dir[GCI_MAX_PATHNAME_LEN] = "";
	char state_filepath[GCI_MAX_PATHNAME_LEN] = "";
	HardwareDevice *device = NULL; 

	microscope_get_filepath_for_illumination_settings(microscope, microscope->illumination_mode, path);         
	microscope_save_settings(microscope, path);

	// save manual device state
	microscope_get_data_directory(microscope, data_dir);
	sprintf(state_filepath, "%s\\manual_device_state.ini", data_dir);
	
	DeleteFile(state_filepath);

	microscope->_save_thread_running = 0;

	CmtWaitForThreadPoolFunctionCompletion (gci_thread_pool(), microscope->_save_thread_id,
           OPT_TP_PROCESS_EVENTS_WHILE_WAITING);

	CmtDiscardTSQ (microscope->_save_queue); 

	number_of_devices = hardware_device_number_of_devices();

	for(i=1; i <= number_of_devices; i++) { 
	
		device = hardware_device_at_index(i);

		if(hardware_device_is_manual(device))
			hardware_save_state_to_file (device, state_filepath, "a");
	}

	if(microscope->_rto != NULL) {
		realtime_overview_destroy(microscope->_rto);
		microscope->_rto = NULL;
	}

	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) {
		gci_camera_set_snap_mode(MICROSCOPE_MASTER_CAMERA(microscope));
		gci_camera_prevent_getting_images(MICROSCOPE_MASTER_CAMERA(microscope));
	}

	GCI_Signal_Suspend_All_Signals();
	// Lets suspend all aync callbacks.
	SuspendAsyncTimerCallbacks ();
//	DiscardAsyncTimer(-1);
	SuspendTimerCallbacks ();

	microscope_stop_all_timers(microscope);
  	
	#ifdef MICROSCOPE_PYTHON_AUTOMATION 
	Py_Finalize(); 
	#endif

	if(microscope->_rs != NULL) {
		region_scan_destroy(microscope->_rs);
		microscope->_rs = NULL;
	}

	#ifdef MICROSCOPE_PYTHON_AUTOMATION
	if(microscope->_tl != NULL) {
		timelapse_destroy(microscope->_tl); 
		microscope->_tl = NULL;
	}
	#endif // MICROSCOPE_PYTHON_AUTOMATION

	if(microscope->_sw_af != NULL) {
		sw_autofocus_destroy(microscope->_sw_af);    
		microscope->_sw_af = NULL;
	}

	if(microscope->_ri != NULL) {
		ref_images_destroy(microscope->_ri);   
		microscope->_ri = NULL;
	}
	
	if(microscope->_optical_cal != NULL) {
		optical_calibration_destroy(microscope->_optical_cal);
		microscope->_optical_cal = NULL;
	}
	
	if(microscope->_wpd != NULL) {
		well_plate_definer_destroy(microscope->_wpd);              
		microscope->_wpd = NULL;
	}
	
	feedback_destroy();     
	
	// destroy this later, things rely on it
	if (microscope->_roi != NULL) {
		region_of_interest_destroy(microscope->_roi);
		microscope->_roi = NULL;
	}
	
	CHECK_MICROSCOPE_VTABLE_PTR(microscope, destroy) 
  	
	CALL_MICROSCOPE_VTABLE_PTR(microscope, destroy) 

	if(MICROSCOPE_MASTER_CAMERA(microscope) != NULL) {
		gci_camera_destroy(MICROSCOPE_MASTER_CAMERA(microscope));
		MICROSCOPE_MASTER_CAMERA(microscope) = NULL;
	}

	if(microscope->_power_switch != NULL) {
	
		power_switch_off_all(microscope->_power_switch);            
		power_switch_destroy(microscope->_power_switch);  
		microscope->_power_switch = NULL;
	}

	ui_module_destroy(UIMODULE_CAST(microscope));
	
	ListDispose(microscope->_menu_item_list);

  	free(microscope);
	
  	return MICROSCOPE_SUCCESS;
}

int microscope_set_overlapped(Microscope* microscope, int value)
{
	CHECK_MICROSCOPE_VTABLE_PTR(microscope, set_overlapped) 
  	
	if( (*microscope->lpVtbl.set_overlapped)(microscope, value) == MICROSCOPE_ERROR ) {
		send_microscope_error_text(microscope, "set_overlapped failed");
		return MICROSCOPE_ERROR;
	}
	
	microscope->_overlapped = value;
	
  	return MICROSCOPE_SUCCESS;
}

int microscope_wait_for_device(Microscope* microscope)
{
	if (!microscope->_overlapped) return MICROSCOPE_SUCCESS;
	
	CHECK_MICROSCOPE_VTABLE_PTR(microscope, wait_for_device) 
  	
	if( (*microscope->lpVtbl.wait_for_device)(microscope) == MICROSCOPE_ERROR ) {
		send_microscope_error_text(microscope, "wait_for_device failed");
		return MICROSCOPE_ERROR;
	}
	
  	return MICROSCOPE_SUCCESS;
}

int microscope_set_com_port(Microscope* microscope, int port)
{
	microscope->_com_port = port;
	
	return MICROSCOPE_SUCCESS;  
}


void microscope_set_error_handler(Microscope* microscope, UI_MODULE_ERROR_HANDLER handler)
{
	ui_module_set_error_handler(UIMODULE_CAST(microscope), handler, microscope);
}


void microscope_disable_timer(Microscope* microscope)
{
	SetAsyncTimerAttribute (microscope->_timer, ASYNC_ATTR_ENABLED,  0);
}

void microscope_enable_timer(Microscope* microscope)
{
	SetAsyncTimerAttribute (microscope->_timer, ASYNC_ATTR_ENABLED,  1);
}

int microscope_display_main_ui(Microscope* microscope)
{
	ui_module_display_main_panel(UIMODULE_CAST(microscope)); 
	
	return MICROSCOPE_SUCCESS;
}


int microscope_hide_main_ui(Microscope* microscope)
{
	ui_module_hide_main_panel(UIMODULE_CAST(microscope));
	
	microscope_disable_timer(microscope); 

	return MICROSCOPE_SUCCESS;
}


int microscope_is_main_ui_visible(Microscope* microscope)
{
	return ui_module_main_panel_is_visible(UIMODULE_CAST(microscope));  
}


int default_microscope_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	static int panel_found = -1;
	int panel, pnl=0, ctrl=0, err = 0;
	static char buffer[500];
	double popup_start_time;
	Microscope *microscope = (Microscope*) callback_data;
	
	GciCmtGetLock(microscope->_lock);

	if(panel_found < 0) {
		find_resource("MicroscopeUI.uir", buffer); 
		panel_found = 1;	
	}

	// Make it easier to see popup.
	MinimizeAllWindows ();

	panel = LoadPanel(0, buffer, ERR_PANEL); 
	ResetTextBox (panel, ERR_PANEL_ERROR_MSG, "");

	SetCtrlVal(panel, ERR_PANEL_TEXTMSG, title);
	SetCtrlVal(panel, ERR_PANEL_ERROR_MSG, error_string);
	
	logger_log(UIMODULE_LOGGER(module), LOGGER_ERROR, (char*) error_string);  

	popup_start_time = Timer(); 
	
	GCI_MovePanelToDefaultMonitorForDialogs(panel);

	if((err = InstallPopup(panel)) < 0) {
		
		MessagePopup("Unknown Error", "Failed to create detailed error popup");
		GciCmtReleaseLock(microscope->_lock);
		return UIMODULE_ERROR_NONE;
	}
	
	while (1) {
			
		if(GetUserEvent (0, &pnl, &ctrl) < 0) {
		
			MessagePopup("Unknown Error", "Failed to get event from popup");
			GciCmtReleaseLock(microscope->_lock);
			return UIMODULE_ERROR_NONE;    
		}
		
		if((Timer() - popup_start_time) > 10.0) {
			RemovePopup(panel);  
			GciCmtReleaseLock(microscope->_lock);
			return UIMODULE_ERROR_RETRY;
		}

		if (pnl == panel) {
			
			if (ctrl == ERR_PANEL_RETRY)  {
				RemovePopup(panel);  
				GciCmtReleaseLock(microscope->_lock);
				return UIMODULE_ERROR_RETRY;
			}
		
			if (ctrl == ERR_PANEL_IGNORE)  {
				RemovePopup(panel); 
				GciCmtReleaseLock(microscope->_lock);
				return UIMODULE_ERROR_IGNORE;
			}
		}
	}
	
	GciCmtReleaseLock(microscope->_lock);

	return UIMODULE_ERROR_NONE;    
}
