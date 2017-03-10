#include "ExcelAutomation.h"
#include <userint.h>
#include "filters.h"
#include "cell_finding.h"
#include "cell_finding_ui.h"
#include "math.h"
#include "focus.h"
#include <utility.h>
#include "CHARM.h"
#include "ipi_object_drawing.h"
#include "imaging.h"
#include "SaveToExcel.h"

////////////////////////////////////////////////////////////////////
// Module to process boxes conatining cells/objects for the microfocus system
// Glenn Pierce - July 2005
////////////////////////////////////////////////////////////////////
// PRB - October 2005
// Mods associated with new CHARME module
////////////////////////////////////////////////////////////////////
// RJL - October 2005
// Correct cell measurements for camera binning and draw cell borders
// with double width lines.
////////////////////////////////////////////////////////////////////
// RJL - 18 Nov 2005
// Correct cell coordinates for camera misalignment
////////////////////////////////////////////////////////////////////
// RJL - 6 March 2006
// Display radius values rather than cell diameters
////////////////////////////////////////////////////////////////////
// RJL - May/June 2006
// Make pre-processing of objects an option as it screws up for near
// confluent cells.
// Also reject images if 25% rather than 75% is bright for the same reason.
////////////////////////////////////////////////////////////////////
// Ros Locke - June 2006
// Speed up writing the cell data to Excel. It was done one line at a time. 
// It's now done frame by frame in cell_boxing.c
////////////////////////////////////////////////////////////////////

static int NormaliseImage(IPIImageRef imin, IPIImageRef imout)
{
	IPIQuantifyElem report;
	int n;
	
	//NB imin and imout must be of the same type e.g. 16 bit
	IPI_Quantify (imin, IPI_NOMASK, &report, NULL, &n);
	if (report.maxValue == 0.0) report.maxValue = 4080.0;   //avoid crashing
	return IPI_Divide (imin, IPI_USECONSTANT, imout, report.maxValue/255.0);
	//IPI_Quantify (imout, IPI_NOMASK, &report, NULL, &n);
}

static IPIImageRef cell_finder_frame_threshold(cell_finder *cf, IPIImageRef frame_image) 
{
	int hist[256], thr, maxv, maxi, threshold_min, threshold_value;
	IPIImageInfo info;
	IPIImageRef temp=0;
	IPIImageRef frame_threshold_image = 0;
	
	//Threshold the image to find bright objects
	IPI_Create (&frame_threshold_image, IPI_PIXEL_U8, 2);      

	//8 bit histogram
	IPI_GetImageInfo (frame_image, &info);
	IPI_Create (&temp, info.pixelType, 2);

	if (info.pixelType != IPI_PIXEL_U8) {
		NormaliseImage(frame_image, temp);
		IPI_Cast (temp, IPI_PIXEL_U8);
	}
	else 
		IPI_Copy (frame_image, temp);

	IPI_Histogram (temp, IPI_NOMASK, 256, 0, 0, hist, NULL);

	maxi = Find_Int_Max (hist, 256, &maxv);  

	threshold_value = maxv * cf->threshold_percentage / 100.0;

	for (threshold_min=maxi; threshold_min < 256; threshold_min++) {
	
		if (hist[threshold_min] < threshold_value)
			break;
	}

	IPI_Threshold (temp, frame_threshold_image, threshold_min, 255, 1.0, 1);
	
	IPI_Dispose(temp);
	
	return frame_threshold_image;
}

void cell_finder_test_threshold(cell_finder *cf)
{
	IPIImageRef frame_threshold_image = 0; 

	frame_threshold_image = cell_finder_frame_threshold(cf, cf->frame_image);
	
	IPI_Resample (frame_threshold_image, frame_threshold_image, 336, 256, 0, IPI_FULL_RECT);
	IPI_WindDraw (frame_threshold_image, 0, "", TRUE);
	IPI_WSetPalette (0, IPI_PLT_BINARY, NULL);
	IPI_Dispose(frame_threshold_image);
}

void cell_finder_define_boxes(cell_finder *cf, int validate_horz_overlap, int validate_vert_overlap)
{
	int threshold, n, not_valid=0, width, height, left, top, right, bottom, number_of_cells, i;
	int params[4] = {IPI_PP_RectLeft,IPI_PP_RectWidth,IPI_PP_RectTop,IPI_PP_RectHeight};
	float *Coeffs=NULL, w, h;
	char msg[60]="";
	Cell cell;
	
	IPIQuantifyElem report;
	IPIFullPReportPtr PReport = NULL;
	IPIImageRef binim=0;
	IPIImageRef frame_threshold_image = 0; 
	IPIImageInfo info;
	
	IPI_GetImageInfo (cf->frame_image, &info);       
	
	frame_threshold_image = cell_finder_frame_threshold(cf, cf->frame_image);
	//IPI_WindDraw (frame_threshold_image, 0, "", TRUE);
	//IPI_WSetPalette (0, IPI_PLT_BINARY, NULL);

	
	//Check out the area - a big number indicates gunk on the slide
	IPI_Quantify (frame_threshold_image, IPI_NOMASK, &report, NULL, &n);
	
	if (report.mean > 0.75) {  //more than a quarter of pixels are red
	
		IPI_Dispose(frame_threshold_image);
		return;
	}
	
	//Filter out small blobs
	IPI_LowHighPass (frame_threshold_image, frame_threshold_image, TRUE, FALSE, 2, IPI_MO_STD3X3);
	
	IPI_Particle (frame_threshold_image, TRUE, &PReport, &(cf->number_of_boxes));
	
	if (cf->number_of_boxes < 1) {
		IPI_free (PReport);
		IPI_Dispose(frame_threshold_image);
		return;
	}
	
	if(cf->boxes != NULL) {
	
		free(cf->boxes);
		cf->boxes = NULL;
	}

	// Each Particle here could be a number of objects, e.g. doublet
	cf->boxes = (Box *) malloc (cf->number_of_boxes * sizeof(Box));

	Coeffs = (float *) malloc(cf->number_of_boxes * 4 * sizeof(float));
	
	IPI_ParticleCoeffs (frame_threshold_image, params, 4, PReport, cf->number_of_boxes, Coeffs);
	
	for (i=0; i < cf->number_of_boxes; i++)  {
	
		//Disregard blobs smaller than the cell diameter
		//Define box x% bigger than the blob (value from criteria panel)
		w = Coeffs[i*4+1];
		h = Coeffs[i*4+3];
		
		// Make the surrounding box larger by the specified percentage
		if ( (w > cf->min_cell_width) && (h > cf->min_cell_height) ) {
		
			cf->boxes[i].valid = 1;
		
			cf->boxes[i].rect.left 		= Coeffs[i*4] 	- w * ( cf->box_width_percentage_larger  / 200 );
			cf->boxes[i].rect.width 	= Coeffs[i*4+1] + w * ( cf->box_width_percentage_larger  / 100 );
///			cf->boxes[i].rect.top 		= Coeffs[i*4+2] - h * ( cf->box_height_percentage_larger / 200 );
///			cf->boxes[i].rect.height 	= Coeffs[i*4+3] + h * ( cf->box_height_percentage_larger / 100 );
			cf->boxes[i].rect.top 		= Coeffs[i*4+2] - h * ( cf->box_width_percentage_larger / 200 );
			cf->boxes[i].rect.height 	= Coeffs[i*4+3] + h * ( cf->box_width_percentage_larger / 100 );
			
			left 	= cf->boxes[i].rect.left;
			top 	= cf->boxes[i].rect.top;
			width 	= cf->boxes[i].rect.width;
			height 	= cf->boxes[i].rect.height;
		
			right = left + width;
			bottom = top + height;
			
			// Make sure the box is not greater than the max cluster size 
///			if ( (width > cf->max_cell_width) || (height > cf->max_cell_height ) )
			if ( (width  > cf->max_cell_width * cf->max_cell_width_percentage_of_diameter) || 
				 (height > cf->max_cell_height * cf->max_cell_height_percentage_of_diameter ) )
				cf->boxes[i].valid = 0;
		
			// Check the box int within the image
			else if ((left < 0) || ((left+width) >= info.width) || (top < 0) || ((top+height) >= info.height))
				cf->boxes[i].valid = 0;

			// Check the box is within the vertical overlap 
			else if (validate_vert_overlap > 0 && bottom < cf->overlap_vert_pixels)
				cf->boxes[i].valid = 0;
	
			// Check the box is within the horizontal overlap 
			else if (validate_horz_overlap > 0 && right < cf->overlap_horz_pixels)
				cf->boxes[i].valid = 0;
				
			if (cf->show_processing) {
				//Need linewidth of 3 to be sure of seeing all the lines on a reduced image
				int penwidth = 4, p;
				Rect tempRect;
				if(cf->boxes[i].valid == 0) {
					//ipi_draw_colour_rect (cf->frame_display_image, cf->boxes[i].rect, 0, 0, 255);   // Blue
					for (p=0; p<penwidth; p++) {
						tempRect = MakeRect (cf->boxes[i].rect.top-p, cf->boxes[i].rect.left-p, cf->boxes[i].rect.height+2*p, cf->boxes[i].rect.width+2*p);
						ipi_draw_colour_rect (cf->frame_display_image, tempRect, 0, 0, 128);		  // Blue
					}
				}
				else {
					//ipi_draw_colour_rect (cf->frame_display_image, cf->boxes[i].rect, 255, 0, 0);   // Red
					for (p=0; p<penwidth; p++) {
						tempRect = MakeRect (cf->boxes[i].rect.top-p, cf->boxes[i].rect.left-p, cf->boxes[i].rect.height+2*p, cf->boxes[i].rect.width+2*p);
						ipi_draw_colour_rect (cf->frame_display_image, tempRect, 128, 0, 0);		// Red
					}
				}
			}
		}
		else 
			cf->boxes[i].valid = 0;
	}
	
	IPI_free (PReport);
	free(Coeffs);
	IPI_Dispose(frame_threshold_image);
}

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

static int cell_finder_measure_object_from_shape(cell_finder *cf, GlShape shape, Box *box, Cell *cell)
{
	double microns_per_pixel;
	int binning;
	IPIImageInfo info;
	double x_px, y_px, abs_x, abs_y;

	//RJL 241005 - correct for camera binning
	microns_per_pixel = gci_camera_get_microns_per_pixel(cf->camera); 
	binning = gci_camera_get_binning_mode(cf->camera);
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera

	//Calculate stage x,y
	cell->image_x = box->rect.left + (int) floor(shape.centroid.x + 0.5);
	cell->image_y = box->rect.top  + (int) floor(shape.centroid.y + 0.5);
	
	IPI_GetImageInfo (cf->frame_image, &info);
	x_px = cell->image_x - info.width/2;
	y_px = cell->image_y - info.height/2;

	//Rotate x,y coords to correct for the camera misalignment
	GCI_ImagingRotateCoords(x_px, y_px, &abs_x, &abs_y);
	
	cell->x = cf->frame_stage_x + (abs_x * microns_per_pixel * binning);
	cell->y = cf->frame_stage_y + (abs_y * microns_per_pixel * binning);
	//cell->x = cf->frame_stage_x + ((cell->image_x - info.width/2) * microns_per_pixel * binning);
	//cell->y = cf->frame_stage_y + ((cell->image_y - info.height/2) * microns_per_pixel * binning);

	cell->area = shape.area * microns_per_pixel * microns_per_pixel * binning * binning;
	cell->diameter = shape.diameter * microns_per_pixel * binning;
	cell->intensity = shape.intensity;
	cell->intensity_stddev = shape.intensityStdDev;
	
	cell->perimeter = shape.perimeter * microns_per_pixel * binning;
	
	cell->shape = shape.shapeFactor;

	return 0;
}



int cell_finder_shape_valid(cell_finder *cf, GlShape shape, int validate_horz_overlap, int validate_vert_overlap)
{
	int bottom, right;
	
	//Check the bounding box of the shape to see if is in the overlap region
	
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

int cell_finder_process_cellbox(cell_finder *cf, Box *box, int validate_horz_overlap, int validate_vert_overlap)
{
	IPIImageInfo info;
	IPIImageRef box_image=0;
	IPIImageRef mask_image[6] = {0,0,0,0,0,0};
	IPIImageRef cellbound=0, allcellbound=0, box_image_copy=0;
	int n=6, i, doublet, ret = -1, view, nShapes, idx;
	char red, green, blue;
	Cell cell;
	GlShape *shapes;
	double microns_per_pixel = gci_camera_get_microns_per_pixel(cf->camera);
	int binning = gci_camera_get_binning_mode(cf->camera);
	double *celltype, *id;
	double *x_position, *y_position;
	double *area, *perimeter, *shape, *intensity, *intensity_stddev, *dose;
	
	if (binning < 1) binning = cf->camera->_camera_window->binning_size;
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera
	
	if(box->valid == 0)
		return 0;
	
	//Extract the box containing this cell
	IPI_GetImageInfo (cf->frame_image, &info);
	IPI_Create (&box_image, info.pixelType, 2);
	if (IPI_Extract (cf->frame_image, box_image, 1, 1, box->rect) != 0) goto Error;

	//Ensure that no pixels < 0
  	IPI_Compare (box_image, IPI_USECONSTANT, box_image, IPI_CP_MAX, 0.0);
	//printf("um per pix = %f. binning = %d\n", microns_per_pixel, binning);
	CHARM_run(box_image, 0, 0, microns_per_pixel*binning, &shapes, &nShapes);

	if (nShapes < 1)
		goto Error;	// Seriously wrong - this should never happen. We know the box had something in it.

	if (shapes==NULL)
	{
		MessagePopup("Error", "shapes is null");
		goto Error;
	}

	id = (double *)calloc(nShapes, sizeof(double));
	x_position = (double *)calloc(nShapes, sizeof(double));
	y_position = (double *)calloc(nShapes, sizeof(double));
	area = (double *)calloc(nShapes, sizeof(double));
	perimeter = (double *)calloc(nShapes, sizeof(double));
	shape = (double *)calloc(nShapes, sizeof(double));
	intensity = (double *)calloc(nShapes, sizeof(double));
	intensity_stddev = (double *)calloc(nShapes, sizeof(double));
	celltype = (double *)calloc(nShapes, sizeof(double));
	dose = (double *)calloc(nShapes, sizeof(double));

	//Display all the objects found.
	idx = 0;
	for (i=0; i < nShapes ; i++) {
	
		//If we haven't done any pre-processing do it now
		if (!cf->pre_process) {
			if (cell_finder_shape_valid(cf, shapes[i], validate_horz_overlap, validate_vert_overlap) < 0)
				continue;
		}
		
		//Get measurements for all objects
		// This update the cell structures shape, permeter, area etc fields.
		if (shapes[i].isPartOfCluster == 0) {
		
			cell = gci_cell_new(++cf->number_of_cells, SINGLE, 0.0, 0.0); 
			cf->number_of_singles++;
			red=255; blue=0;
		}	
		else {
		
			cell = gci_cell_new(++cf->number_of_cells, CLUSTER, 0.0, 0.0); 
			cf->number_of_clusters++;
			red=0; blue=255;
		}	
		
		// This checks the image is within our size bounds and updates the cells position, area, shape etc
		if (cell_finder_measure_object_from_shape(cf, shapes[i], box, &cell ) < 0)
			continue;
	

		cf->cell_diameter_sumation += cell.diameter; 
		SetCtrlVal (cf->automatic_panel, AUTOPNL_MEAN_CELL_DIAMETER, cf->cell_diameter_sumation / cf->number_of_cells);

		if (cf->show_processing) {
			if (binning == 1) {
				ipi_draw_colour_rect (cf->frame_display_image, MakeRect(cell.image_y - 1, cell.image_x - 1, 3, 3), red, 0, blue); 
				ipi_draw_colour_rect (cf->frame_display_image, MakeRect(cell.image_y - 2, cell.image_x - 2, 5, 5), red, 0, blue); 
			}
			else if (binning == 2) {
				ipi_draw_colour_rect (cf->frame_display_image, MakeRect(cell.image_y - 1, cell.image_x - 1, 3, 3), red, 0, blue); 
				ipi_draw_colour_rect (cf->frame_display_image, MakeRect(cell.image_y, cell.image_x, 1, 1), red, 0, blue); 
			}
			else {
				ipi_draw_colour_rect (cf->frame_display_image, MakeRect(cell.image_y, cell.image_x, 1, 1), red, 0, blue); 
			}
			
			//draw_cell_borders_from_shape(cf->frame_display_image, shapes[i], box->rect.left, box->rect.top, red, 0, blue);     
		}
		
		if (cf->add_to_map) {
			gci_cellmap_add_cell(cf->map, cell);
			
			id[idx] = cell.id;
			x_position[idx] = cell.x;
			y_position[idx] = cell.y;
			area[idx] = cell.area;
			perimeter[idx] = cell.perimeter;
			shape[idx] = cell.shape;
			intensity[idx] = cell.intensity;
			intensity_stddev[idx] = cell.intensity_stddev;
			celltype[idx] = cell.type;
			dose[idx] = cell.dose;
			
			idx++;
		}
	}

	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 1, id, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 2, x_position, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 3, y_position, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 4, area, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 5, perimeter, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 6, shape, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 7, intensity, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 8, intensity_stddev, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 9, celltype, idx);
	CellFinding_DoubleColumnToExcel(1, id[0] + 2, 11, dose, idx);
	GCI_SaveExcelWorkbookNoPrompt();
	
	free(id);
	free(x_position);
	free(y_position);
	free(area);
	free(perimeter);
	free(shape);
	free(intensity);
	free(intensity_stddev);
	free(celltype);
	free(dose);

	IPI_Dispose(box_image); 
	
	CHARM_shapesDispose();
	
	return 0;

Error:

	IPI_Dispose(box_image);
	
	for (i=0; i<n; i++) {
		if (mask_image[i]) IPI_Dispose(mask_image[i]);
	}

	if (shapes) CHARM_shapesDispose();

	return -1;
}
