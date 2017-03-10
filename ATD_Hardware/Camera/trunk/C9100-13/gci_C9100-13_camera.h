#ifndef __GCI_C9100_13_CAMERA__
#define __GCI_C9100_13_CAMERA__

#include "camera\gci_camera.h"

#include "dcam\dcamapi.h"

typedef struct C9100_13Camera GciC9100_13Camera;

struct C9100_13Camera {

  GciCamera camera;
  
  HINSTANCE _ghInstance;
  HDCAM _hCam;
  long  _index;
  unsigned int _timeout;
  unsigned int _binning;
  int _trigger_mode;
  int _data_type;
  int _bpp;
  int _readout_mode;
  double _blacklevel;
  unsigned int _light_mode;
  
  GciC9100_13Camera *saved_settings;
  
};

GciCamera* gci_C9100_13_camera_new(HINSTANCE hInstance, const char *name, const char *description);

int  gci_C9100_13_camera_set_photon_mode(GciCamera* camera, int mode);

double  gci_C9100_13_camera_get_temperature(GciCamera* camera);

#endif
