#ifndef __GCI_JVC_CAMERA__
#define __GCI_JVC_CAMERA__

#include "camera\gci_camera.h"

#include <cviauto.h> 

typedef struct JvcCamera GciJvcCamera;

typedef enum {COL_RGB, COL_R, COL_G, COL_B} JVC_COLOUR_TYPE;
typedef enum {GAIN_STEP, GAIN_ALC, GAIN_VGAIN} JVC_GAIN_MODE;
typedef enum {LIVE_L1, LIVE_L2, LIVE_XL1, LIVE_XL2} JVC_STEP_MODE;
typedef enum {ASPECT_4X3, ASPECT_16X9, ASPECT_14X9} JVC_ASPECT_MODE;
typedef enum {PATTERN_OFF, PATTERN_BAR, PATTERN_RAMP, PATTERN_IMPULSE} JVC_TEST_PATTERN_MODE;
typedef enum {CANCEL_MANUAL, CANCEL_1S, CANCEL_3S, CANCEL_5S, CANCEL_OFF} FREEZE_CANCEL_MODE;
typedef enum {MATRIX0, MATRIX1, MATRIX2, MATRIX3, MATRIX4, MATRIX5, MATRIX6, MATRIX7, MATRIX8} JVC_COLOUR_MATRIX_NUMBER;
typedef enum {COLOUR_MATRIX_MODE_ON, COLOUR_MATRIX_MODE_OFF} JVC_COLOUR_MATRIX_MODE; 
typedef enum {DETECT_NORMAL, DETECT_PEAK, DETECT_AVG} JVC_IRIS_AE_DETECT; 
typedef enum {AREA_SQUARE, AREA_SPOT, AREA_FULL, AREA_CIRCLE} JVC_AE_AREA_TYPE;
typedef enum {MODE_AUTO, MODE_MANUAL} JVC_IRIS_MODE;
typedef enum {SHUT_STEP, SHUT_VSCAN, SHUT_EEI, SHUT_OFF, SHUT_RANDOM} JVC_SHUTTER_MODE;  
typedef enum {WB_PRESET, WB_AUTO1, WB_AUTO2, WB_MANUAL} JVC_WHITE_BALANCE_MODE;
typedef enum {SHADING_MODE_ON, SHADING_MODE_OFF} JVC_SHADING_MODE;
typedef enum {DETAIL_MODE_ON, DETAIL_MODE_OFF} JVC_DETAIL_MODE;         
typedef enum {GAMMA_MODE_NORMAL, GAMMA_MODE_ADJUST} JVC_GAMMA_MODE; 
typedef enum {FLARE_MODE_ON, FLARE_MODE_OFF} JVC_FLARE_MODE;
typedef enum {ABL_MODE_ON, ABL_MODE_OFF} JVC_ABL_MODE; 
typedef enum {NOISE_SUPP_LOW, NOISE_SUPP_MIDDLE, NOISE_SUPP_HIGH, NOISE_SUPP_OFF} JVC_DETAIL_NOISE_SUPPRESSION;
typedef enum {DSP_OFF, DSP_BYPASS} JVC_DSP_MODE;
typedef enum {NEGA_MODE_ON, NEGA_MODE_OFF} JVC_NEGA_MODE;      
typedef enum {PIXEL_COMPENSATION_MODE_ON, PIXEL_COMPENSATION_MODE_OFF} JVC_PIXEL_COMPENSATION_MODE;     

struct JvcCamera {

  GciCamera camera;
  
  CAObjHandle handle;
  ERRORINFO ErrorInfo;
  HRESULT _last_error;
  long _gain_mode;
  long _shutter_mode;
  long _white_balance_mode;
  int _max_gain_value;
  int _min_gain_value;
  int _shutter_step_level;
  int _shutter_vscan_level;
  int _shutter_random_level;
  int _tab_control;
  int _image_panel;
  int _exposure_panel;
  int _white_balance_panel;
  int _process_panel;
  int _colour_matrix_panel;
  int _system_panel;
  int _set_live;
  int _grabbed;
  int _ae_area_timer;
  int _colour_type;
  char settings_filepath[500];

  
  GciJvcCamera *saved_settings;
  
};

	
GciCamera* gci_jvc_camera_new(const char *name, const char *description) ;
void gci_jvc_constructor(GciCamera *camera, const char *name, const char *description);


// Image Panel

int					gci_jvc_camera_set_colour_type(GciJvcCamera* jvc_camera, JVC_COLOUR_TYPE val);
int					gci_jvc_camera_get_colour_type(GciJvcCamera* jvc_camera, long *val); 
int 				gci_jvc_camera_set_step (GciJvcCamera* jvc_camera, JVC_STEP_MODE val);
int 				gci_jvc_camera_set_resolution (GciJvcCamera* jvc_camera, JVC_STEP_MODE val);
int 				gci_jvc_camera_get_resolution (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_aspect (GciJvcCamera* jvc_camera, JVC_ASPECT_MODE val); 
int 				gci_jvc_camera_get_aspect (GciJvcCamera* jvc_camera, long *val);

// System Panel
int 				gci_jvc_camera_set_test_pattern_mode (GciJvcCamera* jvc_camera, JVC_TEST_PATTERN_MODE val);
int 				gci_jvc_camera_get_test_pattern_mode (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_test_pattern_level (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_test_pattern_level (GciJvcCamera* jvc_camera, int *val);
int					gci_jvc_camera_set_freeze_cancel (GciJvcCamera* jvc_camera, FREEZE_CANCEL_MODE val);
int					gci_jvc_camera_memory_save (GciJvcCamera* jvc_camera);


// Iris / Exposure / Gain
int 				gci_jvc_camera_set_iris_mode (GciJvcCamera* jvc_camera, JVC_IRIS_MODE val);
int 				gci_jvc_camera_get_iris_mode (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_iris_level (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_iris_level (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_ae_level (GciJvcCamera* jvc_camera, long val);
int 				gci_jvc_camera_get_ae_level (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_draw_ae_area (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_set_ae_area (GciJvcCamera* jvc_camera, JVC_AE_AREA_TYPE val, int draw);
int 				gci_jvc_camera_get_ae_area (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_ae_detect  (GciJvcCamera* jvc_camera, JVC_IRIS_AE_DETECT val);
int 				gci_jvc_camera_get_ae_detect (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_shutter_mode (GciJvcCamera* jvc_camera, JVC_SHUTTER_MODE val);
int 				gci_jvc_camera_get_shutter_mode (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_shutter_level (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_shutter_level (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_restart_shutter (GciJvcCamera* jvc_camera);
int 				gci_jvc_camera_set_alc_max (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_alc_max (GciJvcCamera* jvc_camera, int *val);
int					gci_jvc_camera_set_gain_mode(GciJvcCamera* jvc_camera, JVC_GAIN_MODE mode);
int 				gci_jvc_camera_get_gain_mode(GciJvcCamera* jvc_camera, long *mode);

int 				gci_jvc_camera_set_gain(GciCamera* camera, CameraChannel c, double gain);   
int 				gci_jvc_camera_get_gain(GciCamera* camera, CameraChannel c, double *gain);        
int					gci_jvc_camera_set_eei_limit (GciJvcCamera* jvc_camera, int val);
int					gci_jvc_camera_get_eei_limit (GciJvcCamera* jvc_camera, int *val);


// White Balance

int 				gci_jvc_camera_set_colour_temp (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_colour_temp (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_white_balance_mode  (GciJvcCamera* jvc_camera, JVC_WHITE_BALANCE_MODE val);
int 				gci_jvc_camera_get_white_balance_mode  (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_white_balance_red_level  (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_white_balance_red_level  (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_white_balance_blue_level  (GciJvcCamera* jvc_camera, int val) ;
int 				gci_jvc_camera_get_white_balance_blue_level  (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_white_balance_base_red  (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_white_balance_base_red  (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_white_balance_base_blue  (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_white_balance_base_blue  (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_auto_whitebalance  (GciJvcCamera* jvc_camera);
int 				gci_jvc_camera_set_shading_level_red  (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_shading_level_red  (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_shading_level_green (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_shading_level_green  (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_shading_level_blue  (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_shading_level_blue  (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_shading_mode (GciJvcCamera* jvc_camera, JVC_SHADING_MODE val);
int 				gci_jvc_camera_get_shading_mode (GciJvcCamera* jvc_camera, long *val);


// Process

int 				gci_jvc_camera_set_detail_level (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_detail_level (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_detail_level_depend (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_detail_level_depend (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_detail_mode (GciJvcCamera* jvc_camera, JVC_DETAIL_MODE val);
int 				gci_jvc_camera_get_detail_mode (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_gamma_level (GciJvcCamera* jvc_camera, long val);
int 				gci_jvc_camera_get_gamma_level (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_gamma_mode (GciJvcCamera* jvc_camera, JVC_GAMMA_MODE val);
int 				gci_jvc_camera_get_gamma_mode (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_flare_level_red (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_flare_level_red (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_flare_level_blue (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_flare_level_blue (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_flare_mode (GciJvcCamera* jvc_camera, JVC_FLARE_MODE val);
int				 	gci_jvc_camera_get_flare_mode (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_abl_level (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_abl_level (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_abl_mode (GciJvcCamera* jvc_camera, JVC_ABL_MODE val);
int 				gci_jvc_camera_get_abl_mode (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_detail_noise_suppression (GciJvcCamera* jvc_camera, JVC_DETAIL_NOISE_SUPPRESSION val);
int 				gci_jvc_camera_get_detail_noise_suppression (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_nega_mode (GciJvcCamera* jvc_camera, JVC_NEGA_MODE val);
int 				gci_jvc_camera_get_nega_mode (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_master_black_level (GciJvcCamera* jvc_camera, int val);
int 				gci_jvc_camera_get_master_black_level (GciJvcCamera* jvc_camera, int *val);
int 				gci_jvc_camera_set_pixel_compensation (GciJvcCamera* jvc_camera, JVC_PIXEL_COMPENSATION_MODE val);
int 				gci_jvc_camera_get_pixel_compensation (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_pixel_check (GciJvcCamera* jvc_camera);
int					gci_jvc_camera_set_dsp_bypass (GciJvcCamera* jvc_camera, JVC_DSP_MODE val);

// Colour Matrix

int 				gci_jvc_camera_set_colour_matrix_mode (GciJvcCamera* jvc_camera, JVC_COLOUR_MATRIX_MODE val);
int 				gci_jvc_camera_get_colour_matrix_mode (GciJvcCamera* jvc_camera, long *val);
int 				gci_jvc_camera_set_colour_matrix (GciJvcCamera* jvc_camera, JVC_COLOUR_MATRIX_NUMBER matrix, int level);
int 				gci_jvc_camera_get_colour_matrix (GciJvcCamera* jvc_camera, JVC_COLOUR_MATRIX_NUMBER matrix, int *level);



int  CVICALLBACK JvcCamera_onABLDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onAblLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onAEAreaDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onAEAreaTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onAEDetectDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onAELevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onAlcMaxDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onAspectDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onCameraMemorySave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onColourDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onColourMatrixLevelChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onColourMatrixModeDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onDspBypass(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onEeilimitDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onExtrasQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onFlareBlueLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onFlareDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onFlareRedLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onFreezeCancelDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onGainLevelChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onGainModeDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onGammaDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onGammaLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onIrisLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onIrisModeDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onMasterBlackLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onNegaDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onNoiseSupDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onPixelCheck(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onPixelCompLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onProccessDetailDepLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onProcessDetailDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onProcessDetailLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onResolutionDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onShadingDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onShadingLevelBlue(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onShadingLevelGreen(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onShadingLevelRed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onShutterModeDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onShutterRestart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onShutterSpeedLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onTestPatternDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onTestPatternLevelDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onWhiteBalanceAuto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onWhiteBalanceBaseLevelBlue(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onWhiteBalanceBaseLevelRed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onWhiteBalanceColourTemp(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onWhiteBalanceLevelBlue(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onWhiteBalanceLevelRed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK JvcCamera_onWhiteBalanceModeDropDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#endif
