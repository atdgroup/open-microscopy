#include "toolbox.h"
#include <ansi_c.h>
#include <formatio.h>
#include <utility.h>

#include "hardware.h"
#include "TE2000.h"
#include "TE2000_Zdrive.h"

/////////////////////////////////////////////////////////////////////////////
// TE2000_Zdrive module - RJL Jan 2006
//
/////////////////////////////////////////////////////////////////////////////

//NOTES:

//1)	TE2000 Z drive range = 0 to 200000 steps (= 0.0 to 10000.0 um)
//2)	lowest point is 0.0 um

/////////////////////////////////////////////////////////////////////////////

static int TE2000_z_mounted(TE2000_Z *te2000_z)
{
	int val;
	
	//Is focus drive mounted? 0 - no, 1 - yes, 2 - error
	if (te2000_z == NULL) return Z_ERROR;

	if (TE2000_rx_command_int(te2000_z->te2000, "rSZR\r", &val) != 0)
		return Z_ERROR;
	
	te2000_z->mounted = val;
	
	return Z_SUCCESS;
}


int TE2000_z_init (TE2000_Z *te2000_z)
{
	return TE2000_z_mounted(te2000_z);
}


static int TE2000_z_power_up(TE2000_Z *te2000_z)
{
	// If this is part of a larger application the microscope may already have been powered up
	// If this is a standalone program we power it up if it is off

    #ifdef POWER_VIA_I2C 
     	
	   
        	
    #endif
		

  	return Z_SUCCESS;
}

void TE2000_z_clear (TE2000_Z *te2000_z)
{
	if (te2000_z != NULL) {
		te2000_z->min = Z_MIN;
		te2000_z->max = Z_MAX;
		te2000_z->current_z = 0;
		te2000_z->datum = 0;
	}
}

TE2000_Z* TE2000_z_new(TE2000 *te2000)
{
	TE2000_Z *te2000_z;
	
	//Create new Z drive instance. 
	//The TE2000 passed in must already have been created as this is
	//responsible for PC<->TE2000 communications
	
	if (te2000 == NULL) return NULL;
	
	te2000_z = (TE2000_Z *) malloc (sizeof(TE2000_Z));
	
	if (te2000_z != NULL) {
		te2000_z->mounted = 0;
		te2000_z->min = Z_MIN;
		te2000_z->max = Z_MAX;
		te2000_z->current_z = 0;
		te2000_z->datum = 0;
		te2000_z->resolution = 0;   //100 um per rev
		te2000_z->speed = 0.08;		//1mm takes ~ 13 secs

		te2000_z->te2000 = te2000;
	}
	
	return te2000_z;
}

static int TE2000_z_power_down(TE2000_Z *te2000_z)
{

	return Z_SUCCESS;
}


void TE2000_z_destroy(TE2000_Z *te2000_z)
{
	TE2000_z_power_down(te2000_z);
	
  	free(te2000_z);
  	te2000_z = NULL;
}


int TE2000_z_move_abs (TE2000_Z *te2000_z, double z)
{
	char command[15]="";
	double steps, timeout;
	
	//z = 0 to 10000um (= 0 to 200000 steps)
	
	if (!te2000_z->mounted) return Z_ERROR;
	
	if ((z+te2000_z->datum < te2000_z->min) || ( z+te2000_z->datum > te2000_z->max)) return Z_ERROR;

	if (FP_Compare (z, te2000_z->current_z) == 0) return Z_SUCCESS;
	
	timeout = max(5.0, fabs((te2000_z->current_z - z)*0.02));   //100um takes about 1.3 seconds

	steps = (z+te2000_z->datum) * STEPS_PER_UM;

	sprintf(command, "fSMV%d\r", (int)(steps+0.5));
	
	if (TE2000_tx_command_ftype(te2000_z->te2000, command, timeout) != 0)
		return Z_ERROR;

	te2000_z->current_z = z;

	return Z_SUCCESS;	
}
	
int TE2000_z_move_rel (TE2000_Z *te2000_z, double z)
{
	char command[15]="";
	double target, steps, timeout;
	
	//z = 0 to 10000um (= 0 to 200000 steps)
	
	if (!te2000_z->mounted) return Z_ERROR;

	if (FP_Compare (z, 0.0) == 0) return Z_SUCCESS;
	
	target = (te2000_z->current_z + z);
	if ((target+te2000_z->datum < te2000_z->min) || ( target+te2000_z->datum > te2000_z->max)) return Z_ERROR;
	
	timeout = max (5.0, fabs(z*0.02));   //100um takes about 1.3 seconds
	
	steps = z * STEPS_PER_UM;

	if (steps < 0.0) 
		sprintf(command, "fSDC%d\r", -(int)(steps));
	else 	
		sprintf(command, "fSUC%d\r", (int)(steps+0.5));
	
	if (TE2000_tx_command_ftype(te2000_z->te2000, command, timeout) != 0)
		return Z_ERROR;

	te2000_z->current_z += z;

	return Z_SUCCESS;
}

int TE2000_z_get_position (TE2000_Z *te2000_z, double *z)
{
	int steps;
	
	//z = 0 to 10000um (= 0 to 200000 steps)
	
	if (!te2000_z->mounted) return Z_ERROR;
	
	if (TE2000_rx_command_int(te2000_z->te2000, "rSPR\r", &steps) != 0)
		return Z_ERROR;

	te2000_z->current_z = (double)steps/STEPS_PER_UM - te2000_z->datum;
	*z = te2000_z->current_z;
	
	return 0;
  	
  	return Z_SUCCESS;
}


int TE2000_z_set_datum (TE2000_Z *te2000_z, double z)
{
	if ((z < 0.0) || ( z > te2000_z->max)) return Z_ERROR;

	te2000_z->datum = z;
	return Z_SUCCESS;	
}

int TE2000_z_set_resolution (TE2000_Z *te2000_z, int resolution)
{
	char command[10]="";
	
	//Set the resolution of fine focus wheel
	//0 - 100um/rev, 1 - 50um/rev, 2 - 25um/rev
	
	if (!te2000_z->mounted) return Z_ERROR;

	if ((resolution < 0) || ( resolution > 2)) return Z_ERROR;

	sprintf(command, "cSJS%d\r", resolution);
	if (TE2000_tx_command_ctype(te2000_z->te2000, command) != 0)
		return Z_ERROR;

	return Z_SUCCESS;	
}



