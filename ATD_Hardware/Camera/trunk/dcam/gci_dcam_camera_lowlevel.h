#ifndef __GCI_LOWLEVEL_CAMERA__
#define __GCI_LOWLEVEL_CAMERA__

#include "gci_dcam_camera.h"

int VOID_DCAM_CAMERA_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);
void gci_dcam_error_code_to_string(unsigned long error, char *string);
int gci_dcam_precapture(GciDCamCamera *dcam_camera, int mode);
int gci_dcam_capture(GciDCamCamera *dcam_camera);
int gci_dcam_wait_for_captured_frame(GciDCamCamera *dcam_camera);
int gci_dcam_wait_for_frame_valid(GciDCamCamera *dcam_camera);
int gci_dcam_wait_for_frame_end(GciDCamCamera *dcam_camera);
int gci_dcam_idle(GciDCamCamera *dcam_camera);
int gci_dcam_freeframe(GciDCamCamera *dcam_camera);
int gci_dcam_wait_for_stable(GciDCamCamera *dcam_camera);
int gci_dcam_wait_for_ready(GciDCamCamera *dcam_camera);
int gci_dcam_wait_for_busy(GciDCamCamera *dcam_camera);
int gci_dcam_allocframe(GciDCamCamera *dcam_camera);
int gci_dcam_set_to_ready_mode(GciDCamCamera *dcam_camera);
int gci_dcam_free_camera_resources(GciDCamCamera *dcam_camera);
int DCamSetFeature(GciDCamCamera *dcam_camera, int featureID, double value);
int DCamSetSubarray(GciDCamCamera *dcam_camera, int ox, int oy, int gx, int gy);
int DCamGetSubarray(GciDCamCamera *dcam_camera, int *ox, int *oy, int *gx, int *gy);
int DCamSetDataType(GciDCamCamera *dcam_camera, int bits);
int DCamGetFeature(GciDCamCamera *dcam_camera, int featureID, double *value);
int DCamPrintProperties(GciDCamCamera *dcam_camera);
int DCamQueryCurrentTempertureSupport(GciDCamCamera *dcam_camera, int* supported);
int DCamPowerUp(GciCamera* camera);
FIBITMAP * DCamGetImage(GciCamera* camera, const Rect *rect);

#endif
