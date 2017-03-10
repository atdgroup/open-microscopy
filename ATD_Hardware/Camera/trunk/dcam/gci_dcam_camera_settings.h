#ifndef __ORCA_CAMERA_SETTINGS__
#define __ORCA_CAMERA_SETTINGS__

#include "gci_dcam_camera.h"

int DCamCameraSaveCameraSettings(GciDCamCamera *dcam_camera, const char *filename);

int DCamCameraLoadCameraSettings(GciDCamCamera *dcam_camera, const char *filename);

int CameraLoadDefaultCameraSettings(GciDCamCamera *dcam_camera);

int CameraSaveSettingsAsDefaults(GciDCamCamera *dcam_camera);

int DCamCameraSaveIniCameraSettings(GciDCamCamera *dcam_camera, const char * filepath, const char *mode);

int DCamCameraLoadIniCameraSettings(GciDCamCamera *dcam_camera, const char * filepath);

#endif
