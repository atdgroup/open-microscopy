#ifndef __GCI_CAMERA__
#define __GCI_CAMERA__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#include "HardWareTypes.h"  
#include "OpticalCalibrationDevice.h" 
#include "icsviewer_window.h"
#include "gci_ui_module.h" 
#include "signals.h"
#include "dictionary.h"
#include "focus.h"

#include "Freeimage.h"

#define CAMERA_SUCCESS 0
#define CAMERA_ERROR -1

#define CAMERA_CAST(obj) ((GciCamera*) obj)
#define CAMERA_VTABLE_PTR(ob) (CAMERA_CAST(ob)->_cops)

#define CAMERA_WINDOW(camera) (CAMERA_CAST(camera)->_camera_window)

typedef enum {NO_BINNING=1, BINNING2X2, BINNING3X3, BINNING4X4, BINNING5X5, BINNING6X6, BINNING7X7, BINNING8X8} BinningMode;

typedef enum {BPP8 = 8, BPP10 = 10, BPP12 = 12, BPP14 = 14, BPP16 = 16, BPP24 = 24, BPP32 = 32} DataMode;

typedef enum {LIVE = 1, SNAP, SNAP_SEQUENCE} AquireMode;

typedef enum {MONO_TYPE = 1, RGB_TYPE} ColourType;

typedef enum {LIGHT_MODE_LO = 0, LIGHT_MODE_HI = 1} LightMode;

typedef enum {CAMERA_NO_TRIG, CAMERA_INTERNAL_TRIG, CAMERA_EXTERNAL_TRIG} TriggerMode;

typedef enum {CAMERA_ALL_CHANNELS=0, CAMERA_CHANNEL1=1, CAMERA_CHANNEL2=2} CameraChannel;       

typedef enum {CCD_MODE_NORMAL=1, CCD_MODE_EM = 2} CCDMode;

typedef enum {SW_CUSTOM,
					 SW_QUARTER_SQUARE,
					 SW_HALF_SQUARE,
					 SW_THREE_QUARTER_SQUARE,
					 SW_FULL_SQUARE,
					 SW_FULL_IMAGE} SUBWINDOW_SIZE;

typedef enum {
	CAMERA_FEATURE_BLACKLEVEL=0,
	CAMERA_FEATURE_SENSITIVITY,
	CAMERA_FEATURE_DATAMODE,
	CAMERA_FEATURE_BINNING,
	CAMERA_FEATURE_GAIN,
	CAMERA_FEATURE_LIGHTMODE,
	CAMERA_FEATURE_PHOTON_MODE,
	CAMERA_FEATURE_TEMP,
	CAMERA_FEATURE_CCD_MODE,
	CAMERA_FEATURE_EMGAINPROTECT,
	CAMERA_FEATURE_SUBTRACT,
	
	// Aways comes last !
	CAMERA_FEATURE_COUNT
	
} CameraFeature; 

//parameter
typedef enum
{
	CAM_RES_2048_1536,
	CAM_RES_1024_768,
	CAM_RES_640_480,
	CAM_RES_512_384,
	CAM_RES_1280_1024_ROI

} CameraResolution;

typedef enum {CHANNEL_RED = 0, CHANNEL_GREEN, CHANNEL_BLUE} CAMERA_CHANNEL_COLOUR;

typedef struct _CameraState
{
	BinningMode binning;
	DataMode data_mode;
	int	live;
	int bpp;
	int dual_tap;
	double exposure_time;
	double blacklevel1;
	double blacklevel2;
	double sensitivity;
	double gain1;
	double gain2;
	int light_mode;
	int photon_mode;
	TriggerMode trigger_mode;
	CameraResolution resolution;

} CameraState;


/* Function pointers used as virtual functions */
struct camera_operations
{
	int (*power_up) (GciCamera* camera);
	int (*power_down) (GciCamera* camera);
	
	// Don't put camera functions in init (error_handler is not yet set)
	int (*initialise) (GciCamera* camera);
	int (*destroy) (GciCamera* camera);
	int (*get_colour_type) (GciCamera* camera);
	int (*set_exposure_time) (GciCamera* camera, double exposure, double *actual_exposure);
	int (*set_datamode) (GciCamera* camera, DataMode data_mode); 
	int (*get_highest_datamode) (GciCamera* camera, DataMode *data_mode); 
	int (*get_lowest_datamode) (GciCamera* camera, DataMode *data_mode); 
	int (*get_highest_binning_mode) (GciCamera* camera, BinningMode *binning);
	int (*set_gain) (GciCamera* camera, CameraChannel channel, double gain);
	int (*get_gain) (GciCamera* camera, CameraChannel channel, double *gain);  
	int (*set_live_mode) (GciCamera* camera);
	int (*attempt_recovery) (GciCamera* camera);
	int (*set_snap_mode) (GciCamera* camera);
	int (*set_snap_sequence_mode) (GciCamera* camera);

	int (*supports_fast_mode) (GciCamera* camera);
	// This is used for focusing etc.
	int (*set_fast_mode) (GciCamera* camera, int enable);

	int (*supports_focal_indicator_settings) (GciCamera *camera);
	FocusSettings (*get_focal_indicator_settings) (GciCamera* camera);

	int (*set_size_position) (GciCamera* camera, unsigned int left, unsigned int top,
		unsigned int width, unsigned int height, unsigned char auto_centre);

	int (*get_size_position) (GciCamera* camera, unsigned int *left, unsigned int *top,
		unsigned int *width, unsigned int *height, unsigned char *auto_centre);

	int (*get_info) (GciCamera* camera, char *vendor, char *model, char *bus,
		char *camera_id, char *camera_version, char *driver_version); 
	
	int (*save_settings) (GciCamera* camera, const char *filepath, const char *mode);
	int (*load_settings) (GciCamera* camera, const char *filepath);
	int (*save_settings_as_default) (GciCamera* camera);
	
	int (*set_default_settings) (GciCamera* camera); 
	
	int (*get_state_as_ini_string) (GciCamera* camera, char *buffer);       
	
	int (*save_state) (GciCamera* camera, CameraState *state);
	int (*restore_state) (GciCamera* camera, CameraState *state);
	
	int (*supports_binning) (GciCamera* camera);
	
	int (*set_binning_mode) (GciCamera* camera, BinningMode binning);
	BinningMode (*get_binning_mode) (GciCamera* camera);
	
	int (*set_blacklevel) (GciCamera* camera, CameraChannel channel, double blacklevel);
	int (*get_blacklevel) (GciCamera* camera, CameraChannel channel, double *blacklevel);
	
	int (*set_sensitivity) (GciCamera* camera, CameraChannel channel, double sensitivity);
	int (*get_sensitivity) (GciCamera* camera, CameraChannel channel, double *sensitivity);

	int (*set_ccd_mode) (GciCamera* camera, CCDMode mode);

	int (*set_lightmode) (GciCamera* camera, LightMode light_mode); 
	int (*get_lightmode) (GciCamera* camera); 

	int (*set_trigger_mode) (GciCamera* camera, TriggerMode trig_mode);
	int (*fire_internal_trigger) (GciCamera* camera);

	int (*is_dual_channel_mode) (GciCamera* camera);              

	int (*on_error) (GciCamera* camera);

	double (*get_fps) (GciCamera* camera); 

	int (*set_subwindow) (GciCamera* camera, SUBWINDOW_SIZE size);
	
	FIBITMAP* (*get_image) (GciCamera* camera, const Rect *rect);
	FIBITMAP** (*get_images) (GciCamera* camera, int number_of_images);      
	FIBITMAP* (*get_image_average) (GciCamera* camera, int frames);
};

struct _GciCamera
{
	  OpticalCalibrationDevice parent; 
	  
	  struct camera_operations _cops;

	  LONG_PTR	uimodule_func_ptr;
	  HWND window_hwnd;
	  GCIWindow *_camera_window;
	  dictionary* _metadata;
	  unsigned int features[20];
	  DataMode supported_datamodes[10]; 
	  int number_of_datamodes;
	  unsigned int features_set;
	  unsigned int number_of_binning_modes;
	  BinningMode supported_binning_modes[10];
	  unsigned int _colour_type;
	  DataMode _data_mode;
	  Rect last_subwindow_rect;
	  unsigned int _aquire_mode;
	  unsigned int _min_width;   
	  unsigned int _min_height;   
	  unsigned int _max_width;   
	  unsigned int _max_height; 
	  int _live_thread;
	  int _default_ui;
	  int _horz_flip;
	  int _vert_flip;
	  double _rotation_angle;
	  int _main_ui_panel;
	  int _extra_ui_panel;
	  int _powered_up;
	  int _average_frames;
	  int _fps_supported;
	  int _last_image_average_count;
	  double _fps;
	  double _exposure_time;
	  double _gain;
	  double _min_gain;
	  double _max_gain;
	  double _min_sensitivity;
	  double _max_sensitivity;
	  char _min_gain_text[20];
	  char _max_gain_text[20];
	  int _gain_data_type_precision;
	  double _min_bl;
	  double _max_bl;
	  char _min_bl_text[20];
	  char _max_bl_text[20];
	  double _microns_per_pixel;
	  char _data_dir[GCI_MAX_PATHNAME_LEN];
	  int  _bits;
	  int  _trigger_mode;
	  int  _bad_frame_count;
	  int _lock;
	  int _grabbed_image_lock;
	  int _initialised;
	  int _has_two_channels;
	  volatile int _live_thread_suspend;
	  volatile int _live_thread_suspended;
	  volatile int _exit_thread;
	  volatile int _live_thread_exited;
	  volatile int _prevent_getting_images;
	  int _no_display;
	  int optical_calibration_device_profile_handler_id;

	  signal_table signal_table;

	  FIBITMAP *grabbed_dib;  
};


/* Functions which operate upon seperate camera instances */
GciCamera* gci_camera_new(void);
int gci_camera_constructor (GciCamera* camera, const char *name, const char* description);
void gci_camera_activate_grab_timer(GciCamera* camera);
void gci_camera_deactivate_grab_timer(GciCamera* camera);
int gci_camera_set_feature (GciCamera* camera, CameraFeature feature);
int gci_camera_feature_enabled (GciCamera* camera, CameraFeature feature);
int  send_error_text (GciCamera* camera, char fmt[], ...);
int gci_camera_set_allowed_bitmode (GciCamera* camera, DataMode mode); 
int gci_camera_set_allowed_binningmode (GciCamera* camera, BinningMode mode);                     
int gci_camera_perform_error_cleanup(GciCamera* camera);

int gci_camera_get_number_of_binning_modes (GciCamera* camera);
BinningMode* gci_camera_get_allowed_binning_modes (GciCamera* camera);

void gci_camera_get_lock(GciCamera* camera);
void gci_camera_release_lock(GciCamera* camera);
int  gci_camera_initialise(GciCamera*); 
int  gci_camera_is_initialised(GciCamera *camera);
int  gci_camera_destroy(GciCamera*);
void gci_camera_set_rotate(GciCamera* camera, int angle);
int  gci_camera_set_data_dir(GciCamera* camera, const char *dir);
int  gci_camera_get_data_dir(GciCamera* camera, char* dir); 
int  gci_camera_set_description(GciCamera* camera, const char* description);
int  gci_camera_get_description(GciCamera* camera, char *description);
int  gci_camera_set_name(GciCamera* camera, char* name);
int  gci_camera_get_name(GciCamera* camera, char *name);
int  gci_camera_set_blacklevel(GciCamera* camera, CameraChannel channel, double blacklevel); 
int  gci_camera_get_blacklevel(GciCamera* camera, CameraChannel channel, double *blacklevel);
int  gci_camera_set_sensitivity(GciCamera* camera, CameraChannel channel, double sensitivity);
int  gci_camera_get_sensitivity(GciCamera* camera, CameraChannel channel, double *sensitivity);
int  gci_camera_set_data_mode(GciCamera* camera, DataMode data_mode);
int  gci_camera_get_data_mode(GciCamera* camera);
int  gci_camera_set_highest_data_mode(GciCamera* camera);
int  gci_camera_set_lowest_data_mode(GciCamera* camera);
int  gci_camera_set_light_mode(GciCamera* camera, LightMode light_mode);
int  gci_camera_get_light_mode(GciCamera* camera);
int  gci_camera_set_fast_mode (GciCamera* camera, int enable);
int  gci_camera_set_subwindow(GciCamera* camera, SUBWINDOW_SIZE size);
void gci_camera_bring_to_front(GciCamera* camera);

int  gci_camera_set_average_frame_number(GciCamera* camera, const int frames);
int  gci_camera_get_average_frame_number(GciCamera* camera);
int  gci_camera_get_average_count_of_last_image_displayed(GciCamera* camera);

int  gci_camera_power_up(GciCamera* camera);
int  gci_camera_is_powered_up(GciCamera* camera);
int  gci_camera_power_down(GciCamera* camera);
int  gci_camera_get_colour_type(GciCamera* camera);

int  gci_camera_set_exposure_time(GciCamera* camera, double exposure);
int  gci_camera_set_gain(GciCamera* camera, CameraChannel channel, double gain); 				// Same as mono gain

int  gci_camera_set_min_size(GciCamera* camera, unsigned int width, unsigned int height);
int  gci_camera_set_max_size(GciCamera* camera, unsigned int width, unsigned int height); 
int  gci_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height);
int  gci_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height);

int  gci_camera_set_size_position(GciCamera* camera, unsigned int left, unsigned int top,
	unsigned int width, unsigned int height, unsigned char auto_centre);
													 
int  gci_camera_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
	unsigned int *width, unsigned int *height, unsigned char *auto_centre);

int  gci_camera_get_size(GciCamera* camera, unsigned int *width, unsigned int *height);
int  gci_camera_get_position(GciCamera* camera, unsigned int *left, unsigned int *top, unsigned char *auto_centre);

int  gci_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus,
	char *camera_id, char *camera_version, char *driver_version);

void gci_camera_set_extra_panel_uir(GciCamera* camera, const char *uir_filename, int panel);
int  gci_camera_display_main_ui(GciCamera* camera);
int  gci_camera_hide_main_ui(GciCamera* camera);
int  gci_camera_display_extra_ui(GciCamera* camera);
int  gci_camera_hide_extra_ui(GciCamera* camera);
int  gci_camera_destroy_extra_ui(GciCamera* camera);
int  gci_camera_hide_ui(GciCamera* camera);
int  gci_camera_disable_ui(GciCamera* camera, int disable);

int  gci_camera_snap_image(GciCamera* camera);
int  gci_camera_snap_average_image(GciCamera* camera, unsigned char average);
int  gci_camera_update_live_image(GciCamera* camera);

int  gci_camera_is_live_mode(GciCamera* camera);
int  gci_camera_is_snap_mode(GciCamera* camera);
int  gci_camera_is_snap_sequence_mode(GciCamera* camera);

int  gci_camera_set_live_mode(GciCamera* camera);
int  gci_camera_set_snap_mode(GciCamera* camera);
int  gci_camera_set_snap_sequence_mode(GciCamera* camera);
int  gci_camera_activate_live_display(GciCamera* camera);

int  gci_camera_show_window(GciCamera* camera);

int  gci_camera_set_default_settings(GciCamera* camera);

int  gci_camera_save_settings(GciCamera* camera, const char *filepath, const char *mode);
int  gci_camera_load_settings(GciCamera* camera, const char *filepath);
int  gci_camera_save_settings_as_default(GciCamera* camera);

int  gci_camera_save_state(GciCamera* camera, CameraState *state);
int  gci_camera_restore_state(GciCamera* camera, CameraState *state);

int  gci_camera_get_state_as_ini_string(GciCamera* camera, char *buffer);

double gci_camera_get_exposure_time(GciCamera* camera); 
int gci_camera_get_gain(GciCamera* camera, CameraChannel channel, double *gain);
double gci_camera_get_mono_gain(GciCamera* camera);
double gci_camera_get_frame_rate(GciCamera* camera);

int	gci_camera_set_microns_per_pixel(GciCamera* camera, double factor);
double gci_camera_get_microns_per_pixel(GciCamera* camera);

int gci_camera_supports_focal_indicator_settings(GciCamera *camera);
FocusSettings gci_camera_get_focal_indicator_settings (GciCamera* camera);

void gci_camera_set_rotation(GciCamera* camera, double angle);
void gci_camera_set_horizontal_flip(GciCamera* camera, int flip);
void gci_camera_set_vertical_flip(GciCamera* camera, int flip);

void gci_camera_set_exposure_range(GciCamera* camera, double min, double max, const char *label, int type);
void gci_camera_set_gain_range(GciCamera* camera, double min, double max, int type); 
void gci_camera_set_gain_range_text(GciCamera* camera, char *min, char *max);
void gci_camera_get_gain_range(GciCamera* camera, double *min, double *max);
void gci_camera_set_blacklevel_range(GciCamera* camera, double min, double max);
void gci_camera_set_blacklevel_range_text(GciCamera* camera, char *min, char *max);
void gci_camera_set_sensitivity_range(GciCamera* camera, double min, double max);
void gci_camera_get_sensitivity_range(GciCamera* camera, double *min, double *max);
void gci_camera_disable_gain_control(GciCamera* camera);
void gci_camera_enable_gain_control(GciCamera* camera);
int gci_camera_set_ccd_mode (GciCamera* camera, CCDMode mode);
int gci_camera_attempt_recovery (GciCamera* camera);

int gci_camera_autoexposure(GciCamera* camera, double min_exposure, double max_exposure);

GCIWindow*  gci_camera_get_imagewindow(GciCamera* camera);

#ifdef THREAD_DEBUGGING
FIBITMAP*   gci_camera_get_image(GciCamera* camera, const Rect *rect, ...);
#else
FIBITMAP*   gci_camera_get_image(GciCamera* camera, const Rect *rect);
#endif

FIBITMAP*   gci_camera_get_image_average(GciCamera* camera); 
FIBITMAP**  gci_camera_get_images(GciCamera* camera, int number_of_images);  
FIBITMAP*   gci_camera_get_image_average_for_frames(GciCamera* camera, int frames);
FIBITMAP*   gci_camera_get_displayed_image(GciCamera* camera);

int  gci_camera_display_image(GciCamera* camera, FIBITMAP *dib, char *title);
int  gci_camera_display_image_advanced(GciCamera* camera, FIBITMAP *dib, char *title, int average_count);

/* Signal Connection */
typedef void (*CAMERA_EVENT_HANDLER) (GciCamera* camera, void *data);
typedef void (*CAMERA_IMAGE_EVENT_HANDLER) (GciCamera* camera, FIBITMAP** dib, void *data);
typedef void (*CAMERA_WINDOW_EVENT_HANDLER) (GciCamera* camera, GCIWindow *window, void *data);

int  gci_camera_signal_on_show_handler_is_connected (GciCamera* camera);
int  gci_camera_signal_on_show_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_on_exposure_changed_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_on_gain_changed_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_close_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_datamode_change_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_binning_change_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_enter_live_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER, void *callback_data);
int  gci_camera_signal_exit_live_handler_connect  (GciCamera* camera, CAMERA_EVENT_HANDLER, void *callback_data);
int  gci_camera_signal_enter_snap_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_exit_snap_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_enter_snap_sequence_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_exit_snap_sequence_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_enter_get_image_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER, void *callback_data);
int  gci_camera_signal_exit_get_image_handler_connect  (GciCamera* camera, CAMERA_IMAGE_EVENT_HANDLER, void *callback_data);
int  gci_camera_signal_pre_capture_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_post_capture_handler_connect  (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_trigger_now_handler_connect  (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_image_pre_display_handler_connect (GciCamera* camera, CAMERA_WINDOW_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_image_post_display_handler_connect (GciCamera* camera, CAMERA_WINDOW_EVENT_HANDLER handler, void *callback_data);
int  gci_camera_signal_image_window_close_handler_connect (GciCamera* camera, CAMERA_WINDOW_EVENT_HANDLER handler, void *callback_data);

int  gci_camera_signal_enter_get_image_handler_disconnect (GciCamera* camera, int id);
int  gci_camera_signal_exit_get_image_handler_disconnect  (GciCamera* camera, int id);
int  gci_camera_signal_post_capture_handler_disconnect  (GciCamera* camera, int id);

int  gci_camera_supports_binning(GciCamera *camera);
int  gci_camera_set_binning_mode(GciCamera* camera, BinningMode binning);
BinningMode  gci_camera_get_binning_mode(GciCamera* camera);
int	 gci_camera_set_highest_binning_mode(GciCamera* camera); 

double  gci_camera_get_true_microns_per_pixel(GciCamera* camera);

int  gci_camera_set_trigger_mode(GciCamera* camera, TriggerMode trig_mode);
int  gci_camera_get_trigger_mode(GciCamera* camera, TriggerMode *trig_mode);
int  gci_camera_fire_internal_trigger(GciCamera* camera);

double  gci_camera_get_temperature(GciCamera* camera);

int  gci_camera_dual_channel_enabled(GciCamera* camera);

void gci_camera_prevent_getting_images(GciCamera* camera);
int gci_camera_set_no_display (GciCamera* camera, int val);
int gci_camera_supports_fast_mode(GciCamera* camera); 

		 
#endif
