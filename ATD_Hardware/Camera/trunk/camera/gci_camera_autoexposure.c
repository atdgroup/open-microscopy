#include "HardWareTypes.h"

#include "camera\gci_camera.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Statistics.h"

typedef struct 
{
    unsigned long min_val;
    unsigned long max_val;
    unsigned long average_val;
    unsigned long average_intensity;
	
} HistogramInfo;


static int CalculateIntensity(FIBITMAP *dib, HistogramInfo *info)
{
    int i, count;
	double saturation_tolerance = 0.2;
    int bpp = FreeImage_GetBPP(dib);
    int width = FreeImage_GetWidth(dib);
    int height = FreeImage_GetHeight(dib);
    int number_of_pixels = width * height;
    int sat_tol_in_pixels = 0; // The number of pixels coresponding to the saturation tolerance percentage.
    unsigned long *hist = NULL;
    int max_possible_value = (int) (pow(2, bpp) - 1);
    
    sat_tol_in_pixels = (int) (number_of_pixels * saturation_tolerance / 100.0);
    
    // Create a histogram of the image.
    hist = (long *) malloc(sizeof(long) * (max_possible_value + 1));
    
    // Build the histogram
    if(FIA_Histogram(dib, 0, max_possible_value, max_possible_value + 1, hist) == FIA_ERROR)
    {
		free(hist);
		return CAMERA_ERROR;
	}
		
    // Find the average pixel within the lower saturation tolerance band.
	i = 0;
	info->min_val = 0;
	count = 0; 
	
    while (count < sat_tol_in_pixels)
	{
		count += hist[i];
		info->min_val += (i * hist[i]);
		i++;
	}
	
    info->min_val /= count;	
    
    //  Find the average pixel within the higher saturation tolerance band.
	i = max_possible_value;
    count = 0;
    info->max_val = 0;
	
	while (count < sat_tol_in_pixels && i >= 0)
	{
		count += hist[i];
		info->max_val += (i * hist[i]);
		
		i--;
	}
	
	if(count == 0)
	{
		free(hist);
		return CAMERA_ERROR;  
	}
	
    info->max_val /= count;
	
	free(hist); 
	
	return CAMERA_SUCCESS;
}


typedef enum {AE_GOOD, AE_LOW, AE_HIGH} AEPosition;


static AEPosition GetExposureMeasure(GciCamera* camera)
{
	FIBITMAP *dib = NULL; 
	HistogramInfo info;  
	int bpp, max_possible_value;
	
	dib = gci_camera_get_image(camera, NULL);   
	
	bpp = FreeImage_GetBPP(dib);
    max_possible_value = (int) (pow(2, bpp) - 1);

    CalculateIntensity(dib, &info);
	
	FreeImage_Unload(dib);
	
	// We want to get betwwen 70% and 95 % of max value.   
	if(info.max_val >= (0.7 * max_possible_value) && info.max_val < (0.95 * max_possible_value)) 
		return AE_GOOD;
	
	if(info.max_val < (0.7 * max_possible_value))
		return AE_LOW;
	
	if(info.max_val > (0.95 * max_possible_value))
		return AE_HIGH;
	
	return AE_GOOD;
}


static int gci_camera_find_autoexposure(GciCamera* camera, double min_exposure, double max_exposure, const int max_recusion_depth)
{
    AEPosition pos;
    static int recursion_depth = 0;
	
	double mid_exposure =  (min_exposure + max_exposure) / 2;

	// Unable to find a good exposure in a reasonable time.  
	if(recursion_depth > max_recusion_depth) {
		recursion_depth = 0;
		
		// We are finished was the final measure good enough ?
		pos = GetExposureMeasure(camera);   
		
		if(pos != AE_GOOD)
			return CAMERA_ERROR;  
		
		return CAMERA_SUCCESS;
	}
	
	if( recursion_depth == 0) {
	
		// If we are at the first step check the current image as it may not need adjustment.
		pos = GetExposureMeasure(camera); 
		
		if(pos == AE_GOOD) {
			recursion_depth = 0;
			return CAMERA_SUCCESS; 
		}
	}
	
	gci_camera_set_exposure_time(camera, mid_exposure);   
	gci_camera_snap_image(camera);   
	
	pos = GetExposureMeasure(camera);
	
	if(pos == AE_GOOD) {
		recursion_depth = 0;
		return CAMERA_SUCCESS; 	
	}
	
	// We are going to recurse
	recursion_depth++;
	
	if(pos == AE_LOW) // We must increase exposure
		return gci_camera_autoexposure(camera, mid_exposure + 1, max_exposure); 
	
	if(pos == AE_HIGH) // We are over exposed. We must reduce the exposure
		return gci_camera_autoexposure(camera, min_exposure, mid_exposure - 1); 	
	
	recursion_depth = 0;
	
	return CAMERA_ERROR;  
}


int gci_camera_autoexposure(GciCamera* camera, double min_exposure, double max_exposure)
{
	// Set recurse depth of 15
	return gci_camera_find_autoexposure(camera, min_exposure, max_exposure, 15);   
}
