#include <userint.h>

#include "icsviewer_private.h" 
#include "icsviewer_tools.h" 
#include "histogram_plugin.h"
#include "toolbox.h"
#include <utility.h>
#include "icsviewer_uir.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "GL_CVIRegistry.h"

#include <limits.h>
#include <float.h> 

#include "FreeImageAlgorithms_Statistics.h"
#include "FreeImageAlgorithms_Utilities.h"

#include "icsviewer_window.h"

//#define NUMBER_OF_FRAMES_BEFORE_DRAW 5

static int CVICALLBACK OnAutoScaleCheckBox (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);
	
static int CVICALLBACK OnGraphChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);
		
static int CVICALLBACK OnManualXScaleButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK OnManualXScaleOkButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK OnManualXScaleCancelButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK OnManualYScaleButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK OnManualYScaleOkButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK OnManualYScaleCancelButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);


static int HistogramPanelIsActive(HistogramPlugin *histogram_plugin)
{
	int visible;
	
	if(histogram_plugin->panel_id <= 0)
		return 0;
		
	GetPanelAttribute(histogram_plugin->panel_id, ATTR_VISIBLE, &visible);      
		
	if(!visible)
		return 0;
		
	return 1;
}


static void GCI_SetXAxisLegend(int panel_id, char *string)
{
	int panel_width, x_axis_width, x_axis_left_position;

	GetPanelAttribute (panel_id, ATTR_WIDTH, &panel_width); 
	
	SetCtrlVal(panel_id, HISTPNL_X_LABEL, string);

	GetCtrlAttribute (panel_id, HISTPNL_X_LABEL, ATTR_WIDTH, &x_axis_width);

	x_axis_left_position = (panel_width - x_axis_width  ) / 2; 

	SetCtrlAttribute (panel_id, HISTPNL_X_LABEL, ATTR_LEFT, x_axis_left_position ); 
}



static void SetXAxisExtents(HistogramPlugin *histogram_plugin , double min, double max)
{
	double ymin, ymax;
	
	SetAxisScalingMode (histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_XAXIS, VAL_MANUAL, min, max);
	GetAxisScalingMode (histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, &ymin, &ymax); 
	
	// Set The cursors to the same position
	SetGraphCursor (histogram_plugin->panel_id, HISTPNL_GRAPH, 1, min, ymin);      
	SetGraphCursor (histogram_plugin->panel_id, HISTPNL_GRAPH, 2, max, ymin); 
}


static int DisplayGreyScaleHistogram(HistogramPlugin *histogram_plugin, double min_intensity, double max_intensity)
{
	int i, imtype, is_float;
	unsigned long int scale = 0, max, min_in_hist, max_in_hist;
	float *xaxis;
	char average_text[30], min_intensity_string[50], max_intensity_string[50]; 
	double average, range, range_per_bin; //max_intensity = DBL_MIN, min_intensity = DBL_MAX;
	const int max_histogram_points=256;
	
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) histogram_plugin;
	IcsViewerWindow *window = plugin->window;

	if(!window->panel_dib)
		return GCI_IMAGING_ERROR;

	if(!FIA_IsGreyScale(window->panel_dib))
		return GCI_IMAGING_ERROR;

	//RJL 200906 - check if camera bitmode has changed
	imtype = FreeImage_GetImageType(window->panel_dib);
	if (imtype != histogram_plugin->imtype) {
		histogram_plugin->imtype = imtype;
		
		GCI_ImagingWindow_GetMaxPossibleValuesForDataType(plugin->window,
			&(histogram_plugin->min_x_axis), &(histogram_plugin->max_x_axis));

		SetXAxisExtents(histogram_plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);
		
		histogram_plugin->manual_min_x_axis = histogram_plugin->min_x_axis;	   
		histogram_plugin->manual_max_x_axis = histogram_plugin->max_x_axis;	   
	}
	
	is_float = (FreeImage_GetImageType(window->panel_dib) == FIT_FLOAT);

	if(is_float)
	{
		range = max_intensity - min_intensity;
		histogram_plugin->number_of_bins = max_histogram_points;             
		range_per_bin = range / (histogram_plugin->number_of_bins - 1);   
	}
	else 
	{   // is integer of some kind
		max_intensity = ceil(max_intensity);
		min_intensity = floor(min_intensity);
		range = max_intensity - min_intensity;
		histogram_plugin->number_of_bins = (int) range + 1;  // need one more bin to accomodate the max value pixels
		range_per_bin = 1.0; 
		
		if (histogram_plugin->number_of_bins>max_histogram_points)
		{
			histogram_plugin->number_of_bins=max_histogram_points;
			range_per_bin =  range / (histogram_plugin->number_of_bins - 1);
		}
	}
	
	
	if(histogram_plugin->hist != NULL) {
		free(histogram_plugin->hist);
		histogram_plugin->hist = NULL;	
	}
		
	histogram_plugin->hist = (unsigned long int *) malloc ( sizeof(unsigned long int) * histogram_plugin->number_of_bins );
			
	if (histogram_plugin->hist == NULL) 
		return GCI_IMAGING_ERROR;

	if (IsToolActive((Tool*) plugin->window->roi_tool)) {
		RECT rect;
		FIBITMAP *temp;
		
		GCI_ImagingWindow_GetROIImageRECT(window, &rect);
		temp = FIA_Copy(window->panel_dib, rect.left, rect.top, rect.right, rect.bottom);
		FIA_Histogram(temp, min_intensity, max_intensity,
			histogram_plugin->number_of_bins, histogram_plugin->hist);
		average = FIA_GetGreyLevelAverage(temp);    
		sprintf(average_text, "ROI Average Intensity %.1f", average);
		FreeImage_Unload(temp);
	}
	else {
		FIA_Histogram(window->panel_dib, min_intensity, max_intensity,
			histogram_plugin->number_of_bins, histogram_plugin->hist);
		average = FIA_GetGreyLevelAverage(window->panel_dib);    
		sprintf(average_text, "Average Intensity %.1f", average);
	}

    FIA_FindIntMinMax(histogram_plugin->hist, histogram_plugin->number_of_bins, &min_in_hist, &max_in_hist);
 
	if(is_float) {
	
		SetPanelAttribute (histogram_plugin->panel_id, ATTR_TITLE, "Histogram (Floating Point Image)");
		
		format_intensity_string(window, min_intensity, min_intensity_string);
		format_intensity_string(window, max_intensity, max_intensity_string);
		
		sprintf(histogram_plugin->bin_range_legend, "Intensity: %s - %s, bin width: %f",
			min_intensity_string, max_intensity_string, range_per_bin);
			
		GCI_SetXAxisLegend(histogram_plugin->panel_id, histogram_plugin->bin_range_legend); 
				
		SetCtrlVal(histogram_plugin->panel_id, HISTPNL_AVERAGE_LABEL, average_text);
	}
	else {
	
		GCI_SetXAxisLegend(histogram_plugin->panel_id, "Intensity");
	
		SetCtrlVal(histogram_plugin->panel_id, HISTPNL_AVERAGE_LABEL, average_text);
	}
	
	SetCtrlAttribute (histogram_plugin->panel_id, HISTPNL_GRAPH, ATTR_YNAME, "pixels");
	
	SetAxisScalingMode (histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_XAXIS, VAL_MANUAL, min_intensity, max_intensity);

	max = max_in_hist;
	
	while (max > 10) {
		max /= 10;
		scale ++;
	}
	
	if(histogram_plugin->y_scale_mode == HistScaleMode_Automatic) {  
	
		SetAxisScalingMode(histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, 0.0, (max +1)*pow(10,scale)); 
		SetCtrlAttribute (histogram_plugin->panel_id, HISTPNL_GRAPH, ATTR_YLOOSE_FIT_AUTOSCALING_UNIT, scale);
		SetCtrlAttribute(histogram_plugin->panel_id, HISTPNL_GRAPH, ATTR_YMARK_ORIGIN, 0);
	
		SetAxisScalingMode(histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_LEFT_YAXIS, 
			VAL_MANUAL, 0, ((int)max+1)*pow(10,scale));
	}
	else {
		
		SetAxisScalingMode(histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_LEFT_YAXIS, 
			VAL_MANUAL, histogram_plugin->min_y_axis, histogram_plugin->max_y_axis);	
	}

	xaxis = (float *) malloc ( sizeof(float) * histogram_plugin->number_of_bins );   
	memset(xaxis, 0, sizeof(float) * histogram_plugin->number_of_bins);

	for(i=0; i< histogram_plugin->number_of_bins; i++) {
		xaxis[i] = (float)min_intensity + (float)range_per_bin * (float)i;
	}

	histogram_plugin->plot_handle = PlotXY (histogram_plugin->panel_id, HISTPNL_GRAPH,
		xaxis, histogram_plugin->hist, histogram_plugin->number_of_bins, VAL_FLOAT, VAL_INTEGER,
		VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_BLACK);

	free(xaxis);
	
	return GCI_IMAGING_SUCCESS;
}


static int GCI_ImageWindowRGBHistogram(HistogramPlugin *histogram_plugin, double min_intensity, double max_intensity)
{
	int min, rmax = 0, gmax = 0, bmax = 0, imax, max_rgb_val, scale=0;
	
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) histogram_plugin;
	IcsViewerWindow *window = plugin->window;

	memset(histogram_plugin->histR, 0, sizeof(int) * 256);
	memset(histogram_plugin->histG, 0, sizeof(int) * 256);
	memset(histogram_plugin->histB, 0, sizeof(int) * 256);
	
	SetPanelAttribute (histogram_plugin->panel_id, ATTR_TITLE, "Histogram");

	GCI_SetXAxisLegend(histogram_plugin->panel_id, "Intensity");
	
	SetCtrlAttribute (histogram_plugin->panel_id, HISTPNL_GRAPH, ATTR_YNAME, "pixels");
	
    FreeImage_GetHistogram(window->panel_dib, histogram_plugin->histR, FICC_RED );
    
    //Set suitable y axis scaling
    FIA_FindIntMinMax(histogram_plugin->histR, 255, &min, &rmax);
   										  
	FreeImage_GetHistogram(window->panel_dib, histogram_plugin->histG, FICC_GREEN );

	FIA_FindIntMinMax(histogram_plugin->histG, 255, &min, &gmax); 
	
	FreeImage_GetHistogram(window->panel_dib, histogram_plugin->histB, FICC_BLUE );

	FIA_FindIntMinMax(histogram_plugin->histB, 255, &min, &bmax); 
	
	imax = max(rmax, gmax);
	imax = max(imax, bmax);

	max_rgb_val = imax;

	while (imax > 10) {
		imax /= 10;
		scale ++;
	}
	
	if(histogram_plugin->x_scale_mode == HistScaleMode_Automatic) {  
	
		GCI_ImagingWindow_GetMaxPossibleValuesForDataType(plugin->window,
			&(histogram_plugin->min_x_axis), &(histogram_plugin->max_x_axis));
	}
	else {
		
		SetAxisScalingMode (histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_XAXIS, VAL_MANUAL,
			histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);
	}
	
	
	if(histogram_plugin->y_scale_mode == HistScaleMode_Automatic) {  
	
		SetAxisScalingMode(histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, 0.0, (imax+1)*pow(10,scale)); 
		SetCtrlAttribute (histogram_plugin->panel_id, HISTPNL_GRAPH, ATTR_YLOOSE_FIT_AUTOSCALING_UNIT, scale);
	}
	else {
		
		SetAxisScalingMode(histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_LEFT_YAXIS, 
			VAL_MANUAL, histogram_plugin->min_y_axis, histogram_plugin->max_y_axis);	
	}
	
	histogram_plugin->red_plot_handle = PlotY (histogram_plugin->panel_id, HISTPNL_GRAPH, histogram_plugin->histR, 255, VAL_INTEGER, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_RED);
	histogram_plugin->green_plot_handle = PlotY (histogram_plugin->panel_id, HISTPNL_GRAPH, histogram_plugin->histG, 255, VAL_INTEGER, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_GREEN);
	histogram_plugin->blue_plot_handle = PlotY (histogram_plugin->panel_id, HISTPNL_GRAPH, histogram_plugin->histB, 255, VAL_INTEGER, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_BLUE);

	SetCtrlVal(histogram_plugin->panel_id, HISTPNL_AVERAGE_LABEL, "");
	
	return GCI_IMAGING_SUCCESS;
}



static int CloseHistogram (HistogramPlugin *histogram_plugin)
{
    ImageWindowPlugin *plugin = (ImageWindowPlugin *) histogram_plugin;

	if (!histogram_plugin->panel_id)
		return GCI_IMAGING_SUCCESS;
	
	if(histogram_plugin->histR != NULL) {
		free(histogram_plugin->histR);
		histogram_plugin->histR = NULL; 
	}
	
	if(histogram_plugin->histG != NULL) {
		free(histogram_plugin->histG);
		histogram_plugin->histG = NULL; 
	}
	
	if(histogram_plugin->histB != NULL) {
		free(histogram_plugin->histB);
		histogram_plugin->histB = NULL; 
	}
	
	if(histogram_plugin->hist != NULL) {
		free(histogram_plugin->hist);
		histogram_plugin->hist = NULL; 
	}
		
	ics_viewer_registry_save_panel_position(plugin->window, histogram_plugin->panel_id);

	DiscardPanel(histogram_plugin->panel_id);
	histogram_plugin->panel_id = -1;
	
	return GCI_IMAGING_SUCCESS;
}


static int CVICALLBACK onHistogramQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:

			CloseHistogram (histogram_plugin);

			break;
		}
	return 0;
}

	
static int GCI_ImageWindow_ExportHistogramData(HistogramPlugin *histogram_plugin, char *filepath)
{
	int i, fsize, bpp, type;
	FILE *fp;

	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;
	IcsViewerWindow *window = plugin->window;
	
	bpp = FreeImage_GetBPP(window->panel_dib); 
	type = FreeImage_GetImageType(window->panel_dib);
	
	// If the conf file does exist clear any read only bit 
	if (FileExists (filepath, &fsize))
    	SetFileAttrs (filepath, 0, -1, -1, -1);

    fp = fopen (filepath, "w");
    
    if (fp == NULL)
    	return GCI_IMAGING_ERROR;
	
	if(FIA_IsGreyScale(window->panel_dib))
	{
		if(type == FIT_FLOAT)
			fprintf(fp, "%s\n", histogram_plugin->bin_range_legend);	

		if(type == FIT_INT16 || type == FIT_INT32) {
			
			// We have unsigned data go from - half number of bins
			int half = histogram_plugin->number_of_bins / 2;   
			int smallest = -half;
			
			for(i=smallest; i < half; i++)
    			fprintf( fp, "%d\t%d\n", i, histogram_plugin->hist[i + half] );
		}
		else {
		
			// We have unsigned data
			
			for(i=0; i < histogram_plugin->number_of_bins; i++)
    			fprintf( fp, "%d\t%d\n", i, histogram_plugin->hist[i] );
		}
	}
	else {
			
		fprintf(fp, "%s\n", "Intensity - Red  Green  Blue");

		for(i=0; i < 256; i++)
    		fprintf( fp, "%d\t%d\t%d\t%d\n", i, histogram_plugin->histR[i], histogram_plugin->histG[i], histogram_plugin->histB[i] );
	}

    fclose(fp);

    // set read-only 
    SetFileAttrs (filepath, 1, -1, -1, -1);

	return GCI_IMAGING_SUCCESS;
}
	
	
static int CVICALLBACK OnExportClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *) callbackData; 
	char *default_extensions = "*.dat;";
	char fname[GCI_MAX_PATHNAME_LEN] = "";
	char directory[GCI_MAX_PATHNAME_LEN] = "";
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;
	
	switch (event)
		{
		case EVENT_COMMIT:

			if (FileSelectPopup (GetDefaultDirectoryPath(plugin->window, directory), "*.dat",
				default_extensions, "Export Data As", VAL_OK_BUTTON, 0, 0, 1, 1, fname) <= 0) {
				return -1;
			}

			GCI_ImageWindow_ExportHistogramData(histogram_plugin, fname);
			
			break;
		}
	return 0;
}


static void DrawHistogram(ImageWindowPlugin *plugin, double min, double max)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *) plugin;
	IcsViewerWindow *window = plugin->window;
	
	// Deletes all the plots from the graph 
	DeleteGraphPlot (histogram_plugin->panel_id, HISTPNL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
	RefreshGraph (histogram_plugin->panel_id, HISTPNL_GRAPH);

	if(!FIA_IsGreyScale(window->panel_dib))
		GCI_ImageWindowRGBHistogram(histogram_plugin, min, max);
	else
		DisplayGreyScaleHistogram(histogram_plugin, min, max);  
}

static void GCI_ImagingWindow_DisplayHistogram(ImageWindowPlugin *plugin)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *) plugin;

	if(histogram_plugin->prevent_update)
		return;

	if(histogram_plugin->panel_id < 0) {
		
		histogram_plugin->panel_id = LoadPanel(0, uir_file_path, HISTPNL);

		ics_viewer_set_panel_to_top_left_of_window(plugin->window, histogram_plugin->panel_id);

		/* Setup the close call back */
		if ( InstallCtrlCallback (histogram_plugin->panel_id, HISTPNL_QUIT, onHistogramQuit, histogram_plugin) < 0) {
			return;
		}
		
		if ( InstallCtrlCallback (histogram_plugin->panel_id, HISTPNL_EXPORT, OnExportClicked, histogram_plugin) < 0) {
			return;
		}
		
		if ( InstallCtrlCallback (histogram_plugin->panel_id, HISTPNL_GRAPH, OnGraphChanged, histogram_plugin) < 0)
			return;   
	
		if ( InstallCtrlCallback (histogram_plugin->panel_id, HISTPNL_FIX_XSCALE, OnManualXScaleButtonClicked, histogram_plugin) < 0) {
			return;
		}
		
		if ( InstallCtrlCallback (histogram_plugin->panel_id, HISTPNL_FIX_YSCALE, OnManualYScaleButtonClicked, histogram_plugin) < 0) {
			return;
		}
	
		if((FreeImage_GetImageType(plugin->window->panel_dib) == FIT_FLOAT) &&
			(fabs(plugin->window->panel_max_pixel_value) < 0.01 || fabs(plugin->window->panel_max_pixel_value) > 999)) {
			
			SetCtrlAttribute(histogram_plugin->panel_id, HISTPNL_GRAPH, ATTR_XFORMAT, VAL_SCIENTIFIC_FORMAT); 
		}
		else
			SetCtrlAttribute(histogram_plugin->panel_id, HISTPNL_GRAPH, ATTR_XFORMAT, VAL_FLOATING_PT_FORMAT);
			
			
		// I have to save the plot data so the user can export. This is because I can't set the
		// graph to retain mode to get the data later as stupid cvi doesnt delete the data in the graph
		// correctly even though I called DeleteData.
		histogram_plugin->histR = (int*) malloc(sizeof(int) * 256);
		histogram_plugin->histG = (int*) malloc(sizeof(int) * 256);    
		histogram_plugin->histB = (int*) malloc(sizeof(int) * 256);    
	
		GCI_ImagingWindow_GetMaxPossibleValuesForDataType(plugin->window,
			&(histogram_plugin->min_x_axis), &(histogram_plugin->max_x_axis));

		SetXAxisExtents(histogram_plugin , histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);
		
		histogram_plugin->manual_min_x_axis = histogram_plugin->min_x_axis;	   //RJL 200906
		histogram_plugin->manual_max_x_axis = histogram_plugin->max_x_axis;	   //RJL 200906
		
		DisplayPanel(histogram_plugin->panel_id);	
	}
	
	DrawHistogram(plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);
		
	return;
}


static void CloseManualXScalePanel(HistogramPlugin *histogram_plugin)
{
    ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;
    
	if(histogram_plugin->xaxis_manual_panel > 0)
	{
	    ics_viewer_registry_save_panel_position(plugin->window, histogram_plugin->xaxis_manual_panel);
	    
		DiscardPanel(histogram_plugin->xaxis_manual_panel);
	
		histogram_plugin->xaxis_manual_panel = 0;
	}
}


static void SetManualAxisNumerics(HistogramPlugin *histogram_plugin, int panel, double min, double max)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;
	
	if(FreeImage_GetImageType(plugin->window->panel_dib) == FIT_FLOAT)
	{
		SetCtrlVal(panel, MAN_SCALE_AXIS_MIN, min);
		SetCtrlVal(panel, MAN_SCALE_AXIS_MAX, max);
	}
	else
	{
		SetCtrlVal(panel, MAN_SCALE_AXIS_MIN, (int) min);	
		SetCtrlVal(panel, MAN_SCALE_AXIS_MAX, (int) max);	
	}
}


static void GetManualAxisNumerics(HistogramPlugin *histogram_plugin, int panel, double *min, double *max)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;
	
	if(FreeImage_GetImageType(plugin->window->panel_dib) == FIT_FLOAT)
	{
		GetCtrlVal(panel, MAN_SCALE_AXIS_MIN, min);
		GetCtrlVal(panel, MAN_SCALE_AXIS_MAX, max);
	}
	else
	{
		int imin, imax;
		
		GetCtrlVal(panel, MAN_SCALE_AXIS_MIN, &imin);	
		GetCtrlVal(panel, MAN_SCALE_AXIS_MAX, &imax);
		
		*min = imin;
		*max = imax;
	}
}

static void DisplayManualXScalePanel(HistogramPlugin *histogram_plugin, int is_integer)
{
    ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;
    
	if(histogram_plugin->xaxis_manual_panel > 0)
		return;
	
	histogram_plugin->xaxis_manual_panel = LoadPanel(0, uir_file_path, MAN_SCALE);  

	ics_viewer_set_panel_to_top_left_of_window(plugin->window, histogram_plugin->xaxis_manual_panel);

	if(is_integer) {
		
		SetCtrlAttribute(histogram_plugin->xaxis_manual_panel, MAN_SCALE_AXIS_MIN, ATTR_DATA_TYPE, VAL_INTEGER); 
		SetCtrlAttribute(histogram_plugin->xaxis_manual_panel, MAN_SCALE_AXIS_MAX, ATTR_DATA_TYPE, VAL_INTEGER); 
	}
	else {
		
		SetCtrlAttribute(histogram_plugin->xaxis_manual_panel, MAN_SCALE_AXIS_MIN, ATTR_DATA_TYPE, VAL_DOUBLE); 
		SetCtrlAttribute(histogram_plugin->xaxis_manual_panel, MAN_SCALE_AXIS_MAX, ATTR_DATA_TYPE, VAL_DOUBLE); 	
	}
	
	if ( InstallCtrlCallback (histogram_plugin->xaxis_manual_panel, MAN_SCALE_OK_BUTTON,
		OnManualXScaleOkButtonClicked, histogram_plugin) < 0) {
		return;
	}
	
	if ( InstallCtrlCallback (histogram_plugin->xaxis_manual_panel, MAN_SCALE_CANCEL_BUTTON,
		OnManualXScaleCancelButtonClicked, histogram_plugin) < 0) {
		return;
	}
	
	SetManualAxisNumerics(histogram_plugin, histogram_plugin->xaxis_manual_panel,
		histogram_plugin->manual_min_x_axis, histogram_plugin->manual_max_x_axis);		//RJL 200906
		
	DisplayPanel(histogram_plugin->xaxis_manual_panel);
}


static int CVICALLBACK OnManualXScaleButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *)  callbackData;
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;   
	int val;
	double min, max;
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);
			
			if(val) {
			
				histogram_plugin->x_scale_mode = HistScaleMode_Manual;   
				
				if(FreeImage_GetImageType(plugin->window->panel_dib) != FIT_FLOAT)
					DisplayManualXScalePanel(histogram_plugin, 1);     
				else
					DisplayManualXScalePanel(histogram_plugin, 0);
			}	
			else {
				
				histogram_plugin->x_scale_mode = HistScaleMode_Automatic;
			
				GCI_ImagingWindow_GetMaxPossibleValuesForDataType(plugin->window, &min, &max);
				
				histogram_plugin->min_x_axis = min;			   //RJL 210906
				histogram_plugin->max_x_axis = max;			   //RJL 210906
				
				SetXAxisExtents(histogram_plugin, min, max);
		
				DrawHistogram(plugin, min, max);
			}
			
			break;
		}
	return 0;
}


static int CVICALLBACK OnManualXScaleOkButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *)  callbackData;   
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;   
	
	switch (event)
		{
		case EVENT_COMMIT:

			histogram_plugin->x_scale_mode = HistScaleMode_Manual; 
			
			//GetCtrlVal(histogram_plugin->xaxis_manual_panel, MAN_SCALE_AXIS_MIN, &(histogram_plugin->min_x_axis));
			//GetCtrlVal(histogram_plugin->xaxis_manual_panel, MAN_SCALE_AXIS_MAX, &(histogram_plugin->max_x_axis)); 
			
			GetManualAxisNumerics(histogram_plugin, histogram_plugin->xaxis_manual_panel,
				&(histogram_plugin->min_x_axis), &(histogram_plugin->max_x_axis)); 
			
			GetManualAxisNumerics(histogram_plugin, histogram_plugin->xaxis_manual_panel,
				&(histogram_plugin->manual_min_x_axis), &(histogram_plugin->manual_max_x_axis)); 
			
			SetXAxisExtents(histogram_plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);
		
			DrawHistogram(plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);          
			
			CloseManualXScalePanel(histogram_plugin); 
			
			break;
		}
	return 0;
}


static int CVICALLBACK OnManualXScaleCancelButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *)  callbackData; 

	switch (event)
		{
		case EVENT_COMMIT:

			CloseManualXScalePanel(histogram_plugin) ;     

			break;
		}
	return 0;
}



static int CVICALLBACK OnGraphChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double y;
	HistogramPlugin *histogram_plugin = (HistogramPlugin *) callbackData;
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;  
	
	switch (event)
	{
		case EVENT_COMMIT: 
		{
			histogram_plugin->custom_x_scale = 1;
			histogram_plugin->prevent_update = 0; 
		
			// Toggle the x manual scale button
			SetCtrlVal(panel, HISTPNL_FIX_XSCALE, 1);
			
			DeleteGraphPlot (histogram_plugin->panel_id, HISTPNL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
			
			GetGraphCursor (panel, control, 1, &(histogram_plugin->min_x_axis), &y);      
			GetGraphCursor (panel, control, 2, &(histogram_plugin->max_x_axis), &y);
		
			histogram_plugin->manual_min_x_axis = histogram_plugin->min_x_axis;	//RJL 200906
			histogram_plugin->manual_max_x_axis = histogram_plugin->max_x_axis;	//RJL 200906
			
			SetXAxisExtents(histogram_plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);
		
			DrawHistogram(plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);
			
			histogram_plugin->x_scale_mode = HistScaleMode_Manual; 
			
			break;
		}
		
		case EVENT_LEFT_CLICK:
		{
			histogram_plugin->prevent_update = 1;
		
			break;
		}
	}
		
	return 0;
}


static void CloseManualYScalePanel(HistogramPlugin *histogram_plugin)
{
    ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;  
    
	if(histogram_plugin->yaxis_manual_panel > 0)
	{
	    ics_viewer_registry_save_panel_position(plugin->window, histogram_plugin->yaxis_manual_panel);
	    
		DiscardPanel(histogram_plugin->yaxis_manual_panel);
	
		histogram_plugin->yaxis_manual_panel = 0;
	}
}


static void DisplayManualYScalePanel(HistogramPlugin *histogram_plugin)
{
    ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;
	int axis_scaling_mode;
	double min, max;
	
	if(histogram_plugin->yaxis_manual_panel > 0)
		return;
	
	histogram_plugin->y_scale_mode = HistScaleMode_Manual;    
	
	histogram_plugin->yaxis_manual_panel = LoadPanel(0, uir_file_path, MAN_SCALE);
	
	ics_viewer_set_panel_to_top_left_of_window(plugin->window, histogram_plugin->yaxis_manual_panel);
		
	SetCtrlAttribute(histogram_plugin->yaxis_manual_panel, MAN_SCALE_AXIS_MIN, ATTR_DATA_TYPE, VAL_INTEGER); 
	SetCtrlAttribute(histogram_plugin->yaxis_manual_panel, MAN_SCALE_AXIS_MAX, ATTR_DATA_TYPE, VAL_INTEGER);
	
	if ( InstallCtrlCallback (histogram_plugin->yaxis_manual_panel, MAN_SCALE_OK_BUTTON,
		OnManualYScaleOkButtonClicked, histogram_plugin) < 0) {
		return;
	}
	
	if ( InstallCtrlCallback (histogram_plugin->yaxis_manual_panel, MAN_SCALE_CANCEL_BUTTON,
		OnManualYScaleCancelButtonClicked, histogram_plugin) < 0) {
		return;
	}
	
	if(histogram_plugin->min_y_axis == 0 && histogram_plugin->max_y_axis == 0)
	{
		// Get the current yxais values.
		GetAxisScalingMode (histogram_plugin->panel_id, HISTPNL_GRAPH, VAL_LEFT_YAXIS, &axis_scaling_mode,
			&min, &max);
	
		histogram_plugin->min_y_axis = (int) min;
		histogram_plugin->max_y_axis = (int) max;
	}

	SetCtrlVal(histogram_plugin->yaxis_manual_panel, MAN_SCALE_AXIS_MIN, histogram_plugin->min_y_axis);
	SetCtrlVal(histogram_plugin->yaxis_manual_panel, MAN_SCALE_AXIS_MAX, histogram_plugin->max_y_axis); 	
	
	DisplayPanel(histogram_plugin->yaxis_manual_panel);
}


static int CVICALLBACK OnManualYScaleButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *)  callbackData;
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;   
	int val;
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);
			
			if(val)
				DisplayManualYScalePanel(histogram_plugin);     
			else {
				
				histogram_plugin->y_scale_mode = HistScaleMode_Automatic;
		
				DrawHistogram(plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);
			}
			
			break;
		}
	return 0;
}


static int CVICALLBACK OnManualYScaleOkButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *)  callbackData;   
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) histogram_plugin;   
	
	switch (event)
		{
		case EVENT_COMMIT:

			histogram_plugin->y_scale_mode = HistScaleMode_Manual; 
			
			GetCtrlVal(histogram_plugin->yaxis_manual_panel, MAN_SCALE_AXIS_MIN, &(histogram_plugin->min_y_axis));	
			GetCtrlVal(histogram_plugin->yaxis_manual_panel, MAN_SCALE_AXIS_MAX, &(histogram_plugin->max_y_axis));
			
			DrawHistogram(plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis);          
			
			CloseManualYScalePanel(histogram_plugin); 
			
			break;
		}
	return 0;
}


static int CVICALLBACK OnManualYScaleCancelButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *)  callbackData; 

	switch (event)
		{
		case EVENT_COMMIT:

			CloseManualYScalePanel(histogram_plugin) ;     

			break;
		}
	return 0;
}

static int on_validate_plugin(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	if(plugin->window->panel_dib == NULL)
		return 0;
	
	return 1;   
}
	  

static void on_image_displayed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *) plugin; 

	//histogram_plugin->number_of_frames++;
	
	histogram_plugin->custom_x_scale = 0; 
	
	// If live must we throttle the amount of times we change the histogram
//	if(plugin->window->live_mode) {  
//		
//		if(HistogramPanelIsActive(histogram_plugin) && histogram_plugin->number_of_frames >= NUMBER_OF_FRAMES_BEFORE_DRAW) {
//			DrawHistogram(plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis); 
//			histogram_plugin->number_of_frames = 0;    
//		}
//	}
//	else {
		
		if(HistogramPanelIsActive(histogram_plugin)) {
			DrawHistogram(plugin, histogram_plugin->min_x_axis, histogram_plugin->max_x_axis); 
		}
//	}
}

static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData; 
	
	GCI_ImagingWindow_DisplayHistogram(plugin); 
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	HistogramPlugin *histogram_plugin = (HistogramPlugin *) plugin;  
}

ImageWindowPlugin* histogram_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "HistogramPlugin", sizeof(HistogramPlugin));

	HistogramPlugin *histogram_plugin = (HistogramPlugin *) plugin;

	histogram_plugin->panel_id = -1;
	histogram_plugin->x_scale_mode = HistScaleMode_Automatic;
	histogram_plugin->y_scale_mode = HistScaleMode_Automatic;  
	histogram_plugin->xaxis_manual_panel = 0;
	histogram_plugin->yaxis_manual_panel = 0;
	histogram_plugin->custom_x_scale = 0; 
	histogram_plugin->prevent_update = 0;    
	histogram_plugin->min_x_axis = 0.0;    
	histogram_plugin->max_x_axis = 0.0;    
	histogram_plugin->manual_min_x_axis = 0.0;    //RJL 200906
	histogram_plugin->manual_max_x_axis = 0.0;    //RJL 200906
	histogram_plugin->min_y_axis = 0;
	histogram_plugin->max_y_axis = 0;
	histogram_plugin->imtype = FIT_UNKNOWN;
	
	histogram_plugin->histR = NULL;
	histogram_plugin->histG = NULL;   
	histogram_plugin->histB = NULL; 
	histogram_plugin->hist = NULL;
	
	Plugin_AddMenuItem(plugin, "View//Histogram",
		VAL_MENUKEY_MODIFIER | 'H', on_menu_clicked, plugin);

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin; 
	PLUGIN_VTABLE(plugin, on_image_displayed) = on_image_displayed; 
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}



