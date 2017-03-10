#include "icsviewer_tools.h" 
#include "ImageViewer.h"

typedef struct
{
	Tool parent;
	
	COLORREF colour;
	POINT canvas_point;
	POINT image_point;
	
	int draw;
	int line_width;
	int crosshair_width;
	
} CrossHairTool;


ImageWindowPlugin* crosshair_tool_constructor(IcsViewerWindow *window);

void set_crosshair_viewer_point(CrossHairTool *crosshair_tool, POINT point);

void set_crosshair_image_point(CrossHairTool *crosshair_tool, POINT point);

int crosshair_tool_get_image_pos(Tool *tool, IcsViewerWindow *window, int *x, int *y);

int crosshair_tool_get_viewer_pos(Tool *tool, IcsViewerWindow *window, int *x, int *y);
