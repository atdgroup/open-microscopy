#include "icsviewer_tools.h" 

typedef struct
{
	Tool  parent;
	
	int current_selected;
	int	resize_fit_item_id;
 	int	resize_free_item_id;
 	int	zoom20_item_id;
	int	zoom50_item_id; 
	int	zoom200_item_id;
	int	zoom400_item_id;
	int	zoom800_item_id;
	
} ZoomTool;


ImageWindowPlugin* zoom_tool_constructor(IcsViewerWindow *window);
