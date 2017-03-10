#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "icsviewer_tools.h" 
#include "ImageViewer.h"
#include <utility.h>
#include "icsviewer_uir.h"
#include "ImageViewer_Drawing.h"
#include "counter_plugin.h"
#include "gci_utils.h"

#include "string_utils.h"
#include "GL_CVIRegistry.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_HBitmap.h"
#include "FreeImageAlgorithms_Utilities.h"

#include "FreeImageIcs_IO.h" 

#include "icsviewer_3d.h" 

#define CROSS_SIZE 16 
#define START_TOP 55
#define MAX_COUNTERS 32

static colours[MAX_COUNTERS] = {
			VAL_RED,VAL_GREEN,VAL_BLUE,VAL_CYAN,VAL_MAGENTA,VAL_YELLOW,VAL_DK_RED,VAL_DK_BLUE,
			VAL_DK_GREEN,VAL_DK_CYAN,VAL_DK_MAGENTA,VAL_DK_YELLOW,VAL_LT_GRAY,VAL_DK_GRAY,VAL_BLACK,VAL_WHITE,       
			VAL_RED,VAL_GREEN,VAL_BLUE,VAL_CYAN,VAL_MAGENTA,VAL_YELLOW,VAL_DK_RED,VAL_DK_BLUE,
			VAL_DK_GREEN,VAL_DK_CYAN,VAL_DK_MAGENTA,VAL_DK_YELLOW,VAL_LT_GRAY,VAL_DK_GRAY,VAL_BLACK,VAL_WHITE};

static void draw_cross(CounterPlugin *counter_plugin, POINT point, COLORREF colour)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) counter_plugin;
	
	ImageViewer_DrawCross(plugin->window->canvas_window, point, CROSS_SIZE, colour);  
}

static Counter* get_counter_from_id(CounterPlugin *counter_plugin, int id)
{
	return (Counter*) ListGetPtrToItem(counter_plugin->counters, id);     	 	
}

static Counter* get_counter_from_panel(CounterPlugin *counter_plugin, int panel_id)
{
	int id;
	
	GetCtrlVal(panel_id, CNTER_PNL_ID, &id);	
	
	return get_counter_from_id(counter_plugin, id);   
}

static Counter* get_active_counter(CounterPlugin *counter_plugin)
{
	return get_counter_from_id(counter_plugin, counter_plugin->active_counter_id);  	
}

static void on_buffer_paint(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	CounterPlugin *counter_plugin = (CounterPlugin *) plugin;
	Counter *counter;
	CounterPoint *image_point;
	POINT canvas_point;
	int i, j, counter_size, point_size;
	
	if(!counter_plugin->active)
		return;
	
	counter_size = ListNumItems (counter_plugin->counters);     
	
	for(j=1; j <= counter_size; j++) { 
		
		counter = ListGetPtrToItem (counter_plugin->counters, j);   
		
		point_size = ListNumItems (counter->points);     
	
		for(i=1; i <= point_size; i++) {
	
			image_point = ListGetPtrToItem (counter->points, i);
	
			ImageViewer_TranslateImagePointToWindowPoint(plugin->window->canvas_window, image_point->pt, &canvas_point);
		
			draw_cross(counter_plugin, canvas_point, CviColourToColorRef(counter->colour));
		}
	}
}


static void on_mouse_down (ImageWindowPlugin *plugin, EventData data1, EventData data2)  
{
	
	CounterPlugin *counter_plugin = (CounterPlugin *) plugin;   
	Counter *active_counter = NULL;
	POINT image_point;
	CounterPoint cp;
	char count_str[50], buffer[100];
	int size;
	
	if(!counter_plugin->active)
		return;
	
	if(!ImageViewer_IsPointWithinDisplayedImageRect(PLUGIN_CANVAS(plugin), data2.point))
		return;
	
	ImageViewer_TranslateWindowPointToImagePoint(PLUGIN_CANVAS(plugin), data2.point, &image_point);        

	cp.pt = image_point;
	cp.intensity = 0.0;
	
	// FreeImage's are vertically flipped 
	image_point.y = FreeImage_GetHeight(plugin->window->panel_dib) - image_point.y;
	
	if(FIA_IsGreyScale(plugin->window->panel_dib)) {
		FIA_GetPixelValue(plugin->window->panel_dib, image_point.x, image_point.y, &(cp.intensity));
		sprintf(buffer, "Pixel intensity %.3f", cp.intensity);
	}
	else {
	
		RGBQUAD value;
		
		FreeImage_GetPixelColor(plugin->window->panel_dib, image_point.x, image_point.y, &value);
		
		sprintf(buffer, "Pixel intensity R: %d  G: %d  B: %d", value.rgbRed, value.rgbGreen, value.rgbBlue);
		
		cp.intensity = 0;
		cp.red = value.rgbRed; 
		cp.green = value.rgbGreen;
		cp.blue = value.rgbBlue;
	}
	
	
	SetCtrlVal(counter_plugin->panel_id, COUNT_PNL_LAST_INTENSITY, buffer);
	
	active_counter = get_active_counter(counter_plugin);
	
	ListInsertItem (active_counter->points, &cp, END_OF_LIST);   
	
	ImageViewer_Redraw(plugin->window->canvas_window);
	
	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);

	size = ListNumItems (active_counter->points);          
	
	sprintf(count_str, "Counted objects: %d", size);
	
	SetCtrlVal(active_counter->subpanel_id, CNTER_PNL_COUNT, count_str);
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	Plugin_UnDimMenuPathItem(plugin, "View//Counter");  
	
	return 1;
}


static void CloseCountPlugin(CounterPlugin *counter_plugin)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) counter_plugin;  
	
	//RegistrySavePanelPosition (REGKEY_HKCU, REGISTRY_SUBKEY "CountPanel", counter_plugin->panel_id); 
	ics_viewer_registry_save_panel_position(plugin->window, counter_plugin->panel_id);
	
	Plugin_UncheckMenuItems(plugin);     
	
	counter_plugin->top = START_TOP;
	counter_plugin->active = 0;
	DiscardPanel(counter_plugin->panel_id);
	ListClear(counter_plugin->counters);
	
	plugin->window->cursor_type = CURSOR_NORMAL;   
	ImageViewer_SetInteractMode(PLUGIN_CANVAS(plugin), NO_INTERACT_MODE); 
	
	ImageViewer_Redraw(plugin->window->canvas_window);
	
	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
}


static int CVICALLBACK OnCounterPanelOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	CounterPlugin *counter_plugin = (CounterPlugin *) callbackData;

	switch (event)
	{
		case EVENT_COMMIT:
		{
			CloseCountPlugin(counter_plugin);

			break;
		}
	}
		
	return 0;
}
	

static int CVICALLBACK OnCounterReset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CounterPlugin *counter_plugin = callbackData;  
			Counter* counter = get_counter_from_panel(counter_plugin, panel);         
			ImageWindowPlugin* plugin = (ImageWindowPlugin*) counter_plugin;       
			
			ListClear(counter->points);    
			
			ImageViewer_Redraw(plugin->window->canvas_window);
	
			InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	
			SetCtrlVal(counter->subpanel_id, CNTER_PNL_COUNT, "Counted objects: 0");
			
			break;
		}
	}
		
	return 0;
}


static void SetActiveCounter(CounterPlugin *counter_plugin, int id)
{
	// Turn off all counters
	Counter *counter_ptr;
	int i, num_of_counters = ListNumItems(counter_plugin->counters);
	
	for(i=1; i <= num_of_counters; i++)
	{
		counter_ptr = ListGetPtrToItem(counter_plugin->counters, i);
		SetCtrlVal(counter_ptr->subpanel_id, CNTER_PNL_ACTIVE_ID, 0);  
	}
 
	counter_ptr = get_counter_from_id(counter_plugin, id);  
	SetCtrlVal(counter_ptr->subpanel_id, CNTER_PNL_ACTIVE_ID, 1);  
	
	counter_plugin->active_counter_id = id;
}


static int CVICALLBACK OnChangeActive (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CounterPlugin *counter_plugin = callbackData;  
			Counter* counter = get_counter_from_panel(counter_plugin, panel);       
			
			SetActiveCounter(counter->counter_plugin, counter->id); 
		
			break;
		}
	}
		
	return 0;
}							  


static int CVICALLBACK OnCounterColourChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			CounterPlugin *counter_plugin = callbackData;        
			Counter* counter = get_counter_from_panel(counter_plugin, panel);       
			ImageWindowPlugin* plugin = (ImageWindowPlugin*) counter_plugin;       
			
			GetCtrlVal (panel, control, &(counter->colour) );
			
			ImageViewer_Redraw(plugin->window->canvas_window);
	
			InvalidateRect(plugin->window->canvas_window, NULL, FALSE); 
			
			break;
		}
	}
		
	return 0;
}


static void AddCounter(CounterPlugin *counter_plugin)
{
	Counter counter;
	
	int subwindow_height, subpanel_id;
	int number_of_counters = ListNumItems(counter_plugin->counters);
	char buffer[200];
	
	if( number_of_counters >= MAX_COUNTERS )
		return;
	
	counter.subpanel_id = subpanel_id = LoadPanel(counter_plugin->panel_id, uir_file_path, CNTER_PNL);
	
	GetPanelAttribute(subpanel_id, ATTR_HEIGHT, &subwindow_height);      
	
	SetPanelAttribute(subpanel_id, ATTR_TITLEBAR_VISIBLE, 0);  
	SetPanelAttribute(subpanel_id, ATTR_LEFT, 5);      
	SetPanelAttribute(subpanel_id, ATTR_TOP, counter_plugin->top);
	
	counter_plugin->top += subwindow_height;          
	
	counter.id = number_of_counters + 1;
	
	SetCtrlVal(subpanel_id, CNTER_PNL_ID, counter.id);   
	
	counter.counter_plugin = counter_plugin;
	
	counter.colour = colours[number_of_counters];
	
	sprintf(buffer, "Counter%d", number_of_counters + 1);
	
	SetCtrlVal(subpanel_id, CNTER_PNL_NAME, buffer);   
	
	SetCtrlVal (subpanel_id, CNTER_PNL_COLOUR, counter.colour);
	
	SetCtrlVal(subpanel_id, CNTER_PNL_COUNT, "Counted objects: 0");
	
	counter.points = ListCreate (sizeof(CounterPoint)); 
	
	ListInsertItem (counter_plugin->counters, &counter, END_OF_LIST);

	if ( InstallCtrlCallback (subpanel_id, CNTER_PNL_COLOUR, OnCounterColourChanged, counter_plugin) < 0)
		return;
	
	if ( InstallCtrlCallback (subpanel_id, CNTER_PNL_ACTIVE_ID, OnChangeActive, counter_plugin) < 0) 
		return;
	
	if ( InstallCtrlCallback (subpanel_id, CNTER_PNL_RESET, OnCounterReset, counter_plugin) < 0) 
		return;
	
	SetActiveCounter(counter_plugin, counter.id);  

	SetPanelAttribute(counter_plugin->panel_id, ATTR_HEIGHT, counter_plugin->top);           
	
	DisplayPanel(subpanel_id);     
	
}


static int CVICALLBACK OnCounterPanelAddCounter (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	CounterPlugin *counter_plugin = (CounterPlugin *) callbackData;

	switch (event)
	{
		case EVENT_COMMIT:
		{
			AddCounter(counter_plugin);
			
			break;
		}
	}
		
	return 0;
}


static int ExportData(CounterPlugin *counter_plugin, char *filepath)
{
	int i, j, counter_size, point_size, fsize;
	FILE *fp;
	CounterPoint *cp;
	Counter *counter;       
	char buffer[200];
	
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) counter_plugin;       
	
	// If the conf file does exist clear any read only bit 
	if (FileExists (filepath, &fsize))
    	SetFileAttrs (filepath, 0, -1, -1, -1);

    fp = fopen (filepath, "w");
    
    if (fp == NULL)
    	return GCI_IMAGING_ERROR;
	
	counter_size = ListNumItems (counter_plugin->counters);     
	
	// Write the filename of the image
	fprintf(fp, "%s\n\n", plugin->window->filename);
	
	// We are displaying 3d data specify the dimension and slice being shown
	if(plugin->window->mutidimensional_data) {
		
		int size;
		char label1[10], label2[10];
		
		manipulater3d_get_dimension_details (plugin->window->importer3d,
			plugin->window->last_3d_dimension1_shown, label1, &size) ;    
		
		manipulater3d_get_dimension_details (plugin->window->importer3d,
			plugin->window->last_3d_dimension2_shown, label2, &size) ;  
		
		
		fprintf(fp, "3D Data\n");  
		fprintf(fp, "Viewable Dimensions %s %s\n", label1, label2);  
		fprintf(fp, "Slice %d\n\n", plugin->window->last_3d_slice_shown);  
	}
	else {
		
		fprintf(fp, "2D Data\n"); 
			
	}

	if(FIA_IsGreyScale(plugin->window->panel_dib))   
		fprintf(fp, "Columns: X Y Intensity\n\n");
	else
		fprintf(fp, "Columns: X Y Red Green Blue\n\n");

	for(j=1; j <= counter_size; j++) { 
		
		counter = ListGetPtrToItem (counter_plugin->counters, j);   
		
		point_size = ListNumItems (counter->points);     
		
		// Get the counter name
		GetCtrlVal(counter->subpanel_id, CNTER_PNL_NAME, buffer);
		
		fprintf(fp, "%s  Count: %d\n", buffer, point_size);
		
		if(FIA_IsGreyScale(plugin->window->panel_dib)) {           
				
			for(i=1; i <= point_size; i++) {
	
				cp = ListGetPtrToItem (counter->points, i);
	
				// x, y, intensity
				fprintf(fp, "%d\t%d\t%f\n", cp->pt.x, cp->pt.y, cp->intensity);
			}
		}
		else {
			
			for(i=1; i <= point_size; i++) {
	
				cp = ListGetPtrToItem (counter->points, i);
	
				// x, y, intensity
				fprintf(fp, "%d\t%d\t%d\t%d\t%d\n", cp->pt.x, cp->pt.y, cp->red, cp->green, cp->blue);
			}	
		}
			
		fprintf(fp, "\n");   
	}
	
    fclose(fp);

    // set read-only 
    SetFileAttrs (filepath, 1, -1, -1, -1);

	return GCI_IMAGING_SUCCESS;
}

static int CVICALLBACK OnExportClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	CounterPlugin *counter_plugin = (CounterPlugin *) callbackData; 
	char *default_extensions = "*.txt;";
	char fname[GCI_MAX_PATHNAME_LEN];
	char directory[GCI_MAX_PATHNAME_LEN] = "";
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) counter_plugin;
	
	switch (event)
		{
		case EVENT_COMMIT:

			if (FileSelectPopup (GetDefaultDirectoryPath(plugin->window, directory), "*.txt",
				default_extensions, "Export Data As", VAL_OK_BUTTON, 0, 0, 1, 1, fname) <= 0) {
				return -1;
			}

			ExportData(counter_plugin, fname);
			

			break;
		}
	return 0;
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData;      
	CounterPlugin *counter_plugin = (CounterPlugin *) callbackData;  
	
	if(counter_plugin->active == 1) {
		
		CloseCountPlugin(counter_plugin);
		
		ImageViewer_Redraw(plugin->window->canvas_window);
	
		InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	
		return;
	}
	
	counter_plugin->panel_id = LoadPanel(0, uir_file_path, COUNT_PNL);             
	counter_plugin->active = 1;

	CHECK_MENU_ITEM  
	
	if ( InstallCtrlCallback (counter_plugin->panel_id, COUNT_PNL_OK_BUTTON, OnCounterPanelOk, counter_plugin) < 0)
		return;
	
	if ( InstallCtrlCallback (counter_plugin->panel_id, COUNT_PNL_ADD_BUTTON, OnCounterPanelAddCounter, counter_plugin) < 0) 
		return;
	
	if ( InstallCtrlCallback (counter_plugin->panel_id, COUNT_PNL_EXPORT_BUTTON, OnExportClicked, counter_plugin) < 0) {
		return;
	}
	
	ics_viewer_registry_read_panel_position(plugin->window, counter_plugin->panel_id);

	AddCounter(counter_plugin); 
	
	plugin->window->cursor_type = CURSOR_CROSS;             
	ImageViewer_SetInteractMode(PLUGIN_CANVAS(plugin), NO_INTERACT_MODE);  
	
	DisplayPanel(counter_plugin->panel_id);
}


static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	CounterPlugin *counter_plugin = (CounterPlugin *) plugin;     
	
	ListDispose(counter_plugin->counters);
}


ImageWindowPlugin* counter_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "CounterPlugin", sizeof(CounterPlugin));

	CounterPlugin *counter_plugin = (CounterPlugin *) plugin;     
	
	Plugin_AddMenuItem(plugin, "View//Counter",
		VAL_MENUKEY_MODIFIER | 'C', on_menu_clicked, plugin);

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_buffer_paint) = on_buffer_paint;
	PLUGIN_VTABLE(plugin, on_mouse_down) = on_mouse_down; 
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin;   
	
	counter_plugin->top = START_TOP;
	counter_plugin->active = 0;  
	counter_plugin->panel_id = 0;
	counter_plugin->active_counter_id = 1;
	counter_plugin->counters = ListCreate (sizeof(Counter)); 
	
	return plugin;
}
