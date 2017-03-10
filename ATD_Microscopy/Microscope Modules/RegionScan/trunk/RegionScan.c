#include "RegionScan.h"
#include "RegionScan_ui.h"

#include "stage\stage.h"
#include "icsviewer_signals.h"

#include "focus.h"			   
#include "background_correction.h"
#include "string_utils.h"
#include "file_prefix_dialog.h"   
#include "FreeImageIcs_IO.h"
#include "FreeImageAlgorithms_IO.h" 
#include "cell_finding.h"

#include "microscope.h"
#include "mosaic.h"        

#include <cvirte.h>		
#include <userint.h>
#include <toolbox.h>
#include <formatio.h>
#include <utility.h>

#define RS_BASE_MSG (WM_USER+80)
#define RS_DISPLAY_IMAGE_MSG (RS_BASE_MSG+1) 

// Ok this is the design
// We have one thread that moves the stage the acquires an image
// This thread does acquisition as fast as possible. One an image is taken the image is place
// in a thread safe queue.
// The main thread continouly traverses this doing any neccessary processing.

typedef struct
{
	FIBITMAP *dib;
	int id;
	int	is_processed;
	double x; 	// Stage Points
	double y;   
	double z;   
	
} Tile;


// Create header file for the image stitching application   
// This writes a silly format. Can't be changed due to backward compatibility
// No named params make the file hard to read.
// No version info was on the file so hard to do backward compatibility.. 
// Only allows for storage of one overlap in microns
static void RegionScanSaveVersion1MosaicHeader(region_scan* rs, const char* filename_prefix, const char* ext, FILE *fp)
{
	float overlap_microns = 0.0f;

	// Store region of interest.
	fprintf(fp, "%.2f\t%.2f\t%.2f\t%.2f\n", rs->roi_left, rs->roi_top, rs->roi_width, rs->roi_height);
	
	overlap_microns = rs->x_overlap;  // only allowed one overlap, so use x value

	fprintf(fp, "%.6f\n", overlap_microns);	//overlap in um
	fprintf(fp, "%d\t%d\n", rs->frames_in_x, rs->frames_in_y);	// no. frames
	fprintf(fp, "%.6f\n", 1.0 / gci_camera_get_true_microns_per_pixel(rs->camera));		  //pixels per micron
	fprintf(fp, "%s\n", ext);  //image file extension
}
				

static dictionary* RegionScanSaveVersion2MosaicHeader(region_scan* rs, char* filename_prefix, char* ext)
{
	dictionary *d = dictionary_new(20);
	
	dictionary_set(d, "regionscan sequence info", NULL);
	
	dictionary_setdouble(d, "Version", 2.0);
	dictionary_setdouble_high_precision(d, "Roi Left", rs->roi_left);  
	dictionary_setdouble_high_precision(d, "Roi Top", rs->roi_top);  
	dictionary_setdouble_high_precision(d, "Roi Width", rs->roi_width);  
	dictionary_setdouble_high_precision(d, "Roi Height", rs->roi_height);  
	dictionary_setdouble_high_precision(d, "Horizontal Overlap", rs->x_overlap_percent);	//overlap in percentage
	dictionary_setdouble_high_precision(d, "Vertical Overlap", rs->y_overlap_percent);	//overlap in percentage
	dictionary_setint(d, "Horizontal Frames", rs->frames_in_x);	// rows & cols
	dictionary_setint(d, "Vertical Frames", rs->frames_in_y);		
	dictionary_setdouble_high_precision(d, "Pixels Per Micron", 1.0 / gci_camera_get_true_microns_per_pixel(rs->camera));	
	dictionary_set(d, "Extension", ext);	 
	dictionary_set(d, "File Format", filename_prefix);
	dictionary_set(d, "Correlated", "False"); 
	
	return d;
}


// These are the coordinates of each tile.
// They are relative to the left most and top most tile.
// In this casr that is roi left and roi top
static void WriteImageCoordsToSeq1File(FILE *fp, const char *filename, int left, int top)
{
	fprintf(fp, "%s\t%d\t%d\t\n", filename, left, top);
}

static void WriteImageCoordsToSeq2File(FILE *fp, const char *filename, int left, int top)
{
	fprintf(fp, "%s = (%d, %d)\n", filename, left, top);
}

static void CalcEstimatedTime(region_scan* rs, char *time_taken)
{
	double time_taken_so_far = Timer() - rs->start_time;
	double estimated_total_time = time_taken_so_far * ((double) rs->total_frames / rs->frames_done);

	seconds_to_friendly_time(estimated_total_time - time_taken_so_far, time_taken);
}

static dictionary* regionscan_get_metadata (region_scan* rs, int overall)
{
	Microscope *ms = microscope_get_microscope();    
	dictionary* d = microscope_get_metadata(ms, CAMERA_WINDOW(rs->camera));  
	char buffer[500] = "";	
	int width, height;
	FIBITMAP *fib = NULL;

	double microns_per_pixel = 1.0;

	if(overall) {

		microns_per_pixel = GCI_ImagingWindow_GetMicronsPerPixelFactor(rs->mosaic_window->window);
		fib = GCI_ImagingWindow_GetDisplayedFIB(rs->mosaic_window->window); 

		width = FreeImage_GetWidth(fib);
		height = FreeImage_GetHeight(fib);

		// Overwrite the camera image extents
		sprintf(buffer, "%.3e %.3e", width * microns_per_pixel * 1e-6, height * microns_per_pixel * 1e-6);  
		dictionary_set(d, "extents", buffer);   

		sprintf(buffer, "%.3e", width * microns_per_pixel * 1e-6); 
		dictionary_set(d, "image physical_sizex", buffer);

		sprintf(buffer, "%.3e", height * microns_per_pixel * 1e-6); 
		dictionary_set(d, "image physical_sizey", buffer);

		// Stage pos makes no sense for mosaic
		dictionary_unset(d, "stage pos");
		dictionary_unset(d, "stage positionx");
		dictionary_unset(d, "stage positiony");
		dictionary_unset(d, "stage positionz");
	}
	else {

		microns_per_pixel = GCI_ImagingWindow_GetMicronsPerPixelFactor(CAMERA_WINDOW(rs->camera));
		fib = GCI_ImagingWindow_GetDisplayedFIB(CAMERA_WINDOW(rs->camera));
	}

	sprintf(buffer, "%.2f", rs->roi_left);
	dictionary_set(d, "Roi Left", buffer); 
	sprintf(buffer, "%.2f", rs->roi_top);
	dictionary_set(d, "Roi Top", buffer); 
	sprintf(buffer, "%.2f", rs->roi_width);
	dictionary_set(d, "Roi Width", buffer); 
	sprintf(buffer, "%.2f", rs->roi_height);
	dictionary_set(d, "Roi Height", buffer); 

	sprintf(buffer, "%.2f", rs->nominal_overlap_percent);
	dictionary_set(d, "Percentage Overlap", buffer); 

	sprintf(buffer, "%.1f x %.1f", rs->fov_x, rs->fov_y);
	dictionary_set(d, "Tile Size", buffer); 

	return d;
}

static void regionscan_setup_metadata_for_mosaic_window (region_scan* rs)
{
	dictionary* d = regionscan_get_metadata (rs, 1);

	GCI_ImagingWindow_SetMetaData(rs->mosaic_window->window, d);
}

static void dictionary_ics_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	ICS *ics = (ICS *) data;
	
	FreeImageIcs_IcsAddHistoryString (ics, key, val);         	
}

void regionscan_save_metadata_for_tile(region_scan* rs, char *filename, char *extension, void* callback)
{	 
	ICS *ics=NULL;
	dictionary* d = regionscan_get_metadata (rs, 0);

	if(strcmp(extension, ".ics"))
		return;
	
	FreeImageIcs_IcsOpen(&ics, filename, "rw");  

	dictionary_foreach(d, dictionary_ics_keyval_callback, ics);

	dictionary_del(d);
	FreeImageIcs_IcsClose (ics);   
	
	return;
}

void regionscan_stop(region_scan* rs)
{
   	rs->stop_scan = 1;

	// If we are stopping stop any pauses.
	rs->pause_scan = 0;

	if(rs->panel_id > 0) {
		SetCtrlAttribute (rs->panel_id, ROI_SCAN_PAUSE, ATTR_LABEL_TEXT, "Pause Scan");
		SetCtrlAttribute (rs->panel_id, ROI_SCAN_PAUSE, ATTR_DIMMED, 1);
		SetCtrlAttribute (rs->panel_id, ROI_SCAN_STOP, ATTR_DIMMED, 1);
	}
}

static void RegionScanPause(region_scan* rs, int pause)
{
	if (pause == 1)	{	//Pause scan 
	    	
		SetCtrlAttribute (rs->panel_id, ROI_SCAN_PAUSE, ATTR_LABEL_TEXT, "Resume");
		SetCtrlAttribute (rs->panel_id, ROI_SCAN_PAUSE, ATTR_DIMMED, 0);
		SetCtrlAttribute (rs->panel_id, ROI_SCAN_STOP, ATTR_DIMMED, 0);
				
		rs->pause_scan = 1;
	}
	else {					//Restart scan	
			
    	SetCtrlAttribute (rs->panel_id, ROI_SCAN_PAUSE, ATTR_LABEL_TEXT, "Pause Scan");
		SetCtrlAttribute (rs->panel_id, ROI_SCAN_PAUSE, ATTR_DIMMED, 0);
		SetCtrlAttribute (rs->panel_id, ROI_SCAN_STOP, ATTR_DIMMED, 0);
				
		rs->pause_scan = 0;
	}	
}


static int CVICALLBACK ProcessImageThread(void *data)
{
	region_scan* rs = (region_scan*) data;
	
	double start_time;
	int err, items_in_queue = 1, acquisition_thread_status = 0;
    Tile tile;
	char filepath[GCI_MAX_PATHNAME_LEN] = "";
	static int count = 0;
	FIBITMAP *cell_dib = NULL;

	while(rs->frames_done < rs->total_frames)
	{
		if(rs->stop_scan)
			return 0;

		if(rs->pause_scan) {
			Delay(1.0);
			continue;
		}

		if(CmtReadTSQData (rs->process_queue, &tile, 1, 2000, 0) <= 0) {

			CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), rs->acquisition_thread_id,
				ATTR_TP_FUNCTION_EXECUTION_STATUS, &acquisition_thread_status);

			// Acquisition Thread no longer executing and process queue is empty so we return
			if(acquisition_thread_status != kCmtThreadFunctionExecuting)
				return 0;

			continue;
		}

		if(tile.is_processed == 0)
		{	
			if(ref_images_should_process(rs->ms->_ri)) {
				if(ref_images_in_place_process_forced(rs->ms->_ri, &(tile.dib)) > 0)
					ref_images_disable(rs->ms->_ri); 			
			}

			if(rs->cell_finding_enabled) {

				cell_finder* cf = microscope_get_cellfinder(rs->ms);
				cell_dib = cell_finder_find_cells(cf, tile.dib, tile.x, tile.y, tile.z, 0, 0);
			}

			if(rs->action == RS_SAVE || rs->action == RS_SAVE_DISPLAY) { // Save Images

				sprintf(filepath, "%s%s%d%s", rs->output_dir, rs->output_filename, rs->tile_count++, rs->extension);

				if(strcmp(rs->extension, ".ics") == 0) {				
					dictionary *d = NULL;

					if(rs->cell_finding_enabled && cell_dib != NULL) {
						// For Surrey to save the marked image from cell finding, if possible
						//11022010 - saves cell finding processed tile, but if it finds nothing it returns a null hence this method prevents
						//region scan from crashing as copy of orignial tile is saved instead tile.dib otherwise error comes from outside cell finder
						//if control is required I suggest adding its to region scan as option in GUI
						err = FreeImageIcs_SaveImage(cell_dib, filepath, 1);
					}					
					else {	
						// Glenn tried using the save image thread at some time, not sure why he decided against it (although PB checked this in in Nov 2010).					
						//microscope_saveimage(Microscope* microscope, FIBITMAP *dib, char *filepath)

						#ifdef REAL_TIME_PROFILE
						start_time = Timer();
						#endif

						err = FreeImageIcs_SaveImage(tile.dib, filepath, 1);

						#ifdef REAL_TIME_PROFILE
						printf("FreeImageIcs_SaveImage Time %f\n", Timer() - start_time);
						#endif
					}

					regionscan_save_metadata_for_tile(rs, filepath, ".ics", rs);
				}
				else {
					//err = FIA_SaveFIBToFile(tile.dib, filepath, BIT8);
					err = FIA_SimpleSaveFIBToFile(tile.dib, filepath);
				}

				if(err == FIA_ERROR) {
					RegionScanPause(rs, 1);
					GCI_MessagePopup("Error", "Error saving data. Check the disk is not full");		
					continue;
				}
			}

			// We have the image we now need to proccess it and place it in the mosaic       
			if(mosaic_window_add_image(rs->mosaic_window, tile.dib, tile.x, tile.y) < 0) {
				regionscan_stop(rs);
				return 0;
			}

			tile.is_processed = 1;	
			rs->frames_done++;
		}

		if(rs->cell_finding_enabled) {
			//SendMessage(rs->window_hwnd, RS_DISPLAY_IMAGE_MSG, 0, cell_dib);
			SendMessageTimeout(rs->window_hwnd, RS_DISPLAY_IMAGE_MSG,
					(WPARAM) 0, (LPARAM) cell_dib, SMTO_ABORTIFHUNG, 5000, NULL);  
	
		}
		else {
			//SendMessage(rs->window_hwnd, RS_DISPLAY_IMAGE_MSG, 0, tile.dib);
			SendMessageTimeout(rs->window_hwnd, RS_DISPLAY_IMAGE_MSG,
					(WPARAM) 0, (LPARAM) tile.dib, SMTO_ABORTIFHUNG, 5000, NULL);  
		}

		if(tile.dib != NULL)
			FreeImage_Unload(tile.dib);

		if(cell_dib != NULL)
			FreeImage_Unload(cell_dib);

		#ifdef REAL_TIME_PROFILE
		printf("Processed tile in Process queue\n");
		#endif

		ProcessSystemEvents();
	}

    return 0;	
}


LRESULT CALLBACK RSWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	region_scan* rs = (region_scan*) data;

	switch(message)
	{		
    	case RS_DISPLAY_IMAGE_MSG:
    	{
			FIBITMAP *dib = (FIBITMAP *) lParam;
	
			char filename[50] = "", time_taken[50] = "";			

			if(dib == NULL)
				return 0;	

			if(rs->stop_scan)
				return 0;

			if(rs->action == RS_SAVE_DISPLAY || rs->action == RS_DISPLAY_ONLY) {

				gci_camera_display_image(rs->camera, dib, NULL);
			}

			mosaic_window_update(rs->mosaic_window);  
			
			if(rs->setup_of_mosaic_metadata == 0) {
				regionscan_setup_metadata_for_mosaic_window(rs);
				rs->setup_of_mosaic_metadata = 1;
			}

			if((rs->frames_done % 5) == 0) {
				CalcEstimatedTime(rs, time_taken);
				SetCtrlVal(rs->panel_id, ROI_SCAN_TIME_TAKEN, time_taken);
			}
		}

      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) rs->uimodule_func_ptr,
							hwnd, message, wParam, lParam);
}

void regionscan_status(region_scan *rs, int *has_run, int *active, double *percentage_complete, char *start_time, char *end_time)
{
	*active = !(rs->stop_scan);

	*has_run = rs->has_run;

	if(rs->frames_done == 0.0)
		*percentage_complete = 0.0;
	else
		*percentage_complete = (double) rs->frames_done / rs->total_frames;

	strncpy(start_time, rs->start_date_time, 199);
	strncpy(end_time, rs->end_date_time, 199);
}

// Moves the stage and gets the image.
static int CVICALLBACK AcquisitionThread(void *data)
{
	// Thread controlling the xy stage. 
	// Does a stage move as soon as the previous acquisition is complete.
	
	region_scan* rs = (region_scan*) data;
	Tile tile;
	int items_in_save_queue = 0; //, max_in_save_queue;
	double adjust_delay = 0.0;
    int was_on = 0, number_flushed, count = 1, items_in_queue, direction = 1;
	int next_x_pos=0, x_pos=0, y_pos=0;
	char position_str[500] = "";
	double rel_x = 0.0, rel_y = 0.0;
	double wait_time = 0.0;
	double z_speed = 0.0, z_accel = 0.0, current_z;
    double next_rel_x_pos, fov_x_minus_overlap, fov_y_minus_overlap;

	#ifdef REAL_TIME_PROFILE
	double t1, t2, t3;
	#endif

	tile.is_processed = 0;
	tile.dib = 0;

	PROFILE_START("AcquisitionThread");

	// We are assuming the z move takes less time than the xy
	// stage_calc_move_time(rs->ms->_stage, rs->fov_x, rs->fov_y, &wait_time);

	was_on = stage_get_joystick_status(rs->ms->_stage);

	// Some stage IE LStep dont do Z moves when the joystick
	// is enable so here we work around the bug.
	// It can't go in the stage_move_function as turning off the
	// joystick seems to be a very slow operation.
	if(was_on) {
		stage_set_joystick_off(rs->ms->_stage);
	}

	if(z_drive_is_part_of_stage(MICROSCOPE_MASTER_ZDRIVE(rs->ms))) {
		z_drive_get_speed(MICROSCOPE_MASTER_ZDRIVE(rs->ms), &z_speed);
		z_drive_get_accel(MICROSCOPE_MASTER_ZDRIVE(rs->ms), &z_accel);
	}

	stage_prevent_changed_signal_emission(rs->ms->_stage);
	z_drive_prevent_changed_signal_emission(MICROSCOPE_MASTER_ZDRIVE(rs->ms));

	// Goto the first position waiting until it has completed.
	if(region_of_interest_goto_stage_xy(rs->roi, rs->roi_left, rs->roi_top) < 0){
//		PROFILE_STOP("AcquisitionThread");
//		goto ROI_ERROR;	

		// Assume that if this fails that it is due to being outside the defined safe region.
	}

	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(rs->ms), &current_z);

	fov_x_minus_overlap = rs->fov_x - rs->x_overlap;
	fov_y_minus_overlap = rs->fov_y - rs->y_overlap;

	// Start position
	rel_x -= fov_x_minus_overlap;
	x_pos = -1;

	while (1) {

		if (rs->stop_scan)
			goto FINISHED;
			
		if (rs->pause_scan) {
			Delay(1.0);
			continue;	//User has pressed Pause
		}

		CmtGetTSQAttribute(rs->process_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_queue);		

		// For some reason the acquition thread has got to far ahead of the processing
		// thread. So we now delay a little and keep the images created to 10 
		// This is unlikely to happen in a real world setup. Only when using dummy stuff so
		// the acquisition is really fast.
		if(items_in_queue > 10)
		{
			adjust_delay += 0.2;

			// Allow time to catch up
			while(items_in_queue > 0) 
			{
				CmtGetTSQAttribute(rs->process_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_queue);	
				#ifdef REAL_TIME_PROFILE
				printf("Items in process queue: %d - holding acq thread\n", items_in_queue);
				#endif
				Delay(0.01);
				ProcessSystemEvents();

				if(rs->stop_scan)
					return 0;
			}
		}
/*
		CmtGetTSQAttribute(rs->ms->_save_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_save_queue);		

		max_in_save_queue = MICROSCOPE_SAVE_IMAGE_QUEUE_SIZE / 2;

		if(items_in_save_queue >= max_in_save_queue) {
			adjust_delay += 0.2;

			while(items_in_save_queue > 0) {
				CmtGetTSQAttribute(rs->ms->_save_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_save_queue);		
				ProcessSystemEvents();
			}
		}
*/
		#ifdef REAL_TIME_PROFILE
		printf("items in process q: %d\n", items_in_queue);
		#endif

		next_rel_x_pos = rel_x + (direction * fov_x_minus_overlap);
		next_x_pos = x_pos + direction;
		
		if(next_x_pos >= rs->frames_in_x) {
		    rel_y += fov_y_minus_overlap;
			y_pos++;
			direction = -1;

			if (y_pos >= rs->frames_in_y) {
				PROFILE_STOP("AcquisitionThread");
				goto FINISHED;
			}
		}
		else if(next_x_pos < 0) {
		    rel_y += fov_y_minus_overlap;
			y_pos++;
			direction = 1;

			if (y_pos >= rs->frames_in_y) {
				PROFILE_STOP("AcquisitionThread");
				goto FINISHED;
			}
		}
		else {
			rel_x = rel_x + (direction * fov_x_minus_overlap);
			x_pos += direction;
		}
		
		tile.id = count;
		tile.x = rs->roi_left + rel_x;
		tile.y = rs->roi_top + rel_y;
		tile.z = current_z;

		#ifdef REAL_TIME_PROFILE
		t1 = Timer();
		#endif

		PROFILE_START("Region Scan - region_of_interest_goto_stage_xy");
		
		if(region_of_interest_goto_stage_xy(rs->roi, tile.x, tile.y) < 0){
			//PROFILE_STOP("Region Scan - region_of_interest_goto_stage_xy");
			//PROFILE_STOP("AcquisitionThread");
			//goto ROI_ERROR;	

			// Assume that if this fails that it is due to being outside the defined safe region.
			continue;
		}
		
		/*
		if(region_of_interest_goto_stage_xy_advanced(rs->roi, x, y, wait_time, 
							z_speed, z_accel, current_z) < 0){
			PROFILE_STOP("Region Scan - region_of_interest_goto_stage_xy");
			PROFILE_STOP("AcquisitionThread");
			goto ROI_ERROR;
		}
		*/

		PROFILE_STOP("Region Scan - region_of_interest_goto_stage_xy");

		#ifdef REAL_TIME_PROFILE
		printf("Adjust Delay %f\n", adjust_delay);
		#endif

		Delay(adjust_delay);
		Delay(rs->stage_dwell);

		#ifdef REAL_TIME_PROFILE
		t2 = Timer()-t1;
		#endif

		// Perform an autofocus at every point if required
		if (rs->perform_swautofocus_every_point) {
			sw_autofocus_autofocus(rs->ms->_sw_af);
			gci_camera_set_snap_mode(rs->camera);
		}

		PROFILE_START("Region Scan - gci_camera_get_image");
		
		tile.dib = gci_camera_get_image(rs->camera, NULL);    
		//tile.dib = FIA_LoadFIBFromFile("C:\\test.tif");

		if(tile.dib == NULL)
			continue;

		PROFILE_STOP("Region Scan - gci_camera_get_image");
		
		#ifdef REAL_TIME_PROFILE
		t3 = Timer()-t2-t1;
		#endif

		if (rs->stop_scan) {
			PROFILE_STOP("AcquisitionThread");
			goto FINISHED;	//User has pressed Abort
		}

		CmtWriteTSQData(rs->process_queue, &tile, 1, TSQ_INFINITE_TIMEOUT, &number_flushed);

		#ifdef REAL_TIME_PROFILE
		printf("stage %f image %f\n", t2, t3);
		#endif

		ProcessSystemEvents();

		count++;  
	}

FINISHED:

	stage_allow_changed_signal_emission(rs->ms->_stage);
	z_drive_allow_changed_signal_emission(MICROSCOPE_MASTER_ZDRIVE(rs->ms));

	rs->acquisition_done = 1;

	if(was_on)
		stage_set_joystick_on(rs->ms->_stage);

	return 0;

//ROI_ERROR:

	stage_allow_changed_signal_emission(rs->ms->_stage);
	z_drive_allow_changed_signal_emission(MICROSCOPE_MASTER_ZDRIVE(rs->ms));

	if(was_on)
		stage_set_joystick_on(rs->ms->_stage);

	return -1;
}

int CVICALLBACK cbRegionScanPause (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_scan *rs = (region_scan*) callbackData; 
			
			if(rs->pause_scan == 0)
	    		RegionScanPause(rs, 1);
			else
				RegionScanPause(rs, 0);

			break;
		}
	}
	
	return 0;
}

 
void regionscan_set_roi(region_scan* rs, double left, double top, double width, double height)
{	// Here we expect a region to be scanned
	// region_of_interest stores the range of stage coords that will cover the required area at the current fov 
	int w, h; 
	double factor;
	
	// Get the fov
	gci_camera_get_size(rs->camera, &w, &h);
	factor = gci_camera_get_true_microns_per_pixel(rs->camera);              
	
	// fov in um
	rs->fov_x = (float) (factor * w);   
	rs->fov_y = (float) (factor * h);   

	// correct area to be consistent with region_of_interest
	if (width < rs->fov_x) { // less than 1 image wide
		left += width/2.0;	// store the stage pos that we want
		width = 0;   // no need to move the stage
	}
	else {
		left += rs->fov_x/2.0; // move in left coord 1/2 fov
		width -= rs->fov_x;  // reduce width by 1 fov
	}

	if (height < rs->fov_y) {
		top += height/2.0;
		height = 0;  
	}
	else {
		top += rs->fov_y/2.0;
		height -= rs->fov_y; 
	}

	region_of_interest_set_region(rs->roi, left, top, width, height);
	
	rs->roi_set = 1;
}

static void calculate_roi_frames(region_scan* rs)
{
	int width, height, overlap_pixels; 
	double factor, centre_x, centre_y;
	float overlap;
	
	
	// Get the fov
	gci_camera_get_size(rs->camera, &width, &height);
	factor = gci_camera_get_true_microns_per_pixel(rs->camera);              
	
	// fov in um
	rs->fov_x = (float) (factor * width);   
	rs->fov_y = (float) (factor * height);   
	
	// region_of_interest stores the range of stage coords that will cover the required area at the current fov, 
	// the given area does not cover the required area without the camera fov.
	region_of_interest_get_region(rs->roi, &(rs->roi_left), &(rs->roi_top), &(rs->roi_width), &(rs->roi_height));
	
	GetCtrlVal (rs->panel_id, ROI_SCAN_OVERLAP, &overlap);
	rs->nominal_overlap_percent = overlap;
	// calculate overlap in microns and %, rounded to the nearest pixel
	overlap_pixels        = (int)((float)width * (overlap / 100.0f) + 0.5);  // pixels
	rs->x_overlap         = (float)(factor * (double)(overlap_pixels));  // um
	rs->x_overlap_percent = (float)((double)overlap_pixels / (double)width * 100.0);  // real % overlap used

	overlap_pixels        = (int)((float)height * (overlap / 100.0f) + 0.5);
	rs->y_overlap         = (float)(factor * (double)(overlap_pixels));
	rs->y_overlap_percent = (float)((double)overlap_pixels / (double)height * 100.0);

	// calculate the number of actual frames to use, 
	// add 1 for the poles/gaps problem 
	// (because regionOfInterest passes the difference in stage coords, not the required coverage) 
	rs->frames_in_x = (int) ((int) ceil(rs->roi_width / (rs->fov_x - rs->x_overlap)) + 1);
	rs->frames_in_y = (int) ((int) ceil(rs->roi_height / (rs->fov_y - rs->y_overlap)) + 1);   
	
	SetCtrlVal(rs->panel_id, ROI_SCAN_FRAMES_X, rs->frames_in_x);
	SetCtrlVal(rs->panel_id, ROI_SCAN_FRAMES_Y, rs->frames_in_y);

	rs->total_frames = rs->frames_in_x * rs->frames_in_y;

	// now can set the real roi to use, that covers the FULL AREA with images, 
	// but the stage must move 1 image less, 0.5 off each side, all in stage coords (um)
	centre_x = rs->roi_left + rs->roi_width /2.0;
	centre_y = rs->roi_top  + rs->roi_height/2.0;
	rs->roi_left    = centre_x - ((double)rs->frames_in_x / 2.0 - 0.5) * (rs->fov_x - rs->x_overlap);
	rs->roi_top     = centre_y - ((double)rs->frames_in_y / 2.0 - 0.5) * (rs->fov_y - rs->y_overlap);
	rs->roi_width   = ((double)rs->frames_in_x - 1.0) * (rs->fov_x - rs->x_overlap);
	rs->roi_height  = ((double)rs->frames_in_y - 1.0) * (rs->fov_y - rs->y_overlap);

	if(rs->cell_finding_enabled) {
		cell_finder* cf = microscope_get_cellfinder(rs->ms);
		gci_cellmap_set_region_size_pos(cf->map, rs->roi_top, rs->roi_left, rs->roi_width, rs->roi_height);
	}
}

static int check_diskspace(region_scan* rs, const char *dir, unsigned long tile_byte_size, int number_of_tiles)
{
	UInt64Type total_bytes, free_bytes;
	unsigned long required_disk_space;

	GetDiskSpace (dir, &total_bytes, &free_bytes);  // 64 bit variables

	required_disk_space = number_of_tiles * (tile_byte_size + 2000);	// bytes

	if(UInt64TypeCompareUInt (free_bytes, required_disk_space) <= 0) // free_space is less than required
	{
		return -1;
	}

	return 0;
}

static void ImageWindowCloseEventHandler( GCIWindow *win, void* callback_data )
{
	region_scan *rs = (region_scan*) callback_data;
	
	/* Destroy imaging window */
	GCI_ImagingWindow_Close(rs->image_window);
	
	rs->image_window = NULL;
	
	return;
}


static void OnMosaicClicked (GCIWindow *window, const Point image_point, const Point viewer_point, void* data )
{
	int pixelx, pixely, width, height, tile_width, tile_height, field_of_view_x, field_of_view_y;
	double startx, starty, microns_per_pixel = 1.0;
	double stage_x, stage_y, roi_width, roi_height;
	Microscope *ms;
	region_scan *rs = (region_scan*) data;         
	XYStage *stage;       
	Rect mosaic_rect = mosaic_window_get_region(rs->mosaic_window);
	
	//region_of_interest_get_region(rs->roi, &startx, &starty, &roi_width, &roi_height);
	startx    = mosaic_rect.left;
	starty    = mosaic_rect.top;
	roi_width  = mosaic_rect.width;
	roi_height = mosaic_rect.height;

	ms = microscope_get_microscope();
	stage = microscope_get_stage(rs->ms);       
	//optical_calibration_get_calibration_factor_for_current_objective(ms->_optical_cal, &microns_per_pixel); 
	microns_per_pixel = gci_camera_get_true_microns_per_pixel(rs->camera);  

	if(image_point.x < 0 || image_point.y < 0)
		return;
	
	//Start represents middle of top left tile and y is "upside down"
	pixelx = image_point.x;
	pixely = image_point.y;
	
	width = FreeImage_GetWidth(rs->mosaic_window->mosaic_image);
	height = FreeImage_GetHeight(rs->mosaic_window->mosaic_image);

	gci_camera_get_size(rs->camera, &tile_width, &tile_height);

	field_of_view_x = (int) (tile_width * microns_per_pixel);
	field_of_view_y = (int) (tile_height * microns_per_pixel);

	stage_x = startx - (field_of_view_x/2) + (((float) pixelx / (float) width) * roi_width);
	stage_y = starty - (field_of_view_y/2) + (((float) pixely / (float) height) * roi_height);      
	
	stage_set_joystick_off(rs->ms->_stage);

	region_of_interest_goto_stage_xy(rs->roi, stage_x, stage_y);    
	
	gci_camera_snap_image(rs->camera);
	
	//Moves disable the joystick
	stage_set_joystick_on(stage);     
}

int CVICALLBACK cbRegionScanStop (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_scan *rs = (region_scan*) callbackData;   
			
    		regionscan_stop(rs);
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK cbRegionScanEnableCellFinding (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_scan *rs = (region_scan*) callbackData;   
		
			GetCtrlVal(panel, control, &(rs->cell_finding_enabled));
			
			if(rs->cell_finding_enabled) {

    			SetCtrlAttribute(panel, ROI_SCAN_CF_CONF, ATTR_DIMMED, 0);
			}
			else {

				SetCtrlAttribute(panel, ROI_SCAN_CF_CONF, ATTR_DIMMED, 1);  
			}

			break;
		}
	}
	
	return 0;
}
		
int CVICALLBACK cbRegionScanCellFindingConfigure (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_scan *rs = (region_scan*) callbackData;   
			
    		cell_finder_display(rs->ms->_cf);
			
			break;
		}
	}
	
	return 0;
}
	

static void region_scan_save_default_data(region_scan* rs)
{
	FILE *fd;
    dictionary *d = dictionary_new(10);
	
	char path[GCI_MAX_PATHNAME_LEN], dir[GCI_MAX_PATHNAME_LEN];

	microscope_get_data_directory(rs->ms, dir);
	
    sprintf(path, "%s\\%s", dir, "RegionScanData.ini");
	
	fd = fopen(path, "w");
	
	dictionary_set(d, "Region Scan Data", NULL);

	dictionary_setdouble(d, "stage dwell", rs->stage_dwell);  
    dictionary_setdouble(d, "Overlap Percentage", rs->nominal_overlap_percent);
	
	if(rs->roi_set) {
	
    	dictionary_setdouble(d, "Roi Left", rs->roi_left);
    	dictionary_setdouble(d, "Roi Top", rs->roi_top);
    	dictionary_setdouble(d, "Roi Width", rs->roi_width);
		dictionary_setdouble(d, "Roi Height", rs->roi_height);

		if(region_of_interest_is_focal_plane_valid(rs->roi)) {

			double a, b, c;

			region_of_interest_get_focus_points(rs->roi, &a, &b, &c);

			dictionary_setdouble(d, "Roi Focus Point A", a);
			dictionary_setdouble(d, "Roi Focus Point B", b);
			dictionary_setdouble(d, "Roi Focus Point C", c);
		}
	}

	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
}


static void region_scan_load_default_data(region_scan* rs)
{
	dictionary* d = NULL;
	int file_size;
	char path[GCI_MAX_PATHNAME_LEN], dir[GCI_MAX_PATHNAME_LEN];
	//double a, b, c;

	microscope_get_data_directory(rs->ms, dir);
	
    sprintf(path, "%s\\%s", dir, "RegionScanData.ini");
	
	if(!FileExists(path, &file_size))
		return;	 	
	
	d = iniparser_load(path);  

	if(d == NULL)
		return;
		
	rs->stage_dwell = dictionary_getdouble(d, "Region Scan Data:stage dwell", 0.0f);     
	
	rs->nominal_overlap_percent = (float) dictionary_getdouble(d, "Region Scan Data:Overlap Percentage", 2.0f); 
			   
    rs->roi_width = dictionary_getint(d, "Region Scan Data:Roi Width", -1);
		
	if(rs->roi_width != -1) {
			
		// we have a previous saved roi
		rs->roi_left = dictionary_getdouble(d, "Region Scan Data:Roi Left", 0.0);       
        rs->roi_top = dictionary_getdouble(d, "Region Scan Data:Roi Top", 0.0);
        rs->roi_width = dictionary_getdouble(d, "Region Scan Data:Roi Width", 0.0);
        rs->roi_height = dictionary_getdouble(d, "Region Scan Data:Roi Height", 0.0);
        
		region_of_interest_set_region(rs->roi, rs->roi_left, rs->roi_top, rs->roi_width, rs->roi_height);
	
		/*
		a = dictionary_getdouble(d, "Region Scan Data:Roi Focus Point A", 0.0);
		b = dictionary_getdouble(d, "Region Scan Data:Roi Focus Point B", 0.0);
		c = dictionary_getdouble(d, "Region Scan Data:Roi Focus Point C", 0.0);
	
		region_of_interest_set_focus_points(rs->roi, a, b, c);
		*/

		rs->roi_set = 1;
	}
			
    dictionary_del(d); 
}

int regionscan_start(region_scan *rs, RS_ACTION action, char *output_dir,
								 char *filename_prefix_str, char *filename_ext)
{
    int bpp, tile_width, tile_height, items_in_queue, tile_byte_size, file_size;
    static int callback_id = 0;
    double factor, intensity, start_time, timeout = 5.0;
	char time_taken[20], seq_filepath[GCI_MAX_PATHNAME_LEN] = "", header_filepath[GCI_MAX_PATHNAME_LEN] = "";
	char filename_prefix[500]; //, ext_found[50];
    FILE *seq2_fp = NULL;
    FREE_IMAGE_TYPE type;
    FIBITMAP *dib = NULL;
    dictionary* d = NULL;
	int lastCharIndex, retVal;
	char lastChar[1];

    XYStage *stage = microscope_get_stage(rs->ms);  

	logger_log(UIMODULE_LOGGER(rs), LOGGER_INFORMATIONAL, "Regionscan Started");

//	printf("started rs: 0x%x %d\n", rs, rs->_master_camera_signal_id);

	// PRB May 2012, filename_prefix_str should already be a prefix with no extension, why try to strip an extension here? It stops the use of '.' in filenames.
//	if(get_file_extension(filename_prefix_str, ext_found) > 0)
//		get_file_without_extension(filename_prefix_str, filename_prefix);
//	else
//		strcpy(filename_prefix, filename_prefix_str);
	// just copy it instead
	strcpy(filename_prefix, filename_prefix_str);

	// if last char of filename_prefix is a number and not '_' add '_'
	lastCharIndex = strlen(filename_prefix);
	if (lastCharIndex >= 1){
		lastCharIndex--;
		memcpy(lastChar, &filename_prefix[lastCharIndex], 1); 
		if (isdigit(*lastChar) && (memcmp(lastChar, "_", 1) != 0))
			strcat(filename_prefix, "_");
	}

	strcpy(rs->output_dir, output_dir);
	strcpy(rs->output_filename, filename_prefix);
	strcpy(rs->extension, filename_ext);

	rs->action = action;
	rs->setup_of_mosaic_metadata = 0;
	rs->has_run = 1;

    gci_camera_set_snap_mode(rs->camera);

    focus_set_off(rs->ms->_focus);
            
    if(rs->roi_set != 1) {
		GCI_MessagePopup("Attention", "You must first set a region of interest to scan");
        goto RS_FINISHED;
    }
            
    if(callback_id)
		GCI_ImagingWindow_DisconnectMouseDownHandler(rs->mosaic_window->window, callback_id);
            
    region_of_interest_panel_hide(rs->roi); 

	get_time_string(rs->start_date_time);
 
    microscope_prevent_automatic_background_correction(rs->ms);
	stage_use_cached_data_for_read(rs->ms->_stage, 1);
	// TODO 
	// This could be a problem for sysytems with autofocus hardware
	// as the cached data won't be updated ?
	zdrive_use_cached_data_for_read(MICROSCOPE_MASTER_ZDRIVE(rs->ms), 1);

	microscope_disable_metadata_on_snap(rs->ms, 1);

    stage_set_joystick_off(stage);  
    rs->stop_scan = 0;
    rs->acquisition_done  = 0;
    rs->frames_done = 0;

    ui_module_disable_panel(rs->panel_id, 0);
    SetCtrlAttribute (rs->panel_id, ROI_SCAN_PAUSE, ATTR_DIMMED, 0);
    SetCtrlAttribute (rs->panel_id, ROI_SCAN_STOP, ATTR_DIMMED, 0);

    microscope_disable_all_panels(rs->ms, 1);     

    calculate_roi_frames(rs);
       
    dib = gci_camera_get_image(rs->camera, NULL);

	start_time = Timer();

	// What the hell is this ?
	// I think it was a hack to deal with when the camera returns no image :(
	while(dib == NULL && ((Timer() - start_time) < timeout)) {
		dib = gci_camera_get_image(rs->camera, NULL);
		Delay(0.5);
		ProcessSystemEvents();
		FreeImage_Unload(dib);
	}

	if(dib == NULL)
		goto RS_FINISHED;

    bpp = FreeImage_GetBPP(dib);
    type = FreeImage_GetImageType(dib);
    tile_width = FreeImage_GetWidth(dib);
    tile_height = FreeImage_GetHeight(dib);
    tile_byte_size = (tile_width * tile_height * bpp) / 8;
    FreeImage_Unload(dib);

	cell_finder_clear_map(rs->ms->_cf);

	factor = gci_camera_get_true_microns_per_pixel(rs->camera);      

	mosaic_window_setup(rs->mosaic_window, type, bpp, 
		MakeRect((int) rs->roi_top, (int) rs->roi_left, (int) (rs->roi_height+(factor * ((double)tile_height - (double)rs->y_overlap))), (int)(rs->roi_width+(factor * ((double)tile_width - (double)rs->x_overlap)))));

    mosaic_window_clear(rs->mosaic_window);
    mosaic_window_set_microns_per_pixel(rs->mosaic_window, factor);    
    mosaic_window_show(rs->mosaic_window);
      
    SetCtrlVal(rs->panel_id, ROI_SCAN_TEXTMSG, "Est Time");

    CmtNewTSQ(1000, sizeof(Tile), OPT_TSQ_DYNAMIC_SIZE, &(rs->process_queue));

    rs->tile_count = 1;
  
    if(rs->action == RS_SAVE || rs->action == RS_SAVE_DISPLAY) { // Saving images
                
		strcpy(rs->output_filename, filename_prefix);
		strcpy(rs->extension, filename_ext);

        // Check the disk space.
		if(check_diskspace(rs, output_dir, tile_byte_size, rs->total_frames) < 0) {
			if(!GCI_ConfirmPopup("Problem", IDI_INFORMATION, "There's not enough disk space for this acquisition. Do you wish to continue ?")) {
				goto RS_FINISHED;  
            }			
        }

        sprintf(header_filepath, "%s%s_Metadata.txt", output_dir, filename_prefix);      
        GCI_ImagingWindow_SaveMetaDataToTextFile(rs->camera->_camera_window, header_filepath);

        //sprintf(seq_filepath, "%s%s.seq", output_dir, rs->output_filename); 
        //rs->seq1_fp = fopen(seq_filepath, "w");
                
        sprintf(seq_filepath, "%s%s_v2.seq", output_dir, filename_prefix); 

		// Check if the are any existing files with the same prefix
		// By checking for the metadata file.  
		if(FileExists(seq_filepath, &file_size)) {
			int ret = GCI_ConfirmPopup("Problem", IDI_WARNING, 
				"There appears to be regionscan data already present in this directory with that prefix.\n"
				"Do you wish to overwrite this data ?");
		
			if(ret == 0)
				goto RS_FINISHED;  
		}

        seq2_fp = fopen(seq_filepath, "w"); 

        //RegionScanSaveVersion1MosaicHeader(rs, rs->output_filename, rs->extension, rs->seq1_fp);

        d = RegionScanSaveVersion2MosaicHeader(rs, rs->output_filename, rs->extension);
        dictionary_setint(d, "Tile Width", tile_width);
        dictionary_setint(d, "Tile Height", tile_height);
        dictionary_setint(d, "Tile Bits Per Pixel", bpp);
        dictionary_setint(d, "Tile Image Type", type);

		iniparser_save(d, seq2_fp);   // save seq file so far in case of problem, there may be some recoverable data, max intensity should be added at end
		fclose(seq2_fp);
    }

    microscope_stop_all_timers(rs->ms);
            
    rs->start_time = Timer();    

    CmtScheduleThreadPoolFunction(gci_thread_pool(), ProcessImageThread, rs, &(rs->process_thread_id));
            
    CmtScheduleThreadPoolFunction(gci_thread_pool(), AcquisitionThread, rs, &(rs->acquisition_thread_id));
                
    CmtWaitForThreadPoolFunctionCompletion (gci_thread_pool(), rs->acquisition_thread_id,
           OPT_TP_PROCESS_EVENTS_WHILE_WAITING);

    CmtWaitForThreadPoolFunctionCompletion (gci_thread_pool(), rs->process_thread_id,
           OPT_TP_PROCESS_EVENTS_WHILE_WAITING);

	if(rs->destroying) {
		return 0;
    }

	// Did the region scan complete successfully?
	CmtGetThreadPoolFunctionAttribute(gci_thread_pool(), rs->acquisition_thread_id, ATTR_TP_FUNCTION_RETURN_VALUE, &retVal);
	CmtReleaseThreadPoolFunctionID (gci_thread_pool(), rs->acquisition_thread_id); 
	if (retVal!=0) {
		// Acquisition failed - do something ...
		logger_log(UIMODULE_LOGGER(rs), LOGGER_ERROR, "Regionscan acquisition thread returned error.");
	}

	CmtGetThreadPoolFunctionAttribute(gci_thread_pool(), rs->process_thread_id, ATTR_TP_FUNCTION_RETURN_VALUE, &retVal);
	CmtReleaseThreadPoolFunctionID (gci_thread_pool(), rs->process_thread_id); //???
	if (retVal!=0) {
		// Processing failed - do something ...
		logger_log(UIMODULE_LOGGER(rs), LOGGER_ERROR, "Regionscan processing thread returned error.");
	}

    if(rs->action == RS_SAVE || rs->action == RS_SAVE_DISPLAY) {
                
		// For Mosaic Version2 file write the max intensity
		intensity = GCI_ImageWindow_GetMaxPixelValueInDisplayedImage(rs->mosaic_window->window);  	

		dictionary_setdouble(d, "max intensity", intensity);

		seq2_fp = fopen(seq_filepath, "w"); // resave with the max intensity added
		iniparser_save(d, seq2_fp);
           
		//if(rs->seq1_fp != NULL)
		//	fclose(rs->seq1_fp);

		if(seq2_fp != NULL)
			fclose(seq2_fp);
                
        dictionary_del(d); 

		// save the mosiac file for reference
		sprintf(seq_filepath, "%s%s_Mosaic.ics", output_dir, filename_prefix); 
		GCI_ImagingWindow_SaveImage(rs->mosaic_window->window, seq_filepath);
    }
            
    rs->stop_scan = 1;   
    ProcessSystemEvents();

	microscope_start_all_timers(rs->ms);  
            
    SetCtrlVal(rs->panel_id, ROI_SCAN_TEXTMSG, "Time taken");
    seconds_to_friendly_time(Timer() - rs->start_time, time_taken);
    SetCtrlVal(rs->panel_id, ROI_SCAN_TIME_TAKEN,  time_taken);   
            
    callback_id = GCI_ImagingWindow_SetMouseDownHandler (rs->mosaic_window->window, OnMosaicClicked , rs);    
            
    RS_FINISHED:

	logger_log(UIMODULE_LOGGER(rs), LOGGER_INFORMATIONAL, "Regionscan Finished");

    microscope_allow_automatic_background_correction(rs->ms);
	focus_set_on(rs->ms->_focus);
	stage_use_cached_data_for_read(rs->ms->_stage, 0);

	// TODO 
	// This could be a problem for sysytems with autofocus hardware
	// as the cached data won't be updated ?
	zdrive_use_cached_data_for_read(MICROSCOPE_MASTER_ZDRIVE(rs->ms), 0);

	microscope_disable_metadata_on_snap(rs->ms, 0);

    CmtGetTSQAttribute(rs->process_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_queue);

    CmtDiscardTSQ (rs->process_queue); 
    rs->process_queue = 0;
            
    stage_set_joystick_on(stage);   
    focus_set_off(rs->ms->_focus);  

    ui_module_enable_panel(rs->panel_id, 0);
    SetCtrlAttribute (rs->panel_id, ROI_SCAN_PAUSE, ATTR_DIMMED, 1);
    SetCtrlAttribute (rs->panel_id, ROI_SCAN_STOP, ATTR_DIMMED, 1);

	// if cellfinding export points to file
	if (rs->cell_finding_enabled)
	{
		char path[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];

		microscope_get_data_directory(rs->ms, path);         
		sprintf(filepath, "%s\\temporary_cellfinding_points.pts", path);
		
		cell_finder_save_timelapse_file(rs->ms->_cf, filepath);
		
		#ifdef MICROSCOPE_PYTHON_AUTOMATION

		timelapse_display(rs->ms->_tl);
		
		timelapse_load_data_from_file(rs->ms->_tl, filepath); 
       
		#endif // MICROSCOPE_PYTHON_AUTOMATION

	}

    microscope_disable_all_panels(rs->ms, 0); 

	get_time_string(rs->end_date_time);

	return 0;
}

int CVICALLBACK cbRegionScanStart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_scan *rs = (region_scan*) callbackData;
			RS_ACTION action;
			char dir[GCI_MAX_PATHNAME_LEN] = "", output_dir[GCI_MAX_PATHNAME_LEN] = "", output_filename[GCI_MAX_PATHNAME_LEN] = "", extension[500] = "";
			char title[100] = "";

			gci_camera_set_snap_mode(rs->camera);
			gci_camera_snap_image(rs->camera);

			region_of_interest_clear_button_state(rs->roi);
			region_scan_save_default_data(rs);

			GetCtrlVal (rs->panel_id, ROI_SCAN_ACTION, &action);  

			if(action == RS_SAVE || action == RS_SAVE_DISPLAY) { // We are Saving images
				
				microscope_get_user_data_subdirectory(rs->ms, NULL, dir);
					
				if(SimpleFilePrefixSaveDialog(UIMODULE_MAIN_PANEL_ID(rs), dir, output_dir, output_filename, extension) < 0 )
					return -1;  

				sprintf(title, "Mosaic (%s)", output_filename);
				mosaic_window_set_title(rs->mosaic_window, title);			
			}
			else {
				mosaic_window_set_title(rs->mosaic_window, "Mosaic");	
			}

			regionscan_start(rs, action, output_dir, output_filename, extension);

			break;
		}	
	}
		
	return 0;
}


int CVICALLBACK cbRegionScanSetROI (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_scan *rs = (region_scan*) callbackData;  

			ui_module_clear_attached_panels(UIMODULE_CAST(rs));

			ui_module_attach_panel_to_panel(UIMODULE_CAST(rs), rs->roi->region_panel,
														  UI_MODULE_REL_TOP_RIGHT,
														  5, 0);

			region_of_interest_display(rs->roi);
			//region_of_interest_panel_display(rs->roi, UIMODULE_MAIN_PANEL_ID(rs));
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK cbRegionScanOverlap (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_scan *rs = (region_scan*) callbackData;   
			calculate_roi_frames (rs);

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK cbRegionScanClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_scan *rs = (region_scan*) callbackData;   

			regionscan_stop(rs);

			region_scan_hide(rs);   
				
			break;
		}
	}
	
	return 0;
}

int region_scan_destroy(region_scan* rs)
{
//	printf("Destroying rs: 0x%x %d\n", rs, rs->_master_camera_signal_id); 

	// Set a destroying flag and wait for the acquisition thread to complete.
	rs->destroying = 1;
	regionscan_stop(rs);
	
	if(rs->acquisition_thread_id != 0) {
		CmtWaitForThreadPoolFunctionCompletion (gci_thread_pool(), rs->acquisition_thread_id,
				OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	}

	if(rs->process_thread_id != 0) {
		CmtWaitForThreadPoolFunctionCompletion (gci_thread_pool(), rs->process_thread_id,
				OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	}

	if (rs->process_queue != 0) {
		CmtDiscardTSQ (rs->process_queue); 
		rs->process_queue = 0;
	}

	if(rs->mosaic_window != NULL) {
		mosaic_window_destroy(rs->mosaic_window);  
		rs->mosaic_window = NULL;
	}
	
	if (rs->_master_camera_changed_signal_id >= 0)
		microscope_master_camera_changed_handler_disconnect(rs->ms, rs->_master_camera_changed_signal_id);
//	printf("rs: 0x%x %d DISCONNECTED from master camera change signal.\n", rs, rs->_master_camera_signal_id);
	
	region_of_interest_destroy(rs->roi);

	ui_module_destroy(UIMODULE_CAST(rs));
	
	free(rs);

	return 0;	
}


void region_scan_display(region_scan* rs)
{
	double overlap;
	
	ui_module_display_main_panel(UIMODULE_CAST(rs));
	
	GetCtrlVal(rs->panel_id, ROI_SCAN_OVERLAP, &overlap);
	
	cell_finder_set_frame_overlap(rs->ms->_cf, overlap);  
}

void region_scan_hide(region_scan* rs)
{
	ui_module_hide_main_panel(UIMODULE_CAST(rs));
	region_of_interest_panel_hide(rs->roi);
	mosaic_window_hide(rs->mosaic_window);
}

static void roi_selected (double left, double top, double width, double height, void *data)
{
	region_scan *rs = (region_scan*) data;  
	
	rs->roi_set = 1;

	calculate_roi_frames (rs);
}

static void OnMasterCameraChanged (Microscope* microscope, void *data)
{
	region_scan *rs = (region_scan*) data;  

//	printf("OnMasterCameraChanged called for rs: 0x%x %d\n", rs, rs->_master_camera_signal_id);

    rs->camera = MICROSCOPE_MASTER_CAMERA(rs->ms);

//	printf("Completed rs: 0x%x %d\n", rs, rs->_master_camera_signal_id);

}

static void region_scan_initialise(region_scan* rs)
{
	 if(rs->panel_id == -1) {
		
		int window_handle;

		rs->panel_id = ui_module_add_panel(UIMODULE_CAST(rs), "RegionScan_ui.uir", ROI_SCAN, 1);
		
		InstallCtrlCallback(rs->panel_id, ROI_SCAN_STOP, cbRegionScanStop, rs);
		InstallCtrlCallback(rs->panel_id, ROI_SCAN_PAUSE, cbRegionScanPause, rs); 
		InstallCtrlCallback(rs->panel_id, ROI_SCAN_CLOSE, cbRegionScanClose, rs);  
		InstallCtrlCallback(rs->panel_id, ROI_SCAN_START, cbRegionScanStart, rs);      
		InstallCtrlCallback(rs->panel_id, ROI_SCAN_SET_ROI, cbRegionScanSetROI, rs);
		InstallCtrlCallback(rs->panel_id, ROI_SCAN_CF_ENABLE, cbRegionScanEnableCellFinding, rs);  
		InstallCtrlCallback(rs->panel_id, ROI_SCAN_CF_CONF, cbRegionScanCellFindingConfigure, rs);  
		InstallCtrlCallback(rs->panel_id, ROI_SCAN_OVERLAP, cbRegionScanOverlap, rs);  
		
		GetPanelAttribute (rs->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &(window_handle)); 
		rs->window_hwnd = (HWND) window_handle; 

		rs->uimodule_func_ptr = ui_module_set_window_proc(UIMODULE_CAST(rs), rs->panel_id, (LONG_PTR) RSWndProc);	
	
		region_scan_load_default_data(rs);

		FilePrefixSave_EraseLastUsedEntries(UIMODULE_MAIN_PANEL_ID(rs));	

		rs->_master_camera_changed_signal_id = microscope_master_camera_changed_handler_connect(rs->ms, OnMasterCameraChanged, rs);
//		printf("rs: 0x%x %d conneted to master camera change signal.\n", rs, rs->_master_camera_signal_id);
	 }
}

region_scan* region_scan_new(void)
{
	optical_calibration* cal;
	
	region_scan* rs = (region_scan*) malloc(sizeof(region_scan));

	memset(rs, 0, sizeof(region_scan));
	
	rs->_master_camera_changed_signal_id = -1;
	rs->has_run = 0;
	rs->mosaic_window = NULL;
	rs->cell_finding_enabled = 0;
	rs->destroying = 0;
	rs->panel_id = -1;
	rs->roi_set = 0;
	rs->stop_scan = 1;
	rs->pause_scan = 0;
	rs->output_dir[0] = 0;
	rs->output_filename[0] = 0;
	rs->stage_dwell = 0.0;
	rs->acquisition_thread_id = 0;
	rs->process_queue = 0;
	rs->frames_done = 0;
	rs->perform_swautofocus_every_point = 0;

	ui_module_constructor(UIMODULE_CAST(rs), "Region Scan");
  		
	rs->ms = microscope_get_microscope();
	
	rs->camera = microscope_get_camera(rs->ms);

	// Each Rs object has its own roi
	rs->roi = region_of_interest_selection_new(rs->camera);
	region_of_interest_panel_init(rs->roi);    
	rs->region_selected_handler_id = region_of_interest_selected_handler(rs->roi, roi_selected, rs);

	cal = microscope_get_optical_calibration(rs->ms); 

	if(rs->camera != NULL)
		rs->image_window = rs->camera->_camera_window;
		
	rs->mosaic_window = mosaic_window_new("Regionscan", 100, 100, 500, 500);
	
	region_scan_initialise(rs);

//	printf("Created rs: 0x%x %d\n", rs, rs->_master_camera_signal_id); 

	return rs;	
}

/*
// Manually moves the stage and gets the image.
int region_scan_manual_next_position(region_scan* rs)
{
	double rel_x = 0.0, rel_y = 0.0;
	double z_speed = 0.0, z_accel = 0.0, current_z;
    double next_rel_x_pos, fov_x_minus_overlap, fov_y_minus_overlap;
	int was_on;

	was_on = stage_get_joystick_status(rs->ms->_stage);

	// Some stage IE LStep dont do Z moves when the joystick
	// is enable so here we work around the bug.
	// It can't go in the stage_move_function as turning off the
	// joystick seems to be a very slow operation.
	if(was_on) {
		stage_set_joystick_off(rs->ms->_stage);
	}

	if(z_drive_is_part_of_stage(MICROSCOPE_MASTER_ZDRIVE(rs->ms))) {
		z_drive_get_speed(MICROSCOPE_MASTER_ZDRIVE(rs->ms), &z_speed);
		z_drive_get_accel(MICROSCOPE_MASTER_ZDRIVE(rs->ms), &z_accel);
	}

	// Goto the first position waiting until it has completed.
	if(region_of_interest_goto_stage_xy(rs->roi, rs->roi_left, rs->roi_top) < 0){
		PROFILE_STOP("AcquisitionThread");
		goto ROI_ERROR;	
	}

	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(rs->ms), &current_z);

	fov_x_minus_overlap = rs->fov_x - rs->x_overlap;
	fov_y_minus_overlap = rs->fov_y - rs->y_overlap;

	// Start position
	rel_x -= fov_x_minus_overlap;

	while (1) {

		if (rs->stop_scan)
			goto FINISHED;
			
		if (rs->pause_scan) {
			Delay(1.0);
			continue;	//User has pressed Pause
		}

		CmtGetTSQAttribute(rs->process_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_queue);		

		// For some reason the acquition thread has got to far ahead of the processing
		// thread. So we now delay a little and keep the images created to 50 
		// This is unlikely to happen in a real world setup. Only when using dummy stuff so
		// the acquisition is really fast.
		while(items_in_queue > 50) 
		{
			CmtGetTSQAttribute(rs->process_queue, ATTR_TSQ_ITEMS_IN_QUEUE, &items_in_queue);	
			Delay(0.01);

			if(rs->stop_scan)
				return 0;
		}

		#ifdef REAL_TIME_PROFILE
		printf("items in process q: %d\n", items_in_queue);
		#endif

		next_rel_x_pos = rel_x + (direction * fov_x_minus_overlap);
		
		if(next_rel_x_pos >= rs->roi_width) {
		    rel_y += fov_y_minus_overlap;
			direction = -1;

			if (rel_y >= rs->roi_height) {
				PROFILE_STOP("AcquisitionThread");
				goto FINISHED;
			}
		}
		else if(next_rel_x_pos < 0) {
		    rel_y += fov_y_minus_overlap;
			direction = 1;

			if (rel_y >= rs->roi_height) {
				PROFILE_STOP("AcquisitionThread");
				goto FINISHED;
			}
		}
		else {
			rel_x = rel_x + (direction * fov_x_minus_overlap);
		}
		
		tile.id = count;
		tile.x = rs->roi_left + rel_x;
		tile.y = rs->roi_top + rel_y;
		tile.z = current_z;

		#ifdef REAL_TIME_PROFILE
		t1 = Timer();
		#endif

		PROFILE_START("Region Scan - region_of_interest_goto_stage_xy");
		
		if(region_of_interest_goto_stage_xy(rs->roi, tile.x, tile.y) < 0){
			PROFILE_STOP("Region Scan - region_of_interest_goto_stage_xy");
			PROFILE_STOP("AcquisitionThread");
			goto ROI_ERROR;	
		}
		
		PROFILE_STOP("Region Scan - region_of_interest_goto_stage_xy");
		
		Delay(rs->stage_dwell);

		#ifdef REAL_TIME_PROFILE
		t2 = Timer()-t1;
		#endif

		PROFILE_START("Region Scan - gci_camera_get_image");
		
		tile.dib = gci_camera_get_image(rs->camera, NULL);    

		if(tile.dib == NULL)
			continue;

		PROFILE_STOP("Region Scan - gci_camera_get_image");
		
		#ifdef REAL_TIME_PROFILE
		t3 = Timer()-t2-t1;
		#endif

		if (rs->stop_scan) {
			PROFILE_STOP("AcquisitionThread");
			goto FINISHED;	//User has pressed Abort
		}

		CmtWriteTSQData(rs->process_queue, &tile, 1, TSQ_INFINITE_TIMEOUT, &number_flushed);

		#ifdef REAL_TIME_PROFILE
		printf("stage %f image %f\n", t2, t3);
		#endif

		ProcessSystemEvents();

		count++;  
	}

FINISHED:

	stage_allow_changed_signal_emission(rs->ms->_stage);
	z_drive_allow_changed_signal_emission(MICROSCOPE_MASTER_ZDRIVE(rs->ms));

	rs->acquisition_done = 1;

	if(was_on)
		stage_set_joystick_on(rs->ms->_stage);

	return 0;

ROI_ERROR:

	stage_allow_changed_signal_emission(rs->ms->_stage);
	z_drive_allow_changed_signal_emission(MICROSCOPE_MASTER_ZDRIVE(rs->ms));

	if(was_on)
		stage_set_joystick_on(rs->ms->_stage);

	return -1;
}
*/

void regionscan_set_perform_swautofocus_every_point(region_scan* rs, int val)
{
	rs->perform_swautofocus_every_point = val;
}
