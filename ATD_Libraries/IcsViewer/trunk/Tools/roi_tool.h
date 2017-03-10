#include "icsviewer_tools.h" 

typedef enum {
	
   HANDLE_TOPLEFT,
   HANDLE_TOP,
   HANDLE_TOPRIGHT,
   HANDLE_RIGHT,
   HANDLE_RIGHTBOTTOM,
   HANDLE_BOTTOM,
   HANDLE_LEFTBOTTOM,
   HANDLE_LEFT
	
} HandlePosition;


typedef struct
{
	HandlePosition position;
	CursorType type;
	RECT rect;
	int cursor;
	
} RectHandle;


typedef enum
{
	INTERACT_NONE,
	INTERACT_HANDLE_MOVE,
	INTERACT_GENERAL_MOVE,
	INTERACT_DRAW
	
} InteractType;

typedef struct
{
	Tool  			parent;
	
	RECT 			canvas_rect;
	RECT 			image_rect;   
	POINT			selection_offset;
	Point 			position;
	RectHandle  	rectHandles[8];     
	InteractType	interact_type;
	
	int				draw_rect;
	int				active;
	int				prevent_resize;
	int				mouse_button_is_down;
	int				selectedHandle;
	int 			crop_menu_item;
	
} RoiTool;


ImageWindowPlugin* roi_tool_constructor(IcsViewerWindow *window);
