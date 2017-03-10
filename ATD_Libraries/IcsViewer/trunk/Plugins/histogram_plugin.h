#include "icsviewer_plugin.h" 

typedef enum {HistScaleMode_Automatic, HistScaleMode_Manual} HistScaleMode; 

typedef struct
{
	ImageWindowMenuPlugin parent;
	
	int 		panel_id;
	
	int			xaxis_manual_panel;
	int			yaxis_manual_panel;
	
	int			custom_x_scale;
	
	int			prevent_update;
	
	HistScaleMode	x_scale_mode;
	HistScaleMode	y_scale_mode;
	
	int			plot_handle;
	int			red_plot_handle;
	int			green_plot_handle;
	int			blue_plot_handle;
	
	unsigned long	*histR;
	unsigned long	*histG;
	unsigned long	*histB;
	unsigned long	*hist;
	
	int			number_of_bins;
	
	double		min_x_axis;
	double		max_x_axis;
	double		manual_min_x_axis;
	double		manual_max_x_axis;
	
	int		min_y_axis;
	int		max_y_axis;
	
	int 	imtype;
	
	char		bin_range_legend[100]; 
	
} HistogramPlugin;


ImageWindowPlugin* histogram_plugin_constructor(IcsViewerWindow *window);
