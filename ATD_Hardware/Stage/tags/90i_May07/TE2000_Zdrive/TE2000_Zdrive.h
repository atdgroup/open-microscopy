#ifndef __TE2000_Z__
#define __TE2000_Z__

#include "TE2000.h"

////////////////////////////////////////////////////////////////////
//Nikon TE2000 Z drive functions
//Rosalind Locke - February 2006
////////////////////////////////////////////////////////////////////

#define STEPS_PER_UM	20

#define Z_MIN			0.0		//um
#define Z_MAX			10000.0 //um

#define Z_SUCCESS		0
#define Z_ERROR			-1

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

typedef struct
{
	int mounted;
	double min;
	double max;
	double current_z;
	double datum;
	int resolution;
	double speed;	   //mm/sec
	
	TE2000* te2000;
} TE2000_Z;

TE2000_Z* TE2000_z_new(TE2000 *te2000);
int TE2000_z_init(TE2000_Z *te2000_z);
void TE2000_z_clear (TE2000_Z *te2000_z);
void TE2000_z_destroy(TE2000_Z *te2000_z);

int TE2000_z_move_abs (TE2000_Z *te2000_z, double z);
int TE2000_z_move_rel (TE2000_Z *te2000_z, double z);
int TE2000_z_get_position (TE2000_Z *te2000_z, double *z);
int TE2000_z_set_datum (TE2000_Z *te2000_z, double z);
int TE2000_z_set_resolution (TE2000_Z *te2000_z, int resolution);

#endif
