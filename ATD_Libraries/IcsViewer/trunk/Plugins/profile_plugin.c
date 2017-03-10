#include "icsviewer_window.h"
#include "icsviewer_private.h" 
#include "profile_plugin.h"

#include "string_utils.h"

#include <userint.h>
#include "toolbox.h"
#include <utility.h>

#include "gci_utils.h"
#include "GL_CVIRegistry.h"

#include "ImageViewer_Drawing.h" 

#include "string_utils.h"

#include "icsviewer_uir.h"

#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_Utilities.h"


static int CVICALLBACK onExportClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK onProfileQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK OnManualScaleButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK OnManualScaleOkButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static int CVICALLBACK OnManualScaleCancelButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static void GetGetScaleDetails(ProfilePlugin *profile_plugin, int cvi_type, void *hist, int len, int *scale, double *max_value)
{
	switch(cvi_type)
	{
		case VAL_UNSIGNED_CHAR:
		{
			unsigned char max;
			FIA_FindUCharMax(hist, len, &max);        
			*max_value = (double) max;
			
			break;
		}
		
		case VAL_SHORT_INTEGER:
		{
			short max;
			FIA_FindShortMax(hist, len, &max);        
			*max_value = (double) max;
			
			break;
		}
		
		case VAL_UNSIGNED_SHORT_INTEGER:
		{
			unsigned short max;
			FIA_FindUShortMax(hist, len, &max);        
			*max_value = (double) max;
			
			break;
		}
		
		case VAL_FLOAT:
		{
			float max;
			FIA_FindFloatMax(hist, len, &max);        
			*max_value = (double) max;
			
			break;
		}
		
		case VAL_DOUBLE:
		{
			double max;
			FIA_FindDoubleMax(hist, len, &max);        
			*max_value = (double) max;
			
			break;
		}
	}
	
	while (*max_value > 10.0) {
		*max_value /= 10.0;
		(*scale)++;
	}
			
	return;

}

static int display_greylevel_profile(ProfilePlugin *profile_plugin, POINT image_pt1, POINT image_pt2)
{
	
	int scale=0, i, bpp, len, dx, dy, width, height;
	double *xdata, max, step;
	FREE_IMAGE_TYPE type;
	int bytes_per_pixel;
	void *hist;
	int cvi_type;
	int array_size;
	
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin; 
		
	FIAPOINT image_p1 = GdiToFiaPoint(image_pt1);
	FIAPOINT image_p2 = GdiToFiaPoint(image_pt2);
	
	FIBITMAP *dib = plugin->window->panel_dib;
	
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	if (image_p1.x<0 || image_p1.x>width)   goto Error;
	if (image_p1.y<0 || image_p1.y>height)  goto Error;
	if (image_p2.x<0 || image_p2.x>width)   goto Error;
	if (image_p2.y<0 || image_p2.y>height)  goto Error;
	
	bpp = FreeImage_GetBPP(dib);
	type = FreeImage_GetImageType(dib);
	
	dx = abs(image_p1.x - image_p2.x);
	dy = abs(image_p1.y - image_p2.y);
	
	bytes_per_pixel = bpp / 8;
	
	array_size = dx + dy + 2;
	
	hist = malloc (bytes_per_pixel * array_size );   
	memset(hist, 0, (bytes_per_pixel * array_size));
	
	if(bpp == 8) {
		
		len = FIA_GetUCharPixelValuesForLine (plugin->window->panel_dib, image_p1, image_p2, (unsigned char *)hist); 
		cvi_type = VAL_UNSIGNED_CHAR;

		
		// Reverse if the user draws from right to left.
		// The midpoint line algorithm works by going from left to right so I don't want to change it there.
		// Also I have tried putting the templated version in  FIA_PixelValuesForLine
		// but I get memory coruptions there. I don't know why.
		if(image_p2.x < image_p1.x) 
			FIA_UCharArrayReverse(hist, len);
		
	}
	else if(bpp == 16) {

		if(type == FIT_INT16) {
		
			len = FIA_GetShortPixelValuesForLine (plugin->window->panel_dib, image_p1, image_p2, hist); 
			cvi_type = VAL_SHORT_INTEGER;

			if(image_p2.x < image_p1.x) 
				FIA_ShortArrayReverse(hist, len);
		}
		if(type == FIT_UINT16) {
		
			len = FIA_GetUShortPixelValuesForLine (plugin->window->panel_dib, image_p1, image_p2, hist); 
			cvi_type = VAL_UNSIGNED_SHORT_INTEGER;
			
			if(image_p2.x < image_p1.x) 
				FIA_ShortArrayReverse(hist, len);
		}
	}
	else if(bpp == 32)
	{
		len = FIA_GetFloatPixelValuesForLine (plugin->window->panel_dib, image_p1, image_p2, hist); 
		cvi_type = VAL_FLOAT;
		
		if(image_p2.x < image_p1.x) 
			FIA_FloatArrayReverse(hist, len);
	}
	else if(bpp == 64)
	{
		len = FIA_GetDoublePixelValuesForLine (plugin->window->panel_dib, image_p1, image_p2, hist); 
		cvi_type = VAL_DOUBLE;
		
		if(image_p2.x < image_p1.x) 
			FIA_DoubleArrayReverse(hist, len);
	}

	if(profile_plugin->scale_mode == ScaleMode_Automatic) {  
		
		int rounded_min_value;
		
		GetGetScaleDetails(profile_plugin, cvi_type, hist, len, &scale, &max); 
			
		SetCtrlAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, ATTR_YLOOSE_FIT_AUTOSCALING_UNIT, scale);
		
		rounded_min_value = (int) (floor((plugin->window->panel_min_pixel_value / pow(10,scale))) * pow(10,scale));
		
		SetAxisScalingMode(profile_plugin->panel_id, PROF_PNL_GRAPH, VAL_LEFT_YAXIS, 
			VAL_MANUAL, rounded_min_value, ((int)max+1)*pow(10,scale));
	}
	else {
	
		SetAxisScalingMode(profile_plugin->panel_id, PROF_PNL_GRAPH, VAL_LEFT_YAXIS,
			VAL_MANUAL, profile_plugin->manual_scale_min, profile_plugin->manual_scale_max);      		
	}
	
	assert(array_size > len); 
		
	xdata = (double *)calloc(len, sizeof(double));
		
	step = sqrt(dx*dx + dy*dy) / (double) len;
		
	if (plugin->window->microns_per_pixel == 1.0) {  //pixels
		for (i=0; i < len; i++)
			xdata[i] = i * step;
	}
	else {	   //microns
		
		for (i=0; i < len; i++)
			xdata[i] = ( (double) i * step * plugin->window->binning_size) * plugin->window->microns_per_pixel;
	}
	
	profile_plugin->profile_red_plot_handle = PlotXY (profile_plugin->panel_id, PROF_PNL_GRAPH, xdata,
			hist, len, VAL_DOUBLE, cvi_type,
			VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_DK_YELLOW);
			
	free(hist);
	free(xdata); 
	
	return GCI_IMAGING_SUCCESS;
	
Error:
	GCI_ImagingWindow_DisableLineTool(plugin->window); 
	GCI_ImagingWindow_UnLockProfileButton(plugin->window);  	

	return GCI_IMAGING_ERROR;
}


static int display_colour_profile(ProfilePlugin *profile_plugin, POINT image_p1, POINT image_p2)
{
	char *red_hist, *green_hist, *blue_hist;
	int len, i, dx, dy, scale = 0, width, height;
	unsigned char tmp_max = 0, max = 0, max_intensity = 0;
	double *xdata;
	double step;
	FREE_IMAGE_TYPE type;

	ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin; 
	
	FIBITMAP *dib = plugin->window->panel_dib;  
	
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	if (image_p1.x<0 || image_p1.x>width)   goto Error;
	if (image_p1.y<0 || image_p1.y>height)  goto Error;
	if (image_p2.x<0 || image_p2.x>width)   goto Error;
	if (image_p2.y<0 || image_p2.y>height)  goto Error;
	
	type = FreeImage_GetImageType(plugin->window->panel_dib);

	if(FreeImage_GetBPP(dib) < 24 || (type > FIT_BITMAP && type < FIT_COMPLEX) )
		return GCI_IMAGING_ERROR;

	dx = abs(image_p1.x - image_p2.x);
	dy = abs(image_p1.y - image_p2.y);	
			
	// Allocate enoungh space for the array 
   	red_hist = (char *) malloc (sizeof(char) * (dx + dy + 1) );
   		
   	// Allocate enoungh space for the array 
   	green_hist = (char *) malloc (sizeof(char) * (dx + dy + 1) );
   		
   	// Allocate enoungh space for the array 
   	blue_hist = (char *) malloc (sizeof(char) * (dx + dy + 1) );

	len = FIA_GetRGBPixelValuesForLine (dib, GdiToFiaPoint(image_p1),
		GdiToFiaPoint(image_p2), red_hist, green_hist, blue_hist); 

	// Deletes all the plots from the graph 
	DeleteGraphPlot (profile_plugin->panel_id, PROF_PNL_GRAPH, -1, VAL_IMMEDIATE_DRAW);

	SetPanelAttribute (profile_plugin->panel_id, ATTR_TITLE, "Profile");
	
	if(plugin->window->microns_per_pixel != 1.0) 
		SetCtrlVal(profile_plugin->panel_id, PROF_PNL_X_LABEL, "um");
	else
		SetCtrlVal(profile_plugin->panel_id, PROF_PNL_X_LABEL, "pixels");
	
	SetCtrlAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, ATTR_YNAME, "Intensity");

	if(image_p2.x < image_p1.x) {
		FIA_UCharArrayReverse(red_hist, len);
		FIA_UCharArrayReverse(green_hist, len);  
		FIA_UCharArrayReverse(blue_hist, len);  
	}

	xdata = (double *)calloc(len, sizeof(double));
		
	step = sqrt(dx*dx + dy*dy) / (double)len;
		
	for (i=0; i < len; i++)
		xdata[i] = ( (double) i * step * plugin->window->binning_size) * plugin->window->microns_per_pixel;

	//SetAxisScalingMode (profile_plugin->panel_id, PROF_PNL_GRAPH, VAL_XAXIS, VAL_MANUAL, 0.0, len);

	FIA_FindUCharMax(red_hist, len, &max_intensity);      
	FIA_FindUCharMax(green_hist, len, &tmp_max);   
	
	max_intensity = max(max_intensity, tmp_max);  
	
	FIA_FindUCharMax(blue_hist, len, &tmp_max);   

	max_intensity = max(max_intensity, tmp_max);

	max = max_intensity;

	while (max > 10) {
		max /= 10;
		scale++;
	}
	
	if(profile_plugin->scale_mode == ScaleMode_Automatic) {  
		
		SetCtrlAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, ATTR_YLOOSE_FIT_AUTOSCALING_UNIT, scale);
		SetAxisScalingMode(profile_plugin->panel_id, PROF_PNL_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, 0.0, ((int)max+1)*pow(10,scale));

	}
	else {
		
		SetAxisScalingMode(profile_plugin->panel_id, PROF_PNL_GRAPH, VAL_LEFT_YAXIS,
			VAL_MANUAL, profile_plugin->manual_scale_min, profile_plugin->manual_scale_max);
	}

	
	
	//profile_plugin->profile_red_plot_handle = PlotXY (profile_plugin->panel_id, PROF_PNL_GRAPH, xdata,
	//		hist, len, VAL_DOUBLE, cvi_type,
	//		VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_DK_YELLOW);
	
	profile_plugin->profile_red_plot_handle = PlotXY (profile_plugin->panel_id, PROF_PNL_GRAPH, xdata,
		red_hist, len, VAL_DOUBLE, VAL_UNSIGNED_CHAR, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_RED);
	
	
	profile_plugin->profile_red_plot_handle = PlotXY (profile_plugin->panel_id, PROF_PNL_GRAPH, xdata,
		green_hist, len, VAL_DOUBLE, VAL_UNSIGNED_CHAR, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_GREEN);
	
	profile_plugin->profile_red_plot_handle = PlotXY (profile_plugin->panel_id, PROF_PNL_GRAPH, xdata,
		blue_hist, len, VAL_DOUBLE, VAL_UNSIGNED_CHAR, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_BLUE);
	
	
//	profile_plugin->profile_green_plot_handle = PlotY (profile_plugin->panel_id, PROF_PNL_GRAPH, green_hist, len, VAL_UNSIGNED_CHAR, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_GREEN);
//	profile_plugin->profile_blue_plot_handle = PlotY (profile_plugin->panel_id, PROF_PNL_GRAPH, blue_hist, len, VAL_UNSIGNED_CHAR, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_BLUE);

	free(xdata);
	free(red_hist);
	free(green_hist);
	free(blue_hist);

	return GCI_IMAGING_SUCCESS;

Error:
	GCI_ImagingWindow_DisableLineTool(plugin->window); 
	GCI_ImagingWindow_UnLockProfileButton(plugin->window);  	

	return GCI_IMAGING_ERROR;
}


static int GCI_ImageWindow_ExportProfileData(ProfilePlugin *profile_plugin, char *filepath)
{
	int i, bytes_per_pixel, number_of_points, fsize, bpp, type;
	FILE *fp;
	BYTE *red_data_buffer, *green_data_buffer, *blue_data_buffer;
	
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin;
	IcsViewerWindow *window = plugin->window;
	
	bpp = FreeImage_GetBPP(window->panel_dib); 
	type = FreeImage_GetImageType(window->panel_dib);
	
	bytes_per_pixel = bpp / 8;
	
	// If the conf file does exist clear any read only bit 
	if (FileExists (filepath, &fsize)) {
	
    	SetFileAttrs (filepath, 0, -1, -1, -1);
	}

    fp = fopen (filepath, "w");
    
    if (fp == NULL)
    	return GCI_IMAGING_ERROR;
	
	GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle, ATTR_NUM_POINTS, &number_of_points); 
	
	red_data_buffer = malloc( bytes_per_pixel * number_of_points); 
	green_data_buffer = malloc( bytes_per_pixel * number_of_points); 
	blue_data_buffer = malloc( bytes_per_pixel * number_of_points); 
	
	if(FIA_IsGreyScale(window->panel_dib)) {
	
		if(bpp == 8) {
			GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle,
				ATTR_PLOT_YDATA, red_data_buffer);
		}
		else if(bpp == 16) {
			
			if(type == FIT_UINT16) {
				
				GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle,
					ATTR_PLOT_YDATA, (unsigned short *) red_data_buffer);	
			}
			else {
				
				GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle,
					ATTR_PLOT_YDATA, (short *) red_data_buffer);		
			}
			
			
		}
		else if(bpp == 32) {
			
			 if(type == FIT_UINT32) {    
			 	GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle,
					ATTR_PLOT_YDATA, (unsigned int *) red_data_buffer);
			 }
			 else if(type == FIT_INT32) {    
			 	GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle,
					ATTR_PLOT_YDATA, (int *) red_data_buffer);
			 }
			 else if(type == FIT_FLOAT) {    
			 	GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle,
					ATTR_PLOT_YDATA, (float *) red_data_buffer);
			 }
		}
		else if(bpp == 64) {
			
			GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle,
					ATTR_PLOT_YDATA, (double *) red_data_buffer);
		}
		

		for(i=0; i < number_of_points; i++) {
		
			if(bpp == 8)
				
				fprintf( fp, "%d\t%d\n", i, red_data_buffer[i] );    
			else if(bpp == 16) {
			
				if(type == FIT_UINT16)
					fprintf( fp, "%d\t%d\n", i, ((unsigned short *) red_data_buffer)[i] ); 
				else
					fprintf( fp, "%d\t%d\n", i, ((short *) red_data_buffer)[i] );  
				
			}
			else if(bpp == 32)
				fprintf( fp, "%d\t%f\n", i, ((float *) red_data_buffer)[i] );
		}

	}
	else {
	
		GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_red_plot_handle,
			ATTR_PLOT_YDATA, red_data_buffer);
		
		GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_green_plot_handle,
			ATTR_PLOT_YDATA, green_data_buffer);
		
		GetPlotAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, profile_plugin->profile_blue_plot_handle,
			ATTR_PLOT_YDATA, blue_data_buffer);
	
		for(i=0; i < number_of_points; i++) {
			fprintf( fp, "%d\t%d\t%d\t%d\n", i, red_data_buffer[i], green_data_buffer[i], blue_data_buffer[i] );  
		}
	}

    fclose(fp);
    
    // set read-only 
    SetFileAttrs (filepath, 1, -1, -1, -1);

	free(red_data_buffer);
	free(green_data_buffer);
	free(blue_data_buffer);

	return GCI_IMAGING_SUCCESS;
}


static void create_profile_ui(ProfilePlugin *profile_plugin)
{
    ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin;
    
	if(profile_plugin->panel_id == 0) {
	
		profile_plugin->panel_id = LoadPanel(0, uir_file_path, PROF_PNL);
		
		ics_viewer_set_panel_to_top_left_of_window(plugin->window, profile_plugin->panel_id);

		// Setup the close call back 
		if ( InstallCtrlCallback (profile_plugin->panel_id, PROF_PNL_QUIT, onProfileQuit, profile_plugin) < 0) {
			return;
		}
		
		if ( InstallCtrlCallback (profile_plugin->panel_id, PROF_PNL_EXPORT, onExportClicked, profile_plugin) < 0) {
			return;
		}

		if ( InstallCtrlCallback (profile_plugin->panel_id, PROF_PNL_FIX_SCALE, OnManualScaleButtonClicked, profile_plugin) < 0) {
			return;
		}
		
		//MoveWindowToOtherWindow
		//	(plugin->window->panel_id, profile_plugin->panel_id, 800, 300, MONITOR_POSITION | MONITOR_WORKAREA);
			
		SetPanelAttribute (profile_plugin->panel_id, ATTR_TITLE, "Profile");
		
		SetCtrlAttribute (profile_plugin->panel_id, PROF_PNL_GRAPH, ATTR_YNAME, "Intensity");
		SetAxisScalingMode (profile_plugin->panel_id, PROF_PNL_GRAPH, VAL_XAXIS, VAL_AUTOSCALE, 0.0, 0.0);
	
		if(profile_plugin->scale_mode == ScaleMode_Manual)
			SetCtrlVal(profile_plugin->panel_id, PROF_PNL_FIX_SCALE, 1);
		
	}
}
									

static int GdiPointEqual(POINT p1, POINT p2)
{
	if((p1.x == p2.x) && (p1.y == p2.y))
		return 1;
		
	return 0;
}

static void adjustPoints (ProfilePlugin *profile_plugin, POINT *image_p1, POINT *image_p2)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin;
	int width, height;
	
	// NB the image is upside down
	
	width = FreeImage_GetWidth(plugin->window->panel_dib);
	height = FreeImage_GetHeight(plugin->window->panel_dib);
	
	if (width != profile_plugin->current_image_size.x)
	{
		if (profile_plugin->current_image_size.x > 0) 
		{
			(*image_p1).x = RoundRealToNearestInteger((double)(*image_p1).x * (double)width / (double)profile_plugin->current_image_size.x);
			(*image_p2).x = RoundRealToNearestInteger((double)(*image_p2).x * (double)width / (double)profile_plugin->current_image_size.x);
		}
		profile_plugin->current_image_size.x = width;
	}
	
	if (height != profile_plugin->current_image_size.y)
	{
		if (profile_plugin->current_image_size.y > 0) 
		{
			(*image_p1).y = RoundRealToNearestInteger((double)(*image_p1).y * (double)height / (double)profile_plugin->current_image_size.y);
			(*image_p2).y = RoundRealToNearestInteger((double)(*image_p2).y * (double)height / (double)profile_plugin->current_image_size.y);
		}
		profile_plugin->current_image_size.y = height;
	}
}

static void display_profile_window(ProfilePlugin *profile_plugin, POINT *image_p1, POINT *image_p2)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin;
	int visible, bpp;
	FREE_IMAGE_TYPE type;
	
	if(profile_plugin->panel_id == 0)
		create_profile_ui(profile_plugin);

	adjustPoints (profile_plugin, image_p1, image_p2);
	
	if(GdiPointEqual((*image_p1), (*image_p2))) {
	
		#if _CVI_DEBUG_   
		
		BYTE val;
		char str[50];
	
		FreeImage_GetPixelIndex( plugin->window->panel_dib, (*image_p1).x, (*image_p1).y, &val);
		sprintf(str, "x %d y %d intensity %d\n", (*image_p1).x, (*image_p1).y, val);
		
		SetCtrlVal (profile_plugin->panel_id, PROF_PNL_PIXEL_VAL, str); 
		
		#endif     
		
		return;
	}
	
	// Don't draw profiles for lines less or equal to 1 pixel.	
	if( abs((*image_p2).x - (*image_p1).x) <= 1 && abs((*image_p2).y - (*image_p1).y) <= 1)
		return;
	
	// Reset Graph
	DeleteGraphPlot (profile_plugin->panel_id, PROF_PNL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
	
	if(plugin->window->microns_per_pixel != 1.0) 
		SetCtrlVal(profile_plugin->panel_id, PROF_PNL_X_LABEL, "um");
	else
		SetCtrlVal(profile_plugin->panel_id, PROF_PNL_X_LABEL, "pixels");
		
	bpp = FreeImage_GetBPP(plugin->window->panel_dib);
	type = FreeImage_GetImageType(plugin->window->panel_dib);

	if(type >= FIT_RGB16)
	{
		GCI_MessagePopup("Error", "Image type does not support profile");
		return;
	}

	if((bpp >= 24 && (type == FIT_BITMAP)) || type >= FIT_RGB16)
		display_colour_profile(profile_plugin, (*image_p1), (*image_p2)); 
		
	if((type > FIT_BITMAP && type < FIT_COMPLEX) || bpp == 8) {	
		display_greylevel_profile(profile_plugin, (*image_p1), (*image_p2));
	}

	GetPanelAttribute(profile_plugin->panel_id, ATTR_VISIBLE, &visible);

	if(!visible)
		DisplayPanel(profile_plugin->panel_id);
}


static void DisplayManualScalePanel(ProfilePlugin *profile_plugin)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin;
	IcsViewerWindow *window = plugin->window;
	double min, max;
	
	if(profile_plugin->manual_scale_panel > 0)
		return;
	
	profile_plugin->manual_scale_panel = LoadPanel(0, uir_file_path, MAN_SCALE);
		
	ics_viewer_set_panel_to_top_left_of_window(plugin->window, profile_plugin->manual_scale_panel);

	if ( InstallCtrlCallback (profile_plugin->manual_scale_panel, MAN_SCALE_OK_BUTTON, OnManualScaleOkButtonClicked, profile_plugin) < 0) {
		return;
	}
	
	if ( InstallCtrlCallback (profile_plugin->manual_scale_panel, MAN_SCALE_CANCEL_BUTTON, OnManualScaleCancelButtonClicked, profile_plugin) < 0) {
		return;
	}
	
	FIA_FindMinMax(window->panel_dib, &min, &max);

	if(profile_plugin->manual_scale_min == 0.0 && profile_plugin->manual_scale_max == 0.0)
	{
		SetCtrlVal(profile_plugin->manual_scale_panel, MAN_SCALE_AXIS_MIN, min);
		SetCtrlVal(profile_plugin->manual_scale_panel, MAN_SCALE_AXIS_MAX, max); 
	}
	else {
		
		SetCtrlVal(profile_plugin->manual_scale_panel, MAN_SCALE_AXIS_MIN, profile_plugin->manual_scale_min);
		SetCtrlVal(profile_plugin->manual_scale_panel, MAN_SCALE_AXIS_MAX, profile_plugin->manual_scale_max); 	
	}
		
	DisplayPanel(profile_plugin->manual_scale_panel);
}


static void CloseManualScalePanel(ProfilePlugin *profile_plugin)
{
    ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin;
  
	if(profile_plugin->manual_scale_panel > 0)
	{
	    ics_viewer_registry_save_panel_position(plugin->window, profile_plugin->manual_scale_panel);
	    
		DiscardPanel(profile_plugin->manual_scale_panel);
	
		profile_plugin->manual_scale_panel = 0;
	}
}


void GCI_ImagingWindow_CloseProfile (ProfilePlugin *profile_plugin)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin;
	IcsViewerWindow *window = plugin->window;
	
	// Close manual axis panel if displayed.
	CloseManualScalePanel(profile_plugin);  
	
	if (profile_plugin->panel_id <= 0)
		return;
		
	ics_viewer_registry_save_panel_position(plugin->window, profile_plugin->panel_id);
	    
	DiscardPanel(profile_plugin->panel_id);
	profile_plugin->panel_id = 0;
	
	// Set the line tool to off
	GCI_ImagingWindow_DisableLineTool(window);  
		
	// Update display if line tool is drawn.
	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));       
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
}


static void on_image_displayed(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) plugin; 
	
	int visible;
	
	if(!IsToolActive((Tool *) plugin->window->line_tool) && !IsToolLocked((Tool *) plugin->window->line_tool))
		return;
	
	if(profile_plugin->panel_id == 0)
		return;
	
	GetPanelAttribute(profile_plugin->panel_id, ATTR_VISIBLE, &visible);
	
	if(visible)
		display_profile_window(profile_plugin, &profile_plugin->point1, &profile_plugin->point2);   
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableProfile(IcsViewerWindow *window)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) window->profile_plugin;
	
	profile_plugin->show_profile_on_line_tool = 1;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableProfile(IcsViewerWindow *window)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) window->profile_plugin;

	profile_plugin->show_profile_on_line_tool = 0;
}

static void on_line_tool_drawn(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) plugin;

	profile_plugin->point1 = data1.point;
	profile_plugin->point2 = data2.point; 

	if(profile_plugin->show_profile_on_line_tool)
		display_profile_window(profile_plugin, &data1.point, &data2.point);

	// If the client has specified a function to call when a profile is drawn use that instead.
	if(GCI_Signal_IsConnected(UIMODULE_SIGNAL_TABLE(plugin->window), "Profile"))
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "Profile", GCI_VOID_POINTER, plugin->window, GCI_POINT, profile_plugin->point1, GCI_POINT, profile_plugin->point2);
}


static void on_tool_status_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	Tool *tool = (Tool *) data1.tool; 	
	ImageWindowPlugin *tool_plugin = (ImageWindowPlugin*) tool;    
	ProfilePlugin *profile_plugin = (ProfilePlugin *) plugin;       
	
	if(strcmp(tool_plugin->name, "LineTool") == 0 && !IsToolActive(tool) && !IsToolLocked(tool))
		GCI_ImagingWindow_CloseProfile (profile_plugin);  		
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) plugin; 
}


static void on_binning_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
//	GCI_ImagingWindow_DisableLineTool(plugin->window); 
//	GCI_ImagingWindow_UnLockProfileButton(plugin->window);  	
}


static void on_disk_file_loaded (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) plugin; 
	GCI_ImagingWindow_DisableLineTool(plugin->window);    	
	profile_plugin->current_image_size.x = 0;
	profile_plugin->current_image_size.y = 0;
}

ImageWindowPlugin* profile_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "ProfilePlugin", sizeof(ProfilePlugin)); 
	
	ProfilePlugin *profile_plugin = (ProfilePlugin *) plugin; 
	
	profile_plugin->panel_id = 0;
	profile_plugin->show_profile_on_line_tool = 1;
	profile_plugin->scale_mode = ScaleMode_Automatic;
	profile_plugin->manual_scale_panel = 0;
	profile_plugin->manual_scale = 0;
	profile_plugin->manual_scale_min = 0.0;
	profile_plugin->manual_scale_max = 0.0;
	
	profile_plugin->current_image_size.x = 0;
	profile_plugin->current_image_size.y = 0;
	
	PLUGIN_VTABLE(plugin, on_line_tool_drawn) = on_line_tool_drawn;
	PLUGIN_VTABLE(plugin, on_image_displayed) = on_image_displayed;  
	PLUGIN_VTABLE(plugin, on_tool_status_changed) = on_tool_status_changed;  
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	PLUGIN_VTABLE(plugin, on_binning_changed) = on_binning_changed;      
	PLUGIN_VTABLE(plugin, on_disk_file_loaded) = on_disk_file_loaded;    
	
	return plugin;
}


/* Callbacks */

static int CVICALLBACK onExportClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char *default_extensions = "*.dat;";
	char fname[GCI_MAX_PATHNAME_LEN] = ""; 
	char directory[GCI_MAX_PATHNAME_LEN] = ""; 
	ProfilePlugin *profile_plugin = (ProfilePlugin *) callbackData;
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) profile_plugin;   
	
	switch (event)
		{
		case EVENT_COMMIT:

			if (FileSelectPopup (GetDefaultDirectoryPath(plugin->window, directory), "*.dat", default_extensions,
				"Export Data As", VAL_OK_BUTTON, 0, 0, 1, 1, fname) <= 0) {
				return -1;
			}

			GCI_ImageWindow_ExportProfileData(profile_plugin, fname);

			break;
		}
		
	return 0;
}


static int CVICALLBACK onProfileQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			GCI_ImagingWindow_CloseProfile (profile_plugin);

			break;
		}
	return 0;
}


static int CVICALLBACK OnManualScaleButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) callbackData;
	int val;
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);
			
			if(val)
				DisplayManualScalePanel(profile_plugin) ;     
			else {
				
				profile_plugin->scale_mode = ScaleMode_Automatic;
			
				display_profile_window(profile_plugin, &profile_plugin->point1, &profile_plugin->point2); 
			}
			
			break;
		}
	return 0;
}


static int CVICALLBACK OnManualScaleOkButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:

			profile_plugin->scale_mode = ScaleMode_Manual;
			
			GetCtrlVal(profile_plugin->manual_scale_panel, MAN_SCALE_AXIS_MIN, &(profile_plugin->manual_scale_min));
			GetCtrlVal(profile_plugin->manual_scale_panel, MAN_SCALE_AXIS_MAX, &(profile_plugin->manual_scale_max)); 
			
			display_profile_window(profile_plugin, &profile_plugin->point1, &profile_plugin->point2); 
			
			CloseManualScalePanel(profile_plugin); 
			
			break;
		}
	return 0;
}


static int CVICALLBACK OnManualScaleCancelButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	ProfilePlugin *profile_plugin = (ProfilePlugin *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:

			CloseManualScalePanel(profile_plugin) ;     

			break;
		}
	return 0;
}
