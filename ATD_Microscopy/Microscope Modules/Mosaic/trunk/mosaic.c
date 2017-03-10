#include "mosaic.h"
#include "gci_ui_module.h"
#include "dictionary.h" 
#include "microscope.h"  
#include "GL_CVIRegistry.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Drawing.h"
#include "FreeImageAlgorithms_Palettes.h"
#include "FreeImageAlgorithms.h"

#include "string_utils.h"
#include "toolbox.h"
#include <utility.h>

const int mosaic_image_size = 2000;

////////////////////////////////////////////////////////////////////////////
//RJL June 2006
//GCI Microscopy development. 
//Mosaic image module - adapted from Microfocus version by GP
////////////////////////////////////////////////////////////////////////////

static void AddKeyValueToTree(int panel, int ctrl, const char *key, const char *value)
{
	int index = InsertTreeItem (panel, ctrl, VAL_SIBLING, 0, VAL_LAST, key, 0, 0, 0);

	SetTreeCellAttribute (panel, ctrl, index, 1, ATTR_LABEL_TEXT, key);
	SetTreeCellAttribute (panel, ctrl, index, 2, ATTR_LABEL_TEXT, value);
}

static dictionary* get_mosaic_metadata(Microscope* ms, IcsViewerWindow* window, MosaicWindow *mosaic_window)
{
	dictionary *d = microscope_get_metadata(ms, window);   	
	
	char buffer1[500] = "";
	FIBITMAP *fib = GCI_ImagingWindow_GetDisplayedFIB(window); 

	int width = FreeImage_GetWidth(fib);
	int height = FreeImage_GetHeight(fib);

	double microns_per_pixel = GCI_ImagingWindow_GetMicronsPerPixelFactor(window);

	// Overwrite the camera image extents
	sprintf(buffer1, "%.3e %.3e", width * microns_per_pixel * 1e-6, height * microns_per_pixel * 1e-6);  
	dictionary_set(d, "extents", buffer1);   

	sprintf(buffer1, "%.3e", width * microns_per_pixel * 1e-6); 
	dictionary_set(d, "image physical_sizex", buffer1);

	sprintf(buffer1, "%.3e", height * microns_per_pixel * 1e-6); 
	dictionary_set(d, "image physical_sizey", buffer1);

	// Mosaic Image Width x Height
	 
	sprintf(buffer1, "%d x %d", width, height); 
	dictionary_set(d, "dimensions", buffer1);
	
	// Tile Dimensions
	sprintf(buffer1, "%d", mosaic_window->region.left);   
	dictionary_set(d, "Region Left", buffer1);  
	
	sprintf(buffer1, "%d", mosaic_window->region.top);   
	dictionary_set(d, "Region Top", buffer1);  
	
	sprintf(buffer1, "%d", mosaic_window->region.width);   
	dictionary_set(d, "Region Width", buffer1);

	sprintf(buffer1, "%d", mosaic_window->region.height);   
	dictionary_set(d, "Region Height", buffer1);  
	
	return d;
}

struct panel_ctrl_data
{
	int panel;
	int ctrl;
};

static void dictionary_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	struct panel_ctrl_data * pcd = (struct panel_ctrl_data*) data;
	
	AddKeyValueToTree(pcd->panel, pcd->ctrl, key, val);          	
}

static void dictionary_ics_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	ICS *ics = (ICS *) data;
	
	FreeImageIcs_IcsAddHistoryString (ics, key, val);         	
}


void mosaic_save_metadata(GCIWindow *window, char *filename, char *extension, void* callback)
{
	MosaicWindow *mosaic_window = (MosaicWindow *) callback;  
	
	Microscope *ms = microscope_get_microscope();    
	dictionary* d = get_mosaic_metadata(ms, window, mosaic_window);   
	ICS *ics = NULL;
	
	if(strcmp(extension, ".ics"))
		return;
	
	FreeImageIcs_IcsOpen(&ics, filename, "rw");  
	
	dictionary_foreach(d, dictionary_ics_keyval_callback, ics);
	
	dictionary_del(d);
	FreeImageIcs_IcsClose (ics);  
}

static void mosaic_on_show_metadata (IcsViewerWindow* window, int panel, int ctrl, void* callback)
{
	Microscope *ms = microscope_get_microscope();   
	MosaicWindow *mosaic_window = (MosaicWindow *) callback;
	
	dictionary* d = get_mosaic_metadata(ms, window, mosaic_window);   
		
	struct panel_ctrl_data * pcd = (struct panel_ctrl_data*) malloc(sizeof(struct panel_ctrl_data));

	pcd->panel = panel;
	pcd->ctrl = ctrl;
	
	dictionary_foreach(d, dictionary_keyval_callback, pcd);
	
	free(pcd);
	dictionary_del(d);
}

int mosaic_window_get_panel_id(MosaicWindow *mosaic_window)
{
	return GCI_ImagingWindow_GetPanelID(mosaic_window->window);
}

static void mosaic_registry_save_window_position(MosaicWindow *mosaic_window)
{
  char subkey[200];
  char key[500];
  char panel_define[100] = "";
  
  if(mosaic_window->mosaic_window_id <= 0)
	return;
		
  ics_viewer_get_registry_subkey(mosaic_window->window, subkey);
  
  sprintf(key, "%sMosaic", subkey);
   
  RegistrySavePanelPosition (REGKEY_HKCU, subkey, mosaic_window->mosaic_window_id); 
}

static void mosaic_registry_read_window_position(MosaicWindow *mosaic_window)
{
  char subkey[200];
  char key[500];
  char panel_define[100] = "";
  
  if(mosaic_window->mosaic_window_id <= 0)
	return;
		
  ics_viewer_get_registry_subkey(mosaic_window->window, subkey);
  
  sprintf(key, "%sMosaic", subkey);
  
  RegistryReadPanelPosition (REGKEY_HKCU, subkey, mosaic_window->mosaic_window_id); 
}

MosaicWindow* mosaic_window_new(const char* unique_id, int left, int top, int width, int height)
{
	MosaicWindow *mosaic_window = (MosaicWindow *) malloc (sizeof(MosaicWindow));

	memset(mosaic_window, 0, sizeof(MosaicWindow));

 	mosaic_window->window = GCI_ImagingWindow_CreateAdvanced(unique_id, "Mosaic Image", left, top, width, height, 0, 0);

	GCI_ImagingWindow_Initialise(mosaic_window->window);
	GCI_ImagingWindow_SignalOnlyOnCloseOrExit(mosaic_window->window); 
	
	mosaic_window->mosaic_window_id = GCI_ImagingWindow_GetPanelID(mosaic_window->window);
	
	mosaic_window->mosaic_image = NULL;
	mosaic_window->window_visible = 0;
	mosaic_window->microns_per_pixel = 1.0;
	mosaic_window->max_intensity = 0.0;
	
	// Set the routine that save metadata so the window knows which meta data to save
	GCI_ImagingWindow_SetSaveHandler(mosaic_window->window, mosaic_save_metadata, mosaic_window);
	GCI_ImagingWindow_SetMetaDataProviderCallback(mosaic_window->window, mosaic_on_show_metadata, mosaic_window);
	
	GCI_ImagingWindow_SetResizeFitStyle(mosaic_window->window); 
		
	GCI_ImagingWindow_HideToolBar(mosaic_window->window); 
	GCI_ImagingWindow_HidePaletteBar(mosaic_window->window);      
	
	mosaic_registry_read_window_position(mosaic_window);
	
	GCI_ImagingWindow_SetWindowTitle(mosaic_window->window, "Mosaic");
	
	return mosaic_window;
}

void mosaic_window_set_title(MosaicWindow *mosaic_window, char *title)
{
	GCI_ImagingWindow_SetWindowTitle(mosaic_window->window, title);
}

void mosaic_window_setup(MosaicWindow *mosaic_window, FREE_IMAGE_TYPE pixelType, int bpp, Rect region)
{
	int w, h;
	double image_aspect_ratio = (double)region.width / region.height;
		
	mosaic_window->region = region;
	mosaic_window->pixel_type = pixelType;
	mosaic_window->bpp = bpp;

	// Ok the region maps to an image that has to be the same aspect ratio.
	// We keep the larger dimension pinned to 2000 pixels.
	if(region.width > region.height) {
		w = mosaic_image_size;
		h = (int) (w / image_aspect_ratio + 0.5);
	}
	else {
		h = mosaic_image_size;
		w = (int) (h * image_aspect_ratio + 0.5);
	}

	if(mosaic_window->mosaic_image != NULL)
		FreeImage_Unload(mosaic_window->mosaic_image);

	mosaic_window->mosaic_image = FreeImage_AllocateT(mosaic_window->pixel_type,
		w, h, mosaic_window->bpp, 0, 0, 0);     

	GCI_ImagingWindow_SetMicronsPerPixelFactor(mosaic_window->window, region.width/(double)w);

	if(mosaic_window->mosaic_image == NULL) {
		GCI_MessagePopup("Error", "Failed to create mosaic window");
		return;
	}
}

int mosaic_window_setup_from_ics_file(MosaicWindow *mosaic_window, const char *filepath)
{
	ICS *ics;
	char buffer[ICS_LINE_LENGTH] = "";

	memset(buffer, 0, ICS_LINE_LENGTH);

	if(FreeImageIcs_IcsOpen (&ics, filepath, "r") != IcsErr_Ok)
		return -1;

	if(FreeImageIcs_GetFirstIcsHistoryValueWithKey(ics, "region left", buffer) == FIA_ERROR) {
		goto FAIL;
	}

	sscanf(buffer, "%d", &(mosaic_window->region.left));

	memset(buffer, 0, ICS_LINE_LENGTH);
	if(FreeImageIcs_GetFirstIcsHistoryValueWithKey(ics, "region top", buffer) == FIA_ERROR) {
		goto FAIL;
	}

	sscanf(buffer, "%d", &(mosaic_window->region.top));

	memset(buffer, 0, ICS_LINE_LENGTH);
	if(FreeImageIcs_GetFirstIcsHistoryValueWithKey(ics, "region width", buffer) == FIA_ERROR) {
		goto FAIL;
	}

	sscanf(buffer, "%d", &(mosaic_window->region.width));

	memset(buffer, 0, ICS_LINE_LENGTH);
	if(FreeImageIcs_GetFirstIcsHistoryValueWithKey(ics, "region height", buffer) == FIA_ERROR) {
		goto FAIL;
	}

	sscanf(buffer, "%d", &(mosaic_window->region.height));

	if(mosaic_window->mosaic_image != NULL) {
		FreeImage_Unload(mosaic_window->mosaic_image);
		mosaic_window->mosaic_image = NULL;
	}

	mosaic_window->mosaic_image = FreeImageIcs_LoadFIBFromIcsFile(ics);   

	FreeImageIcs_IcsClose(ics);

	mosaic_window->pixel_type = FreeImage_GetImageType(mosaic_window->mosaic_image);
	mosaic_window->bpp = FreeImage_GetBPP(mosaic_window->mosaic_image);

	if(mosaic_window->mosaic_image == NULL) {
		GCI_MessagePopup("Error", "Failed to create mosaic window");
		return -1;
	}

	GCI_ImagingWindow_SetMicronsPerPixelFactor(mosaic_window->window, mosaic_window->region.width/(double)FreeImage_GetWidth(mosaic_window->mosaic_image));

	mosaic_window_update(mosaic_window);

	return 0;

FAIL:

	GCI_MessagePopup("Error", "Failed to load previous region info. Incorrect format");

	FreeImageIcs_IcsClose(ics);   

	return -1;
}


void mosaic_window_set_microns_per_pixel(MosaicWindow *mosaic_window, double microns_per_pixel) 
{
	mosaic_window->microns_per_pixel = microns_per_pixel;	
}

void mosaic_window_destroy(MosaicWindow *mosaic_window)
{
	if (mosaic_window->mosaic_image != NULL) {
		FreeImage_Unload(mosaic_window->mosaic_image);
		mosaic_window->mosaic_image = NULL;
	}
	
	mosaic_registry_save_window_position(mosaic_window);
	
	if(mosaic_window->window != NULL) {
		GCI_ImagingWindow_DestroyWindow(mosaic_window->window);
		mosaic_window->window = NULL;
	}

  	free(mosaic_window);
  	mosaic_window = NULL;
}

void mosaic_window_hide(MosaicWindow *mosaic_window)
{
	if (mosaic_window == NULL)
		return;
	
	if (mosaic_window->window == NULL)
		return;
	
	mosaic_registry_save_window_position(mosaic_window);
	
	GCI_ImagingWindow_Hide(mosaic_window->window);
	
	mosaic_window->window_visible = 0;
}


void mosaic_window_show(MosaicWindow *mosaic_window)
{
	mosaic_window->window_visible = 1;
	mosaic_registry_read_window_position(mosaic_window);
	GCI_ImagingWindow_Show(mosaic_window->window); 
}

void mosaic_window_update(MosaicWindow *mosaic_window)
{
	if(mosaic_window == NULL || mosaic_window->mosaic_image == NULL)
		return;
	
	if(!(mosaic_window->window_visible))
		return;
	
	GCI_ImagingWindow_LoadImage(mosaic_window->window, mosaic_window->mosaic_image);
}


void mosaic_window_clear(MosaicWindow *mosaic_window)
{
	// Repaint Background
	if(FIA_IsGreyScale(mosaic_window->mosaic_image)) {
		FIA_DrawSolidGreyscaleRect(mosaic_window->mosaic_image,
			MakeFIARect(0,0, FreeImage_GetWidth(mosaic_window->mosaic_image), FreeImage_GetHeight(mosaic_window->mosaic_image)),
			0.0);
	}
	else {
	   FIA_DrawColourSolidRect(mosaic_window->mosaic_image,
			MakeFIARect(0,0, FreeImage_GetWidth(mosaic_window->mosaic_image), FreeImage_GetHeight(mosaic_window->mosaic_image)),
				FIA_RGBQUAD(0,0,0));
	}
}

Point mosaic_window_translate_region_point_to_image_point(MosaicWindow *mosaic_window, Point region_pt)
{
	Point pt;

	int mosaic_width = FreeImage_GetWidth(mosaic_window->mosaic_image);
	int mosaic_height = FreeImage_GetHeight(mosaic_window->mosaic_image);

	pt.x = (int) (((double)(region_pt.x - mosaic_window->region.left) / (double)mosaic_window->region.width) * mosaic_width);
	pt.y = (int) (((double)(region_pt.y - mosaic_window->region.top) / (double)mosaic_window->region.height) * mosaic_height);

	return pt;
}

static int TranslateRegionToImage(MosaicWindow *mosaic_window, Rect region, Rect *translated_region)
{
	Rect ir;

	int mosaic_width = FreeImage_GetWidth(mosaic_window->mosaic_image);
	int mosaic_height = FreeImage_GetHeight(mosaic_window->mosaic_image);

	translated_region->width = (int) (((double) region.width /  mosaic_window->region.width) * mosaic_width);
	translated_region->height = (int) (((double) region.height / mosaic_window->region.height) * mosaic_height);

	if(translated_region->width == 0 || translated_region->height == 0)
		return REALTIME_OVERVIEW_ERROR;

	// We are not in the region. Return Error
	if(RectIntersection(region, mosaic_window->region, &ir) == 0)
		return REALTIME_OVERVIEW_ERROR;

	translated_region->left = (int) (((double)(region.left - mosaic_window->region.left) / mosaic_window->region.width) * mosaic_width);
	translated_region->top = (int) (((double)(region.top -  mosaic_window->region.top) / mosaic_window->region.height) * mosaic_height);

	return REALTIME_OVERVIEW_SUCCESS;
}

Rect mosaic_window_get_region(MosaicWindow *mosaic_window)
{
	return mosaic_window->region;
}

// Changes the region represemted with erases any previously added tiles
void mosaic_window_change_region(MosaicWindow *mosaic_window, Rect new_region)
{
	FIBITMAP *dib = NULL, *old_section_dib = NULL, *old_mosaic_dib = NULL;
	Rect intersection_rect, old_region;
	Rect irect_rel_to_new, irect_image_old, irect_image_new;
	int mosaic_width = FreeImage_GetWidth(mosaic_window->mosaic_image);
	int mosaic_height = FreeImage_GetHeight(mosaic_window->mosaic_image);

	if(mosaic_window->mosaic_image == NULL)
		return;

	RectIntersection(mosaic_window->region, new_region, &intersection_rect);

	old_region = mosaic_window->region;

	irect_rel_to_new = SetRectRelativeToXY(&intersection_rect, new_region.left, new_region.top);

	old_mosaic_dib = FreeImage_Clone(mosaic_window->mosaic_image);

	if(old_mosaic_dib == NULL)
		goto FUNC_ERROR;

	FIA_SetGreyLevelPalette(old_mosaic_dib);

	if(TranslateRegionToImage(mosaic_window, intersection_rect, &irect_image_old) == REALTIME_OVERVIEW_ERROR)
		goto FUNC_ERROR;

	mosaic_window_setup(mosaic_window, mosaic_window->pixel_type, mosaic_window->bpp, new_region);

	old_section_dib = FreeImage_Copy(old_mosaic_dib, irect_image_old.left, irect_image_old.top,
		irect_image_old.left + irect_image_old.width - 1, irect_image_old.top + irect_image_old.height - 1);

	FIA_SetGreyLevelPalette(old_section_dib);

	if(old_section_dib == NULL)
		goto FUNC_ERROR;

	if(TranslateRegionToImage(mosaic_window, intersection_rect, &irect_image_new) == REALTIME_OVERVIEW_ERROR)
		goto FUNC_ERROR;

	if(irect_image_new.left > mosaic_width)
		goto FUNC_ERROR;

	if(irect_image_new.top > mosaic_height)
		goto FUNC_ERROR;

	// We now scale this to the size of irect_image_new
	dib = FreeImage_Rescale(old_section_dib, irect_image_new.width, irect_image_new.height, FILTER_BOX);

	FIA_SetGreyLevelPalette(dib);
	
	if(dib == NULL)
		goto FUNC_ERROR;

	mosaic_window_clear(mosaic_window);

	if(FIA_PasteFromTopLeft(mosaic_window->mosaic_image, dib, irect_image_new.left, irect_image_new.top) == FIA_ERROR) {
		GCI_MessagePopup("Error", "Failed to add tile to mosaic");
		return;
	}

	FreeImage_Unload(dib);
	FreeImage_Unload(old_section_dib);

	return; 

FUNC_ERROR:

	if(dib != NULL)
		FreeImage_Unload(dib);

	if(old_section_dib != NULL)
		FreeImage_Unload(old_section_dib);
}


// This adds and image where x and y are in microns !
int mosaic_window_add_image(MosaicWindow *mosaic_window, FIBITMAP* image, double x, double y)
{
	FIBITMAP* tile=NULL;
	Rect tile_region_rect, tile_image_rect;

	FREE_IMAGE_TYPE type, mosaic_image_type;

	tile_region_rect.left = (int) x;
	tile_region_rect.top = (int) y;
	tile_region_rect.width = (int) (FreeImage_GetWidth(image) * mosaic_window->microns_per_pixel);
	tile_region_rect.height = (int) (FreeImage_GetHeight(image) * mosaic_window->microns_per_pixel);

	if(mosaic_window->mosaic_image == NULL) {
		GCI_MessagePopup("Error", "Failed to create mosaic window");
		return MOSAIC_ERROR_WINDOW_CREATE_FAILED;
	}

	if(tile_region_rect.width == 0 || tile_region_rect.height == 0) {
		GCI_MessagePopup("Mosaic Error", "Mosaic tile has zero size");
		return MOSAIC_ERROR_TILE_ZERO_SIZE;
	}

	type = FreeImage_GetImageType(image);
	mosaic_image_type = FreeImage_GetImageType(mosaic_window->mosaic_image);

	if(type != mosaic_image_type) {
		GCI_MessagePopup("Mosaic Error", "Tile does not have the same image type as the mosaic image");
		return MOSAIC_ERROR_DIFF_IMAGE_TYPES;
	}

	if(TranslateRegionToImage(mosaic_window, tile_region_rect, &tile_image_rect) == REALTIME_OVERVIEW_ERROR)
		return 0;

	// Resizes the image to the new image tile
	tile = FreeImage_Rescale(image, tile_image_rect.width, tile_image_rect.height, FILTER_BOX);   
	
	if(tile == NULL) {
		GCI_MessagePopup("Mosaic Error", "FreeImage_Rescale returned NULL, requested width and height was %d,%d", tile_image_rect.width, tile_image_rect.height);	
		return -1;
	}

	if(FIA_PasteFromTopLeft(mosaic_window->mosaic_image, tile, tile_image_rect.left, tile_image_rect.top) == FIA_ERROR) {
		return MOSAIC_ERROR_PASTE_TILE_FAIL;
	}

	FreeImage_Unload(tile);

	return 0;
}