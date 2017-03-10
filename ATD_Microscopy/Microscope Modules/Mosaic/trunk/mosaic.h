#ifndef __MOSIAC_WINDOW__
#define __MOSIAC_WINDOW__

#include "icsviewer_signals.h"
#include "FreeImage.h" 

#define MOSAIC_ERROR_WINDOW_CREATE_FAILED -2
#define MOSAIC_ERROR_TILE_ZERO_SIZE -3
#define MOSAIC_ERROR_DIFF_IMAGE_TYPES -4
#define MOSAIC_ERROR_PASTE_TILE_FAIL -5

typedef struct
{
	GCIWindow *window; 
	FIBITMAP* mosaic_image;
	FREE_IMAGE_TYPE pixel_type;
	int bpp;
	Rect region;

	int	mosaic_window_id;
	char *header_filepath; 
	double microns_per_pixel;  // This is the scale of the images to be added to the mosaic, not of the mosaic itself
	
	// Used to save to Regionscan sequence file.
	// Speeds up standalone stitcher program
	double max_intensity;	
	
	int window_visible;
	
} MosaicWindow;


MosaicWindow* mosaic_window_new(const char* unique_id, int left, int top, int width, int height);

void mosaic_window_set_title(MosaicWindow *mosaic_window, char *title);

void mosaic_window_setup(MosaicWindow *mosaic_window, FREE_IMAGE_TYPE pixelType, int bpp, Rect region);

int mosaic_window_setup_from_ics_file(MosaicWindow *mosaic_window, const char *filepath);

Rect mosaic_window_get_region(MosaicWindow *mosaic_window);

void mosaic_window_change_region(MosaicWindow *mosaic_window, Rect new_region);

Point mosaic_window_translate_region_point_to_image_point(MosaicWindow *mosaic_window, Point region_pt);

void mosaic_window_set_microns_per_pixel(MosaicWindow *mosaic_window, double microns_per_pixel);

void mosaic_window_set_overlap(MosaicWindow *mosaic_window, double x_percentage_overlap, double y_percentage_overlap);

void mosaic_window_destroy(MosaicWindow *mosaic_window);

void mosaic_window_hide(MosaicWindow *mosaic_window);

int mosaic_window_add_image(MosaicWindow *mosaic_window, FIBITMAP* image, double x, double y);

void mosaic_window_show(MosaicWindow *mosaic_window);

void mosaic_window_clear(MosaicWindow *mosaic_window);

void mosaic_window_update(MosaicWindow *mosaic_window);

double mosaic_window_get_max_intensity(MosaicWindow *mosaic_window);

int mosaic_window_get_panel_id(MosaicWindow *mosaic_window);

#endif
