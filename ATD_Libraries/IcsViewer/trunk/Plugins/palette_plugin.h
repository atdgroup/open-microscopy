#ifndef __PALETTE_PLUGIN__
#define __PALETTE_PLUGIN__

#include "icsviewer_plugin.h" 

typedef struct _PalettePlugin PalettePlugin;

typedef struct _PaletteItem
{
    PalettePlugin* palette_plugin;
    int menuitem_id;   
    int filebased;
    char filepath[GCI_MAX_PATHNAME_LEN];

} PaletteItem;

// Rainbow Palette ImageWindowPlugin
struct _PalettePlugin
{
	ImageWindowPlugin parent;    
	
	int palette_count;
    PaletteItem *palette_item_array;
	int greylevel_palette_menu_id;
	int overload_palette_menu_id;
	int rainbow_palette_menu_id;
	int reverse_rainbow_palette_menu_id;
	int false_colour_palette_menu_id;
	int log_palette_menu_id;
	int temp_palette_menu_id;
	int seismic_palette_menu_id;
	int pileup_palette_menu_id;
	
	int panel_id;
	int canvas_id;
	double wavelength;
	double last_user_chosen_wavength;

    char palette_file_dir[GCI_MAX_PATHNAME_LEN];
};


ImageWindowPlugin* palette_plugin_constructor(IcsViewerWindow *window);

void palette_tool_set_false_colour_palette(PalettePlugin* palette_plugin, double wavelength); 


void false_colour_dialog_create(PalettePlugin* palette_plugin);

#endif
