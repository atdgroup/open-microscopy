#ifndef __C9100_13_CAMERA_SETTINGS__
#define __C9100_13_CAMERA_SETTINGS__

#include "gci_C9100-13_camera.h"

int C9100_13CameraSaveCameraSettings(GciC9100_13Camera *orca_camera, const char *filename);

int C9100_13CameraLoadCameraSettings(GciC9100_13Camera *orca_camera, const char *filename);

int CameraLoadDefaultCameraSettings(GciC9100_13Camera *orca_camera);

int CameraSaveSettingsAsDefaults(GciC9100_13Camera *orca_camera);

int C9100_13CameraSaveIniCameraSettings(GciC9100_13Camera *orca_camera, const char * filepath, const char *mode);

int C9100_13CameraLoadIniCameraSettings(GciC9100_13Camera *orca_camera, const char * filepath);

#endif
