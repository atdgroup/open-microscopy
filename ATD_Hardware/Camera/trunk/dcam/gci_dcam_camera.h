#ifndef __GCI_ORCA_CAMERA__
#define __GCI_ORCA_CAMERA__

#include "camera\gci_camera.h"

#include "dcamapi.h"
#include "features.h"
#include "dcamapix.h"

typedef struct _DCamCamera GciDCamCamera;

struct _DCamCamera {

  GciCamera camera;
  
  HINSTANCE _ghInstance;
  HDCAM _hCam;
  long  _index;
  ListType _properties;
  unsigned int _timeout;
  unsigned int _binning;
  unsigned int _light_mode;
  double _black_level;
  double _sensitivity;
  int _trigger_mode;
  int _data_type;
  int _hammamatsu_data_type;
  int _require_free_resources;
  double _gain_safe_preset_for_sensitivity;
  double _max_safe_sensitivity;

  // Property controls
  int has_properties;
  int prop_timer;
  int prop_panel_id;
  int prop_listbox;
  int prop_attr_textbox;
  int prop_values_listbox;
  int prop_hide_button_id;

  LONG_PTR old_wndproc;

  char default_settings_file_path[GCI_MAX_PATHNAME_LEN];
  
  GciDCamCamera *saved_settings;
  
};

GciCamera* gci_dcam_camera_new(HINSTANCE hInstance, const char *name, const char *description, int max_width, int max_height);
GciCamera* gci_orca_camera_new(HINSTANCE hInstance, const char *name, const char *description);
GciCamera* gci_C8484_camera_new(HINSTANCE hInstance, const char *name, const char *description);
GciCamera* gci_C9100_13_camera_new(HINSTANCE hInstance, const char *name, const char *description);
GciCamera* gci_C11440_10C_camera_new(HINSTANCE hInstance, const char *name, const char *description);

void gci_dcam_constructor(GciCamera *camera, HINSTANCE hInstance, const char *name, const char *description);
int gci_dcam_camera_deconstructor(GciCamera* camera);
void gci_dcam_camera_setup_vtable(GciCamera* camera);
int  gci_dcam_camera_get_light_mode(GciCamera* camera);
int gci_dcam_camera_set_default_settings (GciCamera* camera);
int  gci_dcam_camera_set_photon_mode(GciCamera* camera, int mode);
double  gci_dcam_camera_get_temperature(GciCamera* camera);
int dcam_camera_has_properties(GciDCamCamera *dcam_camera);
void properties_dialog_show(GciDCamCamera *dcam_camera);
void properties_dialog_new(GciDCamCamera *dcam_camera);
int dcam_camera_has_properties(GciDCamCamera *dcam_camera);
int gci_dcam_camera_get_nearest_subwindow_rect(GciCamera* camera, Rect *rect);
int gci_dcam_camera_get_sensitivity(GciCamera* camera, CameraChannel c, double *val);
int gci_dcam_camera_set_sensitivity(GciCamera* camera, CameraChannel c, double sensitivity);
int gci_dcam_set_preset_subwindow(GciCamera* camera, SUBWINDOW_SIZE size);
int gci_dcam_camera_acquire_subtract_reference(GciCamera* camera);
int gci_dcam_camera_enable_subtract(GciCamera* camera, int enable);
int gci_dcam_camera_setoffset(GciCamera* camera, int offset);

int  CVICALLBACK DCAM_Camera_onAutoCenter(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onBinning(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onBlackLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onSensitivty(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onDataMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onPropertyMenuClicked (int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK DCAM_Camera_onExtrasQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onLightMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onPresetSubWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onSetSizePosition(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_SetPhotonMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_TimerTick(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_onSubwindowApplyClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onCCDMode (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onEnableSubtract (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onAcquireSubtractRef (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DCAM_Camera_onSetSubtractOffset (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#endif
