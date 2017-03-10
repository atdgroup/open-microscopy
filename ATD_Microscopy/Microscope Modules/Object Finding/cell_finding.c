#include "cell_finding.h"
#include "cell_finding_ui.h"
#include "camera\gci_camera.h" 
#include "RegionScan_ui.h"
#include "math.h"
#include "focus.h"
#include "microscope.h"
#include "timelapse.h"

#include "FreeImageAlgorithms.h" 
#include "FreeImageAlgorithms_IO.h" 
#include "FreeImageAlgorithms_Filters.h"
#include "FreeImageAlgorithms_Morphology.h" 
#include "FreeImageAlgorithms_Statistics.h" 
#include "FreeImageAlgorithms_Palettes.h" 
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Particle.h"
#include "FreeImageAlgorithms_Drawing.h"
#include "FreeImageAlgorithms_LinearScale.h"

#include <userint.h>  
#include <utility.h>   

#ifdef USE_CHARM
	#include "CHARM.h"
#else
#ifndef __GCI_POINTF_DEFINED
#define __GCI_POINTF_DEFINED
typedef struct {float x; float y;} Pointf;
#endif

typedef enum {BAD, GOOD, BAD_SHAPE} GlQuality;

typedef struct {
	Point focusOfRadialMap;
	Point centreOfMass;
	Pointf centroid;
	double *radialMap;
	int    spokes;
	double responseValue;	 // the response of the image to the filter used to detect the focus starting pt (i.e. result of CHT).
	double medianRadius;	 // median value of the radial map, findBestCircle will use this value
	double medianDiameter;	 // median value of the radial map*2
	double edgeFoundFactor;
	Rect limit;
	double area;
	double calibrated_area;
	double diameter;		 // nominal, calculated from the area and PI and 2!
	double calibrated_diameter;
	double perimeter;
	double calibrated_perimeter;
	double shapeFactor;      // 4.PI.area / (perimeter)^2
	double intensity;
	double intensityStdDev;
	double totalIntensity;   // = intensity (i.e. mean intensity) * are in pixels
	int    isPartOfCluster;  // Has been flagged as touching another object/shape by ResolveConflicts
	double NNdistance;
	int    NNeighbour;
	GlQuality quality;
	} GlShape;

#endif


static int cell_finder_measure_object_from_shape(cell_finder *cf, FIBITMAP *image, GlShape shape, Box *box, Cell *cell)
{
	double microns_per_pixel;
	int width, height;
	double x_px, y_px;
	StageDirection xdir = stage_get_axis_dir(cf->ms->_stage, XAXIS);
	StageDirection ydir = stage_get_axis_dir(cf->ms->_stage, YAXIS);

	microns_per_pixel = gci_camera_get_true_microns_per_pixel(cf->camera); 

	// Calculate image x,y
	cell->image_x = box->rect.left + (int) floor(shape.centroid.x + 0.5);
	cell->image_y = box->rect.top  + (int) floor(shape.centroid.y + 0.5);
	
	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);
	
	// calc image coords relative to image centre
	x_px = cell->image_x - width / 2;
	y_px = cell->image_y - height / 2;

	// Calculate stage x,y
	cell->x = cf->stage_x - xdir*(x_px * microns_per_pixel);
	cell->y = cf->stage_y - ydir*(y_px * microns_per_pixel);

	cell->area = shape.area * microns_per_pixel * microns_per_pixel;
	cell->diameter = shape.diameter * microns_per_pixel;
	cell->intensity = shape.intensity;
	cell->intensity_stddev = shape.intensityStdDev;
	
	cell->perimeter = shape.perimeter * microns_per_pixel;
	
	cell->shape = shape.shapeFactor;

	return 0;
}


static int cell_finder_shape_valid(cell_finder *cf, GlShape shape, int validate_horz_overlap, int validate_vert_overlap)
{
	int bottom, right;
	
	// Check the bounding box of the shape to see if is in the overlap region
	bottom = shape.limit.top + shape.limit.height;
	right = shape.limit.left + shape.limit.width;
	
	// Check the object is not entirely within the top overlap region
	if (validate_vert_overlap > 0 && bottom < cf->overlap_vert_pixels)
		return -1;
	
	// Check the object is not entirely within the left overlap region
	else if (validate_horz_overlap > 0 && right < cf->overlap_horz_pixels)
		return -1;
	
	return 0;
}

static int charme_find_cells(cell_finder *cf, FIBITMAP *colour_draw_image, Box *box, FIBITMAP *box_image)
{
#ifdef USE_CHARM
	FILE *fp = NULL;
	FIARECT cell_center_rect;
	RGBQUAD cell_colour;
	FIBITMAP *charme_out_image = NULL;
	Cell cell;
	int n=6, i, ret = -1, nShapes = 0;
	GlShape *shapes = NULL;
	double microns_per_pixel = gci_camera_get_true_microns_per_pixel(cf->camera);
	char dir[GCI_MAX_PATHNAME_LEN] = "", buffer[500] = "";

	if(box->valid == 0)
		return 0;

	if(cf->save_data) {
		fp = fopen(cf->data_file_path, "w");
		fprintf(fp, "id, x, y, area, perimeter, shape, intensity, intensity_stddev, type, dose");	
	}
	
	CHARM_run(box_image, &charme_out_image, 0, microns_per_pixel, &shapes, &nShapes);

	if (nShapes < 1)
		goto Error;	// Seriously wrong - this should never happen. We know the box had something in it.

	if (shapes==NULL)
	{
		goto Error;
	}

	//Display all the objects found.
	for (i=0; i < nShapes ; i++) {
		
		// Get measurements for all objects
		// This update the cell structures shape, permeter, area etc fields.
		if (shapes[i].isPartOfCluster == 0) {
		
			cell = gci_cell_new(++cf->number_of_cells, SINGLE, 0.0, 0.0); 
			cf->number_of_singles++;
		}	
		else {
		
			cell = gci_cell_new(++cf->number_of_cells, CLUSTER, 0.0, 0.0); 
			cf->number_of_clusters++;
		}	
		
		// Store the Z pos, same for all in this image
		cell.z = cf->stage_z;

		// This checks the image is within our size bounds and updates the cells position, area, shape etc
		if (cell_finder_measure_object_from_shape(cf, box_image, shapes[i], box, &cell) < 0)
			continue;
	
		cf->cell_diameter_sumation += cell.diameter; 
		
		gci_cellmap_add_cell(cf->map, cell);
	
		cell_center_rect.left = cell.image_x - 2;
		cell_center_rect.top = cell.image_y - 2; 
		cell_center_rect.right = cell.image_x + 2; 
		cell_center_rect.bottom = cell.image_y + 2; 
		
		if(cell.type == SINGLE)
			cell_colour = FIA_RGBQUAD(255, 0, 0);
		else
			cell_colour = FIA_RGBQUAD(0, 0, 255);  
		
		FIA_DrawColourSolidRect (colour_draw_image, cell_center_rect, cell_colour);  
		
		// Don't call from non ui thread
		gci_cellmap_plot_cells(cf->map);
		
		// Many spread sheets don't support more than 32000 rows.
		// we should really save this in 32000 line chunks
		if(cf->save_data) {
		
			fprintf(fp, "%f,%f,%f,%f,%f,%f,%f,%f,%f\n", cell.id, cell.x, cell.y, cell.area,
				cell.perimeter, cell.shape, cell.intensity, cell.intensity_stddev, cell.type);				  
		}
	}

	CHARM_shapesDispose();
	
	if(fp != NULL)
		fclose(fp);
	
	return 0;

Error:

	if(fp != NULL)
		fclose(fp);
	
	if (shapes)
		CHARM_shapesDispose();

	return -1;
#else
	return -1;
#endif
}

// Threshold the image to find bright objects  
static FIBITMAP* cell_finder_frame_preprocess(cell_finder *cf, FIBITMAP* frame_image) 
{
	int maxv, maxi, threshold_min;
	unsigned int threshold_value;   
	double found_min = 0.0, found_max = 0.0;
	DWORD hist[256];
	FIBITMAP *frame_threshold_image = NULL;
	FIABITMAP* bordered_image = NULL;

	memset(hist, 0, 256);
	
	bordered_image = FIA_SetBorder(frame_image, 3, 3, BorderType_Constant, 0.0);

	frame_threshold_image = FIA_MedianFilter(bordered_image, 3, 3);   
	
	FIA_Unload(bordered_image);
	
	FIA_InplaceLinearScaleToStandardType(&frame_threshold_image, 0.0, 65000.0, &found_min, &found_max);

	if(cf->threshold_type == CF_DYNAMIC_THRESHOLD) {

		if(FreeImage_GetHistogram(frame_threshold_image, hist, FICC_BLACK) == FALSE)
			return NULL;

		maxi = FIA_FindIntMax(hist, 256, &maxv);

		threshold_value = (unsigned int) (maxv * cf->threshold_percentage / 100.0);

		for (threshold_min=maxi; threshold_min < 256; threshold_min++) {
	
			if (hist[threshold_min] < threshold_value)
				break;
		}
	}
	else {
		
		double threshold;
		GetCtrlVal(cf->setup_panel_id, SETUP_PNL_THRESHOLD, &threshold);
		threshold_min = (int) threshold;
	}

	FIA_InPlaceThreshold(frame_threshold_image, threshold_min, 255, 255.0);      

	return frame_threshold_image;
}


static void MakeFiaRectPercentageLarger(FIARECT *rect, float percentage)
{
	int width = rect->right - rect->left + 1;
	int height = rect->bottom - rect->top  + 1;
	
	int half_change_in_width = (int) ((width - ((percentage * width) / 100.0)) / 2.0);	
	int half_change_in_height = (int) ((height - ((percentage * height) / 100.0)) / 2.0);
	
	rect->left -= half_change_in_width;
	rect->right += half_change_in_width;
	rect->top -= half_change_in_height;
	rect->bottom += half_change_in_height; 
}

void cell_finder_clear_map(cell_finder *cf)
{
	gci_cellmap_clear(cf->map);
}

FIBITMAP* cell_finder_find_cells(cell_finder *cf, FIBITMAP *frame_image, 
								 double stage_left_offset, double stage_top_offset,double stage_z,
								 int validate_horz_overlap, int validate_vert_overlap)
{
	FIBITMAP *frame_threshold_image = NULL;   
	FIBITMAP* colour_image = NULL, *box_image = NULL, *temp_image = NULL;
	PARTICLEINFO *part;
	FIARECT rect;
	Box *box;
	int i, width, height;
	double microns_per_pixel, width_in_microns, height_in_microns;
	timelapse* tl = microscope_get_timelapse(cf->ms);  
	
	int image_width = FreeImage_GetWidth(frame_image);
	int image_height = FreeImage_GetHeight(frame_image);    
	
	microns_per_pixel = gci_camera_get_true_microns_per_pixel(cf->camera);

	cf->stage_x = stage_left_offset;
	cf->stage_y = stage_top_offset;
	cf->stage_z = stage_z;

	if(cf->pre_process) {
	
		frame_threshold_image = cell_finder_frame_preprocess(cf, frame_image);    	
	}
	else
		frame_threshold_image = FreeImage_Clone(frame_image);    
	
	if(FIA_ParticleInfo(frame_threshold_image, &part, 1) == FIA_ERROR) {
		FreeImage_Unload(frame_threshold_image);
		return NULL;
	}

	if(part->number_of_blobs < 1) {
		FreeImage_Unload(frame_threshold_image);
		return NULL;	
	}
	
	FreeImage_Unload(frame_threshold_image);   
	
	temp_image = FreeImage_ConvertToStandardType(frame_image, 1);
	colour_image = FreeImage_ConvertTo24Bits(temp_image);
	FreeImage_Unload(temp_image);

	if(colour_image == NULL) {
		logger_log(UIMODULE_LOGGER(cf), LOGGER_ERROR,
			"Critical Error - Failed to convert image to 24 bit image"); 
		return NULL;
	}

	if(cf->boxes != NULL) {
	
		free(cf->boxes);
		cf->boxes = NULL;
	}

	gci_cellmap_display(cf->map);
	
	// Each Particle here could be a number of objects, e.g. doublet, singles, grit
	cf->boxes = (Box *) malloc (part->number_of_blobs * sizeof(Box));
	
	for (i=0; i < part->number_of_blobs; i++)  {   
	
		rect = part->blobs[i].rect;
		
		width = rect.right - rect.left + 1;
		height = rect.bottom - rect.top  + 1;
		
		width_in_microns = width * microns_per_pixel;
		height_in_microns = height * microns_per_pixel;

		// Make surrounding boxes 20% larger
		MakeFiaRectPercentageLarger(&rect, 20.0);
		
		box = &(cf->boxes[i]);
		
		box->valid = 1; 
		box->rect = rect;
		
		// Make sure the box is not greater than the max cluster size 
		if ( width_in_microns  < cf->min_cell_diameter || height_in_microns >cf->max_cell_diameter) {
			box->valid = 0;
		}
		else if (rect.left < 0 || rect.right >= image_width || rect.top < 0 || rect.bottom >= image_height) {
			// Check the box int within the image       
			box->valid = 0;
		}
		// Check the box is within the vertical overlap 
		else if (validate_vert_overlap > 0 && rect.bottom < cf->overlap_vert_pixels)
			box->valid = 0;
		// Check the box is within the horizontal overlap 
		else if (validate_horz_overlap > 0 && rect.right < cf->overlap_horz_pixels)
			box->valid = 0;
		
		if(box->valid == 0)
			FIA_DrawColourRect (colour_image, rect, FIA_RGBQUAD(0, 0, 255), 2); 
		else {

			FIA_DrawColourRect (colour_image, rect, FIA_RGBQUAD(255, 0, 0), 2);

			// Extract the box containing this cell from the frame image
			box_image = FreeImage_Copy(frame_image, rect.left, rect.top, rect.right, rect.bottom);

			if(box_image == NULL) {
				goto RET_ERROR;
			}

			if(cf->charm_enabled) {
				charme_find_cells(cf, colour_image, box, box_image); 
			}
			else {
				double x_px, y_px;
				Cell cell = gci_cell_new(++cf->number_of_cells, SINGLE, 0.0, 0.0); 
				StageDirection xdir = stage_get_axis_dir(cf->ms->_stage, XAXIS);
				StageDirection ydir = stage_get_axis_dir(cf->ms->_stage, YAXIS);
				
				cf->number_of_singles++;

				// Calculate image x,y
				cell.image_x = part->blobs[i].center_x;
				cell.image_y = part->blobs[i].center_y;
				
				// calc coords relative to image centre
				x_px = cell.image_x - image_width / 2;
				y_px = cell.image_y - image_height / 2;
				
				// calc stage coords
				cell.x = cf->stage_x + xdir*(x_px * microns_per_pixel);
				cell.y = cf->stage_y + ydir*(y_px * microns_per_pixel);
				cell.z = cf->stage_z;

				gci_cellmap_add_cell(cf->map, cell);
	
				rect.left = part->blobs[i].center_x - 2;
				rect.top = part->blobs[i].center_y - 2; 
				rect.right = part->blobs[i].center_x + 2; 
				rect.bottom =  part->blobs[i].center_y + 2; 
		
				FIA_DrawColourSolidRect (colour_image, rect, FIA_RGBQUAD(255, 0, 0));  
				
				gci_cellmap_plot_cells(cf->map);

			}

			FreeImage_Unload(box_image);   
		}
	}
		
	FIA_FreeParticleInfo(part);
	part = NULL;
	
	return colour_image;

RET_ERROR:

	FIA_FreeParticleInfo(part);
	part = NULL;

	return NULL;
}


void cell_finder_set_frame_overlap(cell_finder *cf, double percentage)
{
	int image_width, image_height;
	double microns_per_pixel;
	int binning = gci_camera_get_binning_mode(cf->camera);
	
	gci_camera_get_size(cf->camera, &image_width, &image_height);  

	microns_per_pixel = gci_camera_get_microns_per_pixel(cf->camera); 

	cf->overlap_percentage = percentage;

	//RJL 241005 - Image stitching assumes that x,y overlaps have same number of pixels
	cf->overlap_vert_pixels = (int) ((cf->overlap_percentage / 100.0) * image_width);
	cf->overlap_horz_pixels = (int) ((cf->overlap_percentage / 100.0) * image_width);
	
	cf->overlap_vert_microns = cf->overlap_vert_pixels * microns_per_pixel * binning;
	cf->overlap_horz_microns = cf->overlap_horz_pixels * microns_per_pixel * binning; 
}

cell_finder* cell_finder_new(void) 
{
	cell_finder *cf = (cell_finder *) malloc (sizeof(cell_finder));

	memset(cf, 0, sizeof(cell_finder));
	
	ui_module_constructor(UIMODULE_CAST(cf), "Cell Finder");
	
	cf->ms = microscope_get_microscope();
	cf->camera = microscope_get_camera(cf->ms);
	
	cf->setup_panel_id = ui_module_add_panel(UIMODULE_CAST(cf), "cell_finding_ui.uir", SETUP_PNL, 1);
	cf->criteria_panel_id = ui_module_add_panel(UIMODULE_CAST(cf), "cell_finding_ui.uir", CRITERIA, 0);
	
	cf->pre_process = 1;
	cf->save_data = 0;
	cf->save_data_to_timelapse = 0;
	cf->show_processing = 1;
	cf->charm_enabled = 1;
	cf->threshold_percentage = 2.3;
	
	cf->map = gci_cellmap_new("Cellmap", 50, 50, 400, 400); 

	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_MIN_CELL_DIAMETER, OnMinCellDiameterChanged, cf);     
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_MAX_CELL_DIAMETER, OnMaxCellDiameterChanged, cf);     
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_THRESHOLD, OnCellFindingThreshold, cf);   
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_ADVANCED, OnCellFindingAdvancedPressed, cf);
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_TEST, OnTestPressed, cf);  
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_CLOSE, OnClosePressed, cf);
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_PRE_PROCESS, OnPreProcessClicked, cf);
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_SAVE_DATA, OnSaveDataChecked, cf);
	
	#ifdef DEVELOPMENTAL_FEATURES
	SetCtrlAttribute(cf->setup_panel_id, SETUP_PNL_TEST, ATTR_VISIBLE, 1);
	#endif

	GetCtrlVal(cf->setup_panel_id, SETUP_PNL_THRESHOLD, &(cf->threshold_percentage)); 
	GetCtrlVal(cf->setup_panel_id, SETUP_PNL_MIN_CELL_DIAMETER, &(cf->min_cell_diameter)); 
	GetCtrlVal(cf->setup_panel_id, SETUP_PNL_MAX_CELL_DIAMETER, &(cf->max_cell_diameter)); 

	GetCtrlVal(cf->setup_panel_id, SETUP_PNL_SHOW_PROCESSING, &(cf->show_processing)); 
	GetCtrlVal(cf->setup_panel_id, SETUP_PNL_ENABLE_CHARM, &(cf->charm_enabled)); 
	
	cf->threshold_type = CF_SIMPLE_THRESHOLD;
	SetCtrlAttribute(cf->setup_panel_id, SETUP_PNL_THRESHOLD, ATTR_MIN_VALUE, 0.0);
	SetCtrlAttribute(cf->setup_panel_id, SETUP_PNL_THRESHOLD, ATTR_MAX_VALUE, 255.0);
	SetCtrlVal(cf->setup_panel_id, SETUP_PNL_THRESHOLD, 100.0);

	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_SHOW_PROCESSING, OnShowProcessingChecked, cf);
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_ENABLE_CHARM, OnEnableCharmChecked, cf);
	InstallCtrlCallback(cf->setup_panel_id, SETUP_PNL_THRESHOLD_TYPE, OnSimpleThresholdTypeChecked, cf);
	
	InstallCtrlCallback(cf->criteria_panel_id, CRITERIA_CHARME, OnCharmeParametersClicked, cf);
	InstallCtrlCallback(cf->criteria_panel_id, CRITERIA_CHARME_OPT, OnCharmeOptimizerClicked, cf);
	InstallCtrlCallback(cf->criteria_panel_id, CRITERIA_CLOSE, OnCriteriaCloseClicked, cf);

#ifdef USE_CHARM
	CHARM_setup (REGKEY_HKCU, "software\\GCI\\Microscope\\", NULL);
#endif

	return cf;
}


void cell_finder_destroy(cell_finder* cf) 
{
	ui_module_destroy(UIMODULE_CAST(cf));

	gci_cellmap_destroy(cf->map); 

	free(cf);
	cf = NULL;
}


void cell_finder_display(cell_finder* cf) 
{
	ui_module_display_main_panel(UIMODULE_CAST(cf));
}

int cell_finder_save_timelapse_file(cell_finder* cf, char *filepath) 
{
	Cell *next_cell;
	CellLinkedList list=cf->map->cell_list;
	int size = cell_list_count(list), i=0;
	FILE *fp; 

	fp = fopen(filepath, "w");

	if (fp==NULL) return -1;
	
	fprintf(fp, "%d\n", size);   

	while( (next_cell = cell_list_get_next(list)) != NULL) {
	
        fprintf(fp, "%f\t%f\t%f\n", next_cell->x, next_cell->y, next_cell->z); 
		 
		i++;
	}

    
    fclose(fp);

	return 0;
}

/*
static int cell_finder_measure_object(cell_finder *cf, IPIImageRef box_image, IPIImageRef mask_image, GlShape shape, Box *box, Cell *cell)
{
	int n, binning, params[1] = {IPI_PP_Perimeter};
	float x, y, Coeffs[1];
	double surface, microns_per_pixel, perim;
	double x_px, y_px, abs_x, abs_y;
	IPIFullPReportPtr PReport = NULL;
	IPIQuantifyElemPtr regionReport=0;
	IPIQuantifyElem globalReport;
	IPIImageInfo info;

	microns_per_pixel = gci_camera_get_microns_per_pixel(cf->camera); 
	binning = gci_camera_get_binning_mode(cf->camera);
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera

	IPI_Centroid (box_image, mask_image, &x, &y );

	cell->image_x = box->rect.left + (int) floor(x + 0.5);
	cell->image_y = box->rect.top  + (int) floor(y + 0.5);
	
	IPI_GetImageInfo (cf->frame_image, &info);
	x_px = cell->image_x - info.width/2;
	y_px = cell->image_y - info.height/2;

	//Rotate x,y coords to correct for the camera misalignment
	GCI_ImagingRotateCoords(x_px, y_px, &abs_x, &abs_y);
	
	cell->x = cf->frame_stage_x + (abs_x * microns_per_pixel * binning);
	cell->y = cf->frame_stage_y + (abs_y * microns_per_pixel * binning);
	//cell->x = cf->frame_stage_x + ((cell->image_x - info.width/2) * microns_per_pixel * binning);
	//cell->y = cf->frame_stage_y + ((cell->image_y - info.height/2) * microns_per_pixel * binning);
	
	IPI_Quantify (box_image, mask_image, &globalReport, &regionReport, &n);
	
	if (n != 1)
		return -1;	//should never happen

	surface = regionReport[0].area;
	cell->area = surface * microns_per_pixel * microns_per_pixel * binning * binning;
	cell->diameter = 2 * sqrt(cell->area / PI);
	cell->intensity = regionReport[0].mean;
	cell->intensity_stddev = regionReport[0].stdDeviation;

	IPI_Particle (mask_image, TRUE, &PReport, &n);

	if (n != 1)
		return -1;	//should never happen
	
	IPI_ParticleCoeffs (mask_image, params, 1, PReport, n, Coeffs);
	perim = Coeffs[0];
	cell->perimeter = perim * microns_per_pixel * binning;

	// shape factor 1 = circle
	//cell->shape = (1.0 - (12.566 * surface)) / (perim*perim);
	cell->shape = (4.0*PI * surface) / (perim*perim);

	IPI_free(PReport);
	IPI_free(regionReport);
	
	return 0;
}
*/

int CVICALLBACK OnCharmeParametersClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
#ifdef USE_CHARM
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;

			CHARM_displayParameterPanel();

			break;
		}
	}
#endif
	return 0;
}


int CVICALLBACK OnCriteriaCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;  
		
			ui_module_hide_panel(UIMODULE_CAST(cf), cf->criteria_panel_id);

			break;
		}
	}
	return 0;
}

int CVICALLBACK OnCharmeOptimizerClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
#ifdef USE_CHARM
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;
			FIBITMAP *displayed_dib = NULL, *dib = NULL;
			double microns_per_pixel;

			// Must snap a new image as if we have run a scan the displayed image maybe a processed 24bit image.
			gci_camera_snap_image(cf->camera);

			displayed_dib = gci_camera_get_displayed_image(cf->camera);
			
			dib = FreeImage_Clone(displayed_dib);

			microns_per_pixel = gci_camera_get_true_microns_per_pixel(cf->camera);

			if(microns_per_pixel != 0.0)
				CHARM_setUnitText("microns");
			
			// TODO needs removal sometime
			//FIA_EnableOldBrokenCodeCompatibility();

			CHARM_displayParameterOptimizerPanel (dib, NULL, microns_per_pixel);

			FreeImage_Unload(dib);

			break;
		}
	}
#endif
	return 0;
}

int CVICALLBACK OnCellFindingAdvancedPressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;

			ui_module_display_panel(UIMODULE_CAST(cf), cf->criteria_panel_id);
			
			break;
		}
	}
	return 0;
}

int CVICALLBACK OnCellFindingThreshold (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			FIBITMAP* fib = NULL, *threshold_fib = NULL;
			
			cell_finder* cf = (cell_finder*) callbackData;
			
			GetCtrlVal(panel, control, &(cf->threshold_percentage)); 
			
			fib = gci_camera_get_image(cf->camera, NULL);
				
			threshold_fib = cell_finder_frame_preprocess(cf, fib);      
			
			FreeImage_Unload(fib);
			
			gci_camera_display_image(cf->camera, threshold_fib, NULL);   
			
			FreeImage_Unload(threshold_fib); 
			
			break;
		}
	}
	return 0;
}

int CVICALLBACK OnMaxCellDiameterChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;  
			
			GetCtrlVal(panel, control, &(cf->max_cell_diameter));

			break;
		}
	}
	return 0;
}

int CVICALLBACK OnMinCellDiameterChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;  
			
			GetCtrlVal(panel, control, &(cf->min_cell_diameter));
		}
	}
	return 0;
}


int CVICALLBACK OnTestPressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			#ifdef DEVELOPMENTAL_FEATURES    
			
			cell_finder* cf = (cell_finder*) callbackData;  
			char path[GCI_MAX_PATHNAME_LEN] = "";
			FIBITMAP *fib = NULL, *out_fib = NULL;
			
			gci_camera_snap_image(cf->camera);
				
			fib = gci_camera_get_displayed_image(cf->camera);
			
			gci_cellmap_set_region_size_pos(cf->map, 0.0, 0.0, FreeImage_GetWidth(fib), FreeImage_GetHeight(fib));

			out_fib = cell_finder_find_cells(cf, fib, 0, 0, 0, 0, 0);

			gci_camera_display_image(cf->camera, out_fib, NULL);   
			
			FreeImage_Unload(fib);   
			FreeImage_Unload(out_fib);       
			
			#endif
		}
	}
	return 0;
}


int CVICALLBACK OnPreProcessClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;  
		
			GetCtrlVal(panel, control, &(cf->pre_process));

			if(cf->pre_process == 0) {
		
				SetCtrlAttribute(panel, SETUP_PNL_MAX_CELL_DIAMETER, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, SETUP_PNL_MIN_CELL_DIAMETER, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, SETUP_PNL_THRESHOLD, ATTR_DIMMED, 1);
			}
			else {

				SetCtrlAttribute(panel, SETUP_PNL_MAX_CELL_DIAMETER, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, SETUP_PNL_MIN_CELL_DIAMETER, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, SETUP_PNL_THRESHOLD, ATTR_DIMMED, 0);
			}
	
			break;
		}
	}
	return 0;
}


int CVICALLBACK OnClosePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;  
		
			ui_module_hide_all_panels(UIMODULE_CAST(cf));
			
			break;
		}
	}
	return 0;
}

int CVICALLBACK OnSaveDataChecked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			char dir[GCI_MAX_PATHNAME_LEN] = "";
			
			cell_finder* cf = (cell_finder*) callbackData;  
		
			// disable saving of data to a file for now
			cf->save_data = 0;

			GetCtrlVal(panel, control, &(cf->save_data_to_timelapse));  
			
//			microscope_get_user_data_directory(cf->ms, dir);  
			
//			if (LessLameFileSelectPopup (cf->setup_panel_id, dir, "cell_data.csv", "*.csv",
//							 "Save Cell Data", VAL_SAVE_BUTTON, 0, 1, 1, 1, cf->data_file_path) < 0)
//			{
//				return -1;	
//			}

			if (cf->save_data_to_timelapse)
			{
				if (!GCI_ConfirmPopup("Warning", IDI_WARNING, "Ok to overwrite time lapse points list?"))
					cf->save_data = 0;
					SetCtrlVal(panel, control, cf->save_data_to_timelapse);  
			}


			break;
		}
	}
	return 0;
}

int CVICALLBACK OnShowProcessingChecked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;  
		
			GetCtrlVal(panel, control, &(cf->show_processing));  
			
			break;
		}
	}
	return 0;
}


int CVICALLBACK OnEnableCharmChecked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			cell_finder* cf = (cell_finder*) callbackData;  
		
			GetCtrlVal(panel, control, &(cf->charm_enabled));  
			
			break;
		}
	}
	return 0;
}


int CVICALLBACK OnSimpleThresholdTypeChecked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;

			cell_finder* cf = (cell_finder*) callbackData;  
		
			GetCtrlVal(panel, control, &val);  
			
			cf->threshold_type = (CF_THRESHOLD) val;

			if(cf->threshold_type == CF_SIMPLE_THRESHOLD) {

				SetCtrlAttribute(panel, SETUP_PNL_THRESHOLD, ATTR_MIN_VALUE, 0.0);
				SetCtrlAttribute(panel, SETUP_PNL_THRESHOLD, ATTR_MAX_VALUE, 255.0);
				SetCtrlVal(panel, SETUP_PNL_THRESHOLD, 100.0);

			}
			else {

				SetCtrlAttribute(panel, SETUP_PNL_THRESHOLD, ATTR_MIN_VALUE, 0.01);
				SetCtrlAttribute(panel, SETUP_PNL_THRESHOLD, ATTR_MAX_VALUE, 100.0);
				SetCtrlVal(panel, SETUP_PNL_THRESHOLD, 0.5);
			}

			break;
		}
	}
	return 0;
}