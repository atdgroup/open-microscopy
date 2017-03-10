#ifndef __IMAGING_WINDOW_PLUGINS__
#define __IMAGING_WINDOW_PLUGINS__

#include "icsviewer_window.h"
#include "icsviewer_plugin_menu.h"
#include "toolbox.h"
#include "profile.h"

typedef enum {PLUGIN, TOOL_PLUGIN} PluginType;

enum MOUSE_BUTTON {LEFT_MOUSE_BUTTON, RIGHT_MOUSE_BUTTON};    


typedef union
{
	int button;
	int	width;
	int height;
	int binning;
	int zoomed_by_user_interaction;
	
	double zoom_factor;
	double microns_per_pixel;
	
	FIBITMAP *dib;
	POINT point;
	RECT rect;
	
	Tool *tool;
	
} EventData;


typedef struct
{
	// Takes no params
	void (*on_paint) (ImageWindowPlugin *plugin, EventData data1, EventData data2);
	
	// Takes no params 
	void (*on_buffer_paint) (ImageWindowPlugin *plugin, EventData data1, EventData data2); 
	
	// Takes int button, POINT point params
	void (*on_mouse_down) (ImageWindowPlugin *plugin, EventData data1, EventData data2);
	void (*on_mouse_up) (ImageWindowPlugin *plugin, EventData data1, EventData data2);
	void (*on_mouse_move) (ImageWindowPlugin *plugin, EventData data1, EventData data2);

	// Takes double microns_per_pixel
	void (*on_microns_per_pixel_changed) (ImageWindowPlugin *plugin, EventData data1, EventData data2); 

	// Takes nothing
	// dont want EventData union to be size of char[500]
	void (*on_disk_file_loaded) (ImageWindowPlugin *plugin, EventData data1, EventData data2); 
	
	// Takes FreeImage bitmap ref in data1 for validate the original image ref is passed as data2
	int  (*on_validate_plugin) (ImageWindowPlugin *plugin, EventData data1, EventData data2); 
	void (*on_image_displayed) (ImageWindowPlugin *plugin, EventData data1, EventData data2);
	void (*on_image_load) (ImageWindowPlugin *plugin, EventData data1, EventData data2);  
	
	// Takes int width and height
	void (*on_resize) (ImageWindowPlugin *plugin, EventData data1, EventData data2);      
	void (*on_exit_resize) (ImageWindowPlugin *plugin, EventData data1, EventData data2);  
	
	// Takes no params
	void (*on_destroy) (ImageWindowPlugin *plugin, EventData data1, EventData data2);
	
	// Takes double zoom_factor
	void (*on_zoom_changed) (ImageWindowPlugin *plugin, EventData data1, EventData data2);  
	
	// Takes FreeImage bitmap ref in data1  
	void (*on_image_crop) (ImageWindowPlugin *plugin, EventData data1, EventData data2);
	
	// Takes no params 
 	void (*on_palette_changed) (ImageWindowPlugin *plugin, EventData data1, EventData data2);  
  	
  	// Takes start and end POINTS
    void (*on_line_tool_drawn) (ImageWindowPlugin *plugin, EventData data1, EventData data2);  
    
    // Takes int old binning as data1 new binning as data2
    void (*on_binning_changed) (ImageWindowPlugin *plugin, EventData data1, EventData data2);
    
  	void (*on_region_of_interest) (ImageWindowPlugin *plugin, RECT rect); 
	
	// Takes pointer to tool in data1.
	void (*on_tool_status_changed) (ImageWindowPlugin *plugin, EventData data1, EventData data2);  
	
	void (*on_tool_activated) (ImageWindowPlugin *plugin, EventData data1, EventData data2);   
	
	void (*on_tool_deactivated) (ImageWindowPlugin *plugin, EventData data1, EventData data2);

} PluginOperations; 

// Plugins
struct _ImageWindowPlugin
{
	PluginOperations *vtable;
	
	PluginType type;
	IcsViewerWindow *window;
	
	ListType menuItemList;
	
	char name[100];
};

struct _ImageWindowMenuPlugin
{
	struct _ImageWindowPlugin parent;
	
	int menu_item_id;
};

struct _ImageWindowMenuDialogPlugin
{
	ImageWindowMenuPlugin parent;
	
	char uir_path[GCI_MAX_PATHNAME_LEN];
	
	int dialog_panel_id;
	int panel_identifier;
	int ok_button;
	int close_button;
} ;

typedef ImageWindowPlugin* (*IMAGEWINDOW_PLUGIN_CONSTRUCTOR) (IcsViewerWindow *window); 

ImageWindowPlugin* Plugin_CreatePlugin(IMAGEWINDOW_PLUGIN_CONSTRUCTOR contructor, IcsViewerWindow *window);

ImageWindowPlugin* Plugin_NewPluginType(IcsViewerWindow *window, char *name, size_t size);

void UnDimMenuItems(ImageWindowPlugin *plugin);  
			
void DimAndUncheckMenuItems(ImageWindowPlugin *plugin);			

void UncheckPluginMenuItems(ImageWindowPlugin *plugin);
  
void DestroyAllPlugins(IcsViewerWindow *window);

EventData NewEventData(void);

void DiableAllPlugins(IcsViewerWindow *window);

#define MENUBAR_ID(plugin) ((ImageWindowPlugin *)plugin)->window->panel_menu->menubar_id

#define MENUITEM_ID(plugin) ((ImageWindowMenuPlugin *)plugin)->menu_item_id 

#define PLUGIN_WINDOW(plugin) ((ImageWindowPlugin *)plugin)->window

#define PLUGIN_CANVAS(plugin) ((ImageWindowPlugin *)plugin)->window->canvas_window 

#define PLUGIN_TYPE(plugin) ((ImageWindowPlugin *)plugin)->type   

#define PLUGIN_VTABLE(plugin, member) ((ImageWindowPlugin *)plugin)->vtable->member

ImageWindowPlugin* GetPluginPtrInList(IcsViewerWindow *window, int i); 


#define SEND_EVENT(window, function, data1, data2, fun_name) \
do {	  \
	/* char temp[100]; */ \
	int i; ImageWindowPlugin *t_plugin = NULL; \
	for(i=1; i <= window->number_of_plugins; i++) {  \
		t_plugin = GetPluginPtrInList(window, i); \
		/* sprintf(temp, "%s_%s", t_plugin->name, fun_name);*/ \
		/* PROFILE_START(temp); */  \
		if(t_plugin->vtable->function != NULL) {(t_plugin->vtable->function)(t_plugin, data1, data2);}  \
		/* PROFILE_STOP(temp);   */                 \
	} \
} while (0);



#define ACTIVATE_FOR_ALL_IMAGES(plugin) \
do { \
	(plugin->window->panel_dib == NULL) ? Plugin_DimAndUncheckMenuItems (plugin) : Plugin_UnDimMenuItems(plugin);  \
} while (0);		


#define ACTIVATE_FOR_GRAYSCALE_IMAGES(plugin) \
do { \
	((plugin->window->panel_dib == NULL) || (!FreeImageAlgorithms_IsGreyScale(plugin->window->panel_dib))) ? Plugin_DimAndUncheckMenuItems (plugin) : Plugin_UnDimMenuItems(plugin);  \
} while (0);


#define DISABLE_FOR_LIVE_MODE(plugin) \
do { \
	if((plugin)->window->live_mode == 1) return 0;  \
} while (0);	

int Plugin_IsImageGreyScale(ImageWindowPlugin *plugin); 


#define Plugin_GetImageWidth(plugin) \
	FreeImage_GetWidth(((ImageWindowPlugin *)plugin)->window->panel_dib)	


#define Plugin_GetImageHeight(plugin) \
	FreeImage_GetHeight(((ImageWindowPlugin *)plugin)->window->panel_dib)	
	
#define Plugin_GetMicronsPerPixel(plugin) \
	(((ImageWindowPlugin *)plugin)->window->microns_per_pixel)	

#define Plugin_GetImageBPP(plugin) \
	FreeImage_GetBPP(((ImageWindowPlugin *)plugin)->window->panel_dib)	


#define Plugin_UnDimMenuItems(plugin) \
	UnDimMenuItems(((ImageWindowPlugin *)plugin))


#define Plugin_DimAndUncheckMenuItems(plugin) \
	DimAndUncheckMenuItems(((ImageWindowPlugin *)plugin))


#define Plugin_UncheckMenuItems(plugin) \
	UncheckPluginMenuItems(((ImageWindowPlugin *)plugin))


int Is16BitImage(FIBITMAP* dib);  

int IsFloatingPointImage(FIBITMAP* dib); 

int IsStandardTypeImage(FIBITMAP* dib);

void DestroyPlugin(ImageWindowPlugin* plugin);

#endif
