#ifndef __SPC__
#define __SPC__

#include "gci_ui_module.h"
#include "icsviewer_window.h"
#include "camera\gci_camera.h" 
#include "scanner.h" 
#include "microscope.h"
#include "spcm_def.h"  
#include "FilterSet.h"

////////////////////////////////////////////////////////////////////////////
//RJL May 2007
//GCI HTS Microscope system. 
//Single Photon Counting using Becker & Hickl board control.
////////////////////////////////////////////////////////////////////////////

// Convention
// Files beginning with bh_ are lowlevel functions that should not be public
// Public functions begin with spc_

#define SPC_SUCCESS 0
#define SPC_ERROR -1

#define Round  RoundRealToNearestInteger

// This file contains all settings of ours and the b&h specific settings
#define DEFAULT_SPC_FILENAME "spcm.ini"      

typedef enum {SPC_PARAM_NONE, SPC_PARAM_ADC, SPC_PARAM_SCAN_SIZE} SPC_PARAMETER;
typedef enum {SPC_IMAGE_TYPE_INTENSITY, SPC_IMAGE_TYPE_TAU_SHORT, SPC_IMAGE_TYPE_TAU_LONG} SPC_IMAGE_TYPE;

typedef enum {BH_SCOPE_MODE = 0, BH_SCAN_MODE = 2} BH_OPERATION_MODE;
enum {INTENSITY, TAU_SHORT, TAU_LONG};

typedef enum {SPC_ACQ_LIMIT_TYPE_SECONDS=1,
			  SPC_ACQ_LIMIT_TYPE_FRAMES=2,
			  SPC_ACQ_LIMIT_TYPE_MAX_COUNT=3,
			  SPC_ACQ_LIMIT_TYPE_MEAN_COUNT=4,
			 } SPC_ACQUISITION_LIMIT_TYPE;


typedef struct _HighestResImage
{
	int			_scanner_zoom;
	int			_scanner_res;
	Point		_scanner_shift;
	char 		_image_path[GCI_MAX_PATHNAME_LEN];

	double stage_x;
	double stage_y;

} HighestResImage;

typedef struct _StageScanAcquisition
{
	stage_scan	*ss;
	region_of_interest *roi; 

	int	   roi_set;

	double frame_roi_left;			//um
	double frame_roi_top;			//um 
	double frame_roi_width;			//um 
	double frame_roi_height; 		//um 

} StageScanAcquisition;

typedef struct _AcquireParamData
{
	Spc					 *spc;

	SPC_ACQUISITION_LIMIT_TYPE limit_type;
	int limit_val;
	int repeat;
	float repeat_time;
	int should_display;
	double display_time;
	double cycle_start_time;
	int accumulate;
	int autosave;
	int	current_frame;
	int first_frame_displayed;
	int passed_in_filepath;
	int is_3d;
	char output_dir[GCI_MAX_PATHNAME_LEN];
	char output_filename[GCI_MAX_PATHNAME_LEN];
	char output_filename_ext[GCI_MAX_PATHNAME_LEN];

} AcquireParamData;


typedef struct _ScannerSettings
{
	int min_x_shift;
	int min_y_shift;
	int max_x_shift;
	int max_y_shift;

	int x_shift;
	int y_shift;

	int	displayed_image_scanner_zoom;
	int	displayed_image_x_shift;
	int	displayed_image_y_shift;

} ScannerSettings;

typedef struct _OscilloscopeSettings
{
	double*	 _x_data;
 
} OscilloscopeSettings;

struct _Spc
{
  UIModule parent;

  HWND       window_hwnd;
  LONG_PTR	 uimodule_func_ptr;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int	 	 _pileup_ui_panel;
  int	 	 _graph_ui_panel;
  int	 	 _rates_ui_panel;
  int	 	 _params_ui_panel;
  
  GCIWindow  *_spc_window;
  dictionary *_metadata;
  dictionary *_scope_metadata;

  Microscope		*_ms;
  GciCamera  		*_camera;
  Scanner	 		*_scanner;
  FilterSetCollection *_filter_set;

  double		 _umPerPixel;
 
  int		 _chans_enabled[4];
  
  int		 _scan_pattern;
  int		 _acq_limit_type;
  int		 _acq_limit_val;
  
  double	 _frame_time;
  double	 _scan_time;
  double	 _acc_time;
  int		 _acc_frames;
  double 	 _old_adc_resolution;
  int		 _tac_select;
  float	 	 _tac_val;
  double 	 _mc_tac_offset;
  int		 _autorange;
  int		 _time_window_min;
  int		 _time_window_max;
  int		 _pileup_lut_on;
  int		 _max_count;
  int		 _max_scope_count;
  int		 _max_scale_for_tau;
  int		 _change_palette_for_filter;

  int		 _scan_max_count;
  
  int		 _initialised;
  int		 _simulation;
  int		 _modules_active[8];
  int		 _no_modules;
  int		 _no_active_modules;
  int		 _active_module;
  int		 _ignore_scanner_signals;
  int		 _mod_info_found;

  float		 _oscilloscope_collect_time;
  float		 _scan_collect_time;

  volatile int		 _acquire;
  volatile int		 _acquire_prompt;
  volatile int		 _acquire_finished;
  volatile int		 _waiting_for_next_repeat;

  ScannerSettings	scanner_settings;
  HighestResImage	roi_overview_image;
  StageScanAcquisition	ss_acquire;
  AcquireParamData	acq_data;
  OscilloscopeSettings oscilloscope_settings;
  SPCMemConfig  mem_info;
  SPCdata 	 *_spc_data;
  SPCModInfo	mod_info;

  unsigned short* _buffer;
};

// Signals
typedef void (*SPC_EVENT_HANDLER) (Spc* spc, void *data);

//Parameters which modify on board memory usage
enum {ADC_RES, SCAN_X, SCAN_Y, ROUTING_X, ROUTING_Y};

void bh_set_error_handler(Spc* spc, UI_MODULE_ERROR_HANDLER handler);
//int  bh_show_error(Bh *bh, int errcode);

int  bh_destroy(Spc* spc);

void bh_disable_timer(Spc* spc);
void bh_enable_timer(Spc* spc);

void bh_on_change(Spc* spc);

int  bh_display_rates_ui (Spc* spc);
int  bh_hide_rates_ui (Spc* spc);
	 
void spc_dim_sys_param_controls(Spc* spc, int dimmed);
void bh_set_sys_param_controls_mode(Spc* spc, int mode);	  //0=indicator, 1=hot 
void spc_update_params_panel(Spc* spc);
int spc_send_sys_params(Spc* spc);

SPC_PARAMETER display_mem_error_dialog(Spc *spc);

double bh_get_sync_rate(Spc* spc);

int spc_get_default_bh_ini_file_path(Spc* spc, char *filepath);
int spc_save_settings(Spc* spc, const char *filepath);
int spc_save_settings_dialog(Spc* spc);
int spc_save_default_params(Spc* spc);
int spc_load_settings(Spc* spc, const char *filepath);
int spc_load_settings_dialog(Spc* spc);
int spc_load_default_params(Spc* spc);
int spc_save_gci_panel_settings_to_file(Spc* spc);
int spc_set_borders(Spc *spc, unsigned short left, unsigned short top);
int spc_get_max_value_for_adc_res(Spc *spc);
int spc_init_main_panel(Spc* spc);
int spc_save_prompt_image(Spc *spc);

void spc_adapt_parameters_for_memory(Spc* spc, int warn_if_required_less_than_real);

void read_settings_from_main_panel(Spc* spc);

int bh_read_rates(Spc* spc);

void bh_set_adc_res(Spc* spc, int adc_resolution);
int bh_enable_sequencer(Spc* spc, int enable);
//void bh_check_scan_size(Spc* spc, int control);

int bh_set_operation_mode(Spc* spc, unsigned short mode);
int	bh_scope(Spc* spc);

int bh_clear_memory(Spc* spc);
int bh_stop(Spc* spc);
float bh_get_tpp(Spc* spc);

int bh_mode(Spc* spc);
int bh_set_oscilloscope_mode(Spc* spc);
int bh_arm(Spc* spc);
int bh_read_xy_results(Spc* spc);

// Signals

int bh_signal_hide_handler_connect (Spc* spc, SPC_EVENT_HANDLER handler, void *callback_data);
int bh_changed_handler_connect(Spc* spc, SPC_EVENT_HANDLER handler, void *data );
int bh_signal_acq_start_handler_connect (Spc* spc, SPC_EVENT_HANDLER handler, void *callback_data);
int bh_signal_acq_stop_handler_connect (Spc* spc, SPC_EVENT_HANDLER handler, void *callback_data);

void spc_dim_controls(Spc* spc, int dimmed);
int spc_start_oscilloscope(Spc* spc);
int bh_init_params_panel(Spc* spc);
int bh_init_rates_panel(Spc* spc);
void disable_rate_count_timer(Spc* spc);
void enable_rate_count_timer(Spc* spc);

int spc_check_timewindow_valid(Spc *spc);

Spc* spc_new(Microscope *ms, char *name, char *description, UI_MODULE_ERROR_HANDLER handler, char *data_dir);

int spc_check_error(Spc *spc, int errcode);

int spc_initialise (Spc* spc);

int  send_spc_error_text (Spc* spc, char fmt[], ...);
void spc_set_error_handler(Spc* spc, UI_MODULE_ERROR_HANDLER handler);
int  spc_show_error(Spc *spc, int errcode);

int  spc_destroy(Spc* spc);

void spc_disable_timer(Spc* spc);
void spc_enable_timer(Spc* spc);

int  spc_display_main_ui (Spc* spc);
int  spc_hide_ui (Spc* spc);
int  spc_hide_params_ui (Spc* spc);
int  spc_display_tw_ui (Spc* spc);
int  spc_hide_tw_ui (Spc* spc);
int  spc_display_pileup_ui (Spc* spc);
int  spc_hide_pileup_ui (Spc* spc);
int  spc_display_graph_ui (Spc* spc);
int  spc_hide_graph_ui (Spc* spc);

void spc_update_main_panel_parameters(Spc* spc);
int  spc_send_main_panel_parameters(Spc* spc);

int spc_set_operation_mode(Spc* spc);
int	spc_start_scope(Spc* spc, int acquire_prompt);
int	spc_stop_scope(Spc* spc);
void spc_set_scope_x_axis_scale(Spc* spc);
void spc_set_scope_y_axis_scale(Spc* spc, unsigned short* buffer);

void spc_set_linear_scale(Spc* spc);
void spc_smooth_image(Spc* spc);
void spc_set_acq_limit_adv(Spc* spc, int acq_limit_type, int acq_limit_val);
void spc_set_acq_limit(Spc* spc);
void spc_set_display_time(Spc* spc);

int spc_acquire_prompt(Spc *spc);

int spc_start(Spc* spc);

int spc_start_advanced_params(Spc* spc, AcquireParamData data);
int spc_start_advanced(Spc* spc, SPC_ACQUISITION_LIMIT_TYPE limit_type, int acq_limit_val, int repeat, float repeat_time, int should_display, double display_time,
					   int accumulate, int autosave,  int image_3d, const char *filepath);

int spc_stop(Spc* spc);

void spc_scanner_value_update(Scanner* scanner, int control, void *data);

int spc_load_Plut_params(Spc* spc, char *fname);
int spc_save_Plut_params(Spc* spc, char *fname);
int spc_set_pileup_palette(Spc* spc);

int spc_save_3D_image(Spc* spc, const char* pathname);

dictionary* spc_add_metadata(Spc* spc, dictionary *d);
void spc_save_3d_image_from_spc_data (Spc *spc, char *filepath, int prompt);

int spc_signal_hide_handler_connect (Spc* spc, SPC_EVENT_HANDLER handler, void *callback_data);
int spc_changed_handler_connect(Spc* spc, SPC_EVENT_HANDLER handler, void *data );
int spc_signal_acq_start_handler_connect (Spc* spc, SPC_EVENT_HANDLER handler, void *callback_data);
int spc_signal_acq_stop_handler_connect (Spc* spc, SPC_EVENT_HANDLER handler, void *callback_data);

dictionary* spc_get_metadata(Spc* spc, dictionary *d);

int  CVICALLBACK cbAcqLimit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbCheckScanSize(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbConfigGraph(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbFLIM(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbKeypress(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbLoadPlutParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbLoadSysParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbLoadTimeWindows(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbMultiChanDisplay(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbPileupLUTon(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbQuitPlutParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbQuitSysParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbQuitTimeWindows(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRateMonitor(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRateTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSaveAsResponse(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSavePlutParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSaveRGB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSaveSysParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSaveTimeWindows(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSpcScannerControlsChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSendMainPanelADCres(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSendMainPanelParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSendSysParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetDisplayTime(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetPlutPalette(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetTimeWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetupPileUp(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbShowSysParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbShowTimeWindows(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSpcClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSPCscope(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStartSPC(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStartSpcLive (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStopSPC(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSysParamOpMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbTRimtype(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbUpdateMainPanelParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOscilloscopeClosed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSpcSavePrompt (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDisplayImageTimerTick (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
