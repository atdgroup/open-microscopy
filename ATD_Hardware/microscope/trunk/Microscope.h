#ifndef __MICROSCOPE__
#define __MICROSCOPE__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#include "HardWareTypes.h"      

#define MICROSCOPE_SAVE_IMAGE_QUEUE_SIZE 10

#ifdef BUILD_MODULE_SINGLE_RANGE_HW_DEVICE
#include "single_range_hardware_device.h"
#endif

#include "gci_ui_module.h"  
#include "icsviewer_window.h"
#include "icsviewer_signals.h" 

#ifdef MICROSCOPE_PYTHON_AUTOMATION
#include "pyconfig.h"
#include "Python.h"
#include "gci_python_wrappers.h" 
#include "microscope_python_wrappers.h"
#endif

// Experiments
#include "cell_finding.h"
#include "AutoFocus.h" 
#include "RegionOfInterest.h"    
#include "timelapse.h"
#include "RegionScan.h" 
#include "StageScan.h" 
//#include "WellPlateDefiner.h"
#include "realtime_overview.h"

#include "camera\gci_camera.h" 
#include "focus.h"    
#include "background_correction.h"
#include "optical_calibration.h"
#include "AutoFocus.h" 
#include "ZDrive.h"

#include "utility.h"

#define MAX_ALLOWED_CAMERAS 5

#define MICROSCOPE_SUCCESS 0
#define MICROSCOPE_ERROR -1

#define MICROSCOPE_INIT_STATUS_SUCCESS 1
#define MICROSCOPE_INIT_STATUS_FAILED 0

#define MICROSCOPE_VTABLE_PTR(ob, member) ((ob)->lpVtbl.member)
#define MICROSCOPE_VTABLE(ob, member) (*((ob)->lpVtbl.member))

#define CHECK_MICROSCOPE_VTABLE_PTR(ob, member) if(MICROSCOPE_VTABLE_PTR(ob, member) == NULL) { \
    send_microscope_error_text(ob, "member not implemented"); \
    return MICROSCOPE_ERROR; \
}  
 
#define CALL_MICROSCOPE_VTABLE_PTR(ob, member) if( MICROSCOPE_VTABLE(ob, member)(ob) == MICROSCOPE_ERROR ) { \
	send_microscope_error_text(ob, "member failed");  \
	return MICROSCOPE_ERROR; \
}


#define MICROSCOPE_MASTER_ZDRIVE(ms) ((ms)->_master_z_drive)
#define MICROSCOPE_MASTER_CAMERA(ms) ((ms)->_master_camera)

typedef enum {FIRMWARE=1, SOFTWARE, NO_INTERLOCK} Interlock;  

typedef enum 
{
    MICROSCOPE_UNDEFINED = -1, 
    MICROSCOPE_FLUORESCENCE = 0, 
    MICROSCOPE_BRIGHT_FIELD = 1, 
    MICROSCOPE_LASER_SCANNING = 2,
	MICROSCOPE_PHASE_CONTRAST = 3, 
	MICROSCOPE_FLUOR_NO_SHUTTER = 4 

} MicroscopeIlluminationMode; 

typedef struct
{
	int (*destroy) (Microscope* microscope);
	int (*set_overlapped) (Microscope* microscope, int value);
	int (*wait_for_device) (Microscope* microscope);

	int (*initialise_hardware_user_interfaces) (Microscope* microscope); 	
	int (*initialise_hardware) (Microscope* microscope);
    int (*set_focusing_mode) (Microscope* microscope);
    int (*set_hi_resolution_mode) (Microscope* microscope);
    
	int (*microscope_post_initialise) (Microscope* microscope);
	int (*microscope_get_detection_device_count) (Microscope* microscope);
	int (*microscope_get_detection_device_names) (Microscope* microscope, const char** path);
	int (*microscope_save_settings) (Microscope* microscope, const char* path);
	int (*microscope_load_settings) (Microscope* microscope, const char* path);
	int (*switch_illumination_mode) (Microscope* microscope, MicroscopeIlluminationMode mode, MicroscopeIlluminationMode old_mode, int save_old_settings);  
		
	int (*save_image_metadata) (IcsViewerWindow *window, char *filename, char *extension, void* callback); 
	int (*on_camera_exposure_changed) (Microscope* microscope);    
	int (*on_camera_gain_changed) (Microscope* microscope);    
	int (*on_camera_binning_changed) (Microscope* microscope); 
	int (*on_camera_datamode_changed) (Microscope* microscope); 
    int (*on_camera_live_enter) (Microscope* microscope);
    int (*on_camera_live_exit) (Microscope* microscope);
    int (*on_camera_enter_snap_sequence_or_snapmode) (Microscope* microscope);
    int (*on_camera_pre_capture) (Microscope* microscope);
    int (*on_camera_post_capture) (Microscope* microscope);
	int (*on_camera_trigger_now) (Microscope* microscope);  
	int (*on_camera_exit_get_image) (Microscope* microscope, FIBITMAP** dib);     

	// Called when the objectives have started to changed and
	// have finished changing.
	void (*on_objective_changed_start)(ObjectiveManager* manager, int pos, void *data); 
	void (*on_objective_changed_end)(ObjectiveManager* manager, int pos, void *data);  
	
	void (*start_all_timers) (Microscope* microscope);  
	void (*stop_all_timers) (Microscope* microscope); 
	
	int  (*disable_all_panels) (Microscope* microscope, int disable);
								   
} MicroscopeVtbl;

struct _Microscope
{
  UIModule parent;   
	
  MicroscopeVtbl lpVtbl;
 
  focus* 	 _focus;
	  
  LONG_PTR   _old_wndproc_func_ptr;
  int	 	 _com_port;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int		 _overlapped;
  int		 _perform_full_init;
  int		 _autosnap;
  int		 _ics_compression_level;
  int        _init_panel;
  int		 _username_splash_panel;
  int		 _manual_devices_splash_panel;
  int		 _disable_metadata_on_snap;
  int		 _save_thread_running;
  int		 _save_thread_id;
  int		 _save_queue;		// Thread safe queue		

  double	 _init_timeout;

  volatile int  _requested_mode_cube_position;
  volatile int  _requested_mode_op_position;

  int		 _objective_z_adjust;
  int		 _objective_z_link;
  int		 _objective_interlock;
  int		 _prevent_automatic_background_correction;
  double	 _stored_stage_x;
  double	 _stored_stage_y;
  double	 _stored_focus_z;
  double     _optical_zoom_value;
  int		_change_camera_palette;
  
  double	 _startup_time;
  char		_data_dir[GCI_MAX_PATHNAME_LEN];
  char		_user_data_dir[GCI_MAX_PATHNAME_LEN];
  char		_user_name[256];
  char		_study[256];
  char		_experiment[256];

  ListType	_menu_item_list;
  MicroscopeIlluminationMode illumination_mode;
  
  signal_table signal_table;
  
  Analyzer*  			      _analyzer;
  ObjectiveManager* 	      _objective;
  FluoCubeManager*  	      _fluor_cube;
  OpticalPathManager* 	      _optical_path;
  CondenserManager* 	      _condenser;
  Lamp* 				      _lamp;
  Shutter* 	 			      _shutter;
  SingleRangeHardwareDevice*  _aperture_stop;
  SingleRangeHardwareDevice*  _field_stop;
  SingleRangeHardwareDevice*  _epi_field_stop;
  SingleRangeHardwareDevice*  _optical_zoom;
  StagePlateModule*		      _stage_plate_module;
  XYStage* 	 			      _stage;
  Intensilight*			      _intensilight;
  Scanner* 	 			      _scanner; 
  AutofocusCtrl* 		      _autofocusCtrl;
  Z_Drive*				      _master_z_drive; 
  Z_Drive*				      _z_drive;
  CoarseZDrive*				  _coarse_zdrive;
  TemperatureMonitor*	      _temperature_monitor;
  OpticalLift*			      _optical_lift;
  PowerSwitch*			      _power_switch;   
  realtime_overview*	      _rto;
  region_of_interest*	      _roi;
  BatchCounterA1*		      _batch_counter;
  PmtSet*				      _pmt_set;
  FilterSetCollection*	      _filter_set;
  Laser*				      _laser;
  precisExcite*	              _precise_excite;
  LaserPowerMonitor*		  _laser_power_monitor;
  RobotInterface*			  _robot_interface;
  
  GciCamera*			      _cameras[MAX_ALLOWED_CAMERAS];
  GciCamera*			      _master_camera;
  int						  _number_of_cameras;

  OpticalCalibrationDevice*	  _current_optical_calibration_device;

  // Background correction module
  ref_images* 			_ri;
  optical_calibration*  _optical_cal;
  
  // Experiments
  timelapse*			_tl;
  region_scan*			_rs;
  stage_scan*		    _ss;
  well_plate_definer*	_wpd;
  cell_finder*			_cf;
  SWAutoFocus*			_sw_af;
  Spc* 					_spc;

  Logger* 				_logger; 
};


void start_xml_rpc_server(Microscope *microscope);

void microscope_constructor(Microscope* microscope, char *name, char *description);

void microscope_add_camera(Microscope* microscope, GciCamera *camera);
void microscope_switch_current_optical_calibration_device(Microscope* microscope, OpticalCalibrationDevice *device);
void microscope_switch_camera_to_master(Microscope* microscope, GciCamera *camera);

int microscope_add_menuitem(Microscope *microscope, char *path, 
	int shortCutKey, MenuCallbackPtr eventFunction, void *callback_data);

void microscope_dim_menuitems(Microscope *microscope, int dim);
void microscope_dim_menubar(Microscope *microscope, int dim);

Microscope* microscope_get_microscope(void);
void microscope_get_uptime(Microscope *microscope, double *uptime);
void microscope_disable_metadata_on_snap(Microscope *microscope, int val);

void microscope_error_handler (const char *title, const char *error_string, void *callback_data);

int microscope_add_init_device(Microscope* microscope, char* name);
int microscope_add_init_device_status(Microscope* microscope, char* name, int status);
int microscope_initilise_devices(Microscope* microscope);
int microscope_initialise(Microscope* microscope);
int microscope_load_ui(Microscope* microscope);

int microscope_set_data_directory(Microscope* microscope, const char *directory);
int microscope_set_user_data_directory(Microscope* microscope, const char *directory); 
void microscope_get_safe_z_position(Microscope *microscope, int *pos);

int microscope_get_temp_directory(char *path);
int microscope_get_data_directory(Microscope* microscope, char *path);
int microscope_get_user_data_directory(Microscope* microscope, char *path); 
int microscope_get_data_subdirectory(Microscope* microscope, const char *subdirectory_name, char *path);
int microscope_get_user_data_subdirectory(Microscope* microscope, const char *subdirectory_name, char *path);     

void microscope_update_display_if_live(Microscope *microscope);
void microscope_update_display(Microscope *microscope);

int  send_microscope_error_text (Microscope* microscope, char fmt[], ...);

void microscope_set_error_handler(Microscope* microscope, UI_MODULE_ERROR_HANDLER handler);

MicroscopeIlluminationMode microscope_get_illumination_mode (Microscope* microscope);

int  microscope_reset_all_mode_settings_to_default(Microscope* microscope);

int  microscope_post_initialise (Microscope* microscope);
int  microscope_save_settings (Microscope* microscope, const char* path);
int  microscope_load_settings (Microscope* microscope, const char* path);
int  microscope_switch_illumination_mode (Microscope* microscope, MicroscopeIlluminationMode mode, int save_old_settings); 
int  microscope_set_illumination_mode (Microscope* microscope, MicroscopeIlluminationMode mode); 

int  microscope_disable_all_panels(Microscope* microscope, int disable);

int  microscope_set_focusing_mode (Microscope* microscope);
int  microscope_set_hi_resolution_mode (Microscope* microscope);
	
void microscope_start_all_timers(Microscope* microscope);
void microscope_stop_all_timers(Microscope* microscope);

void microscope_setup_optical_calibration(Microscope *microscope);

int  microscope_set_com_port(Microscope* microscope, int port);

int  microscope_destroy(Microscope* microscope);
int  microscope_set_overlapped(Microscope* microscope, int value);
int  microscope_wait_for_device(Microscope* microscope);

int  microscope_display_main_ui (Microscope* microscope);
int  microscope_hide_main_ui (Microscope* microscope);
int  microscope_is_main_ui_visible (Microscope* microscope);
int  microscope_set_optical_path(Microscope* microscope);
int  microscope_set_objectives_zdrive_link(Microscope* microscope, int link); 
int microscope_saveimage(Microscope* microscope, FIBITMAP *dib, char *filepath);
void microscope_get_filepath_for_illumination_settings(Microscope* microscope, MicroscopeIlluminationMode mode, char *path);
void microscope_get_filepath_for_default_illumination_settings(Microscope* microscope, MicroscopeIlluminationMode mode, char *path);
void microscope_update_display_if_live(Microscope *microscope);
void microscope_update_display(Microscope *microscope);
dictionary* microscope_get_metadata(Microscope* ms, IcsViewerWindow* window);
dictionary* microscope_get_camera_image_metadata(Microscope* ms, IcsViewerWindow* window);
dictionary* microscope_get_flim_image_metadata(Microscope* ms, IcsViewerWindow* window);
void microscope_add_microscope_metadata(Microscope* ms, dictionary *d);    
void microscope_save_metadata(GCIWindow *window, char *filename, char *extension, void* callback);
void microscope_save_metadata_to_text_file(GCIWindow *window, char *filename);
void microscope_on_show_metadata (IcsViewerWindow* window, int panel, int ctrl, void* callback);
int microscope_get_detection_device_count (Microscope* microscope);
int microscope_get_detection_device_names (Microscope* microscope, const char** path);
void microscope_add_hardware_metadata(Microscope* ms, dictionary *d);

//Access to devices. Any which aren't present will return NULL
Analyzer* microscope_get_analyser(Microscope* microscope);
ObjectiveManager* microscope_get_objective(Microscope* microscope);
FluoCubeManager* microscope_get_cube_manager(Microscope* microscope);
CondenserManager* microscope_get_condenser(Microscope* microscope);
Lamp* microscope_get_lamp(Microscope* microscope);
Shutter* microscope_get_shutter(Microscope* microscope);
SingleRangeHardwareDevice* microscope_get_aperture_stop(Microscope* microscope);
SingleRangeHardwareDevice* microscope_get_field_stop(Microscope* microscope);
SingleRangeHardwareDevice* microscope_get_epi_field_stop(Microscope* microscope);
SingleRangeHardwareDevice* microscope_get_optical_zoom(Microscope* microscope);
OpticalPathManager* microscope_get_optical_path_manager(Microscope* microscope); 
optical_calibration* microscope_get_optical_calibration(Microscope* microscope);      
XYStage* microscope_get_stage(Microscope* microscope);
Z_Drive* microscope_get_master_zdrive(Microscope* microscope);
Lamp* microscope_get_lamp(Microscope* microscope);
Scanner* microscope_get_scanner(Microscope* microscope);
FilterSetCollection* microscope_get_filter_set(Microscope* microscope);
AutofocusCtrl* microscope_get_autofocus_ctrl(Microscope* microscope);
precisExcite* microscope_get_precise_excite(Microscope* microscope);
Intensilight* microscope_get_intensilight(Microscope* microscope);
TemperatureMonitor* microscope_get_temperature_monitor(Microscope* microscope);
OpticalLift* microscope_get_optical_lift(Microscope* microscope);
StagePlateModule* microscope_get_stage_plate_module(Microscope* microscope);
realtime_overview* microscope_get_realtime_overview(Microscope* microscope);

void microscope_set_image_scale_by_optical_zoom (Microscope* microscope, double new_value);

GciCamera* microscope_get_camera(Microscope* microscope);

int microscope_set_master_zdrive(Microscope* microscope, Z_Drive *zd);

void microscope_freeimage_error_handler(FREE_IMAGE_FORMAT fif, const char *msg);

timelapse* microscope_get_timelapse(Microscope* microscope); 
cell_finder* microscope_get_cellfinder(Microscope* microscope);
ref_images* microscope_get_background_correction(Microscope* microscope);
region_scan* microscope_get_region_scan(Microscope* microscope); 
stage_scan* microscope_get_stage_scan(Microscope* microscope); 
BatchCounterA1* microscope_get_batchcounter(Microscope* microscope); 
Spc* microscope_get_spc(Microscope* microscope);
region_of_interest* microscope_get_region_of_interest(Microscope* microscope);	

void microscope_focus_username_panel(Microscope *ms);

void microscope_configure_background_correction(Microscope* microscope);

void microscope_prevent_automatic_background_correction(Microscope* microscope);
void microscope_allow_automatic_background_correction(Microscope* microscope);
int microscope_is_automatic_background_correction_disabled(Microscope* microscope);
FIBITMAP* perform_backround_correction_on_image(Microscope* microscope, FIBITMAP *dib);

// Signals
typedef void (*MICROSCOPE_EVENT_HANDLER) (Microscope* microscope, void *data); 
typedef void (*MICROSCOPE_PANEL_DIMMED_HANDLER) (Microscope* microscope, int state, void *data);
typedef void (*MICROSCOPE_ZAXIS_OBJECTIVE_LINK_EVENT_HANDLER) (Microscope* microscope, int state, void *data); 

int microscope_master_camera_changed_handler_connect (Microscope* microscope, MICROSCOPE_EVENT_HANDLER *handler, void *callback_data);
int microscope_master_camera_changed_handler_disconnect(Microscope* microscope, int id);
int microscope_current_optical_calibration_device_changed_handler_connect(Microscope* microscope, void *handler, void *data);
int microscope_signal_hide_handler_connect (Microscope* microscope, MICROSCOPE_EVENT_HANDLER handler, void *callback_data);
int microscope_changed_handler_connect(Microscope* microscope, MICROSCOPE_EVENT_HANDLER handler, void *data );
int microscope_master_z_drive_changed_handler_connect(Microscope* microscope, void *handler, void *data );
int microscope_master_z_drive_changed_handler_disconnect(Microscope* microscope, int id );
int microscope_dimmed_handler_connect(Microscope* microscope, MICROSCOPE_PANEL_DIMMED_HANDLER handler, void *data );

int default_microscope_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data);

void microscope_zaxis_objective_link_on_changed_handler(Microscope* microscope,
	MICROSCOPE_ZAXIS_OBJECTIVE_LINK_EVENT_HANDLER handler, void *data);

int MENU_SETUP_PRECISEXCITE;
void CVICALLBACK cbMicroscopePrecisExcite (int menuBar, int menuItem, void *callbackData, int panel);
int MENU_SETUP_INTENSILIGHT;
void CVICALLBACK cbMicroscopeIntensilight (int menuBar, int menuItem, void *callbackData, int panel);

#ifdef __cplusplus
}	// extern "C" 
#endif

#endif

 
