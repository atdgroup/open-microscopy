#include "icsviewer_tools.h" 

typedef struct
{
	Tool parent;
	
	COLORREF colour;
	
	int			panel_id;
	int			autoscale;
	
	POINT		canvas_p1;
	POINT		canvas_p2;
	
	POINT		image_p1;
	POINT		image_p2;
	
	int			mouse_button_is_down;
	int			draw;

	POINT		current_image_size;
	
} LineTool;


ImageWindowPlugin* line_tool_constructor(IcsViewerWindow *window);

