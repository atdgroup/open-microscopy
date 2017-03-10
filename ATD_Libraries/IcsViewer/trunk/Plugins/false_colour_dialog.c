#include "icsviewer_window.h"
#include "icsviewer_uir.h"
#include "palette_plugin.h"

#include "ImageViewer.h"

#include "gci_utils.h"

#include "GL_CVIRegistry.h"

#include "string_utils.h"

#include "FreeImageAlgorithms.h"
#include <analysis.h>
#include <utility.h>
#include "FreeImageAlgorithms_Palettes.h"
#include "FreeImageAlgorithms_Utilities.h"

void ics_viewer_registry_save_panel_size_position(IcsViewerWindow *window, int panel_id);
void ics_viewer_set_panel_to_top_left_of_window(IcsViewerWindow *window, int panel_id);

static int GetColourFromWavelength(double wavelength)
{
	double red, green, blue;
	
	//Get RGB components for false colour image for a particular wavelength
	//The numbers come from a fortran program that BV found on the internet

	if ((wavelength >= 380) && (wavelength < 440)) {
		red = (440 - wavelength)/(440 - 380) * 255;
		green = 0;
		blue = 255;
	}
	else if ((wavelength >= 440) && (wavelength < 490)) {
		red = 0;
		green = (wavelength - 440)/(490 - 440) * 255;
		blue = 255;
	}
	else if ((wavelength >= 490) && (wavelength < 510)) {
		red = 0;
		green = 255;
		blue = (510 - wavelength )/(510 - 490) * 255;
	}
	else if ((wavelength >= 510) && (wavelength < 580)) {
		red = (wavelength - 510)/(580 - 510) * 255;
		green = 255;
		blue = 0;
	}
	else if ((wavelength >= 580) && (wavelength < 645)) {
		red = 255;
		green = (645 - wavelength )/(645 - 580) * 255;
		blue = 0;
	}
	else if ((wavelength >= 645) && (wavelength < 780)) {
		red = 255;
		green = 0;
		blue = 0;
	}
	else {		  //display as mono
	
		red = 255;
		green = 255;
		blue = 255;
	}
	
	return MakeColor((unsigned int) red, (unsigned int)green, (unsigned int) blue);
}

static int GCI_ImagingWindow_DrawWavelengthScale(PalettePlugin* palette_plugin)
{
	Rect scale_rect, temp;

	int width, height, i, wavelength_section_top;
	int wavelength_colour;
	int pixel_size = 0;
	double wavelength;
  	
	GetCtrlAttribute(palette_plugin->panel_id, palette_plugin->canvas_id, ATTR_WIDTH, &width);
	GetCtrlAttribute(palette_plugin->panel_id, palette_plugin->canvas_id, ATTR_HEIGHT, &height);

	scale_rect.left = 0;
	scale_rect.top = 0;
	scale_rect.width = width;
	scale_rect.height = height;
	
	temp.left = scale_rect.left;
	temp.width = scale_rect.width;
	
	wavelength_section_top = 0;
	
	for(i = 0; i < height; i++) {

		wavelength = (( (double) i / height) * 390.0) + 390.0;
		
		wavelength_colour = GetColourFromWavelength(wavelength);
		
		temp.top = scale_rect.top + i;
        temp.height = temp.top + 1;
      	
    	SetCtrlAttribute(palette_plugin->panel_id, palette_plugin->canvas_id, ATTR_PEN_FILL_COLOR, wavelength_colour);
    
  		CanvasDrawRect (palette_plugin->panel_id, palette_plugin->canvas_id, temp, VAL_DRAW_INTERIOR);   
    
     	wavelength_section_top += pixel_size;
	 }

     return 0;
}



static int CVICALLBACK OnOkButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	PalettePlugin* palette_plugin = (PalettePlugin*) callbackData; 
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) palette_plugin;  
	double wavelength;
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			GetCtrlAttribute(palette_plugin->panel_id, WAVE_PNL_WAVELENGTH_NUMERIC, ATTR_CTRL_VAL, &wavelength);
	
			FIA_GetFalseColourPalette(plugin->window->current_palette, palette_plugin->wavelength); 
	
			ImageWindow_SetPalette(plugin->window);  
	
			ics_viewer_registry_save_panel_size_position(plugin->window, palette_plugin->panel_id);

			DiscardPanel(palette_plugin->panel_id);

			break;
		 }
		 
	return 0;
}


static int CVICALLBACK OnFalseColourChosen (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	PalettePlugin* palette_plugin = (PalettePlugin*) callbackData;
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) palette_plugin;  

	switch (event)
	{
		case EVENT_LEFT_CLICK:
		{
			int y, top, width, height;
			
			GetCtrlAttribute(palette_plugin->panel_id, palette_plugin->canvas_id, ATTR_TOP, &top);
			
			y = eventData1 - top;
			
			GetCtrlAttribute(palette_plugin->panel_id, palette_plugin->canvas_id, ATTR_WIDTH, &width);
			GetCtrlAttribute(palette_plugin->panel_id, palette_plugin->canvas_id, ATTR_HEIGHT, &height);
	
			palette_plugin->last_user_chosen_wavength = palette_plugin->wavelength = (( (double) y / height) * 390.0) + 390.0;  
			
			SetCtrlVal(palette_plugin->panel_id, WAVE_PNL_WAVELENGTH_NUMERIC, palette_plugin->wavelength); 
	
			FIA_GetFalseColourPalette(plugin->window->current_palette, palette_plugin->wavelength);       
			
			ImageWindow_SetPalette(plugin->window); 
			
			break;
		}
	}
		 
	return 0;
}


static int CVICALLBACK OnNumericChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	PalettePlugin* palette_plugin = (PalettePlugin*) callbackData;
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) palette_plugin;  

	switch (event)
	{
		case EVENT_COMMIT:
		{
			GetCtrlVal(palette_plugin->panel_id, WAVE_PNL_WAVELENGTH_NUMERIC, &(palette_plugin->wavelength)); 
	
			FIA_GetFalseColourPalette(plugin->window->current_palette, palette_plugin->wavelength);       
			
			ImageWindow_SetPalette(plugin->window); 
			
			break;
		}
	}
		 
	return 0;
}


void false_colour_dialog_create(PalettePlugin* palette_plugin)
{
	HWND panel_window_handle;
	int window_handle;
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) palette_plugin; 

	palette_plugin->panel_id = LoadPanel(0, uir_file_path, WAVE_PNL); 
	
	ics_viewer_set_panel_to_top_left_of_window(plugin->window, palette_plugin->panel_id);

	GetPanelAttribute (palette_plugin->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);	
		
	panel_window_handle = (HWND) window_handle;	
	
	/* Setup the change call back */
	if ( InstallCtrlCallback (palette_plugin->panel_id, WAVE_PNL_WAVELENGTH_NUMERIC, OnNumericChanged, palette_plugin) < 0) {
		return;
	}
	
	/* Setup the ok call back */
	if ( InstallCtrlCallback (palette_plugin->panel_id, WAVE_PNL_OK_BUTTON, OnOkButtonClicked, palette_plugin) < 0) {
		return;
	}

	palette_plugin->canvas_id = NewCtrl (palette_plugin->panel_id, CTRL_CANVAS, 0, 20, 20);
	
	SetCtrlAttribute(palette_plugin->panel_id, palette_plugin->canvas_id, ATTR_WIDTH, 70);
	SetCtrlAttribute(palette_plugin->panel_id, palette_plugin->canvas_id, ATTR_HEIGHT, 235);

	GCI_ImagingWindow_DrawWavelengthScale(palette_plugin);

	if ( InstallCtrlCallback (palette_plugin->panel_id, palette_plugin->canvas_id, OnFalseColourChosen, palette_plugin) < 0) {
		return;
	}

	SetCtrlVal(palette_plugin->panel_id, WAVE_PNL_WAVELENGTH_NUMERIC, palette_plugin->wavelength); 
	
	FIA_GetFalseColourPalette(plugin->window->current_palette, palette_plugin->wavelength);       
			
	ImageWindow_SetPalette(plugin->window); 

	DisplayPanel(palette_plugin->panel_id);
}   
