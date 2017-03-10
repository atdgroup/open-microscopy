#ifndef __MOSIAC_WINDOW__
#define __MOSIAC_WINDOW__

#include "icsviewer_signals.h"
#include "freeimage_imaq.h"

typedef struct
{
	GCIWindow *window; 
	IPIImageRef mosaic_image;
	IPIPixelType pixeltype;
	
	int	mosaic_window_id;
	char *header_filepath; 
	
	double x_percentage_overlap;
	double y_percentage_overlap;
	
	double x_overlap;
	double y_overlap;

	double um_per_pixel;
	
	int	composite_image_created;
	int tile_width;
	int tile_height;
	
	int screen_height;
	int screen_width;
	
	int number_of_cols;
	int number_of_rows;
	
} MosaicWindow;


MosaicWindow* mosaic_window_new(int left, int top, int width, int height);

void mosaic_window_set_pixel_type(MosaicWindow *mosaic_window, IPIPixelType pixelType) ;

void mosaic_window_set_row_and_col_size(MosaicWindow *mosaic_window, int number_of_cols, int number_of_rows);

void mosaic_window_set_overlap(MosaicWindow *mosaic_window, double x_percentage_overlap, double y_percentage_overlap);

void mosaic_window_destroy(MosaicWindow *mosaic_window);

void mosaic_window_hide(MosaicWindow *mosaic_window);

void mosaic_window_add_image(MosaicWindow *mosaic_window, IPIImageRef image, int col, int row);

void mosaic_window_show(MosaicWindow *mosaic_window);

void mosaic_window_clear(MosaicWindow *mosaic_window);

void mosaic_window_update(MosaicWindow *mosaic_window);

#endif
