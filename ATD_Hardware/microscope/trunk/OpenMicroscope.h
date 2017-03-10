#ifndef __DUMMY_MICROSCOPE__
#define __DUMMY_MICROSCOPE__ 

#include "microscope.h"

#include "objectives.h"
#include "CubeSlider.h"
#include "OpticalPath.h"
#include "stage\stage.h"
#include "lamp.h"
#include "scanner.h"
#include "ZDrive.h"

typedef struct {
 
  Microscope parent;   
	
  GciCamera *_original_master_camera;
  GciCamera *_colour_camera;
  double popup_start_time;
  PrototypeAutoFocus*	 _proto_af;

  volatile int  _stage_thread_completed;
  volatile int  _camera_thread_completed;
  volatile int  _colour_camera_thread_completed;
  volatile int  _opt_path_thread_completed;
  volatile int  _cube_thread_completed;

} OpenMicroscope;


int microscope_destroy (Microscope* microscope);

Microscope* microscope_new(HINSTANCE hInstance);

int save_microscope_settings(Microscope* microscope, const char* path);
int load_microscope_settings(Microscope* microscope, const char* path);

#endif
