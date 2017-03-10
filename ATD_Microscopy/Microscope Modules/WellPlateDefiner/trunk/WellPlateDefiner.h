#ifndef __WPD__
#define __WPD__

#include "HardwareTypes.h"
#include "microscope.h"

#define TOP_LEFT     0
#define TOP_RIGHT    1
#define BOTTOM_LEFT  2
#define BOTTOM_RIGHT 3

#define POLY_PT_1    4+1
#define POLY_PT_2    4+2
#define POLY_PT_3    4+3

typedef struct
{
    int well_count;
    int horizontal_size;
    int vertical_size;

} WellPlate;

typedef enum {WELL_PLATE_CUSTOM=-1,
              WELL_PLATE_6=0,
              WELL_PLATE_12,
              WELL_PLATE_24,
              WELL_PLATE_48,
              WELL_PLATE_96,
              WELL_PLATE_396,
             } PredefinedWellPlate; 

typedef struct
{
    double x;
    double y;
    double z;   
	
} FPoint;

/* mrowley - 081210 */
typedef struct
{
	unsigned short row;
	unsigned short col;
} WCoord;

typedef struct
{
	int       measured; /* 1 = valid measured value stored, could be extended to measured / fitted for use in reducing range for auto-focusing */
	WCoord    coord;    /* the (i,j) coordinate on the plate of the well */
	FPoint    pt;
} WPoint;

struct _well_plate_definer
{
    UIModule parent;
    
	int panel_id;
	
//	FPoint points[4];

	/* mrowley - 081210 */
	WPoint pts_corners[4]; //corners plus those measured in the centre
	WPoint pts_others[3];
	
	double         poly_coeffs[7];
	unsigned short poly_num_of_coeffs;
	WCoord         poly_pts_template[3];
	
	Microscope *ms;
	GciCamera *camera;
	XYStage *stage;
	Z_Drive *z_drive;	

	int _microscope_master_z_drive_changed_signal_id;
};


well_plate_definer* well_plate_definer_new(Microscope *ms);

void well_plate_definer_panel_display(well_plate_definer* wpd);

int well_plate_definer_panel_hide(well_plate_definer* wpd);

void well_plate_definer_destroy(well_plate_definer* wpd);

int poly_build_measured_pt_vectors_for_fit(well_plate_definer* wpd, double *x, double *y, double *z);

int poly_determine_wells_to_measure(well_plate_definer* wpd);

#endif
