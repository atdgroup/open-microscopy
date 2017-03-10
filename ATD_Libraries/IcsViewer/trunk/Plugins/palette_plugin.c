#include "palette_plugin.h"

#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_Palettes.h"
#include "FreeImageAlgorithms_Utilities.h"

#include "gci_utils.h"
#include "string_utils.h"
#include "gci_menu_utils.h"

#include "ImageViewer.h"

#include <analysis.h>
#include <utility.h>


// Reads palettes from the same formatted files that ImageJ uses 
static int GetPaletteFromFile(const char *filepath, RGBQUAD *palette)
{
	FILE *fp;
	int i, index, red, green, blue;
	char dummy[100];
	
	if (palette == NULL)
    {
        return FIA_ERROR;
    }

	fp = fopen(filepath, "r");
	
	if(fp == NULL)
		return FIA_ERROR;
	
	// Read Header Line
	fscanf(fp, "%s\t%s\t%s\t%s\n", dummy, dummy, dummy, dummy); 
	
    for(i = 0; i < 256; i++)
    {
		fscanf(fp, "%d\t%d\t%d\t%d\n", &index, &red, &green, &blue); 
		
        palette[i].rgbRed = red;
        palette[i].rgbGreen = green;
        palette[i].rgbBlue = blue;
    }
	
	fclose(fp);
	fp = NULL;
	
    return FIA_SUCCESS;
}

static void CVICALLBACK
	palette_on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	PaletteItem* item = (PaletteItem*) callbackData;         
	PalettePlugin* palette_plugin = item->palette_plugin; 
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) palette_plugin;  
	
	if(IS_MENU_CHECKED) {
	
		// Turn off
		Plugin_UncheckMenuItems(plugin); 
		Plugin_CheckMenuPathItem(plugin, "Display//Palettes//GreyLevel Palette");
		FIA_GetGreyLevelPalette(plugin->window->current_palette); 
		
	}
	else {
	
		BYTE sizes[3] = {10, 10, 10};
		
		if(item->filebased) {
			
			// We have a palette defined in a file lets read it
			GetPaletteFromFile(item->filepath, plugin->window->current_palette); 

		}
		else {
		
			if(menuItem == palette_plugin->false_colour_palette_menu_id)
				false_colour_dialog_create(palette_plugin);   	
		
			if(menuItem == palette_plugin->greylevel_palette_menu_id)
				FIA_GetGreyLevelPalette(plugin->window->current_palette);   
		
			if(menuItem == palette_plugin->overload_palette_menu_id)
				FIA_GetGreyLevelOverLoadPalette(plugin->window->current_palette);  	
		
			if(menuItem == palette_plugin->rainbow_palette_menu_id)
				FIA_GetRainBowPalette(plugin->window->current_palette);  
		
			if(menuItem == palette_plugin->reverse_rainbow_palette_menu_id)
				FIA_GetReverseRainBowPalette(plugin->window->current_palette);  
	
			if(menuItem == palette_plugin->temp_palette_menu_id)
				FIA_GetTemperaturePalette(plugin->window->current_palette);
			
			if(menuItem == palette_plugin->log_palette_menu_id)
				FIA_GetLogColourPalette(plugin->window->current_palette);  
		
			if(menuItem == palette_plugin->seismic_palette_menu_id)
				FIA_GetSeismicColourPalette(plugin->window->current_palette);
	
			if(menuItem == palette_plugin->pileup_palette_menu_id) {
				FIA_GetPileUpPalette(plugin->window->current_palette, FIA_RGBQUAD(255, 255, 0),
																				  FIA_RGBQUAD(0, 255, 0),
																		  		  FIA_RGBQUAD(0, 255, 255), sizes);      
			}
		}
		
		Plugin_UncheckMenuItems(plugin);
		SetMenuBarAttribute (menubar, menuItem, ATTR_CHECKED, 1);        
	}
	
	ImageWindow_SetPalette(plugin->window);  
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetFalseColourPaletteAsDefault(IcsViewerWindow *window, double wavelength)
{
	 Plugin_UncheckMenuItems(window->palette_plugin);
	 Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//False Colour Palette");  

	 palette_tool_set_false_colour_palette((PalettePlugin*)(window->palette_plugin), wavelength);      
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetPileUpPaletteAsDefault(IcsViewerWindow *window, RGBQUAD colour1, RGBQUAD colour2, RGBQUAD colour3, BYTE *sizes)
{
	 FIA_GetPileUpPalette(window->current_palette, colour1, colour2, colour3, sizes); 
	 
	 Plugin_UncheckMenuItems(window->palette_plugin);
	 Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//Pileup Palette");  
}



void IW_DLL_CALLCONV
GCI_ImagingWindow_SetDefaultPalette(IcsViewerWindow *window, PALETTE_TYPE palette)
{
	Plugin_UncheckMenuItems(window->palette_plugin);      
	
	switch(palette)
	{
		case GREYSCALE_PALETTE:
			FIA_GetGreyLevelPalette(window->current_palette);  
			Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//GreyLevel Palette");    
			break;
			
		case OVERLOAD_PALETTE:
			FIA_GetGreyLevelOverLoadPalette(window->current_palette);
			Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//Overload Palette");    
			break;
			
		case LOG_PALETTE:
			FIA_GetLogColourPalette(window->current_palette);   
			Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//Log Palette");    
			break;
			
		case RAINBOW_PALETTE:
			
			FIA_GetRainBowPalette(window->current_palette);
			Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//Rainbow Palette");    
			break;
			
		case REVERSE_RAINBOW_PALETTE:
			FIA_GetReverseRainBowPalette(window->current_palette);
			Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//Reverse Rainbow Palette");
			break;
			
		case SEISMIC_PALETTE:
			FIA_GetSeismicColourPalette(window->current_palette);
			Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//Seismic Palette");    
			break;
			
		case TEMPARATURE_PALETTE:
			FIA_GetTemperaturePalette(window->current_palette); 
			Plugin_CheckMenuPathItem(window->palette_plugin, "Display//Palettes//Temperature Palette");    
			break;
	}
}

static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	if(FIA_IsGreyScale(data1.dib)) {
		Plugin_UnDimMenuPathItem(plugin, "Display//Palettes"); 
		return 1;
	}
	else {
		Plugin_DimMenuPathItem(plugin, "Display//Palettes");
	}
		
 	return 0;
}


void palette_tool_set_false_colour_palette(PalettePlugin* palette_plugin, double wavelength)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) palette_plugin;
	
	if(palette_plugin == NULL)
		return;

	palette_plugin->wavelength = wavelength;
	
	if(!Plugin_IsMenuPathItemChecked(plugin, "Display//Palettes//False Colour Palette"))
		return;
				  
	if(wavelength > 0.0)
		FIA_GetFalseColourPalette(plugin->window->current_palette, palette_plugin->wavelength);       
	else
		FIA_GetFalseColourPalette(plugin->window->current_palette, palette_plugin->last_user_chosen_wavength);

	ImageWindow_SetPalette(plugin->window);
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	PalettePlugin* palette_plugin = (PalettePlugin*) plugin;  
	
	if(palette_plugin == NULL)
		return;

	free(palette_plugin->palette_item_array);
}

static PaletteItem* CreatePaletteItem(PalettePlugin* palette_plugin, const char *menuname, const char *filepath)
{
    ImageWindowPlugin* plugin = (ImageWindowPlugin*) palette_plugin;
	char buffer[500];
    PaletteItem *pi_ptr = &(palette_plugin->palette_item_array[palette_plugin->palette_count]); 
	
	if(palette_plugin->palette_count > 99) {
		
		GCI_MessagePopup("Error", "Maximum number of palette file installed");
		return NULL;
	}
		
    pi_ptr->palette_plugin = palette_plugin;

    sprintf(buffer, "%s%s", "Display//Palettes//", menuname);
    pi_ptr->menuitem_id = Plugin_AddMenuItem(plugin, buffer, 0, palette_on_menu_clicked, pi_ptr);
    
    if(filepath == NULL)
        pi_ptr->filebased = 0;    
    else {
        pi_ptr->filebased = 1;
        strcpy(pi_ptr->filepath, filepath);
    }
	
	palette_plugin->palette_count++;
	
    return pi_ptr;
}


static int format_filename(const char* filename, char *name)
{
  	get_file_without_extension(filename, name);
	
	// Upper Case First Letter
	name[0] = toupper(name[0]);     
	
	return 1;
}

ImageWindowPlugin* palette_plugin_constructor(IcsViewerWindow *window)
{
    char search_str[500], full_filename[GCI_MAX_PATHNAME_LEN], filename[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN], palette_file_dir[GCI_MAX_PATHNAME_LEN];
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "PalettePlugin", sizeof(PalettePlugin)); 
    PaletteItem* item = NULL;
	
	PalettePlugin* palette_plugin = (PalettePlugin*) plugin; 
    palette_plugin->palette_item_array = malloc(sizeof(PaletteItem) * 100);
	palette_plugin->palette_count = 0; 
	
	palette_plugin->wavelength = 390.0;       

	item = CreatePaletteItem(palette_plugin, "GreyLevel Palette", NULL);
    palette_plugin->greylevel_palette_menu_id = item->menuitem_id;
    
    item = CreatePaletteItem(palette_plugin, "Overload Palette", NULL);
    palette_plugin->overload_palette_menu_id = item->menuitem_id;
	
    item = CreatePaletteItem(palette_plugin, "Rainbow Palette", NULL);
    palette_plugin->rainbow_palette_menu_id = item->menuitem_id;
    
	item = CreatePaletteItem(palette_plugin, "Reverse Rainbow Palette", NULL);
    palette_plugin->reverse_rainbow_palette_menu_id = item->menuitem_id;
	
	item = CreatePaletteItem(palette_plugin, "False Colour Palette", NULL);
    palette_plugin->false_colour_palette_menu_id = item->menuitem_id;

    item = CreatePaletteItem(palette_plugin, "Log Palette", NULL);
    palette_plugin->log_palette_menu_id = item->menuitem_id;
		
	item = CreatePaletteItem(palette_plugin, "Temperature Palette", NULL);
    palette_plugin->temp_palette_menu_id = item->menuitem_id;
	
	item = CreatePaletteItem(palette_plugin, "Seismic Palette", NULL);
    palette_plugin->seismic_palette_menu_id = item->menuitem_id;
	
	item = CreatePaletteItem(palette_plugin, "Pileup Palette", NULL);
    palette_plugin->pileup_palette_menu_id = item->menuitem_id;
	
    // Here we search for a folder next to the exe or in the system path call IcsViewerPalettes
    if(find_resource("IcsViewerPalettes", palette_file_dir) == 0) {

        // Loop through the xls files in this dir and create an entry for each in the menu.
	    sprintf(search_str, "%s\\*.xls", palette_file_dir);
	
	    if(GetFirstFile (search_str, 1, 1, 0, 0, 0, 0, full_filename) < 0)
	    	return NULL;

	    sprintf(filepath, "%s\\%s", palette_file_dir, full_filename);
		format_filename(full_filename, filename);   
		
        CreatePaletteItem(palette_plugin, filename, filepath);
	
	    while (!GetNextFile (full_filename))
	    {
		    sprintf(filepath, "%s\\%s", palette_file_dir, full_filename); 
			format_filename(full_filename, filename); 
			
			if(CreatePaletteItem(palette_plugin, filename, filepath) == NULL)
				break;
	    }        
    }

	Plugin_DimMenuPathItem(plugin, "Display//Palettes");  
	Plugin_CheckMenuPathItem(plugin, "Display//Palettes//GreyLevel Palette");
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
