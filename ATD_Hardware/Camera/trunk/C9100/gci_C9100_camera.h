#ifndef __GCI_C9100_CAMERA__
#define __GCI_C9100_CAMERA__

#include "gci_orca_camera.h"

typedef struct C9100Camera GciC9100Camera;

struct C9100Camera
{
  GciOrcaCamera orca_camera;
};

GciCamera* gci_C9100_camera_new(HINSTANCE hInstance, const char *name, const char *description);

int gci_C9100_camera_constructor(GciCamera* camera, HINSTANCE hInstance, const char *name, const char *description);

#endif
