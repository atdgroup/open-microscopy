#include "icsviewer_tools.h" 

typedef enum {ScaleMode_Automatic, ScaleMode_Manual} ScaleMode;

typedef struct
{
	ImageWindowPlugin parent;  
	
	COLORREF colour;
	
	ScaleMode	scale_mode;
	int			panel_id;
	int			manual_scale_panel;
	int			manual_scale;
	
	double		manual_scale_min;
	double		manual_scale_max;
	
	int			profile_red_plot_handle;
	int			profile_green_plot_handle;
	int			profile_blue_plot_handle;
	
	int			show_profile_on_line_tool;

	POINT		point1;
	POINT		point2;

	POINT		current_image_size;
	
} ProfilePlugin;


ImageWindowPlugin* profile_plugin_constructor(IcsViewerWindow *window);
