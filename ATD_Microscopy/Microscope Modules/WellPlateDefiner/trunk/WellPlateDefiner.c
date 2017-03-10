#include "WellPlateDefiner.h"
#include "PolyAlgos.h"
#include "WellPlateDefiner_ui.h"

#include "camera\gci_camera.h"
#include "stage\stage.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "timelapse.h"

#include "microscope.h" 

#include <utility.h>
#include "math.h"

//#define WPD_TEST_PTS_ENABLED

static WellPlate predefined_plates[] = 
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

static int poly_check_for_coincident_well_locations(well_plate_definer* wpd)
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

int poly_determine_wells_to_measure(well_plate_definer* wpd)
{
    int M, N;
    int imin, imax, jmin, jmax, k;

    GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, &N);
    GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_DOWN,   &M);

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

int poly_build_measured_pt_vectors_for_fit(well_plate_definer* wpd, double *x, double *y, double *z)
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

static int calculate_well_xy_position(well_plate_definer* wpd, unsigned short i, unsigned short j, double *x, double *y)
{
    unsigned int N, M;
    double dx, dy, idx, jdy, one_idx, one_jdy;
    
    GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, &N);
    GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_DOWN,   &M);
    
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
       
    *x = (wpd->pts_corners[TOP_LEFT].pt.x * one_idx * one_jdy) + (wpd->pts_corners[TOP_RIGHT].pt.x * idx * one_jdy) +
         (wpd->pts_corners[BOTTOM_LEFT].pt.x * one_idx * jdy) + (wpd->pts_corners[BOTTOM_RIGHT].pt.x * idx * jdy);
    
    *y = (wpd->pts_corners[TOP_LEFT].pt.y * one_idx * one_jdy) + (wpd->pts_corners[TOP_RIGHT].pt.y * idx * one_jdy) +
         (wpd->pts_corners[BOTTOM_LEFT].pt.y * one_idx * jdy) + (wpd->pts_corners[BOTTOM_RIGHT].pt.y * idx * jdy);
     
    return 0;
}



static int calculate_well_z_position_linear(well_plate_definer* wpd, unsigned short i, unsigned short j, double *z)
{
    unsigned int N, M;
    double       dx, dy, idx, jdy, one_idx, one_jdy;
    
    GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, &N);
    GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_DOWN, &M);
    
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

    *z = (wpd->pts_corners[TOP_LEFT].pt.z * one_idx * one_jdy) + (wpd->pts_corners[TOP_RIGHT].pt.z * idx * one_jdy) +
         (wpd->pts_corners[BOTTOM_LEFT].pt.z * one_idx * jdy) + (wpd->pts_corners[BOTTOM_RIGHT].pt.z * idx * jdy);

	return POLY_ERR_NO_ERROR;
}



static int calculate_well_z_position_poly(well_plate_definer* wpd, double x, double y, double *z)
{
	double       basis_fct_values[7];
	unsigned int i;

	poly_compute_basis_fct_values(x, y, basis_fct_values, wpd->poly_num_of_coeffs);

	*z = 0.0;

	for (i=0; i<wpd->poly_num_of_coeffs; i++)
		*z += basis_fct_values[i]*wpd->poly_coeffs[i];

    return POLY_ERR_NO_ERROR;
}



static int check_if_measured_well(well_plate_definer* wpd, unsigned short col, unsigned short row, FPoint *pt)
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



static int calculate_well_position(well_plate_definer* wpd, unsigned short i, unsigned short j, FPoint *pt)
{
	double x, y, z;
	int    error, linear_fit;

	error = POLY_ERR_NO_ERROR;

	/* First check whether this well (i,j) is one of the measured wells, there may be no need to fit... */
	if (!check_if_measured_well(wpd, i+1, j+1, pt))
	{
		calculate_well_xy_position(wpd, i, j, &x, &y);

		GetCtrlVal(wpd->panel_id, WELL_PANEL_LINEAR_FIT, &linear_fit);

		if (linear_fit)
			error = calculate_well_z_position_linear(wpd, i, j, &z);
		else
			error = calculate_well_z_position_poly(wpd, x, y, &z);

		pt->x = x;
		pt->y = y;
		pt->z = z;	
	}

    return error;
}



static int set_well_pt_details(WPoint *well, unsigned int row, unsigned int col, FPoint pt)
{
	well->measured  = 1;
	well->coord.row = row;
	well->coord.col = col;
	well->pt        = pt;

	return 1;
}



static void force_and_dim_linear_fit_checkbox(well_plate_definer *wpd)
{
	SetCtrlVal(wpd->panel_id, WELL_PANEL_LINEAR_FIT, 1);
	SetCtrlAttribute(wpd->panel_id, WELL_PANEL_LINEAR_FIT, ATTR_DIMMED, 1);
}



static void check_well_config_for_forced_linear_fit(well_plate_definer *wpd, int horizontal_size, int vertical_size)
{
	/* Force a linear fit if the well configuration too small for polynomial fitting... */
	if ((horizontal_size * vertical_size) < 7)
		force_and_dim_linear_fit_checkbox(wpd);
}



static char* fpoint_text(char *buf, FPoint pt)
{
	memset(buf, 0, 1);
    sprintf(buf, "(%.2f,%.2f)", pt.x, pt.y);
    
    return buf;
}



static char* wpoint_text(char *buf, WPoint well)
{
	memset(buf, 0, 1);

	if (well.measured)
		sprintf(buf, "(%d, %d), (%.2f, %.2f, %.2f)", well.coord.row, well.coord.col, well.pt.x, well.pt.y, well.pt.z);
	else
		sprintf(buf, "Not yet defined");

    return buf;
}



static void update_displayed_defined_wells(well_plate_definer* wpd)
{
    char buf[50] = "";

	SetCtrlVal(wpd->panel_id, WELL_PANEL_TL_POINT, wpoint_text(buf, wpd->pts_corners[TOP_LEFT]));
    SetCtrlVal(wpd->panel_id, WELL_PANEL_TR_POINT, wpoint_text(buf, wpd->pts_corners[TOP_RIGHT]));
    SetCtrlVal(wpd->panel_id, WELL_PANEL_BL_POINT, wpoint_text(buf, wpd->pts_corners[BOTTOM_LEFT]));
    SetCtrlVal(wpd->panel_id, WELL_PANEL_BR_POINT, wpoint_text(buf, wpd->pts_corners[BOTTOM_RIGHT]));

	SetCtrlVal(wpd->panel_id, WELL_PANEL_POLY_POINT_1, wpoint_text(buf, wpd->pts_others[0]));
	SetCtrlVal(wpd->panel_id, WELL_PANEL_POLY_POINT_2, wpoint_text(buf, wpd->pts_others[1]));
	SetCtrlVal(wpd->panel_id, WELL_PANEL_POLY_POINT_3, wpoint_text(buf, wpd->pts_others[2]));
}



static void update_displayed_defined_well(well_plate_definer* wpd, int text_id, WPoint well)
{
    char buf[50] = "";

	SetCtrlVal(wpd->panel_id, text_id, wpoint_text(buf, well));
}



void well_plate_definer_panel_display(well_plate_definer* wpd)
{
    ui_module_display_main_panel(UIMODULE_CAST(wpd)); 
}



void well_plate_definer_destroy(well_plate_definer* wpd)
{
	if (wpd->_microscope_master_z_drive_changed_signal_id >=0)
		microscope_master_z_drive_changed_handler_disconnect(wpd->ms, wpd->_microscope_master_z_drive_changed_signal_id);
	
	ui_module_destroy(UIMODULE_CAST(wpd));
    
    free(wpd);
}



//Get the user to define an xy point
static int well_plate_definer_focus_on_point(well_plate_definer* wpd, char* message)
{
    int msgPnl, pnl, ctrl;
    FIBITMAP *dib = NULL;
	char buffer[500];
	CameraState state;
	
    // Remember camera settings
    gci_camera_save_state(wpd->camera, &state);

    microscope_set_focusing_mode(wpd->ms);
    gci_camera_set_live_mode(wpd->camera);
    gci_camera_activate_live_display(wpd->camera);

    stage_set_joystick_on (wpd->stage);

	find_resource("WellPlateDefiner_ui.uir", buffer); 

	msgPnl = LoadPanel(0, buffer, MSGPANEL); 
	
    SetPanelAttribute (msgPnl, ATTR_TITLE, "Set Extreme Well Position");
    SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, message);
    DisplayPanel(msgPnl);

    while (1) {
    
        ProcessSystemEvents();

        GetUserEvent (0, &pnl, &ctrl);
        
        if ((pnl == msgPnl) && (ctrl == MSGPANEL_OK))
            break; 

		#ifndef THREADED_CAM_AQ
		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(wpd->camera, NULL); 			
		gci_camera_display_image(wpd->camera, dib, NULL);	
		FreeImage_Unload(dib);   
		#endif
    }

    DiscardPanel(msgPnl);
    
    //Restore camera settings
	gci_camera_restore_state(wpd->camera, &state);   
	
    gci_camera_set_snap_mode(wpd->camera);
    
    
    return 0;
}



static int check_valid_well_location(well_plate_definer* wpd, double x, double y)
{
	double xmin, xmax, ymin, ymax;
	int    i;
	
	xmin = wpd->pts_corners[TOP_LEFT].pt.x;
	xmax = wpd->pts_corners[TOP_LEFT].pt.x;
	
	ymin = wpd->pts_corners[TOP_LEFT].pt.y;
	ymax = wpd->pts_corners[TOP_LEFT].pt.y;

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



static int well_plate_definer_stage_auto_move_then_focus_on_point(well_plate_definer* wpd, int i, int j, char* message)
{
    int         msgPnl, pnl, ctrl;
    FIBITMAP   *dib = NULL;
	char        buffer[500];
	CameraState state;
	double      x, y;
	
    // Remember camera settings
    gci_camera_save_state(wpd->camera, &state);

	stage_set_joystick_off (wpd->stage);

	calculate_well_xy_position(wpd, i-1, j-1, &x, &y);
	
	if (check_valid_well_location(wpd, x, y))
		stage_async_goto_xy_position(wpd->stage, x, y);

    microscope_set_focusing_mode(wpd->ms);
    gci_camera_set_live_mode(wpd->camera);
    gci_camera_activate_live_display(wpd->camera);

    stage_set_joystick_on (wpd->stage);

	find_resource("WellPlateDefiner_ui.uir", buffer); 

	msgPnl = LoadPanel(0, buffer, MSGPANEL); 
	
    SetPanelAttribute (msgPnl, ATTR_TITLE, "Set Well Position");
    SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, message);
    DisplayPanel(msgPnl);

    while (1) {
    
        ProcessSystemEvents();

        GetUserEvent (0, &pnl, &ctrl);
        
        if ((pnl == msgPnl) && (ctrl == MSGPANEL_OK))
            break;

		#ifndef THREADED_CAM_AQ
		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(wpd->camera, NULL); 			
		gci_camera_display_image(wpd->camera, dib, NULL);	
		FreeImage_Unload(dib);   
		#endif
    }

    DiscardPanel(msgPnl);
    
    //Restore camera settings
	gci_camera_restore_state(wpd->camera, &state);   
	
    gci_camera_set_snap_mode(wpd->camera);
    
    
    return 0;
}



static int check_all_corner_wells_measured(well_plate_definer* wpd)
{
	if ((wpd->pts_corners[TOP_LEFT].measured     == 1) &&
		(wpd->pts_corners[TOP_RIGHT].measured    == 1) &&
		(wpd->pts_corners[BOTTOM_LEFT].measured  == 1) &&
		(wpd->pts_corners[BOTTOM_RIGHT].measured == 1))
		return 1;
	else
		return 0;
}



static int check_all_poly_wells_measured(well_plate_definer* wpd)
{
	if ((wpd->pts_others[0].measured == 1) &&
		(wpd->pts_others[1].measured == 1) &&
		(wpd->pts_others[2].measured == 1))
		return 1;
	else
		return 0;
}



static int check_all_wells_measured_for_pts_file_generation(well_plate_definer* wpd)
{
	unsigned int linear_fit;

	GetCtrlVal(wpd->panel_id, WELL_PANEL_LINEAR_FIT, &linear_fit);

	if (linear_fit)
		return (check_all_corner_wells_measured(wpd));
	else
		return (check_all_corner_wells_measured(wpd) & check_all_poly_wells_measured(wpd));
}



static void poly_activate_well_details_display(int panel)
{
	SetCtrlAttribute(panel, WELL_PANEL_TEXTMSG_11,   ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, WELL_PANEL_TEXTMSG_14,   ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, WELL_PANEL_POLY_POINT_1, ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, WELL_PANEL_TEXTMSG_12,   ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, WELL_PANEL_POLY_POINT_2, ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, WELL_PANEL_TEXTMSG_13,   ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, WELL_PANEL_POLY_POINT_3, ATTR_DIMMED, 0);
}



static void poly_deactivate_well_details_display(int panel)
{
	SetCtrlAttribute(panel, WELL_PANEL_TEXTMSG_11,   ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, WELL_PANEL_TEXTMSG_14,   ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, WELL_PANEL_POLY_POINT_1, ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, WELL_PANEL_TEXTMSG_12,   ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, WELL_PANEL_POLY_POINT_2, ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, WELL_PANEL_TEXTMSG_13,   ATTR_DIMMED, 1);
	SetCtrlAttribute(panel, WELL_PANEL_POLY_POINT_3, ATTR_DIMMED, 1);
}




static int poly_measure_wells_and_do_fit(well_plate_definer* wpd)
{
	int       i;
	int       row, col;
	int       errcode, err1, err2, err3;
	double   *x, *y, *z, chisq;
	FPoint    pt;
    int       msgPnl, pnl, ctrl;
    FIBITMAP *dib = NULL;
	char      buffer[500];

	if (POLY_ERR_NO_ERROR != poly_determine_wells_to_measure(wpd))
	{
		/* Force a linear fit */
		SetCtrlVal(wpd->panel_id, WELL_PANEL_LINEAR_FIT, 1);
		SetCtrlAttribute(wpd->panel_id, WELL_PANEL_LINEAR_FIT, ATTR_DIMMED, 1);

		/* Notify user that an error has occured in polynomial fitting... */
		find_resource("WellPlateDefiner_ui.uir", buffer); 

		msgPnl = LoadPanel(0, buffer, MSGPANEL); 
		
		SetPanelAttribute (msgPnl, ATTR_TITLE, "ERROR:");
		SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, "Location map error.\nLinear fit possible.");
		DisplayPanel(msgPnl);

		while (1) {
	    
			ProcessSystemEvents();

			GetUserEvent (0, &pnl, &ctrl);
	        
			if ((pnl == msgPnl) && (ctrl == MSGPANEL_OK))
				break;

			#ifndef THREADED_CAM_AQ
			//For some unfathomable reason the presence of the message panel 
			//prevents camera timer tick events, so get and display an image.
			dib = gci_camera_get_image(wpd->camera, NULL); 			
			gci_camera_display_image(wpd->camera, dib, NULL);	
			FreeImage_Unload(dib);   
			#endif 
		}

		DiscardPanel(msgPnl);

		return -1;
	}

	poly_activate_well_details_display(wpd->panel_id);

	for (i=0; i<3; i++)
	{
		row = (wpd->poly_pts_template[i].row);
		col = (wpd->poly_pts_template[i].col);

		well_plate_definer_stage_auto_move_then_focus_on_point(wpd, col, row, "Once stage has relocated,\nFocus in selected well.");

		stage_get_xy_position(wpd->stage, &(pt.x), &(pt.y));
		z_drive_get_position(wpd->z_drive, &(pt.z));

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

//				update_displayed_defined_well(wpd, WELL_PANEL_POLY_POINT_1, wpd->pts_others[0]);



//				update_displayed_defined_well(wpd, WELL_PANEL_POLY_POINT_2, wpd->pts_others[1]);



//				update_displayed_defined_well(wpd, WELL_PANEL_POLY_POINT_3, wpd->pts_others[2]);

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



int CVICALLBACK OnWellDefine (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
	unsigned int N, M;
	unsigned int linear_fit;
	int          errcode;

    switch (event)
    {
        case EVENT_COMMIT:
        {
            FPoint pt;
            well_plate_definer* wpd = (well_plate_definer*) callbackData;
        
            //Define a new start point - adjust the wpd dimensions
            well_plate_definer_focus_on_point(wpd, "Move stage to the well\nposition and focus.");

            stage_get_xy_position (wpd->stage, &(pt.x), &(pt.y));
			z_drive_get_position (wpd->z_drive, &(pt.z));

			GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, &N);
			GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_DOWN,   &M);

			check_well_config_for_forced_linear_fit(wpd, M, N);
        
            switch(control)
            {
                case WELL_PANEL_DEF_TOP_LEFT:
					{
						set_well_pt_details(&(wpd->pts_corners[TOP_LEFT]), 1, 1, pt);

#ifdef WPD_TEST_PTS_ENABLED //to test fitting, remove later...
				wpd->pts_corners[TOP_LEFT].measured  = 1;
				wpd->pts_corners[TOP_LEFT].coord.row = 1;
				wpd->pts_corners[TOP_LEFT].coord.col = 1;
				wpd->pts_corners[TOP_LEFT].pt.x      = -40152.79;
				wpd->pts_corners[TOP_LEFT].pt.y      = -14152.66;
				wpd->pts_corners[TOP_LEFT].pt.z      = 3.85;
#endif

						break;
					}
				case WELL_PANEL_DEF_TOP_RIGHT:
					{
						set_well_pt_details(&(wpd->pts_corners[TOP_RIGHT]), 1, N, pt);

#ifdef WPD_TEST_PTS_ENABLED //to test fitting, remove later...
				wpd->pts_corners[TOP_RIGHT].measured  = 1;
				wpd->pts_corners[TOP_RIGHT].coord.row = 1;
				wpd->pts_corners[TOP_RIGHT].coord.col = 10;
				wpd->pts_corners[TOP_RIGHT].pt.x      = 41126.17;
				wpd->pts_corners[TOP_RIGHT].pt.y      = -14553.98;
				wpd->pts_corners[TOP_RIGHT].pt.z      = -44.49;
#endif

						break;
					}    
				case WELL_PANEL_DEF_BOTTOM_LEFT:
					{
						set_well_pt_details(&(wpd->pts_corners[BOTTOM_LEFT]), M, 1, pt);

#ifdef WPD_TEST_PTS_ENABLED //to test fitting, remove later...
				wpd->pts_corners[BOTTOM_LEFT].measured  = 1;
				wpd->pts_corners[BOTTOM_LEFT].coord.row = 6;
				wpd->pts_corners[BOTTOM_LEFT].coord.col = 1;
				wpd->pts_corners[BOTTOM_LEFT].pt.x      = -40805.36;
				wpd->pts_corners[BOTTOM_LEFT].pt.y      = 30901.53;
				wpd->pts_corners[BOTTOM_LEFT].pt.z      = 14.59;
#endif

						break;
					}    
				case WELL_PANEL_DEF_BOTTOM_RIGHT:
					{
						set_well_pt_details(&(wpd->pts_corners[BOTTOM_RIGHT]), M, N, pt);

#ifdef WPD_TEST_PTS_ENABLED //to test fitting, remove later...
				wpd->pts_corners[BOTTOM_RIGHT].measured  = 1;
				wpd->pts_corners[BOTTOM_RIGHT].coord.row = 6;
				wpd->pts_corners[BOTTOM_RIGHT].coord.col = 10;
				wpd->pts_corners[BOTTOM_RIGHT].pt.x      = 39605.17;
				wpd->pts_corners[BOTTOM_RIGHT].pt.y      = 30901.53;
				wpd->pts_corners[BOTTOM_RIGHT].pt.z      = -17.80;
#endif

						break;
					}    
            }

            //update_displayed_defined_positions(wpd);

			update_displayed_defined_wells(wpd);
            
            //Leave in live mode to keep BV happy
            gci_camera_set_live_mode(wpd->camera);
            gci_camera_activate_live_display(wpd->camera);

			GetCtrlVal(wpd->panel_id, WELL_PANEL_LINEAR_FIT, &linear_fit);

			if (check_all_corner_wells_measured(wpd))
			{
                if (linear_fit)
                {
                    /* Activate the export and time-lapse buttons... */
                    SetCtrlAttribute(wpd->panel_id, WELL_PANEL_EXPORT, ATTR_DIMMED, 0);
                    SetCtrlAttribute(wpd->panel_id, WELL_PANEL_TO_TIME_LAPSE, ATTR_DIMMED, 0);                
                }
                else
                {
				    errcode = poly_measure_wells_and_do_fit(wpd);

				    if (POLY_ERR_NO_ERROR != errcode)
					    force_and_dim_linear_fit_checkbox(wpd);                

                    /* Activate the export and time-lapse buttons... */
                    SetCtrlAttribute(wpd->panel_id, WELL_PANEL_EXPORT, ATTR_DIMMED, 0);
                    SetCtrlAttribute(wpd->panel_id, WELL_PANEL_TO_TIME_LAPSE, ATTR_DIMMED, 0);
                }
			}

            break;
        }
    }

    return 0;
}










int CVICALLBACK OnOk (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;   

			ui_module_hide_main_panel(UIMODULE_CAST(wpd));
			
            //Ensure joystick enabled
            stage_set_joystick_on (wpd->stage);
            
            break;
        }
    }

    return 0;
}



#if 0
static int calculate_well_position(well_plate_definer* wpd, unsigned short i, unsigned short j, FPoint *pt)
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
   
    //x1 = wpd->points[TOP_LEFT].x;
    //x2 = wpd->points[TOP_RIGHT].x;
    //x3 = wpd->points[BOTTOM_LEFT].x;
    //x4 = wpd->points[BOTTOM_RIGHT].x;
       
    pt->x = (wpd->points[TOP_LEFT].x * one_idx * one_jdy) + (wpd->points[TOP_RIGHT].x * idx * one_jdy) +
           (wpd->points[BOTTOM_LEFT].x * one_idx * jdy) + (wpd->points[BOTTOM_RIGHT].x * idx * jdy);

    //y1 = wpd->points[TOP_LEFT].y;
    //y2 = wpd->points[TOP_RIGHT].y;
    //y3 = wpd->points[BOTTOM_LEFT].y;
    //y4 = wpd->points[BOTTOM_RIGHT].y;
    
    pt->y = (wpd->points[TOP_LEFT].y * one_idx * one_jdy) + (wpd->points[TOP_RIGHT].y * idx * one_jdy) +
           (wpd->points[BOTTOM_LEFT].y * one_idx * jdy) + (wpd->points[BOTTOM_RIGHT].y * idx * jdy);
    
    pt->z = (wpd->points[TOP_LEFT].z * one_idx * one_jdy) + (wpd->points[TOP_RIGHT].z * idx * one_jdy) +
           (wpd->points[BOTTOM_LEFT].z * one_idx * jdy) + (wpd->points[BOTTOM_RIGHT].z * idx * jdy);
     
    return 0;
}  
#endif



int export_points_to_file (well_plate_definer* wpd, char *filepath)
{
	FILE *fp; 
    int HORIZONTAL = 0, shortest_path = 0;
    int i = 0, j = 0, M, N;
    FPoint pt;
	
	GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, &N);
	GetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_DOWN, &M);

    fp = fopen(filepath, "w");

	if (fp==NULL) return -1;
	
	fprintf(fp, "%d\n", N * M);   
	
    // what Direction are we exporting the plates ?
    GetCtrlVal(wpd->panel_id, WELL_PANEL_DIRECTION_HORZ, &HORIZONTAL);
	GetCtrlVal(wpd->panel_id, WELL_PANEL_SHORTEST_PATH, &shortest_path);       
    
    if(HORIZONTAL)
    {
        int LEFT_TO_RIGHT = 0;
        
        // Are we going from left to right or right to left ?
        GetCtrlVal(wpd->panel_id, WELL_PANEL_LEFT_TOP_CHECK, &LEFT_TO_RIGHT);
    
        for(j=0; j < M; j++) {   // Export horizontally first 
	        if((LEFT_TO_RIGHT && !shortest_path) || (LEFT_TO_RIGHT && shortest_path && j%2==0) || (!LEFT_TO_RIGHT && shortest_path && j%2==1))
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
        int TOP_TO_BOTTOM = 0;
        
        // Are we going from top to bottom or bottom to top ?
        GetCtrlVal(wpd->panel_id, WELL_PANEL_RIGHT_BOTTOM_CHECK, &TOP_TO_BOTTOM);
    
       	for(i=0; i < N; i++) {        
	        if((TOP_TO_BOTTOM && !shortest_path) || (TOP_TO_BOTTOM && shortest_path && i%2==0) || (!TOP_TO_BOTTOM && shortest_path && i%2==1))
        	{
                for(j=M-1; j >= 0; j--) {
					
                    if(calculate_well_position(wpd, i, j, &pt) < 0)
						goto WPD_ERROR;
					
                    fprintf(fp, "%f\t%f\t%f\n", pt.x, pt.y, pt.z);   
                }
            }    
	        else
    	    {
        	    // Going BOTTOM to TOP
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



int CVICALLBACK OnExport (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;   
	
			char path[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];

			//check here that required pts have been measured
			if (check_all_wells_measured_for_pts_file_generation(wpd))
			{
				//Save positions to a text file. Save relative to the zero limit switch.
				microscope_get_user_data_directory(wpd->ms, path);         

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



int CVICALLBACK OnTimeLapse (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;   
			timelapse* tl = microscope_get_timelapse(wpd->ms);  
	
			char path[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];             

			microscope_get_data_directory(wpd->ms, path);         
			sprintf(filepath, "%s\\temporary_wellplate_points.pts", path);
			
			if (export_points_to_file (wpd, filepath)<0) 
			{
				GCI_MessagePopup("Error", "Could not save a temporary file");
				return -1;
			}

			#ifdef MICROSCOPE_PYTHON_AUTOMATION

			timelapse_display(tl);
			
			timelapse_load_data_from_file(tl, filepath); 
           
			#endif // MICROSCOPE_PYTHON_AUTOMATION

            break;
        }
    }

    return 0;
}



int CVICALLBACK OnDirectionChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;   

            // If horizontal sected we turn off vertical as set the start positions of left / right
            if(control == WELL_PANEL_DIRECTION_HORZ)
            {
                SetCtrlVal(wpd->panel_id, WELL_PANEL_DIRECTION_VERT, 0);
                SetCtrlAttribute(wpd->panel_id, WELL_PANEL_LEFT_TOP_CHECK, ATTR_LABEL_TEXT, "Left");
                SetCtrlAttribute(wpd->panel_id, WELL_PANEL_RIGHT_BOTTOM_CHECK, ATTR_LABEL_TEXT, "Right");
            }
            else
            {
                SetCtrlVal(wpd->panel_id, WELL_PANEL_DIRECTION_HORZ, 0);
                SetCtrlAttribute(wpd->panel_id, WELL_PANEL_LEFT_TOP_CHECK, ATTR_LABEL_TEXT, "Top");
                SetCtrlAttribute(wpd->panel_id, WELL_PANEL_RIGHT_BOTTOM_CHECK, ATTR_LABEL_TEXT, "Bottom");
            }
            
            break;
        }
    }

    return 0;
}



int CVICALLBACK OnStartPositionChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;   

            // If left/top selected turn off right/bottom and vice versa
            if(control == WELL_PANEL_LEFT_TOP_CHECK)
                SetCtrlVal(wpd->panel_id, WELL_PANEL_RIGHT_BOTTOM_CHECK, 0);
            else
                SetCtrlVal(wpd->panel_id, WELL_PANEL_LEFT_TOP_CHECK, 0);
 
            break;
        }
    }

    return 0;
}



int CVICALLBACK OnPreDefinedWellRingChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;   

            int val;
            
            GetCtrlVal(panel, control, &val);
            
            if(val == WELL_PLATE_CUSTOM) {
            
                SetCtrlAttribute(panel, WELL_PANEL_WELLS_ACROSS, ATTR_DIMMED, 0);
                SetCtrlAttribute(panel, WELL_PANEL_WELLS_DOWN, ATTR_DIMMED, 0);
            }
            else {
                WellPlate plate;
                
                SetCtrlAttribute(panel, WELL_PANEL_WELLS_ACROSS, ATTR_DIMMED, 1);
                SetCtrlAttribute(panel, WELL_PANEL_WELLS_DOWN, ATTR_DIMMED, 1);
                
                // Ok val will be 0 and above so we can index into the predefined well plate array.
                plate = predefined_plates[val];
                
                SetCtrlVal(panel, WELL_PANEL_WELLS_ACROSS, plate.horizontal_size);
                SetCtrlVal(panel, WELL_PANEL_WELLS_DOWN, plate.vertical_size);

				check_well_config_for_forced_linear_fit(wpd, plate.horizontal_size, plate.vertical_size);
			}

            break;
        }
    }

    return 0;
}



int CVICALLBACK OnWellsAcross (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;   

            int val, M, N;
            
            GetCtrlVal(panel, WELL_PANEL_PRE_WELL_RING, &val);
            
            if(val == WELL_PLATE_CUSTOM)
			{
                GetCtrlVal(panel, control, &N);
				GetCtrlVal(panel, WELL_PANEL_WELLS_DOWN, &M);

				check_well_config_for_forced_linear_fit(wpd, N, M);
            }

            break;
        }
    }

    return 0;
}



int CVICALLBACK OnWellsDown (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;   

            int val, M, N;
            
            GetCtrlVal(panel, WELL_PANEL_PRE_WELL_RING, &val);
            
            if(val == WELL_PLATE_CUSTOM)
			{
                GetCtrlVal(panel, WELL_PANEL_WELLS_ACROSS, &N);
				GetCtrlVal(panel, control, &M);

				check_well_config_for_forced_linear_fit(wpd, N, M);
            }

            break;
        }
    }

    return 0;
}

static void OnMasterZDriveChanged (Microscope* microscope, void *data)
{
    microscope->_wpd->z_drive = microscope_get_master_zdrive (microscope); 	
}

static void clear_existing_pts_and_poly_coeffs(well_plate_definer *wpd)
{
    int    i;
    WCoord nullwcoord;
    WPoint nullwpoint;

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
}



int CVICALLBACK OnClearPoints (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
        {
		    well_plate_definer* wpd = (well_plate_definer*) callbackData;

            clear_existing_pts_and_poly_coeffs(wpd);
            
            update_displayed_defined_wells(wpd);

            poly_deactivate_well_details_display(wpd->panel_id);

            /* Remove linear fitting as may have been forced... */
            SetCtrlVal(wpd->panel_id, WELL_PANEL_LINEAR_FIT, 0);
            SetCtrlAttribute(wpd->panel_id, WELL_PANEL_LINEAR_FIT, ATTR_DIMMED, 0);

            /* Restore default plate setting... */
            SetCtrlVal(wpd->panel_id, WELL_PANEL_PRE_WELL_RING, 4);
            SetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, 12);
            SetCtrlVal(wpd->panel_id, WELL_PANEL_WELLS_DOWN,    8);
            SetCtrlAttribute(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, ATTR_DIMMED, 1);
            SetCtrlAttribute(wpd->panel_id, WELL_PANEL_WELLS_DOWN, ATTR_DIMMED, 1);

            /* Deactivate the export and time-lapse buttons until plate defined... */
            SetCtrlAttribute(wpd->panel_id, WELL_PANEL_EXPORT, ATTR_DIMMED, 1);
            SetCtrlAttribute(wpd->panel_id, WELL_PANEL_TO_TIME_LAPSE, ATTR_DIMMED, 1);

            break;
        }
    }

    return 0;
}



static void poly_init_coeff_details(well_plate_definer *wpd, unsigned short numofcoeffs)
{
	wpd->poly_num_of_coeffs = numofcoeffs;
	memset(wpd->poly_coeffs, 0, sizeof(double) * numofcoeffs);
}




well_plate_definer* well_plate_definer_new(Microscope *ms)
{
    well_plate_definer *wpd = (well_plate_definer*) malloc (sizeof(well_plate_definer));
    
    wpd->ms = ms;
    wpd->camera = microscope_get_camera(wpd->ms);
    wpd->stage = microscope_get_stage(wpd->ms);
    wpd->z_drive = microscope_get_master_zdrive (wpd->ms);

	memset(wpd->pts_corners, 0, sizeof(WPoint) * 4);
	memset(wpd->pts_others,  0, sizeof(WPoint) * 3);

	poly_init_coeff_details(wpd, 6);
	
    ui_module_constructor(UIMODULE_CAST(wpd), "Well Plate Definer"); 
    
    wpd->panel_id = ui_module_add_panel(UIMODULE_CAST(wpd), "WellPlateDefiner_ui.uir", WELL_PANEL, 1);
    
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_DEF_TOP_LEFT, OnWellDefine, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_DEF_TOP_RIGHT, OnWellDefine, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_DEF_BOTTOM_LEFT, OnWellDefine, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_DEF_BOTTOM_RIGHT, OnWellDefine, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_OK, OnOk, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_EXPORT, OnExport, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_TO_TIME_LAPSE, OnTimeLapse, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_DIRECTION_HORZ, OnDirectionChanged, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_DIRECTION_VERT, OnDirectionChanged, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_LEFT_TOP_CHECK, OnStartPositionChanged, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_RIGHT_BOTTOM_CHECK, OnStartPositionChanged, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_PRE_WELL_RING, OnPreDefinedWellRingChanged, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_WELLS_ACROSS, OnWellsAcross, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_WELLS_DOWN, OnWellsDown, wpd);
    InstallCtrlCallback(wpd->panel_id, WELL_PANEL_CLEAR_POINTS, OnClearPoints, wpd);

	wpd->_microscope_master_z_drive_changed_signal_id = microscope_master_z_drive_changed_handler_connect(wpd->ms, OnMasterZDriveChanged, wpd);

    update_displayed_defined_wells(wpd);
		
    return wpd;
}