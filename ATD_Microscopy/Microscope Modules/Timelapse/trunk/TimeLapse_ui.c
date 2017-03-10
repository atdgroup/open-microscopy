#include "TimeLapse_ui.h"
#include "timelapse.h"
#include "TimeLapse-PointWizardImportUI.h"

#include "stage\stage.h"
#include "timelapse_ui.h"
#include "shellapi.h" 
#include "string_utils.h"
#include "microscope.h"
#include "StagePlate.h"

#include <utility.h>
#include <userint.h>

#include "Config.h"
#include "gci_menu_utils.h"

#ifdef MICROSCOPE_PYTHON_AUTOMATION
	
static int OnWizardStepDone(wizard_step *step, int panel_id, void *data)
{
	timelapse *tl = (timelapse*) data;  

	// Here we choose between doing stage plate import or manually define a plate
	if(step->panel_resource_id == MAN_OR_ST) {

		int val;

		GetCtrlVal(panel_id, MAN_OR_ST_CHOICE, &val);
		
		if(val == 1)  {// We want to manually define a plate.
		
			tl->wpd->generate_from_stage_plate = 0;

			return STEP1_1;
		}
		else {

			StagePlateModule* stage_plate_module = (StagePlateModule* ) tl->stage->_stage_plate_module;

			stage_plate_load_active_plates_into_list_control(stage_plate_module, tl->wpd->plate_select_pnl, SP_SELECT_POS);

			tl->wpd->generate_from_stage_plate = 1;

			return SP_SELECT;
		}
	}
	else if(step->panel_resource_id == STEP1_1) {

		wizard_set_next_button_dimmed(tl->wiz, 1);

		return DEF_CORNER;
	}
	else if(step->panel_resource_id == SP_SELECT)
	{
		if(tl->wpd->generate_from_stage_plate == 1) {

			return DEF_CORNER;
		}	
	}
	else if(step->panel_resource_id == DEF_CORNER) {

		return DIR_START;
	}

	return 0;
}

static int OnWizardFinished(wizard *wiz, void *data)
{
	char path[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];             

	timelapse *tl = (timelapse*) data;  

	// THIS TOP 1/2 USES THE STAGE PLATE MODULE TO GENERATE A LIST OF POINTS

	if(tl->wpd->generate_from_stage_plate == 1) {

		int shortest_path, is_horizontal, start_left, start_right, regions;
		double xsize, ysize;

		//GenerateStagePlatePoints(tl, tl->wpd->use_selected_wells);

		GetCtrlVal(tl->wpd->direction_pnl, DIR_START_SHORTEST_PATH, &shortest_path);
		GetCtrlVal(tl->wpd->direction_pnl, DIR_START_DIR_GEN, &is_horizontal);
		GetCtrlVal(tl->wpd->direction_pnl, DIR_START_START_POSITION, &start_left);
		GetCtrlVal(tl->wpd->direction_pnl, DIR_START_START_POSITION_TB, &start_right);
		GetCtrlVal(tl->wpd->direction_pnl, DIR_START_CHOICE, &regions);
		GetCtrlVal(tl->wpd->direction_pnl, DIR_START_XSIZE, &xsize);
		GetCtrlVal(tl->wpd->direction_pnl, DIR_START_YSIZE, &ysize);

		GenerateStagePlatePointsWithDirInfo(tl, tl->wpd->use_selected_wells,
			is_horizontal,
			(StagePlateHorizontalStartPosition) start_left,
			(StagePlateVerticalStartPosition) start_right, shortest_path,
			regions, xsize, ysize);

		return 0;
	}

	// FROM HERE IS THE OLD CODE THAT USES THE MANUALLY DEFINED (OR PREDEFINED BUT NOT FROM STAGE PLATE MODULE) PLATE AND COORDS

	microscope_get_data_directory(tl->ms, path);         
	sprintf(filepath, "%s\\temporary_wellplate_points.pts", path);
			
	// This handles the direction of the generation etc
	if (timelapse_well_plate_definer_export_points_to_file (tl->wpd, filepath) < 0 ) 
	{
		GCI_MessagePopup("Error", "Could not save a temporary file");
		return -1;
	}

	#ifdef MICROSCOPE_PYTHON_AUTOMATION
	
	timelapse_load_data_from_file(tl, filepath); 
           
	#endif // MICROSCOPE_PYTHON_AUTOMATION

	return 0;
}

int create_wizard_steps(timelapse *tl, wizard* wiz)
{
	tl->wpd = timelapse_timelapse_well_plate_definer_new(tl);
	
	return 0;
}
/*
static void OnWizardStagePlateChanged(StagePlateModule* stage_plate_module, int pos, void *data)
{
	timelapse *tl = (timelapse*) data;  

	int plate_position;
	StagePlate plate;
			
	GetCtrlVal(tl->wpd->plate_select_pnl, SP_SELECT_POS, &plate_position);

	stage_plate_move_to_position_no_emit(tl->ms->_stage_plate_module, plate_position);
}
*/
int CVICALLBACK OnDefinePointsClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int number_of_plates;
			timelapse *tl = (timelapse*) callbackData;  

			if (tl->stage->_stage_plate_module == NULL) { // stage plate module not instantiated for this microscope
				GCI_MessagePopup("Error", "The Stage Plate feature is required to define points.");
				return 0;
			}
			
			tl->wiz = wizard_new("Point define wizard", "TimeLapse-PointWizardImportUI.uir");

			wizard_set_on_step_shown_callback(tl->wiz, OnWizardStepDone, tl);
			wizard_set_on_finished_callback(tl->wiz, OnWizardFinished, tl);

			create_wizard_steps(tl, tl->wiz);

			wizard_start(tl->wiz, MAN_OR_ST);

			stage_plate_get_number_of_plates(tl->stage->_stage_plate_module, &number_of_plates);

			if(number_of_plates <= 0)
				SetCtrlAttribute(tl->wpd->manual_or_auto_pnl, MAN_OR_ST_CHOICE, ATTR_DIMMED, 1);
			else
				SetCtrlAttribute(tl->wpd->manual_or_auto_pnl, MAN_OR_ST_CHOICE, ATTR_DIMMED, 0);

			break;
		}
	}
		
	return 0;
}

int CVICALLBACK OnNewPointClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;  
			
			timelapse_new_point(tl);     
	
			break;
		}
	}
		
	return 0;
}


// Silly complicated code to work around lack of cvi features.
// This comes from their forum.
static void CVICALLBACK get_row (void * callbackData)
{
	timelapse *tl = (timelapse*) callbackData;  

	Rect cellselection;
	Point activecell;

	memset(&cellselection, 0, sizeof(Rect));

	GetActiveTableCell (tl->panel_id, TIMELAPSE_TABLE, &activecell);  

	cellselection = MakeRect(activecell.y, 1, 1, TABLE_COL_NUMBER);
		
	SetTableSelection (tl->panel_id, TIMELAPSE_TABLE, cellselection);
}			

int CVICALLBACK OnUpdatePointClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;          
			Rect range;

			//GetActiveTableCell (tl->panel_id, TIMELAPSE_TABLE, &cell);
			GetTableSelection (tl->panel_id, TIMELAPSE_TABLE, &range);

			if(range.height > 1) {
				GCI_MessagePopup("Timelapse Error", "We can only update one point at a time. Select one point only");
				return -1;
			}
			else if(range.height < 1) {
				GCI_MessagePopup("Timelapse Error", "No point selected");
				return -1;
			}

			if(timelapse_update_centre_point(tl, range.top) == TIMELAPSE_ERROR) {
				GCI_MessagePopup("Timelapse Error", "Point does not exist");
				return -1;
			}
			
			break;
		}
	}
		
	return 0;
}

int CVICALLBACK OnTimeLapseTableEdited (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	timelapse *tl = (timelapse*) callbackData;   

	switch (event)
	{
		case EVENT_LEFT_CLICK:
		{
			int mouseDown, vertPos, horzPos;
			Rect rect;

			GetRelativeMouseState (tl->panel_id, TIMELAPSE_TABLE, 0, 0, &mouseDown, 0, 0);

			vertPos = eventData1;
			horzPos = eventData2; 

			GetCtrlAttribute(panel, TIMELAPSE_DUMMY_AREA, ATTR_LEFT, &(rect.left));
			GetCtrlAttribute(panel, TIMELAPSE_DUMMY_AREA, ATTR_TOP, &(rect.top));
			GetCtrlAttribute(panel, TIMELAPSE_DUMMY_AREA, ATTR_WIDTH, &(rect.width));
			GetCtrlAttribute(panel, TIMELAPSE_DUMMY_AREA, ATTR_HEIGHT, &(rect.height));

			// User has selected vertical scrollbar just break
			if(RectContainsPoint (rect, MakePoint(horzPos, vertPos)))
				break;

			while (mouseDown) {
				GetRelativeMouseState (tl->panel_id, TIMELAPSE_TABLE, 0, 0, &mouseDown, 0, 0);
				ProcessSystemEvents();
			}
		
			PostDeferredCall (get_row, tl);
		
			break;
		}

		case EVENT_COMMIT:
		{	       
			int row = eventData1;
			GCI_FPOINT pt, size;
			double vals[TABLE_COL_NUMBER];
			
			// top, left, height, width
			GetTableCellRangeVals (tl->panel_id, TIMELAPSE_TABLE, MakeRect (row, 1, 1, TABLE_COL_NUMBER), vals, VAL_ROW_MAJOR);

			pt.x = vals[0];
			pt.y = vals[1];   
			pt.z = vals[2];   
			size.x = vals[3];
			size.y = vals[4];
			
			if(timelapse_edit_centre_point(tl, row, pt) == TIMELAPSE_ERROR) {
			
				GCI_MessagePopup("Error", "No such point in list ?");
			}

			if(timelapse_edit_ROI_size(tl, row, size) == TIMELAPSE_ERROR) {
			
				GCI_MessagePopup("Error", "No such point in list ?");
			}

			timelapse_draw_points(tl);
			
			break;
		}
	}
	return 0;
}

int CVICALLBACK OnGotoPointClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{

			timelapse *tl = (timelapse*) callbackData;          
			Rect range;

			//GetActiveTableCell (tl->panel_id, TIMELAPSE_TABLE, &cell);
			GetTableSelection (tl->panel_id, TIMELAPSE_TABLE, &range);

			if(range.height > 1) {
				GCI_MessagePopup("Timelapse Error", "We can only goto one point at a time. Select one point only");
				return -1;
			}
			else if(range.height < 1) {
				GCI_MessagePopup("Timelapse Error", "No point selected");
				return -1;
			}

			if(timelapse_move_to_point(tl, range.top) == TIMELAPSE_ERROR) {
				GCI_MessagePopup("Timelapse Error", "Point does not exist");
				return -1;
			}
		
			if(!gci_camera_is_live_mode(tl->camera))
			{
				stage_wait_for_stop_moving(tl->stage);
				Delay(0.1);
				gci_camera_snap_image(tl->camera); 
			}

			stage_set_joystick_on(tl->stage);
			
			break;
		}
	}
		
	return 0;
}

int CVICALLBACK OnNextClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData; 
			
			timelapse_move_to_next_point(tl);
		
			if(!gci_camera_is_live_mode(tl->camera))
			{
				stage_wait_for_stop_moving(tl->stage);
				Delay(0.1);
				gci_camera_snap_image(tl->camera); 
			}

			stage_set_joystick_on(tl->stage);
			
			break;
		}
	}
		
	return 0;
}

int CVICALLBACK OnPrevClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData; 
			
			timelapse_move_to_previous_point(tl);
			
			if(!gci_camera_is_live_mode(tl->camera))
			{
				stage_wait_for_stop_moving(tl->stage);
				Delay(0.1);
				gci_camera_snap_image(tl->camera); 
			}

			stage_set_joystick_on(tl->stage);
			
			break;
		}
	}
		
	return 0;
}

int CVICALLBACK OnTimeLapseLoadClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;
			
			timelapse_load_data_from_selected_filepath(tl);
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnTimeLapseSaveClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;
			
			timelapse_save_data_to_selected_filepath(tl);
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnClearAllClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;
			
			if(GCI_ConfirmPopup("", IDI_WARNING, "Do you really wish to clear all points?"))    
				timelapse_clear_points(tl);   
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnUseCrossHairClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;

	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;

			GetCtrlVal(panel, control, &val);

			if (val){
				tl->window = gci_camera_get_imagewindow(tl->camera);
				if (tl->window != NULL) {
					GCI_ImagingWindow_ActivateCrossHairTool(tl->window);
					tl->_crosshair_signal_id = GCI_ImagingWindow_SetCrosshairHandler(tl->window, OnCrosshairClicked, tl); 
				}
			}
			else {
				tl->window = gci_camera_get_imagewindow(tl->camera);
				if (tl->window != NULL) {
					GCI_ImagingWindow_DisableCrossHair(tl->window);
					GCI_ImagingWindow_DisconnectCrosshairHandler(tl->window, tl->_crosshair_signal_id); 
				}
			}
		
			break;
		}
	}
	
	return 0;
}
int CVICALLBACK OnUseCrossHairClicked_2 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	stage_scan *ss=NULL;
	realtime_overview *rto=NULL;

	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;

			GetCtrlVal(panel, control, &val);

			if (val){
				
				// set this to NULL to test attaching to ss or rto
				if (tl->mosaic_window != NULL) {
					GCI_ImagingWindow_DisableCrossHair(tl->mosaic_window->window);
					GCI_ImagingWindow_DisconnectCrosshairHandler(tl->mosaic_window->window, tl->_crosshair_signal_id); 
					tl->mosaic_window = NULL;
				}
				
				// try to attach to the stage scan window
				ss = microscope_get_stage_scan(tl->ms);
				if (ss!=NULL)
					if (ss->rto!=NULL)
						tl->mosaic_window = ss->rto->mosaic_window;

				// if no joy, try the microscope rto
				if (tl->mosaic_window == NULL){
					rto = microscope_get_realtime_overview(tl->ms);
					if (rto!=NULL)
						tl->mosaic_window = rto->mosaic_window;
				}

				if (tl->mosaic_window != NULL) {
					GCI_ImagingWindow_ActivateCrossHairTool(tl->mosaic_window->window);
					tl->_crosshair_signal_id = GCI_ImagingWindow_SetCrosshairHandler(tl->mosaic_window->window, OnCrosshairMosaicClicked, tl); 
				}
			else
				GCI_MessagePopup("Error", "A Stage Scan or Realtime Overview is required.");
			}
			else {

				if (tl->mosaic_window != NULL) {
//					GCI_ImagingWindow_DisableCrossHair(tl->mosaic_window->window);  // probably still want this on a mosaic window
					GCI_ImagingWindow_DisconnectCrosshairHandler(tl->mosaic_window->window, tl->_crosshair_signal_id); 
					tl->mosaic_window = NULL;
				}
			}
		
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnDeleteClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;
			
			timelapse_remove_point(tl, tl->current_point);  
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnDeleteClicked2 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;
			Rect range;

			GetTableSelection (tl->panel_id, TIMELAPSE_TABLE, &range);

			if(range.height > 1) {
				GCI_MessagePopup("Timelapse Error", "We can only delete one point at a time. Select one point only");
				return -1;
			}
			else if(range.height < 1) {
				GCI_MessagePopup("Timelapse Error", "No point selected");
				return -1;
			}

// Can only select one row so this code is redundant but it may allow deleting more than one row
//			for (i=range.top; i<range.top+range.height; i++) {
//				if(timelapse_remove_point(tl, i) == TIMELAPSE_ERROR) {
//				GCI_MessagePopup("Timelapse Error", "Point does not exist");
//				}
//			}

			if(timelapse_remove_point(tl, range.top) == TIMELAPSE_ERROR) {
				GCI_MessagePopup("Timelapse Error", "Point does not exist");
			}
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnStartClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;

/*			if (tl->region_mode) {  // check focal plane has been set if necessary
				if (tl->focus_mode==TIMELAPSE_USEGLOBALFOCALPLANE && tl->has_global_focalPlane==0) {
					GCI_MessagePopup("Error", "Global focal plane has not been setup.\nUse: Define Focal Planes");
					break;
				}
				else {
					int i, n = ListNumItems(tl->points);
					TimelapseTableEntry pt;

					for (i=1; i<=n; i++) {
						ListGetItem(tl->points, &pt, i);
						if (tl->focus_mode==TIMELAPSE_USEINDFOCALPLANE && pt.hasFocalPlane==0) {
							GCI_MessagePopup("Error", " focal plane has not been setup.\nUse: Define Focal Planes");
							break;
						}
					}
					break;
				}
			}
*/

			logger_log(UIMODULE_LOGGER(tl), LOGGER_INFORMATIONAL, "Timelapse started");   
			timelapse_perform_sequence(tl); 
			
			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnStopClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    
			logger_log(UIMODULE_LOGGER(tl), LOGGER_INFORMATIONAL, "Timelapse stopped by user");   
			
			tl->active = 0;

			if(tl->on_abort_callable != NULL) {
				if(PyObject_CallObject(tl->on_abort_callable, NULL) == NULL)  
					PyErr_Print();
			}

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnRevisitClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			timelapse_update_revisit_buttons(tl);
			
			ui_module_display_panel(UIMODULE_CAST(tl), tl->revisit_panel);

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnRevisitCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			ui_module_hide_panel(UIMODULE_CAST(tl), tl->revisit_panel);

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnUseRegionsClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			timelapse_region_mode_on(tl);
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnUseDefaultCubeOptions (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			timelapse_setup_cube_options(tl);
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnCubeSelect (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    
			int val;
			TimelapseCubeOptions cube;

			GetCtrlVal(tl->panel_id, TIMELAPSE_CUBE, &val);

			ListGetItem(tl->cube_options, &cube, val);

			SetCtrlVal(tl->panel_id, TIMELAPSE_ACQEXPOSURE, cube.exposure);
			SetCtrlVal(tl->panel_id, TIMELAPSE_ACQGAIN, cube.gain);
			SetCtrlVal(tl->panel_id, TIMELAPSE_ACQFOCUSOFFSET, cube.offset);

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnCubeOptionsChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    
			int val;
			TimelapseCubeOptions *cube;

			GetCtrlVal(tl->panel_id, TIMELAPSE_CUBE, &val);

			cube = (TimelapseCubeOptions *)ListGetPtrToItem(tl->cube_options, val);

			GetCtrlVal(tl->panel_id, TIMELAPSE_ACQEXPOSURE, &(cube->exposure));
			GetCtrlVal(tl->panel_id, TIMELAPSE_ACQGAIN, &(cube->gain));
			GetCtrlVal(tl->panel_id, TIMELAPSE_ACQFOCUSOFFSET, &(cube->offset));

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnGetCameraValsClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			timelapse_get_vals_from_camera(tl);

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnSetCameraValsClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			timelapse_set_vals_to_camera(tl);

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnRegionTypeClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			SetCtrlVal(tl->regions_panel, TLREGIONS_FIXED_SIZE, 0);
			SetCtrlVal(tl->regions_panel, TLREGIONS_VARIABLE, 0);
			SetCtrlVal(panel, control, 1);

			if (control == TLREGIONS_FIXED_SIZE) {
				SetCtrlAttribute(tl->regions_panel, TLREGIONS_WIDTH, ATTR_DIMMED, 0);
				SetCtrlAttribute(tl->regions_panel, TLREGIONS_HEIGHT, ATTR_DIMMED, 0);
			}
			else {
				SetCtrlAttribute(tl->regions_panel, TLREGIONS_WIDTH, ATTR_DIMMED, 1);
				SetCtrlAttribute(tl->regions_panel, TLREGIONS_HEIGHT, ATTR_DIMMED, 1);
			}

			check_mosaic_region_needed(tl);

		}
	}
	
	return 0;
}

int CVICALLBACK OnFocusTypeClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			SetCtrlVal(tl->regions_panel, TLREGIONS_USEGLOBALFOCALPLANE, 0);
			SetCtrlVal(tl->regions_panel, TLREGIONS_USEINDFOCALPLANE, 0);
			SetCtrlVal(tl->regions_panel, TLREGIONS_USECONSTFOCALPLANE, 0);
			SetCtrlVal(tl->regions_panel, TLREGIONS_USEAUTOEVERY, 0);
			SetCtrlVal(panel, control, 1);

			if (control == TLREGIONS_USEGLOBALFOCALPLANE) {
				tl->focus_mode = TIMELAPSE_USEGLOBALFOCALPLANE; 
			}
			else if (control == TLREGIONS_USEINDFOCALPLANE) {
				tl->focus_mode = TIMELAPSE_USEINDFOCALPLANE; 
			}
			else if (control == TLREGIONS_USECONSTFOCALPLANE) {
				tl->focus_mode = TIMELAPSE_USECONSTFOCALPLANE; 
			}
			else if (control == TLREGIONS_USEAUTOEVERY) {
				tl->focus_mode = TIMELAPSE_USEAUTOEVERY; 
			}
		}
	}
	
	return 0;
}

int CVICALLBACK OnNewRegionClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;  
			double w, h;
			int fixed=0;

			GetCtrlVal(tl->regions_panel, TLREGIONS_FIXED_SIZE, &fixed);

			if (fixed) {
				GetCtrlVal(tl->regions_panel, TLREGIONS_WIDTH, &w);
				GetCtrlVal(tl->regions_panel, TLREGIONS_HEIGHT, &h);
				timelapse_new_region_WH (tl, w, h);  			
			}
			else {
				RECT rect;
				double x, y, z, w, h, startx, starty, roi_width, roi_height;
				int width, height;

				if (tl->mosaic_window != NULL) {

					// get region from the mosaic window
					GCI_ImagingWindow_GetROIImageRECT(tl->mosaic_window->window, &rect);

					if(rect.left < 0 || rect.top < 0  || rect.right < 0  || rect.bottom < 0)
						return 0;

					startx =		tl->mosaic_window->region.left;
					starty =		tl->mosaic_window->region.top; 
					roi_width =		tl->mosaic_window->region.width; 
					roi_height =	tl->mosaic_window->region.height;
					
					width = FreeImage_GetWidth(tl->mosaic_window->mosaic_image);
					height = FreeImage_GetHeight(tl->mosaic_window->mosaic_image);

					// centre
					x = startx + (((float) (rect.left+rect.right)/2.0 / width) * roi_width);
					y = starty + (((float) (rect.top+rect.bottom)/2.0 / height) * roi_height);      
					w = (((float) (rect.right-rect.left) / width) * roi_width);      
					h = (((float) (rect.bottom-rect.top) / height) * roi_height);      

					z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), &z);   // use global focal plane?

					timelapse_new_region_XYZWH (tl, x, y, z, w, h);
				}
			}
		}
	}
	
	return 0;
}

void defineFocalPlaneForPoint(timelapse* tl, int point_number)
{
	region_scan *rs = region_scan_new();
	TimelapseTableEntry *pt=NULL;
	double new_z;
	GCI_FPOINT new_pt;
	
	pt = ListGetPtrToItem(tl->points, point_number);

	if (pt==NULL)
		return;

	regionscan_set_roi(rs, pt->centre.x - pt->regionSize.x/2.0, pt->centre.y - pt->regionSize.y/2.0, pt->regionSize.x, pt->regionSize.y);

	pt->hasFocalPlane = region_of_interest_setup_focal_plane(rs->roi, 0);

	if (pt->hasFocalPlane)
		region_of_interest_get_focus_points(rs->roi, &(pt->focalPlane_a), &(pt->focalPlane_b), &(pt->focalPlane_c));

	// replace the z value of the centre point with a value from the focal plane
	if (region_of_interest_z_for_xy(rs->roi, pt->centre.x, pt->centre.y, &new_z)>=0) {
		new_pt.x = pt->centre.x;
		new_pt.y = pt->centre.y;
		new_pt.z = new_z;
		timelapse_edit_centre_point(tl, point_number, new_pt);
	}

	region_scan_destroy(rs);

	return;
}

void defineGlobalFocalPlane(timelapse* tl)
{
	region_scan *rs = region_scan_new();
	int i, n;
	GCI_FPOINT min, max;
	TimelapseTableEntry pt;
	double new_z;
	GCI_FPOINT new_pt;

	min.x = 1.0e10;
	min.y = 1.0e10;
	max.x = -1.0e10;
	max.y = -1.0e10;

	n = ListNumItems(tl->points);
	for (i=1; i<=n; i++) {
		ListGetItem(tl->points, &pt, i);
		if (pt.centre.x - pt.regionSize.x/2.0 < min.x)
			min.x = pt.centre.x - pt.regionSize.x/2.0;
		if (pt.centre.x + pt.regionSize.x/2.0 > max.x)
			max.x = pt.centre.x + pt.regionSize.x/2.0;
		if (pt.centre.y - pt.regionSize.y/2.0 < min.y)
			min.y = pt.centre.y - pt.regionSize.y/2.0;
		if (pt.centre.y + pt.regionSize.y/2.0 > max.y)
			max.y = pt.centre.y + pt.regionSize.y/2.0;
	}

	regionscan_set_roi(rs, min.x, min.y, max.x-min.x, max.y-min.y);

	tl->has_global_focalPlane = region_of_interest_setup_focal_plane(rs->roi, 0);

	if (tl->has_global_focalPlane)
		region_of_interest_get_focus_points(rs->roi, &(tl->global_focalPlane_a), &(tl->global_focalPlane_b), &(tl->global_focalPlane_c));
	
	// replace the z values of the centre points with values from the focal plane
	for (i=1; i<=n; i++) {
		ListGetItem(tl->points, &pt, i);
		if (region_of_interest_z_for_xy(rs->roi, pt.centre.x, pt.centre.y, &new_z)>=0) {
			new_pt.x = pt.centre.x;
			new_pt.y = pt.centre.y;
			new_pt.z = new_z;
			timelapse_edit_centre_point(tl, i, new_pt);
		}
	}

	region_scan_destroy(rs);

	return;
}

int CVICALLBACK OnFocalPlanesClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i, n=0;

	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			switch (tl->focus_mode)
			{
			case TIMELAPSE_USEGLOBALFOCALPLANE:

				defineGlobalFocalPlane(tl);

				break;
			case TIMELAPSE_USEINDFOCALPLANE:

				n = ListNumItems(tl->points);
				for (i=1; i<=n; i++)						
					defineFocalPlaneForPoint(tl, i);
								
				break;
			case TIMELAPSE_USECONSTFOCALPLANE:
				break;
			case TIMELAPSE_USEAUTOEVERY:
				break;
			}
		}
	}
	
	return 0;
}

int CVICALLBACK OnRegionsCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    

			timelapse_region_mode_off(tl);
		}
	}
	
	return 0;
}

int CVICALLBACK OnTimeLapseCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    
			
			timelapse_hide(tl);

			break;
		}
	}
	
	return 0;
}


static void EditScript(timelapse* tl, const char *filepath)   
{
	// CVI doesnt format nice xml
	// so I invoke a .Net app to do it.
	int window_handle, ret;
	char escaped_path[GCI_MAX_PATHNAME_LEN] = "";
	
	sprintf(escaped_path, "\"%s\"", filepath);
	
	GetPanelAttribute (tl->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);
 
	// Opening the file in the default app will cause it to be opened in the python interpreter
	// opens MyFile with MyEditor program
	ret = (int) ShellExecute((HWND) window_handle, NULL, "NotePad++.exe", escaped_path, NULL, SW_SHOWNORMAL);
	
	if(ret == 32)
		ShellExecute((HWND) window_handle, NULL, "NotePad.exe", escaped_path, NULL, SW_SHOWNORMAL); 
}

static int CopyScriptDialog(timelapse *tl, char *name, char *description, char *author)
{	// Displays a panel for the user to fill in script name, description etc.
    int file_size = 0, panel_id = 0, pnl = 0, ctrl = 0, left, top;
    char fullpath[GCI_MAX_PATHNAME_LEN] = "", panel_define[100] = "";
    char reg_key[500] = "";
    
    if(find_resource("TimeLapse_ui.uir", fullpath) < 0)
        return -1;
        
    panel_id = LoadPanel(0, fullpath, SCRIPT_SEL);    

    SetCtrlVal(panel_id, SCRIPT_SEL_NAME, name);

	if(UIMODULE_MAIN_PANEL_ID(tl) > 0) {
		GetPanelAttribute (UIMODULE_MAIN_PANEL_ID(tl), ATTR_LEFT, &left);
		GetPanelAttribute (UIMODULE_MAIN_PANEL_ID(tl), ATTR_TOP, &top);
		SetPanelPos (panel_id, top+10, left+10);
	}
	else {

		SetPanelAttribute(panel_id, ATTR_LEFT, VAL_AUTO_CENTER);           
		SetPanelAttribute(panel_id, ATTR_TOP, VAL_AUTO_CENTER);
	}

	DisplayPanel(panel_id);
	
	while (1) {
		
        ProcessSystemEvents();
			
		GetUserEvent (0, &pnl, &ctrl);
		
        if (pnl != panel_id)
            continue;
        
		if (ctrl == SCRIPT_SEL_OK)
            break;
                
        if (ctrl == SCRIPT_SEL_CANCEL) {
            DiscardPanel(panel_id);
			return -1;
		}
	}

	GetCtrlVal(panel_id, SCRIPT_SEL_NAME, name);
	GetCtrlVal(panel_id, SCRIPT_SEL_DESCRIPTION, description);
	GetCtrlVal(panel_id, SCRIPT_SEL_AUTHOR, author);

    DiscardPanel(panel_id);  

    return 0;
}

int CVICALLBACK OnTimeLapseEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;
			int i=2;
			
			int count = 0, read_only, system, hidden, archive, ret;
			char filepath[GCI_MAX_PATHNAME_LEN];  
	
			// Are there any installed scripts.
			GetNumListItems(tl->panel_id, TIMELAPSE_ACTIONS, &count);
	
			if(count == 0)
				return -1;
	
			// Get the path to the selected script
			GetCtrlVal(tl->panel_id, TIMELAPSE_ACTIONS, filepath);
	
			GetFileAttrs (filepath, &read_only, &system, &hidden, &archive); 

			if(read_only) {

				ret = GCI_ConfirmPopup("Warning", IDI_INFORMATION, "The script you wish to edit is readonly"
							"\nDo you wish to copy this script to a new file and continue editing.");

				if(ret) {

					FILE *fp = NULL, *in_fp = NULL;
					int err;
					char filename[GCI_MAX_PATHNAME_LEN] = "", new_filename[GCI_MAX_PATHNAME_LEN] = "", drive[50] = "", dir[GCI_MAX_PATHNAME_LEN] = "";
					char filename_prefix[500] = "", description[500] = "", author[500] = "", new_filepath[GCI_MAX_PATHNAME_LEN] = "";

					SplitPath(filepath, drive, dir, filename);
					get_file_without_extension(filename, filename_prefix);

					sprintf(new_filename, "%s_copy.py", filename_prefix);
					sprintf(new_filepath, "%s%s%s", drive, dir, new_filename);

					while (FileExists(new_filepath, NULL)){
						sprintf(new_filename, "%s_copy(%d).py", filename_prefix, i++);
						sprintf(new_filepath, "%s%s%s", drive, dir, new_filename);
					}

					if(CopyScriptDialog(tl, new_filename, description, author) >= 0) {

						sprintf(new_filepath, "%s%s%s", drive, dir, new_filename);

						if((err = CopyFile(filepath, new_filepath)) < 0) {
							ui_module_send_error(UIMODULE_CAST(tl), "Timelapse Error", "%s", "Failed to copy file");
							return -1;	
						}

						// change the name & description etc
						in_fp = fopen(filepath, "r");
						fp = fopen(new_filepath, "w");

						fprintf(fp, "# Name: %s\n", new_filename);
						fprintf(fp, "# Description: %s\n", description);
						fprintf(fp, "# Author: %s\n", author);
						fprintf(fp, "# Category: %s\n\n", "Default");

						if ( fp != NULL )
						{
							char line [ 1000 ]; /* or other suitable maximum line size */

							// Find first line without a comment, can be empty line
							if(fgets ( line, sizeof line, in_fp ) != NULL) {

								while ( fgets ( line, sizeof line, in_fp ) != NULL ) /* read a line */
								{			
									if (str_trim_whitespace(line)[0] != '#')
										break;
								}
							}

							// write that line
							fputs ( line, fp ); /* write the line */

							// write the rest of the file
							while ( fgets ( line, sizeof line, in_fp ) != NULL ) /* read a line */
							{
								fputs ( line, fp ); /* write the line */
							}				
						}
								
						fclose(fp);
						fclose(in_fp);

						EditScript(tl, new_filepath);   
					}
				}
				else {

					return 0;
				}
			}
			else {

				EditScript(tl, filepath);     
			}
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnTimeLapseHelp (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			timelapse *tl = (timelapse*) callbackData;    
			
#ifdef RESOURCES_DIRECTORY
			ShowHtmlHelp (RESOURCES_DIRECTORY "\\Help\\Microscopy.chm", HH_DISPLAY_TOPIC, NULL);
#endif

			break;
		}
	}
	
	return 0;
}

void CVICALLBACK MenuItemChanged(int menuBarHandle, int menuItemID, void *callbackData, 
		int panelHandle)
{
	char item[64], lwr_item[64];
	int index;
	timelapse *tl = (timelapse*) callbackData;    

	TimelapseScriptDetails details;

	GetMenuBarAttribute(menuBarHandle, menuItemID, ATTR_ITEM_NAME, item);

	strtolwr(item, lwr_item);

	index = dictionary_getint(tl->script_details_index, lwr_item, -1);

	if(index < 0)
		return;

	tl->last_script_index = index;
	ListGetItem(tl->script_details, &details, index);

	ClearListCtrl(panelHandle, TIMELAPSE_ACTIONS);
	InsertListItem(panelHandle, TIMELAPSE_ACTIONS, -1, item, details.filepath);
}

static int UncheckCheckedMenuItem(int menuBar, int menuOrMenuItem)
{
	int num=0, cur, i, menuItem=0, subMenu=0, bole, checked;

	if (!menuOrMenuItem) {
		GetMenuBarAttribute(menuBar, 0, ATTR_NUM_MENUS, &num);
		GetMenuBarAttribute(menuBar, 0, ATTR_FIRST_MENU_ID, &cur);
		}
	else {
		bole = SetBOLE(FALSE);
		GetMenuBarAttribute(menuBar, menuOrMenuItem, ATTR_NUM_MENU_ITEMS, &num);
		if (!num) {
			GetMenuBarAttribute(menuBar, menuOrMenuItem, ATTR_SUBMENU_ID, &subMenu);
			if (subMenu) {
				GetMenuBarAttribute(menuBar, subMenu, ATTR_NUM_MENU_ITEMS, &num);
				menuOrMenuItem = subMenu;
				}
			}
		SetBOLE(bole);
		if (num)
			GetMenuBarAttribute(menuBar, menuOrMenuItem, ATTR_FIRST_ITEM_ID, &cur);
		}
	if (!num) {
		GetMenuBarAttribute(menuBar, menuOrMenuItem, ATTR_CHECKED, &checked);
		if (checked) {
			SetMenuBarAttribute(menuBar, menuOrMenuItem, ATTR_CHECKED, 0);
			return 1; // quit - assume only one is checked 
			}
		else
			return 0;
		}
	
	for (i=0; i<num; i++) {
		if (UncheckCheckedMenuItem(menuBar, cur))
			break;
		if (!menuOrMenuItem)
			GetMenuBarAttribute(menuBar, cur, ATTR_NEXT_MENU_ID, &cur);
		else
			GetMenuBarAttribute(menuBar, cur, ATTR_NEXT_ITEM_ID, &cur);
		}
	return 0;
}


int CVICALLBACK OnTimeLapseMenuRingChanged(int panel, int control, int event, void *callbackData, 
	int eventData1, int eventData2)
{
	int showMenu=0;
	int top=0, left=0, height;
	timelapse *tl = (timelapse*) callbackData;  

	switch (event)
	{
		case EVENT_LEFT_CLICK:
			showMenu = 1;
			break;
		case EVENT_KEYPRESS:
			if (eventData1 == ' ') 
				showMenu = 1;
			break;
	}
	
	if (showMenu) {

		int 	menuItem, index;
		char 	label[64];
		
		InsertFilesInToList(tl);

		GetCtrlAttribute(panel, control, ATTR_TOP, &top);
		GetCtrlAttribute(panel, control, ATTR_HEIGHT, &height);
		GetCtrlAttribute(panel, control, ATTR_LEFT, &left);
		GetCtrlIndex(panel, control, &index);
		GetLabelFromIndex(panel, control, index, label);
		UncheckCheckedMenuItem(tl->action_menubar, 0);

		//menuItem = FindMenuItemFromLabel(tl->action_menubar, 0, label);
		menuItem = FindMenuItemIdFromNameInMenu(tl->action_menubar, 0, label, 0);

		if (menuItem)
			SetMenuBarAttribute(tl->action_menubar, menuItem, ATTR_CHECKED, 1);

		RunPopupMenu(tl->action_menubar, tl->action_menu, panel, top+height, left, 0, 0, 0, 0);
		}
	
	return (showMenu) ? 1 : 0;
}

int CVICALLBACK OnRepeatOptionChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;
			
			GetCtrlVal(panel, control, &val);
			
			if(val == REPEAT_FOREVER) {
				
				SetCtrlAttribute(panel, TIMELAPSE_REPEAT_VAL, ATTR_DIMMED, 1);	
			}
			else {
				
				SetCtrlAttribute(panel, TIMELAPSE_REPEAT_VAL, ATTR_DIMMED, 0);	
			}
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnTimeLapseTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_TIMER_TICK:
		{
			timelapse *tl = (timelapse*) callbackData;
	
			// Auto save points file
			timelapse_auto_save_data(tl);

			break;
		}
	}
	return 0;
}

#endif // MICROSCOPE_PYTHON_AUTOMATION