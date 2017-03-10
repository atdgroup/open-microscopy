////////////////////////////////////////////////////////////////////
// Module to stage/camera angle for the microfocus system
// Glenn Pierce and Ros Locke - November 2005
////////////////////////////////////////////////////////////////////

#ifndef __ALIGNMENT__
#define __ALIGNMENT__

#include "gci_camera.h"
#include "icsviewer_window.h"

#define ALIGNMENT_ERROR 0
#define ALIGNMENT_SUCCESS 1

#define NUMBER_OF_POINTS 10

typedef struct {double x; double y;} Pointd;

typedef struct _Alignment Alignment;

Alignment* alignment_step_sampled_new(GciCamera *camera);

void alignment_display_panel(Alignment* alignment);

double alignment_step_sampled_get_slope(Alignment* alignment);

//Stage/camera angle in radians
double alignment_step_sampled_get_angle(Alignment* alignment);

void alignment_step_sampled_destroy(Alignment* alignment);

#endif
