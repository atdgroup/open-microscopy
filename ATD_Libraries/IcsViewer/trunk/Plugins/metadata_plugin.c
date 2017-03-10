#include <userint.h>
#include "icsviewer_window.h"
#include "toolbox.h"
#include <utility.h>
#include "icsviewer_private.h"
#include "metadata_plugin.h"
#include "icsviewer_uir.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "GL_CVIRegistry.h"

#include "FreeImageAlgorithms_IO.h"

#define kCornerRight  (1<<0)
#define kCornerBottom (1<<1)


void 	ConvertToEditableTreeCells (int panel, int tree, int string);
void 	EditTreeCell (int panel, int tree, int string, int index, int col);
void 	EndTreeCellEdit (int panel, int tree, int string);
void 	UpdateStringCtrl (int panel, int tree, int string, int index, int col, int toString);
int 	GetTreeColumnLeft (int panel, int tree, int col);
int 	GetTreeItemTop (int panel, int tree, int index);
int 	GetStringWidthForColumn (int panel, int tree, int col, int colLeft);

int CVICALLBACK OnTreeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK StringKeyPressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

static void GetKeyValueFromTreeItem(MetaDataPlugin *metadata_plugin, int itemIndex, char *key, char *value)
{

	GetTreeCellAttribute (metadata_plugin->info_panel, INFO_TREE, itemIndex, 1, ATTR_LABEL_TEXT, key);

	GetTreeCellAttribute (metadata_plugin->info_panel, INFO_TREE, itemIndex, 2, ATTR_LABEL_TEXT, value); 
}


static void AddKeyValueToTree(MetaDataPlugin *metadata_plugin, const char *key, const char *value)
{
	int index = InsertTreeItem (metadata_plugin->info_panel, INFO_TREE,
							VAL_SIBLING, 0, VAL_LAST, key, 0, 0, 0);

	SetTreeCellAttribute (metadata_plugin->info_panel, INFO_TREE, index, 1, ATTR_LABEL_TEXT, key);

	SetTreeCellAttribute (metadata_plugin->info_panel, INFO_TREE, index, 2, ATTR_LABEL_TEXT, value);
}


static void AddKeyValueHistoryToList(MetaDataPlugin *metadata_plugin, const char *key, const char *value)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) metadata_plugin;
	
	AddKeyValueToTree(metadata_plugin, key, value);           
	
	GCI_ImagingWindow_SetMetaDataKey(plugin->window, key, value);
}

static void dictionary_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	MetaDataPlugin *metadata_plugin = (MetaDataPlugin *) data; 
	
	AddKeyValueHistoryToList(metadata_plugin, key, val);     	
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_SetMetaDataFromDictionary(IcsViewerWindow *window, dictionary *d)
{
	dictionary_foreach(d, dictionary_keyval_callback, window->metadata_plugin);
	
	return 0;
}

static int CVICALLBACK OnExportClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	MetaDataPlugin *metadata_plugin = (MetaDataPlugin *) callbackData; 
	char *default_extensions = "*.dat;";
	char fname[GCI_MAX_PATHNAME_LEN] = "";
	char directory[GCI_MAX_PATHNAME_LEN] = "";
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) metadata_plugin;
	FILE *fp;
	int i, fsize, number_of_items;
	char key[ICS_LINE_LENGTH], value[ICS_LINE_LENGTH];
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetNumTreeItems (metadata_plugin->info_panel, INFO_TREE, VAL_SIBLING, 0, 0, VAL_NEXT_PLUS_SELF, 0, &number_of_items);

			if(!number_of_items)
				return -1;
				
			if (FileSelectPopup (GetDefaultDirectoryPath(plugin->window, directory), "*.dat",
				default_extensions, "Export Data As", VAL_OK_BUTTON, 0, 0, 1, 1, fname) <= 0) {
				return -1;
			}

			// If the conf file does exist clear any read only bit 
			if (FileExists (fname, &fsize))
    			SetFileAttrs (fname, 0, -1, -1, -1);

    		fp = fopen (fname, "w");
    
    		if (fp == NULL)
    			return GCI_IMAGING_ERROR;
			
			for(i=0; i < number_of_items; i++) {
	
				GetKeyValueFromTreeItem(metadata_plugin, i, key, value);
		
				fprintf(fp, "%s\t%s\n", key, value);
			}

			fclose(fp);
    
    		// set read-only 
   			SetFileAttrs (fname, 1, -1, -1, -1);

			break;
		}
			 
	return 0;
}



static int CVICALLBACK OnOkButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	MetaDataPlugin *metadata_plugin = (MetaDataPlugin *) callbackData;
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) metadata_plugin;  
	
	int i, number_of_items;     
	char key[ICS_LINE_LENGTH], value[ICS_LINE_LENGTH];      
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			GetNumTreeItems (metadata_plugin->info_panel, INFO_TREE, VAL_SIBLING, 0, 0, VAL_NEXT_PLUS_SELF, 0, &number_of_items);

			if(number_of_items) {
	
				// Store all history strings
				GCI_ImagingWindow_EmptyMetaData(plugin->window);

				for(i=0; i < number_of_items; i++) {
					
					GetKeyValueFromTreeItem(metadata_plugin, i, key, value);
		
					GCI_ImagingWindow_SetMetaDataKey(plugin->window, key, value);  
				}	
			}

			ics_viewer_registry_save_panel_size_position(plugin->window, metadata_plugin->info_panel);

			DiscardPanel(metadata_plugin->info_panel);
			metadata_plugin->info_panel = 0;

			break;
		}
		
	return 0;
}


static int CVICALLBACK OnAddButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	MetaDataPlugin *metadata_plugin = (MetaDataPlugin *) callbackData; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			AddKeyValueHistoryToList(metadata_plugin, "New Key", "New Value");
			
			break;
		}
	return 0;
}

static int CVICALLBACK OnRemoveButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int itemIndex;
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetActiveTreeItem (panel, INFO_TREE, &itemIndex); 
			
			DeleteListItem (panel, INFO_TREE, itemIndex, 1);

			break;
		}
	return 0;
}

static void OnMetaDataIterate (dictionary * d, const char *key, const char *val, void *data)
{
	MetaDataPlugin *metadata_plugin = (MetaDataPlugin *) data; 

	AddKeyValueToTree(metadata_plugin, key, val); 
}

static void metadata_update_metadata (MetaDataPlugin *metadata_plugin) 
{
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) metadata_plugin;

	//GetCtrlAttribute(metadata_plugin->info_panel, INFO_TREE, ATTR_FIRST_VISIBLE_LINE, &line);
	//SCROLLINFO si = { sizeof(si), SIF_TRACKPOS };
    
    //GetScrollInfo((HWND) metadata_plugin->window_handle, SB_VERT, &si);
    
	if(ClearListCtrl (metadata_plugin->info_panel, INFO_TREE) < 0)
		return;

	dictionary_foreach(plugin->window->metadata, OnMetaDataIterate, plugin);

	//SetCtrlAttribute(metadata_plugin->info_panel, INFO_TREE, ATTR_FIRST_VISIBLE_LINE, line);

	//SetScrollInfo((HWND) metadata_plugin->window_handle, SB_VERT, &si, 1);
}

static void resize_controls(MetaDataPlugin *metadata_plugin)
{
	ui_module_anchor_control_to_panel_edge(metadata_plugin->info_panel, INFO_TREE, 10, 10, 10, 50);

	// Move Hide Button 
	ui_module_move_control_pixels_from_right(metadata_plugin->info_panel, INFO_OK_BUTTON, 10);     
	ui_module_move_control_pixels_from_bottom(metadata_plugin->info_panel, INFO_OK_BUTTON, 10); 

	ui_module_move_control_pixels_from_right(metadata_plugin->info_panel, INFO_EXPORT, 70);     
	ui_module_move_control_pixels_from_bottom(metadata_plugin->info_panel, INFO_EXPORT, 10); 

	SetCtrlAttribute(metadata_plugin->info_panel, INFO_ADD, 10); 
	ui_module_move_control_pixels_from_bottom(metadata_plugin->info_panel, INFO_ADD, 10); 

	SetCtrlAttribute(metadata_plugin->info_panel, INFO_REMOVE, 70); 
	ui_module_move_control_pixels_from_bottom(metadata_plugin->info_panel, INFO_REMOVE, 10);
}

static LRESULT CALLBACK StatusWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MetaDataPlugin *metadata_plugin = (MetaDataPlugin *) GetWindowLongPtr (hwnd, GWLP_USERDATA); 

	switch(message) {
			
    	case WM_EXITSIZEMOVE:
    	{
			resize_controls(metadata_plugin);
			
			break;
    	}

		// Enoying flicker. Not got time to fix yet
		//case WM_SIZING:
    	//{
		//	resize_controls(metadata_plugin);
			
		//	return 0;
    	//}

      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) metadata_plugin->original_wnd_proc,
							hwnd, message, wParam, lParam);
}

static void metadata_setup_wnd_proc(MetaDataPlugin *metadata_plugin, int panel_id)
{
	int window_handle;

	GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   

	metadata_plugin->window_handle = (HWND) window_handle;

	metadata_plugin->original_wnd_proc = GetWindowLongPtr ((HWND) window_handle, GWL_WNDPROC);

	/* Store the window structure with the window for use in WndProc */
	SetWindowLongPtr ((HWND)window_handle, GWLP_USERDATA, (LONG_PTR) metadata_plugin);
	
	 /* Set the new Wnd Proc to be called */	
	SetWindowLongPtr ((HWND)window_handle, GWL_WNDPROC, (LONG_PTR) StatusWndProc);
}

static void metadata_restore_cvi_wnd_proc(MetaDataPlugin *metadata_plugin)
{
	/* Set the original Wnd Proc to be called */
	if(metadata_plugin->original_wnd_proc != 0)
		SetWindowLongPtr ((HWND) metadata_plugin->window_handle, GWL_WNDPROC, metadata_plugin->original_wnd_proc);
}

static void metadata_show_dialog (MetaDataPlugin *metadata_plugin) 
{
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) metadata_plugin; 
	
	char buffer[100];
	
	if(metadata_plugin->info_panel != 0)
		return;

	metadata_plugin->info_panel = LoadPanel(0, uir_file_path, INFO);

	//GetPanelAttribute (metadata_plugin->info_panel, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);
	//metadata_plugin->window_handle = (HWND) window_handle;

	ics_viewer_set_panel_to_top_left_of_window(plugin->window, metadata_plugin->info_panel);

	memset(buffer, 0, 100);
	
	if(ClearListCtrl (metadata_plugin->info_panel, INFO_TREE) < 0)
		return;

	// The add and remove buttons will remain dimmed when the app using the image window
	// is proving the meta data.
	SetCtrlAttribute(metadata_plugin->info_panel, INFO_ADD, ATTR_DIMMED, 1);
	SetCtrlAttribute(metadata_plugin->info_panel, INFO_REMOVE, ATTR_DIMMED, 1);
		
	metadata_setup_wnd_proc(metadata_plugin, metadata_plugin->info_panel);   

	// Don't allow adding and removing meta data when a larger app is providing it.
	if ( InstallCtrlCallback (metadata_plugin->info_panel, INFO_ADD, OnAddButtonClicked, metadata_plugin) < 0)
		return;
		
	if ( InstallCtrlCallback (metadata_plugin->info_panel, INFO_REMOVE, OnRemoveButtonClicked, metadata_plugin) < 0)
		return;
		
	SetCtrlAttribute(metadata_plugin->info_panel, INFO_ADD, ATTR_DIMMED, 0);
	SetCtrlAttribute(metadata_plugin->info_panel, INFO_REMOVE, ATTR_DIMMED, 0);
		
	if ( InstallCtrlCallback (metadata_plugin->info_panel, INFO_EXPORT, OnExportClicked, metadata_plugin) < 0)
		return;
	
	if ( InstallCtrlCallback (metadata_plugin->info_panel, INFO_OK_BUTTON, OnOkButtonClicked, metadata_plugin) < 0)
		return;	
	
	if ( InstallCtrlCallback (metadata_plugin->info_panel, INFO_STRING, StringKeyPressed, metadata_plugin) < 0)
		return;
	
	if ( InstallCtrlCallback (metadata_plugin->info_panel, INFO_TREE, OnTreeChanged, metadata_plugin) < 0)
		return;
	
	ConvertToEditableTreeCells (metadata_plugin->info_panel, INFO_TREE, INFO_STRING);  
	
	DisplayPanel(metadata_plugin->info_panel);
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	MetaDataPlugin *metadata_plugin = (MetaDataPlugin *) callbackData;

	metadata_show_dialog(metadata_plugin);	

	metadata_update_metadata (metadata_plugin);
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	MetaDataPlugin * metadata_plugin = (MetaDataPlugin *) plugin; 

	metadata_restore_cvi_wnd_proc(metadata_plugin);
}

static void on_image_displayed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	MetaDataPlugin *metadata_plugin = (MetaDataPlugin *) plugin;

	metadata_update_metadata (metadata_plugin);
}

ImageWindowPlugin* metadata_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "MetaDataPlugin", sizeof(MetaDataPlugin));

	MetaDataPlugin * metadata_plugin = (MetaDataPlugin *) plugin;
	
	metadata_plugin->info_panel = 0;
	metadata_plugin->app_provided_metadata = NULL;
	
	Plugin_AddMenuItem(plugin, "View//Image Information",
		VAL_MENUKEY_MODIFIER | 'M', on_menu_clicked, plugin);

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	PLUGIN_VTABLE(plugin, on_image_displayed) = on_image_displayed;

	return plugin;
}


// Editable Tree Stuff
int CVICALLBACK OnTreeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Point 	pt;
	int		index, col;
	
	switch (event)
		{
		case EVENT_LEFT_DOUBLE_CLICK:
		
			SetCtrlAttribute (panel, control, ATTR_FULL_ROW_SELECT, 0); 
			
			pt.y = eventData1;
			pt.x = eventData2;
			GetIndexFromPoint (panel, control, pt, &index, 0, &col);
			EditTreeCell (panel, control, INFO_STRING, index, col);
			
			break;
			
		case EVENT_LEFT_CLICK:
		
			pt.y = eventData1;
			pt.x = eventData2;
			GetIndexFromPoint (panel, control, pt, &index, 0, &col);
			
			SetCtrlAttribute (panel, control, ATTR_FULL_ROW_SELECT, 1);   
			
			if(index >= 0) {
				SetActiveTreeItem (panel, control, index, VAL_REPLACE_SELECTION_WITH_ITEM);
			}

			break;
			
		case EVENT_DISCARD:
			break;
			
		case EVENT_LOST_FOCUS:
			if (eventData1 == INFO_STRING)
				break;
			/* fall thru */
		default:
		
			EndTreeCellEdit (panel, INFO_TREE, INFO_STRING);
				
			break;
		}
	return 0;
}

int CVICALLBACK StringKeyPressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		
		case EVENT_COMMIT:
			
			if (eventData1 == VAL_ENTER_VKEY || eventData1 == VAL_ESC_VKEY)
				EndTreeCellEdit (panel, INFO_TREE, INFO_STRING);

			break;
	}
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
		}
	return 0;
}

void ConvertToEditableTreeCells (int panel, int tree, int string)
{
	SetCtrlAttribute (panel, string, ATTR_VISIBLE, 0);

	SetCtrlAttribute (panel, string, ATTR_ZPLANE_POSITION, 0);   
}

void EditTreeCell (int panel, int tree, int string, int index, int col)
{
	int 	top, left, frameThickness, colLeft, width, intVal;
	char 	buf[256];
	
	if (col < 0 || index < 0) 
		goto Done;
	
	GetCtrlAttribute (panel, string, ATTR_FRAME_THICKNESS, &frameThickness);
	colLeft = GetTreeColumnLeft (panel, tree, col);
	width = GetStringWidthForColumn (panel, tree, col, colLeft) + frameThickness;
	top = GetTreeItemTop (panel, tree, index) - frameThickness;
	left = colLeft - frameThickness;
	
	SetCtrlAttribute (panel, string, ATTR_TOP,      top);
	SetCtrlAttribute (panel, string, ATTR_LEFT,     left);
	SetCtrlAttribute (panel, string, ATTR_WIDTH,    width);
	UpdateStringCtrl (panel, tree,   string, index, col, 1);
	
	//GetCtrlAttribute (panel, tree,   ATTR_LABEL_LEFT, &left);
	//GetCtrlAttribute (panel, tree,   ATTR_LABEL_TOP,  &top);
	SetCtrlAttribute (panel, string, ATTR_LABEL_LEFT, 0);
	SetCtrlAttribute (panel, string, ATTR_LABEL_TOP,  -50);
	
	/* set text attrs of string control */
	
	GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_BGCOLOR, &intVal);
	SetCtrlAttribute     (panel, string,           ATTR_TEXT_BGCOLOR, intVal);
	GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_BOLD, &intVal);
	SetCtrlAttribute     (panel, string,           ATTR_TEXT_BOLD, intVal);
	GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_COLOR, &intVal);
	SetCtrlAttribute     (panel, string,           ATTR_TEXT_COLOR, intVal);
	GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_FONT, buf);
	SetCtrlAttribute     (panel, string,           ATTR_TEXT_FONT, buf);
	GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_ITALIC, &intVal);
	SetCtrlAttribute     (panel, string,           ATTR_TEXT_ITALIC, intVal);
	GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_POINT_SIZE, &intVal);
	SetCtrlAttribute     (panel, string,           ATTR_TEXT_POINT_SIZE, intVal);
	GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_STRIKEOUT, &intVal);
	SetCtrlAttribute     (panel, string,           ATTR_TEXT_STRIKEOUT, intVal);
	GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_UNDERLINE, &intVal);
	SetCtrlAttribute     (panel, string,           ATTR_TEXT_UNDERLINE, intVal);
	
	SetCtrlAttribute (panel, string, ATTR_VISIBLE,  1);
	SetActiveCtrl (panel, string);
	
	SetCtrlAttribute (panel, tree,   ATTR_DISABLE_TOOLTIPS, 1); /* todo: store value */

Done:
	return;
}

int GetStringWidthForColumn (int panel, int tree, int col, int colLeft)
{
	int 	colWidth, treeWidth, treeLeft, scrollBarWidth, scrollBars;

	GetTreeColumnAttribute (panel, tree, col, ATTR_COLUMN_WIDTH, &colWidth);
	GetCtrlAttribute (panel, tree, ATTR_WIDTH,       &treeWidth);
	GetCtrlAttribute (panel, tree, ATTR_SCROLL_BARS, &scrollBars);
	GetCtrlAttribute (panel, tree, ATTR_LEFT,        &treeLeft);
	if (scrollBars & VAL_VERT_SCROLL_BAR)
		{
		GetCtrlAttribute (panel, tree, ATTR_SCROLL_BAR_SIZE, &scrollBarWidth);
		treeWidth -= scrollBarWidth;
		}
	if (colLeft + colWidth > treeLeft + treeWidth)
		colWidth -= (colLeft+colWidth) - (treeLeft+treeWidth);
	return colWidth;
}

int GetTreeColumnLeft (int panel, int tree, int col)
{
	int 	i, colLeft = 0, numCols, frameThickness, visible, 
			width, offset;
	
	GetCtrlAttribute (panel, tree, ATTR_LEFT,            &colLeft);
	GetCtrlAttribute (panel, tree, ATTR_FRAME_THICKNESS, &frameThickness);
	GetCtrlAttribute (panel, tree, ATTR_HSCROLL_OFFSET,  &offset);
	colLeft += frameThickness + 1 - offset;
	GetNumTreeColumns (panel, tree, &numCols);
	for (i=0; i<numCols && i < col; i++)
		{
		GetTreeColumnAttribute (panel, tree, i, ATTR_COLUMN_VISIBLE, &visible);
		if (!visible)
			continue;
		GetTreeColumnAttribute (panel, tree, i, ATTR_COLUMN_WIDTH, &width);
		colLeft += width;
		}
	return colLeft;
}

int GetTreeItemTop (int panel, int tree, int index)
{
	int itemTop = 0, i = 0, numItems, height, colLabelHeight,
		frameThickness, offset;
	
	GetCtrlAttribute (panel, tree, ATTR_TOP,                  &itemTop);
	GetCtrlAttribute (panel, tree, ATTR_FRAME_THICKNESS,      &frameThickness);
	GetCtrlAttribute (panel, tree, ATTR_COLUMN_LABELS_HEIGHT, &colLabelHeight);
	GetCtrlAttribute (panel, tree, ATTR_VSCROLL_OFFSET,       &offset);
	itemTop += frameThickness + colLabelHeight - offset;
	
	GetNumListItems (panel, tree, &numItems);
	if (numItems <= 0)
		goto Done;

	while (i >= 0 && i < index)
		{
		GetTreeItemAttribute (panel, tree, i, ATTR_ITEM_ACTUAL_HEIGHT, &height);
		itemTop += height;
		GetTreeItem (panel, tree, VAL_ALL, 0, i, VAL_NEXT, VAL_EXPOSED, &i);
		} 
		
Done:
	return itemTop;
}

void GetCtrlCornerCoordinates (int panel, int ctrl, int corner, Point *pt)
{
	int height, width, left, top;
	
	if (!pt)
		goto Done;
	GetCtrlAttribute (panel, ctrl, ATTR_TOP,    &top);
	GetCtrlAttribute (panel, ctrl, ATTR_LEFT,   &left);
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH,  &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);
	pt->y = top;
	pt->x = left;
	if (corner & kCornerRight)
		pt->x += width;
	if (corner & kCornerBottom)
		pt->y += height;
		
Done:
	return;
}

void EndTreeCellEdit (int panel, int tree, int string)
{
	int 	editing, index, col, frameThickness;
	Point   pt;
	
	GetCtrlAttribute (panel, string, ATTR_VISIBLE, &editing);

	if (editing)
	{
		SetActiveCtrl (panel, tree);
		SetCtrlAttribute (panel, string, ATTR_VISIBLE, 0);
		GetCtrlAttribute (panel, string, ATTR_FRAME_THICKNESS, &frameThickness);
		GetCtrlCornerCoordinates (panel, string, 0, &pt);
		pt.x += frameThickness + 1;
		pt.y += frameThickness + 1;
		GetIndexFromPoint (panel, tree, pt, &index, 0, &col);
	
		if (index < 0 || col < 0)
		{
			GetCtrlCornerCoordinates (panel, string, kCornerRight, &pt);
			pt.x -= frameThickness + 1;
			pt.y += frameThickness + 1;
			GetIndexFromPoint (panel, tree, pt, &index, 0, &col);
		}
			
		if (index < 0 || col < 0)
		{
			GetCtrlCornerCoordinates (panel, string, kCornerBottom, &pt);
			pt.x += frameThickness + 1;
			pt.y -= frameThickness + 1;
			GetIndexFromPoint (panel, tree, pt, &index, 0, &col);
		}
		
		if (index < 0 || col < 0)
		{
			GetCtrlCornerCoordinates (panel, string, kCornerRight | kCornerBottom, &pt);
			pt.x -= frameThickness + 1;
			pt.y -= frameThickness + 1;
			GetIndexFromPoint (panel, tree, pt, &index, 0, &col);
		}
			
		if (index >= 0 && col >= 0)
			UpdateStringCtrl (panel, tree, string, index, col, 0);
			
		SetCtrlAttribute (panel, tree, ATTR_DISABLE_TOOLTIPS, 0); /* todo: restore value */
	}
	
	return;
}

void UpdateStringCtrl (int panel, int tree, int string, int index, int col, int toString)
{
	char buf[256];

	if (toString)
	{
		GetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_TEXT, buf);
		SetCtrlVal (panel, string, buf);
	}
	else
	{
		GetCtrlVal (panel, string, buf);
		SetTreeCellAttribute (panel, tree, index, col, ATTR_LABEL_TEXT, buf);
	}
}



