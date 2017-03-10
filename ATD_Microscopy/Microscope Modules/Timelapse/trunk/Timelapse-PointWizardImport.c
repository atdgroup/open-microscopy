#include "camera\gci_camera.h"
#include "stage\stage.h"
#include "StagePlate.h"
#include "StagePlateUI.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "timelapse.h"
#include "PolyAlgos.h"
#include "TimeLapse-PointWizardImportUI.h"
#include "radioGroup.h" 

#include "microscope.h" 

#include <utility.h>
#include "math.h"

//#define WPD_TEST_PTS_ENABLED
#define TL_DIRECTION_HORZONTAL 0
#define TL_DIRECTION_VERTICAL  1
#define TL_START_POS_LEFT 1
#define TL_START_POS_RIGHT 0
#define TL_START_POS_TOP 1
#define TL_START_POS_BOTTOM 0

static TimelapseWellPlate predefined_plates[] = 
{
    {6, 3, 2},
    {12, 4, 3},
    {24, 6, 4},
    {48, 6, 8},
    {96, 12, 8},
    {396, 24, 16}
};


/*=================================================================================*/
/* Function:  poly_check_for_coincident_well_locations                             */
/*---------------------------------------------------------------------------------*/
/* Purpose:   Checks whether any of the additional well locations are coincident   */
/*            with each other or the corner well locations.                        */
/*---------------------------------------------------------------------------------*/
/* Ref:       None.                                                                */
/*---------------------------------------------------------------------------------*/
/* Notes:     Assumed that none of corner wells are coincident with each other.    */
/*---------------------------------------------------------------------------------*/
/* Arguments: wpd: the WPD 'global' data storage location, for the coordinates of  */
/*                 the corner wells and additional wells for polynomial fitting.   */
/*---------------------------------------------------------------------------------*/
/* Return:    0:   none of the corner well and additional wells are coincident.    */
/*            1:   at least one pair of well locations are coincident.             */
/*=================================================================================*/

static int poly_check_for_coincident_well_locations(TimelapseWellPlateDefiner* wpd)
{
    int a, b, c;

    for (a=0; a<3; a++)
    {
        /* Check whether any of the extra well location coordinates are coincident with each other... */
        for (b=a+1; b<3; b++)
        {
            if ((wpd->poly_pts_template[a].row == wpd->poly_pts_template[b].row) &&
                (wpd->poly_pts_template[a].col == wpd->poly_pts_template[b].col))
            {
                return (1);
            }
        }

        /* Check whether any of the extra well location coordinates are coincident with a corner well... */
        for (c=0; c<4; c++)
        {
            if ((wpd->poly_pts_template[a].row == wpd->pts_corners[c].coord.row) &&
                (wpd->poly_pts_template[a].col == wpd->pts_corners[c].coord.col))
            {
                return (1);
            }
        }
    }

    return (0);
}


/*=================================================================================*/
/* Function:  poly_determine_wells_to_measure                                      */
/*---------------------------------------------------------------------------------*/
/* Purpose:   Determines three sensible additional wells locations to be measured  */
/*            for polynomial fitting, according to [2] - Eqns. 2a, 2b, 2c.         */
/*---------------------------------------------------------------------------------*/
/* Ref:       [1] - Design documentation: Polynomial Well Plate Fitting,           */
/*                                        Mark Rowley, March 2009.                 */
/*---------------------------------------------------------------------------------*/
/* Notes:     None.                                                                */
/*---------------------------------------------------------------------------------*/
/* Arguments: wpd: the WPD 'global' data storage location, in this case for the    */
/*                 configuration details (i.e. wells accross, wells down) and      */
/*                 storage of additional well locations for polynomial fitting.    */
/*---------------------------------------------------------------------------------*/
/* Return:    Appropriate error code.                                              */
/*=================================================================================*/

static int poly_determine_wells_to_measure(TimelapseWellPlateDefiner* wpd)
{
    int M, N;
    int imin, imax, jmin, jmax, k;

    GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, &N);
    GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN,   &M);

    imin = 1;
    imax = M;
    jmin = 1;
    jmax = N;

    wpd->poly_pts_template[0].row = M<3 ? 1 : M/3;
    wpd->poly_pts_template[0].col = N%3 ? ((int)((1.0+(float)N-(float)N/3.0))) : (N-N/3);

    wpd->poly_pts_template[1].row = M<2 ? 1 : M/2;
    wpd->poly_pts_template[1].col = N<3 ? 1 : N/3;

    wpd->poly_pts_template[2].row = M%3 ? ((int)(1.0+(float)M-(float)M/3.0)) : (M-M/3);
    wpd->poly_pts_template[2].col = N%2 ? ((int)((float)N-(float)N/2.0)) : (N-N/2);

    /* Check that the coordinates are within the limits of the configuration... */
    for (k=0; k<3; k++)
    {
        if ((wpd->poly_pts_template[k].row < imin) ||
            (wpd->poly_pts_template[k].row > imax) ||
            (wpd->poly_pts_template[k].col < jmin) ||
            (wpd->poly_pts_template[k].col > jmax))
            return POLY_ERR_ILLEGAL_WELL_COORD;
    }

    /* Check that none are coincident with corner wells or each other (important for small plates)... */
    if (poly_check_for_coincident_well_locations(wpd))
        return POLY_ERR_COINCIDENT_WELL_COORD;

    return POLY_ERR_NO_ERROR;
}

/*=================================================================================*/
/* Function:  poly_build_measured_pt_vectors_for_fit                               */
/*---------------------------------------------------------------------------------*/
/* Purpose:   Builds single data point vectors from the measured corner well and   */
/*            additional well (x,y,z) triplets, as required by fitting routines.   */
/*---------------------------------------------------------------------------------*/
/* Ref:       None.                                                                */
/*---------------------------------------------------------------------------------*/
/* Notes:     None.                                                                */
/*---------------------------------------------------------------------------------*/
/* Arguments: wpd:     for access to the corner and additional well measurements.  */
/*            x, y, z: vectors for data point storage.                             */
/*---------------------------------------------------------------------------------*/
/* Return:    POLY_ERR_NO_ERROR.                                                    */
/*=================================================================================*/

static int poly_build_measured_pt_vectors_for_fit(TimelapseWellPlateDefiner* wpd, double *x, double *y, double *z)
{
	int i;

	for (i=0; i<4; i++)
	{
		x[i+1] = wpd->pts_corners[i].pt.x;
		y[i+1] = wpd->pts_corners[i].pt.y;
		z[i+1] = wpd->pts_corners[i].pt.z;
	}

	for (i=0; i<3; i++)
	{
		x[i+1+4] = wpd->pts_others[i].pt.x;
		y[i+1+4] = wpd->pts_others[i].pt.y;
		z[i+1+4] = wpd->pts_others[i].pt.z;
	}

	return POLY_ERR_NO_ERROR;
}

static int calculate_well_xy_position(TimelapseWellPlateDefiner* wpd, unsigned short i, unsigned short j, double *x, double *y)
{
    unsigned int N, M;
    double dx, dy, idx, jdy, one_idx, one_jdy;
    
	GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, &N);
    GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN,   &M);

	if(N < 1 || M < 1)
		return -1;
	
    dx = 1.0 / (N - 1.0);
    dy = 1.0 / (M - 1.0);
    
	if(dx < 0.00001 || dy < 0.00001)
		return -1;
	
    idx = i * dx;
    jdy = j * dy;
    one_idx = 1 - idx;
    one_jdy = 1 - jdy;
       
    *x = (wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.x * one_idx * one_jdy) + (wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].pt.x * idx * one_jdy) +
         (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].pt.x * one_idx * jdy) + (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].pt.x * idx * jdy);
    
    *y = (wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.y * one_idx * one_jdy) + (wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].pt.y * idx * one_jdy) +
         (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].pt.y * one_idx * jdy) + (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].pt.y * idx * jdy);
     
    return 0;
}


static int calculate_well_z_position_linear_fromxy(TimelapseWellPlateDefiner* wpd, double x, double y, double *z)
{
    double       dx, dy, idx, jdy, one_idx, one_jdy;
    
	dx = 1.0 / (wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].pt.x - wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.x);
	dy = 1.0 / (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].pt.y - wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.y);

	if(dx < 0.00001 || dy < 0.00001)
		return POLY_ERR_GENERAL_ERROR;

	idx = (x - wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.x) * dx;     // frac x pos relative to top left
	jdy = (y - wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.y) * dy;     // frac y pos relative to top left
    one_idx = 1 - idx;
    one_jdy = 1 - jdy;

    *z = (wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.z * one_idx * one_jdy) + (wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].pt.z * idx * one_jdy) +
         (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].pt.z * one_idx * jdy) + (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].pt.z * idx * jdy);

	return POLY_ERR_NO_ERROR;
}

static int calculate_well_z_position_linear(TimelapseWellPlateDefiner* wpd, unsigned short i, unsigned short j, double *z)
{
    unsigned int N, M;
    double       dx, dy, idx, jdy, one_idx, one_jdy;
    
    GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, &N);
    GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN,   &M);

	if(N < 1 || M < 1)
		return POLY_ERR_GENERAL_ERROR;
	
    dx = 1.0 / (N - 1.0);
    dy = 1.0 / (M - 1.0);
    
	if(dx < 0.00001 || dy < 0.00001)
		return POLY_ERR_GENERAL_ERROR;
	
    idx = i * dx;
    jdy = j * dy;
    one_idx = 1 - idx;
    one_jdy = 1 - jdy;

    *z = (wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.z * one_idx * one_jdy) + (wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].pt.z * idx * one_jdy) +
         (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].pt.z * one_idx * jdy) + (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].pt.z * idx * jdy);

	return POLY_ERR_NO_ERROR;
}

static int calculate_well_z_position_poly(TimelapseWellPlateDefiner* wpd, double x, double y, double *z)
{
	double       basis_fct_values[7];
	unsigned int i;

	poly_compute_basis_fct_values(x, y, basis_fct_values, wpd->poly_num_of_coeffs);

	*z = 0.0;

	for (i=0; i<wpd->poly_num_of_coeffs; i++)
		*z += basis_fct_values[i]*wpd->poly_coeffs[i];

    return POLY_ERR_NO_ERROR;
}

static int check_if_measured_well(TimelapseWellPlateDefiner* wpd, unsigned short col, unsigned short row, GCI_FPOINT *pt)
{
	int k;

	for (k=0; k<4; k++)
	{
		if ((wpd->pts_corners[k].coord.row == row) && (wpd->pts_corners[k].coord.col == col))
		{
			pt->x = wpd->pts_corners[k].pt.x;
			pt->y = wpd->pts_corners[k].pt.y;
			pt->z = wpd->pts_corners[k].pt.z;
			return 1;
		}
	}

	for (k=0; k<3; k++)
	{
		if ((wpd->pts_others[k].coord.row == row) && (wpd->pts_others[k].coord.col == col))
		{
			pt->x = wpd->pts_others[k].pt.x;
			pt->y = wpd->pts_others[k].pt.y;
			pt->z = wpd->pts_others[k].pt.z;
			return 1;
		}
	}

	return 0;
}

int calculate_well_position(TimelapseWellPlateDefiner* wpd, unsigned short i, unsigned short j, GCI_FPOINT *pt)
{
	double x, y, z;
	int    error; // linear_fit;

	error = POLY_ERR_NO_ERROR;

	/* First check whether this well (i,j) is one of the measured wells, there may be no need to fit... */
	if (!check_if_measured_well(wpd, i+1, j+1, pt))
	{
		calculate_well_xy_position(wpd, i, j, &x, &y);

		if (wpd->linear_fit)
			error = calculate_well_z_position_linear(wpd, i, j, &z);  // NB pass i,j here
		else
			error = calculate_well_z_position_poly(wpd, x, y, &z);    // NB pass x,y here

		pt->x = x;
		pt->y = y;
		pt->z = z;	
	}

    return error;
}

int calculate_well_z_position_for_xy(TimelapseWellPlateDefiner* wpd, double x, double y, GCI_FPOINT *pt)
{
	double z;
	int error = POLY_ERR_NO_ERROR;

	// cannot check for existing measured position in this case as only have xy coord.

	if (wpd->linear_fit)
		error = calculate_well_z_position_linear_fromxy(wpd, x, y, &z);
	else
		error = calculate_well_z_position_poly(wpd, x, y, &z);

	pt->x = x;
	pt->y = y;
	pt->z = z;	

    return error;
}

static int set_well_pt_details(TimelapseWPoint *well, unsigned int row, unsigned int col, GCI_FPOINT pt)
{
	well->measured  = 1;
	well->coord.row = row;
	well->coord.col = col;
	well->pt        = pt;

	return 1;
}

static char* fpoint_text(char *buf, GCI_FPOINT pt)
{
	memset(buf, 0, 1);
    sprintf(buf, "(%.2f,%.2f)", pt.x, pt.y);
    
    return buf;
}

static char* wpoint_text(char *buf, TimelapseWPoint well)
{
	memset(buf, 0, 1);

	if (well.measured)
		sprintf(buf, "(%d, %d), (%.2f, %.2f, %.2f)", well.coord.row, well.coord.col, well.pt.x, well.pt.y, well.pt.z);
	else
		sprintf(buf, "Not yet defined");

    return buf;
}


static void update_displayed_defined_wells(TimelapseWellPlateDefiner* wpd)
{
    char buf[50] = "";

	SetCtrlVal(wpd->custom_pnl, DEF_CORNER_TL_POINT, wpoint_text(buf, wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT]));
    SetCtrlVal(wpd->custom_pnl, DEF_CORNER_TR_POINT, wpoint_text(buf, wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT]));
    SetCtrlVal(wpd->custom_pnl, DEF_CORNER_BL_POINT, wpoint_text(buf, wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT]));
    SetCtrlVal(wpd->custom_pnl, DEF_CORNER_BR_POINT, wpoint_text(buf, wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT]));

	SetCtrlVal(wpd->custom_pnl, DEF_CORNER_POLY_POINT_1, wpoint_text(buf, wpd->pts_others[0]));
	SetCtrlVal(wpd->custom_pnl, DEF_CORNER_POLY_POINT_2, wpoint_text(buf, wpd->pts_others[1]));
	SetCtrlVal(wpd->custom_pnl, DEF_CORNER_POLY_POINT_3, wpoint_text(buf, wpd->pts_others[2]));

	if (wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].measured) SetCtrlAttribute(wpd->custom_pnl, DEF_CORNER_DEF_TOP_LEFT, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
	else SetCtrlAttribute(wpd->custom_pnl, DEF_CORNER_DEF_TOP_LEFT, ATTR_CMD_BUTTON_COLOR, 0x99FF99);
	if (wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].measured) SetCtrlAttribute(wpd->custom_pnl, DEF_CORNER_DEF_TOP_RIGHT, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
	else SetCtrlAttribute(wpd->custom_pnl, DEF_CORNER_DEF_TOP_RIGHT, ATTR_CMD_BUTTON_COLOR, 0x99FF99);
	if (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].measured) SetCtrlAttribute(wpd->custom_pnl, DEF_CORNER_DEF_BOTTOM_LEFT, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
	else SetCtrlAttribute(wpd->custom_pnl, DEF_CORNER_DEF_BOTTOM_LEFT, ATTR_CMD_BUTTON_COLOR, 0x99FF99);
	if (wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].measured) SetCtrlAttribute(wpd->custom_pnl, DEF_CORNER_DEF_BOTTOM_RIGHT, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
	else SetCtrlAttribute(wpd->custom_pnl, DEF_CORNER_DEF_BOTTOM_RIGHT, ATTR_CMD_BUTTON_COLOR, 0x99FF99);

}

void timelapse_well_plate_definer_destroy(TimelapseWellPlateDefiner* wpd)
{
	if (wpd->tl->_microscope_master_z_drive_changed_signal_id >= 0)
		microscope_master_z_drive_changed_handler_disconnect(wpd->tl->ms, wpd->tl->_microscope_master_z_drive_changed_signal_id);
	
	free(wpd);
}

//Get the user to define an xy point
static int timelapse_well_plate_definer_focus_on_point(TimelapseWellPlateDefiner* wpd, char* message)
{
    int msgPnl, pnl, ctrl;
    FIBITMAP *dib = NULL;
	char buffer[500];
	CameraState state;
	
    // Remember camera settings
    gci_camera_save_state(wpd->tl->camera, &state);

    microscope_set_focusing_mode(wpd->tl->ms);
    gci_camera_set_live_mode(wpd->tl->camera);
    gci_camera_activate_live_display(wpd->tl->camera);

    stage_set_joystick_on (wpd->tl->stage);

	find_resource("TimeLapse-PointWizardImportUI.uir", buffer); 

	msgPnl = LoadPanel(0, buffer, MSGPANEL); 
	
    SetPanelAttribute (msgPnl, ATTR_TITLE, "Set Extreme Well Position");
    SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, message);
    
	ui_module_clear_attached_panels(UIMODULE_CAST(wpd->tl->wiz));

	ui_module_attach_panel_to_panel(UIMODULE_CAST(wpd->tl->wiz), msgPnl,
												  UI_MODULE_REL_CENTRE,
												  0, 0);
	
	DisplayPanel(msgPnl);

    while (1) {
    
        ProcessSystemEvents();

        GetUserEvent (0, &pnl, &ctrl);
        
        if ((pnl == msgPnl) && (ctrl == MSGPANEL_OK))
            break; 

		#ifndef THREADED_CAM_AQ
		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(wpd->tl->camera, NULL); 			
		gci_camera_display_image(wpd->tl->camera, dib, NULL);	
		FreeImage_Unload(dib);   
		#endif
    }

    DiscardPanel(msgPnl);
    
    //Restore camera settings
	gci_camera_restore_state(wpd->tl->camera, &state);   
	
    gci_camera_set_snap_mode(wpd->tl->camera);
    
    
    return 0;
}


static int check_valid_well_location(TimelapseWellPlateDefiner* wpd, double x, double y)
{
	double xmin, xmax, ymin, ymax;
	int    i;
	
	xmin = wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.x;
	xmax = wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.x;
	
	ymin = wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.y;
	ymax = wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.y;

	for (i=1; i<4; i++)
	{
	    if (xmin > wpd->pts_corners[i].pt.x)
			xmin = wpd->pts_corners[i].pt.x;
		
		if (xmax < wpd->pts_corners[i].pt.x)
			xmax = wpd->pts_corners[i].pt.x;
		
		if (ymin > wpd->pts_corners[i].pt.y)
			ymin = wpd->pts_corners[i].pt.y;
		
		if (ymax < wpd->pts_corners[i].pt.y)
			ymax = wpd->pts_corners[i].pt.y;
	}
	
	if (((x>xmin)&&(x<xmax)) && ((y>ymin)&&(y<ymax)))
		return 1;
	else
		return 0;
}

static int timelapse_well_plate_definer_stage_auto_move_then_focus_on_point(TimelapseWellPlateDefiner* wpd, int i, int j, char* message)
{
    int         msgPnl, pnl, ctrl;
    FIBITMAP   *dib = NULL;
	char        buffer[500];
	CameraState state;
	double      x, y;
	
    // Remember camera settings
    gci_camera_save_state(wpd->tl->camera, &state);

	stage_set_joystick_off (wpd->tl->stage);

	calculate_well_xy_position(wpd, i-1, j-1, &x, &y);
	
	if (check_valid_well_location(wpd, x, y))
		stage_async_goto_xy_position(wpd->tl->stage, x, y);

    microscope_set_focusing_mode(wpd->tl->ms);

	// Wait for the stage to stop moving before allowing the user to focus
	stage_wait_for_stop_moving(wpd->tl->stage);

	gci_camera_set_live_mode(wpd->tl->camera);
    gci_camera_activate_live_display(wpd->tl->camera);

    stage_set_joystick_on (wpd->tl->stage);

	find_resource("TimeLapse-PointWizardImportUI.uir", buffer); 

	msgPnl = LoadPanel(0, buffer, MSGPANEL); 
	
    SetPanelAttribute (msgPnl, ATTR_TITLE, "Set Well Position");
    SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, message);
   

	ui_module_clear_attached_panels(UIMODULE_CAST(wpd->tl->wiz));

	ui_module_attach_panel_to_panel(UIMODULE_CAST(wpd->tl->wiz), msgPnl,
												  UI_MODULE_REL_CENTRE,
												  0, 0);

	 DisplayPanel(msgPnl);

    while (1) {
    
        ProcessSystemEvents();

        GetUserEvent (0, &pnl, &ctrl);
        
        if ((pnl == msgPnl) && (ctrl == MSGPANEL_OK))
            break;

		#ifndef THREADED_CAM_AQ
		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(wpd->tl->camera, NULL); 			
		gci_camera_display_image(wpd->tl->camera, dib, NULL);	
		FreeImage_Unload(dib);   
		#endif
    }

    DiscardPanel(msgPnl);
    
    //Restore camera settings
	gci_camera_restore_state(wpd->tl->camera, &state);   
	
    gci_camera_set_snap_mode(wpd->tl->camera);
    
    return 0;
}

static int check_all_corner_wells_measured(TimelapseWellPlateDefiner* wpd)
{
	if ((wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].measured     == 1) &&
		(wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].measured    == 1) &&
		(wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].measured  == 1) &&
		(wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].measured == 1))
		return 1;
	else
		return 0;
}

static int check_all_poly_wells_measured(TimelapseWellPlateDefiner* wpd)
{
	if ((wpd->pts_others[0].measured == 1) &&
		(wpd->pts_others[1].measured == 1) &&
		(wpd->pts_others[2].measured == 1))
		return 1;
	else
		return 0;
}

static int check_all_wells_measured_for_pts_file_generation(TimelapseWellPlateDefiner* wpd)
{
	if (wpd->linear_fit)
		return (check_all_corner_wells_measured(wpd));
	else
		return (check_all_corner_wells_measured(wpd) & check_all_poly_wells_measured(wpd));
}

static void poly_activate_well_details_display(int panel)
{
	SetCtrlAttribute(panel, DEF_CORNER_TEXTMSG_11,   ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, DEF_CORNER_TEXTMSG_14,   ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, DEF_CORNER_POLY_POINT_1, ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, DEF_CORNER_TEXTMSG_12,   ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, DEF_CORNER_POLY_POINT_2, ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, DEF_CORNER_TEXTMSG_13,   ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, DEF_CORNER_POLY_POINT_3, ATTR_DIMMED, 0);
}

static void poly_deactivate_well_details_display(int panel)
{
	SetCtrlAttribute(panel, DEF_CORNER_TEXTMSG_11,   ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, DEF_CORNER_TEXTMSG_14,   ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, DEF_CORNER_POLY_POINT_1, ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, DEF_CORNER_TEXTMSG_12,   ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, DEF_CORNER_POLY_POINT_2, ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, DEF_CORNER_TEXTMSG_13,   ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, DEF_CORNER_POLY_POINT_3, ATTR_DIMMED, 1);
}

static int poly_measure_wells_and_do_fit(TimelapseWellPlateDefiner* wpd)
{
	int       i;
	int       row, col;
	int       errcode, err1, err2, err3;
	double   *x, *y, *z, chisq;
	GCI_FPOINT    pt;
    FIBITMAP *dib = NULL;

	errcode = poly_determine_wells_to_measure(wpd); 
	if (POLY_ERR_NO_ERROR != errcode)
	{
		// Force a linear fit 
		wpd->linear_fit = 1;

		// original code notifies the user here, do we need to do this?

		return errcode; // is this the right thing to do?
	}

	poly_activate_well_details_display(wpd->custom_pnl);

	for (i=0; i<3; i++)
	{
		row = (wpd->poly_pts_template[i].row);
		col = (wpd->poly_pts_template[i].col);

		timelapse_well_plate_definer_stage_auto_move_then_focus_on_point(wpd, col, row, "Once stage has relocated,\nFocus in selected well.");

		stage_get_xy_position(wpd->tl->stage, &(pt.x), &(pt.y));
		z_drive_get_position(wpd->tl->z_drive, &(pt.z));

		set_well_pt_details(&(wpd->pts_others[i]), row, col, pt);

#ifdef WPD_TEST_PTS_ENABLED

        switch (i)
        {
            case 0:
                {
                    wpd->pts_others[0].measured  = 1;
				    wpd->pts_others[0].coord.row = 2;
				    wpd->pts_others[0].coord.col = 7;
				    wpd->pts_others[0].pt.x      = 12968.10;
				    wpd->pts_others[0].pt.y      = -5499.33;
				    wpd->pts_others[0].pt.z      = 56.37;
                    break;            
                }

            case 1:
                {
				    wpd->pts_others[1].measured  = 1;
				    wpd->pts_others[1].coord.row = 3;
				    wpd->pts_others[1].coord.col = 3;
				    wpd->pts_others[1].pt.x      = -22965.24;
				    wpd->pts_others[1].pt.y      = 2451.39;
				    wpd->pts_others[1].pt.z      = 69.89;
                    break;
                }

            case 2:
                {
				    wpd->pts_others[2].measured  = 1;
				    wpd->pts_others[2].coord.row = 5;
				    wpd->pts_others[2].coord.col = 8;
				    wpd->pts_others[2].pt.x      = 21596.29;
				    wpd->pts_others[2].pt.y      = 21019.89;
				    wpd->pts_others[2].pt.z      = -56.01;
                    break;
                }

            default:
                break;
        }

#endif

                 update_displayed_defined_wells(wpd);
	}

	x = mtrx_CreateDoubleVector(0,7,&err1);
	y = mtrx_CreateDoubleVector(0,7,&err2);
	z = mtrx_CreateDoubleVector(0,7,&err3);

	if ((err1 != MATRIX_ALLOCATION_OKAY) ||
		(err2 != MATRIX_ALLOCATION_OKAY) ||
		(err2 != MATRIX_ALLOCATION_OKAY))

	wpd->poly_num_of_coeffs = 6;

	poly_build_measured_pt_vectors_for_fit(wpd, x, y, z);

	errcode = poly_determine_coeffs(wpd->poly_coeffs, wpd->poly_num_of_coeffs, x, y, z, 7, &chisq);

	free_dvector(x,0,7);
	free_dvector(y,0,7);
	free_dvector(z,0,7);

#if 0
	if (errcode < POLY_ERR_NO_ERROR)
	{
		/* Need to display a message panel, user can only do linear fit, error code  */
		/* Need to force linear fitting */
	}
#endif

	return POLY_ERR_NO_ERROR;
}



int CVICALLBACK OnTimelapseManualWellDefine (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
	unsigned int N, M, was_live=0;

    switch (event)
    {
        case EVENT_COMMIT:
        {
            GCI_FPOINT pt;
            TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;

			was_live = gci_camera_is_live_mode(wpd->tl->camera);
        
            //Define a new start point - adjust the wpd dimensions

			if(wpd->generate_from_stage_plate) { // USING A STAGE PLATE DEFINITION

				POINT top_left, bottom_right;
				StagePlate plate;

				StagePlateModule* spm = microscope_get_stage_plate_module(wpd->tl->ms);
				stage_plate_get_current_plate(spm, &plate);

				stage_plate_get_topleft_and_bottomright_centres_for_plate( 
					 spm, &plate, &top_left, &bottom_right);

				// Make sure the number of wells is set in the ctrls for manual selection
				// as some code uses these values and this is a bit of a hack of old and new code.
				stage_plate_get_rows_cols_for_selected_wells_on_plate(spm, &plate, &M, &N);
				SetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, N);
				SetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN,   M);

				switch(control)
				{
					case DEF_CORNER_DEF_TOP_LEFT: {
						stage_goto_xy_position(wpd->tl->stage, top_left.x, top_left.y);
						break;
					}
					case DEF_CORNER_DEF_TOP_RIGHT: {
						stage_goto_xy_position(wpd->tl->stage, bottom_right.x, top_left.y);
						break;
					}
					case DEF_CORNER_DEF_BOTTOM_LEFT: {
						stage_goto_xy_position(wpd->tl->stage, top_left.x, bottom_right.y);
						break;
					}
					case DEF_CORNER_DEF_BOTTOM_RIGHT: {
						stage_goto_xy_position(wpd->tl->stage, bottom_right.x, bottom_right.y);
						break;
					}
				}

				timelapse_well_plate_definer_focus_on_point(wpd, "Please focus.");
			}
			else   // OLD CODE THAT USES MANUAL OR PREDEFINED PLATE (NOT STAGE PLATE)
				timelapse_well_plate_definer_focus_on_point(wpd, "Move stage to the well\nposition and focus.");

            
			// COMBINED CODE FOR ALL INSTANCES

			stage_get_xy_position (wpd->tl->stage, &(pt.x), &(pt.y));
			z_drive_get_position (wpd->tl->z_drive, &(pt.z));

			GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, &N);
			GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN,   &M);

            switch(control)
            {
                case DEF_CORNER_DEF_TOP_LEFT:
					{
						set_well_pt_details(&(wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT]), 1, 1, pt);

#ifdef WPD_TEST_PTS_ENABLED //to test fitting, remove later...
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].measured  = 1;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].coord.row = 1;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].coord.col = 1;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.x      = -40152.79;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.y      = -14152.66;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_LEFT].pt.z      = 3.85;
#endif

						break;
					}
				case DEF_CORNER_DEF_TOP_RIGHT:
					{
						set_well_pt_details(&(wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT]), 1, N, pt);

#ifdef WPD_TEST_PTS_ENABLED //to test fitting, remove later...
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].measured  = 1;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].coord.row = 1;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].coord.col = 10;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].pt.x      = 41126.17;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].pt.y      = -14553.98;
				wpd->pts_corners[TIMELAPSE_PLATE_TOP_RIGHT].pt.z      = -44.49;
#endif

						break;
					}    
				case DEF_CORNER_DEF_BOTTOM_LEFT:
					{
						set_well_pt_details(&(wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT]), M, 1, pt);

#ifdef WPD_TEST_PTS_ENABLED //to test fitting, remove later...
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].measured  = 1;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].coord.row = 6;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].coord.col = 1;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].pt.x      = -40805.36;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].pt.y      = 30901.53;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_LEFT].pt.z      = 14.59;
#endif

						break;
					}    
				case DEF_CORNER_DEF_BOTTOM_RIGHT:
					{
						set_well_pt_details(&(wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT]), M, N, pt);

#ifdef WPD_TEST_PTS_ENABLED //to test fitting, remove later...
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].measured  = 1;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].coord.row = 6;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].coord.col = 10;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].pt.x      = 39605.17;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].pt.y      = 30901.53;
				wpd->pts_corners[TIMELAPSE_PLATE_BOTTOM_RIGHT].pt.z      = -17.80;
#endif

						break;
					}    
            }

			update_displayed_defined_wells(wpd);
            
			if (was_live) {//Leave in live mode to keep BV happy
				gci_camera_set_live_mode(wpd->tl->camera);
				gci_camera_activate_live_display(wpd->tl->camera);
			}

			if (check_all_corner_wells_measured(wpd))
			{
                if (!wpd->linear_fit)
                {
				    if (POLY_ERR_NO_ERROR != poly_measure_wells_and_do_fit(wpd))
						wpd->linear_fit = 1;
				}
	
				// Undim Next button
				wizard_set_next_button_dimmed(wpd->tl->wiz, 0);
			}

            break;
        }
    }

    return 0;
}


#if 0
static int calculate_well_position(TimelapseWellPlateDefiner* wpd, unsigned short i, unsigned short j, GCI_FPOINT *pt)
{
    unsigned int N, M;
    double dx, dy, idx, jdy, one_idx, one_jdy;
    
    GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, &N);
    GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_DOWN, &M);
    
	if(N < 1 || M < 1)
		return -1;
	
    dx = 1.0 / (N - 1.0);
    dy = 1.0 / (M - 1.0);
    
	if(dx < 0.00001 || dy < 0.00001)
		return -1;
	
    idx = i * dx;
    jdy = j * dy;
    one_idx = 1 - idx;
    one_jdy = 1 - jdy;
   
    //x1 = wpd->points[TIMELAPSE_PLATE_TOP_LEFT].x;
    //x2 = wpd->points[TIMELAPSE_PLATE_TOP_RIGHT].x;
    //x3 = wpd->points[TIMELAPSE_PLATE_BOTTOM_LEFT].x;
    //x4 = wpd->points[TIMELAPSE_PLATE_BOTTOM_RIGHT].x;
       
    pt->x = (wpd->points[TIMELAPSE_PLATE_TOP_LEFT].x * one_idx * one_jdy) + (wpd->points[TIMELAPSE_PLATE_TOP_RIGHT].x * idx * one_jdy) +
           (wpd->points[TIMELAPSE_PLATE_BOTTOM_LEFT].x * one_idx * jdy) + (wpd->points[TIMELAPSE_PLATE_BOTTOM_RIGHT].x * idx * jdy);

    //y1 = wpd->points[TIMELAPSE_PLATE_TOP_LEFT].y;
    //y2 = wpd->points[TIMELAPSE_PLATE_TOP_RIGHT].y;
    //y3 = wpd->points[TIMELAPSE_PLATE_BOTTOM_LEFT].y;
    //y4 = wpd->points[TIMELAPSE_PLATE_BOTTOM_RIGHT].y;
    
    pt->y = (wpd->points[TOP_LEFT].y * one_idx * one_jdy) + (wpd->points[TIMELAPSE_PLATE_TOP_RIGHT].y * idx * one_jdy) +
           (wpd->points[BOTTOM_LEFT].y * one_idx * jdy) + (wpd->points[TIMELAPSE_PLATE_BOTTOM_RIGHT].y * idx * jdy);
    
    pt->z = (wpd->points[TOP_LEFT].z * one_idx * one_jdy) + (wpd->points[TIMELAPSE_PLATE_TOP_RIGHT].z * idx * one_jdy) +
           (wpd->points[BOTTOM_LEFT].z * one_idx * jdy) + (wpd->points[TIMELAPSE_PLATE_BOTTOM_RIGHT].z * idx * jdy);
     
    return 0;
}  
#endif



int CVICALLBACK OnTimelapseRegionSelect (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
            TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;	
			int val;

			GetCtrlVal(wpd->direction_pnl, DIR_START_CHOICE, &val);

			SetCtrlAttribute(wpd->direction_pnl, DIR_START_XSIZE, ATTR_DIMMED, !val);
			SetCtrlAttribute(wpd->direction_pnl, DIR_START_YSIZE, ATTR_DIMMED, !val);
		}
	}
	return 0;
}


int timelapse_well_plate_definer_export_points_to_file (TimelapseWellPlateDefiner* wpd, char *filepath)
{
	FILE *fp; 
    int direction = TL_DIRECTION_HORZONTAL, shortest_path = 0, start_position = TL_START_POS_LEFT;
    int i = 0, j = 0, M, N;
    GCI_FPOINT pt;
	
	GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, &N);
	GetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN, &M);

    fp = fopen(filepath, "w");

	if (fp==NULL) return -1;
	
	fprintf(fp, "#Version 1.0\n");
	fprintf(fp, "#Microscope %s\n", UIMODULE_GET_NAME(wpd->tl->ms));
	fprintf(fp, "%d\n", N * M);   
	
    // what Direction are we exporting the plates ?
    GetCtrlVal(wpd->direction_pnl, DIR_START_DIR_GEN, &direction);
	GetCtrlVal(wpd->direction_pnl, DIR_START_SHORTEST_PATH, &shortest_path);       

    if(direction == TL_DIRECTION_HORZONTAL)
    {
        // Are we going from left to right or right to left ?
        GetCtrlVal(wpd->direction_pnl, DIR_START_START_POSITION, &start_position);
    
        for(j=0; j < M; j++) {   // Export horizontally first 
	        if(((start_position == TL_START_POS_LEFT) && !shortest_path) || ((start_position == TL_START_POS_LEFT) 
				&& shortest_path && j%2==0) || ((start_position != TL_START_POS_LEFT) && shortest_path && j%2==1))
	        {
                for(i=0; i < N; i++) {      
                    
					if(calculate_well_position(wpd, i, j, &pt) < 0)
						goto WPD_ERROR; 
					
                    fprintf(fp, "%f\t%f\t%f\n", pt.x, pt.y, pt.z);
                }
            }
	        else
	        {
   	             // Going RIGHT to LEFT                   
                for(i=N-1; i >= 0; i--) {  
					
                    if(calculate_well_position(wpd, i, j, &pt) < 0)
						goto WPD_ERROR; 
					
                    fprintf(fp, "%f\t%f\t%f\n", pt.x, pt.y, pt.z);   
                }
            }
        }  
    }
    else
    {
        // We are exporting vertically first
        // Are we going from top to bottom or bottom to top ?
         GetCtrlVal(wpd->direction_pnl, DIR_START_START_POSITION_TB, &start_position);
    
       	for(i=0; i < N; i++) {      

			// Going BOTTOM to TOP

	        if(((start_position == TL_START_POS_BOTTOM) && !shortest_path) || ((start_position == TL_START_POS_BOTTOM) 
				&& shortest_path && i%2==0) || ((start_position != TL_START_POS_BOTTOM) && shortest_path && i%2==1))
        	{
                for(j=M-1; j >= 0; j--) {
					
                    if(calculate_well_position(wpd, i, j, &pt) < 0)
						goto WPD_ERROR;
					
                    fprintf(fp, "%f\t%f\t%f\n", pt.x, pt.y, pt.z);   
                }
            }    
	        else
    	    {
        	    
				// Going Top to Bottom

                for(j=0; j < M; j++) {
					
                    if(calculate_well_position(wpd, i, j, &pt) < 0)
						goto WPD_ERROR; 
					
                    fprintf(fp, "%f\t%f\t%f\n", pt.x, pt.y, pt.z);   
                }
            }
        }
    }
    
    fclose(fp);
	
	return 0;
	
	WPD_ERROR:
	
		fclose(fp);  
		return -1;
}

/*
int CVICALLBACK OnTimelapseWellplateExport (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;   
	
			char path[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];

			//check here that required pts have been measured
			if (check_all_wells_measured_for_pts_file_generation(wpd))
			{
				//Save positions to a text file. Save relative to the zero limit switch.
				microscope_get_user_data_directory(wpd->tl->ms, path);         

				if (FileSelectPopup (path, "*.pts", "*.pts", "Save Points", VAL_SAVE_BUTTON, 0, 0, 1, 1, filepath) < 1)
					return -1;
	            
				if (export_points_to_file (wpd, filepath)<0) 
				{
					GCI_MessagePopup("Error", "Could not save file");
					return -1;
				}			
			}
			else
			{
				GCI_MessagePopup("Error", "Sufficient wells not yet defined");
				return -1;			
			}

			
            break;
        }
    }

    return 0;
}
*/

int CVICALLBACK OnTimelapseWellplatePreDefinedWellRingChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;   

            int val;
            
            GetCtrlVal(panel, control, &val);
            
            if(val == TIMELAPSE_WELL_PLATE_CUSTOM) {
            
                SetCtrlAttribute(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, ATTR_DIMMED, 0);
                SetCtrlAttribute(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN, ATTR_DIMMED, 0);
            }
            else {
                TimelapseWellPlate plate;
                
                SetCtrlAttribute(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, ATTR_DIMMED, 1);
                SetCtrlAttribute(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN, ATTR_DIMMED, 1);
                
                // Ok val will be 0 and above so we can index into the predefined well plate array.
                plate = predefined_plates[val];
                
                SetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, plate.horizontal_size);
                SetCtrlVal(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN, plate.vertical_size);
			}

            break;
        }
    }

    return 0;
}



int CVICALLBACK OnTimelapseWellplateWellsAcross (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;   

            int val, M, N;
            
            GetCtrlVal(panel, STEP1_1_PRE_WELL_RING, &val);
            
            if(val == TIMELAPSE_WELL_PLATE_CUSTOM)
			{
                GetCtrlVal(panel, control, &N);
				GetCtrlVal(panel, STEP1_1_WELLS_DOWN, &M);
            }

            break;
        }
    }

    return 0;
}



int CVICALLBACK OnTimelapseWellplateWellsDown (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;   

            int val, M, N;
            
            GetCtrlVal(panel, STEP1_1_PRE_WELL_RING, &val);
            
            if(val == TIMELAPSE_WELL_PLATE_CUSTOM)
			{
                GetCtrlVal(panel, STEP1_1_WELLS_ACROSS, &N);
				GetCtrlVal(panel, control, &M);
            }

            break;
        }
    }

    return 0;
}

static void OnMasterZDriveChanged (Microscope* microscope, void *data)
{
	timelapse *tl = (timelapse*) data;

    tl->z_drive = microscope_get_master_zdrive (tl->ms); 	
}

static void clear_existing_pts_and_poly_coeffs(TimelapseWellPlateDefiner *wpd)
{
    int    i;
    TimelapseWCoord nullwcoord;
    TimelapseWPoint nullwpoint;

    nullwcoord.col       = 0;
    nullwcoord.row       = 0;
    nullwpoint.measured  = 0;
    nullwpoint.coord     = nullwcoord;
    nullwpoint.pt.x      = 0.0;
    nullwpoint.pt.y      = 0.0;
    nullwpoint.pt.z      = 0.0;

    for (i=0; i<7; i++)
        wpd->poly_coeffs[i] = 0.0;

    for (i=0; i<4; i++)
        wpd->pts_corners[i] = nullwpoint;

    for (i=0; i<3; i++)
        wpd->pts_others[i]  = nullwpoint;

	wpd->linear_fit = 0;
}



int CVICALLBACK OnTimelapseWellplateClearPoints (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;

            clear_existing_pts_and_poly_coeffs(wpd);
            
            update_displayed_defined_wells(wpd);

            poly_deactivate_well_details_display(wpd->custom_pnl);

            /* Restore default plate setting... */
            SetCtrlVal(wpd->direction_pnl, STEP1_1_PRE_WELL_RING, 4);
            SetCtrlVal(wpd->direction_pnl, STEP1_1_WELLS_ACROSS, 12);
            SetCtrlVal(wpd->direction_pnl, STEP1_1_WELLS_DOWN,    8);
            SetCtrlAttribute(wpd->direction_pnl, STEP1_1_WELLS_ACROSS, ATTR_DIMMED, 1);
            SetCtrlAttribute(wpd->direction_pnl, STEP1_1_WELLS_DOWN, ATTR_DIMMED, 1);

            break;
        }
    }

    return 0;
}


static void poly_init_coeff_details(TimelapseWellPlateDefiner *wpd, unsigned short numofcoeffs)
{
	wpd->poly_num_of_coeffs = numofcoeffs;
	memset(wpd->poly_coeffs, 0, sizeof(double) * numofcoeffs);
}

static void reset_define_corners_panel (TimelapseWellPlateDefiner* wpd)
{
            clear_existing_pts_and_poly_coeffs(wpd);
            update_displayed_defined_wells(wpd);
            poly_deactivate_well_details_display(wpd->custom_pnl);
}


static int CVICALLBACK OnWizardStagePlateChooseWellsPressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;
			timelapse *tl = wpd->tl;

			StagePlateModule* stage_plate_module = microscope_get_stage_plate_module(tl->ms);

			PlateDialogResult result = stage_plate_display_region_selection_dialog(stage_plate_module);

			wpd->use_selected_wells = 1;

			if (result > PLATE_SEL_DIALOG_NONE){  // something changed
				// Reset Define Corners panel as stage plate has changed
				reset_define_corners_panel(wpd);
			}

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnWizardStagePlateChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int pos = -1;
			TimelapseWellPlateDefiner* wpd = (TimelapseWellPlateDefiner*) callbackData;
			timelapse *tl = wpd->tl;

			StagePlateModule* stage_plate_module = microscope_get_stage_plate_module(tl->ms);

			GetCtrlVal(panel, control, &pos);
			
			stage_plate_move_to_position(stage_plate_module, pos);

			// Reset Define Corners panel as stage plate has changed
			reset_define_corners_panel(wpd);

			break;
		}
	}
	
	return 0;
}

TimelapseWellPlateDefiner* timelapse_timelapse_well_plate_definer_new(timelapse *tl)
{
    TimelapseWellPlateDefiner *wpd = (TimelapseWellPlateDefiner*) malloc (sizeof(TimelapseWellPlateDefiner));

	wpd->tl = tl;
	wpd->use_selected_wells = 0;

	memset(wpd->pts_corners, 0, sizeof(TimelapseWPoint) * 4);
	memset(wpd->pts_others,  0, sizeof(TimelapseWPoint) * 3);
	wpd->linear_fit = 0;

	poly_init_coeff_details(wpd, 6);
	
	wpd->manual_or_auto_pnl = wizard_list_add_step(tl->wiz,  "ManualOrAutomaticChoice", "Manual or Automatic Well Positions",
				"Choose whether to define points using the installed Stage Plate or a manually defined plate", MAN_OR_ST);   
	
	wpd->custom_or_predefined_pnl = wizard_list_add_step(tl->wiz,  "DefineManualPlate", "Manual Plate Layout",
				"Chose the well layout for the plate", STEP1_1);

	wpd->custom_pnl = wizard_list_add_step(tl->wiz,  "DefineCorners", "Define Plate Extremes and Focus Map",
		"Automaic Well Positions: Optionally click 'Define' to visit corner wells + 3 internal wells for focussing.\nManual Well Positions: Please click 'Define' and move the stage to the corner well centres and focus.", DEF_CORNER);

	wpd->direction_pnl = wizard_list_add_step(tl->wiz,  "DerectionGeneration", "Well Visiting Order",
				"Choose the order and direction in which the wells will be visited.", DIR_START);

	wpd->plate_select_pnl = wizard_list_add_step(tl->wiz,  "StagePlateSelection", "Stage Plate Selection",
				"Choose the installed stage plate you wish to use.\nOptionally choose a smaller selection of wells from the plate.", SP_SELECT);
	

	//if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage_plate_module->dc), "ConfigChanged", OnConfigChanged, stage_plate_module) == SIGNAL_ERROR) {
	//	hardware_device_send_error_text(HARDWARE_DEVICE_CAST(stage_plate_module), "Can not connect signal handler for ConfigChanged signal");
	//	return STAGE_PLATE_MODULE_ERROR;
	//}	

	Radio_ConvertFromTree (wpd->manual_or_auto_pnl, MAN_OR_ST_CHOICE);

	InstallCtrlCallback (wpd->plate_select_pnl, SP_SELECT_POS, OnWizardStagePlateChanged, wpd);
	InstallCtrlCallback (wpd->plate_select_pnl, SP_SELECT_CHOOSE_WELLS, OnWizardStagePlateChooseWellsPressed, wpd);

	InstallCtrlCallback(wpd->custom_or_predefined_pnl, STEP1_1_PRE_WELL_RING, OnTimelapseWellplatePreDefinedWellRingChanged, wpd);
	InstallCtrlCallback(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_ACROSS, OnTimelapseWellplateWellsAcross, wpd);
	InstallCtrlCallback(wpd->custom_or_predefined_pnl, STEP1_1_WELLS_DOWN, OnTimelapseWellplateWellsDown, wpd);

	InstallCtrlCallback(wpd->custom_pnl, DEF_CORNER_DEF_TOP_LEFT, OnTimelapseManualWellDefine, wpd);
    InstallCtrlCallback(wpd->custom_pnl, DEF_CORNER_DEF_TOP_RIGHT, OnTimelapseManualWellDefine, wpd);
    InstallCtrlCallback(wpd->custom_pnl, DEF_CORNER_DEF_BOTTOM_LEFT, OnTimelapseManualWellDefine, wpd);
    InstallCtrlCallback(wpd->custom_pnl, DEF_CORNER_DEF_BOTTOM_RIGHT, OnTimelapseManualWellDefine, wpd);

	InstallCtrlCallback(wpd->direction_pnl, DIR_START_CHOICE, OnTimelapseRegionSelect, wpd);

	wpd->tl->_microscope_master_z_drive_changed_signal_id = microscope_master_z_drive_changed_handler_connect(wpd->tl->ms, OnMasterZDriveChanged, wpd->tl);

    update_displayed_defined_wells(wpd);
		
    return wpd;
}