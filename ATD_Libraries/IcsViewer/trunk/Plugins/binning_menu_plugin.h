#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowPlugin parent;
	
	// Menu items
	int square_3x3_menu_id;
	int square_5x5_menu_id;
	int square_7x7_menu_id;
	int square_9x9_menu_id;
	int square_11x11_menu_id;

	int circular_3x3_menu_id;
	int circular_5x5_menu_id;
	int circular_7x7_menu_id;
	int circular_9x9_menu_id;
	int circular_11x11_menu_id;

	int gaussian_3x3_menu_id;
	int gaussian_5x5_menu_id;
	int gaussian_7x7_menu_id;
	int gaussian_9x9_menu_id;
	int gaussian_11x11_menu_id;


} BinningPlugin;


ImageWindowPlugin* binning_plugin_constructor(IcsViewerWindow *window);
