#include "RegionOfInterest.h"
#include "cell.h"
#include "cellmap.h"
#include "cellmap_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "camera\gci_camera.h"
#include "cell_info_ui.h"

#include <utility.h>
#include <userint.h>
#include <ansi_c.h>
#include <formatio.h>
#include "toolbox.h"


////////////////////////////////////////////////////////////////////
// Module to manage cell/object map for the microfocus system
// Glenn Pierce - July 2005
////////////////////////////////////////////////////////////////////
// RJL October 2006
// When you click on an object the stage moves to position the object at the
// image centre and also implements the focal plane.
// Set aspect ratio of panel to match the ROI.
// Added functions gci_cellmap_current_cell(), gci_cellmap_next_cell(),
// gci_cellmap_get_cell(), gci_cellmap_set_cell_type() and gci_cellmap_first_cell()
////////////////////////////////////////////////////////////////////
// Rosalind Locke - 18 Nov 2005
// Also apply the misalignment angle to the stage moves.
////////////////////////////////////////////////////////////////////////////
// Ros Locke - 30 Nov 2005
// Implement GP's nearest neighbour algorithm.
// Typically reduce distance to travel by 30%
////////////////////////////////////////////////////////////////////
// Ros Locke - 1 Dec 2005
// Changes required to revisit only objects of the type specified.
////////////////////////////////////////////////////////////////////
// Ros Locke - 28 Feb 2005
// Beam position was wrong for binned images.
// Stop timers when cell map clicked on to prevent sluggish response.
////////////////////////////////////////////////////////////////////
// Ros Locke - June 2006
// Speed up gci_cellmap_load_file() by reading entire columns from the file
// Don't write the cell data to Excel one line at a time. It's now done
// frame by frame in cell_boxing.c
////////////////////////////////////////////////////////////////////


static int VOID_MAP_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (CellMap *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (CellMap *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int VOID_CELL_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Cell *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Cell *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

void gci_cellmap_mapClickedHandler(CellMap* map, void (*handler) (int cell_id, void *callback_data), void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(map), "MapClick", handler, data) == SIGNAL_ERROR) {
		return;
	}
}


int CVICALLBACK onGraphMousePress (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int top, left;
	char msg[50];
	Cell cell, nearest_cell;
	CellMap *map = (CellMap*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			if (map->empty)
				break;						//empty map
				
			//GCI_ImagingStopAllTimers();
			//GCI_ElectronBeamSource_StopTimer();
			
			/* Get the cursor's position */
        	GetGraphCursor (map->parent_panel, map->cellmap_graph_id, 1, &(cell.x), &(cell.y));

			nearest_cell = gci_cellmap_find_nearest_neighbour(map, &cell);

			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(map), "MapClick", GCI_INT, nearest_cell.id);
			
			//GCI_ImagingStartAllTimers();
			//GCI_ElectronBeamSource_StartTimer();
			break;
			
		case EVENT_LEFT_DOUBLE_CLICK:
			//Display the cell info panel over the point clicked on
			GetPanelAttribute (map->parent_panel, ATTR_LEFT, &left);
			GetPanelAttribute (map->parent_panel, ATTR_TOP, &top);
			top += eventData1;
			left += eventData2;
			SetPanelPos (map->cellinfo_panel, top, left);
			
        	GetGraphCursor (map->parent_panel, map->cellmap_graph_id, 1, &(cell.x), &(cell.y));
			nearest_cell = gci_cellmap_find_nearest_neighbour(map, &cell);
			SetCtrlVal(map->cellinfo_panel, CELL_PANEL_TYPE_RING, nearest_cell.type);
			sprintf(msg, "x=%.0f, y=%.0f",nearest_cell.x, nearest_cell.y);
			SetCtrlVal(map->cellinfo_panel, CELL_PANEL_POSITION_TEXT, msg);
			
			DisplayPanel(map->cellinfo_panel);
		
			break;
			
			
		}
	return 0;
}


int CVICALLBACK onMapClose (int panel, int event, void *callbackData,
                            int eventData1, int eventData2)
{
	CellMap *map = (CellMap*) callbackData;

	switch (event)
	{
		case EVENT_CLOSE:

			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(map), "MapClose");

			break;
	}
	
	return 0;
}


int gci_cellmap_set_cell_type(CellMap* map, int id, int type)
{
	Cell cell;
	long cell_colour;

	if (id < 1) return -1;
	if ((type < 1) || (type > 3)) return -1;
	
	cell = cell_list_get_cell_with_id(map->cell_list, id);
	if (&cell == NULL) return -1;
	
	cell.type = type;

	cell_list_update_cell_with_id(map->cell_list, id, cell); 
	map->cell_list_invalidated = 1;
	map->projected_array_invalidated = 1;

	cell_colour = gci_cell_get_colour_for_type(&cell);

	PlotPoint (map->parent_panel, map->cellmap_graph_id, cell.x, cell.y, VAL_SMALL_SOLID_SQUARE, cell_colour);
	map->empty = 0;

	return CELLMAP_SUCCESS;
}

int CVICALLBACK onCellInfoOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int type;
	long cell_colour;
	Cell cell;
	CellMap *map = (CellMap*) callbackData;

	switch (event)
	{
		case EVENT_COMMIT:
	
			GetCtrlVal(map->cellinfo_panel, CELL_PANEL_TYPE_RING, &type);

			cell = cell_list_get_cell_with_id(map->cell_list, map->current_cell.id);
			cell.type = type;

			cell_list_update_cell_with_id(map->cell_list, map->current_cell.id, cell); 
			map->cell_list_invalidated = 1;
			map->projected_array_invalidated = 1;

			cell_colour = gci_cell_get_colour_for_type(&cell);

			PlotPoint (map->parent_panel, map->cellmap_graph_id, cell.x, cell.y, VAL_SMALL_SOLID_SQUARE, cell_colour);
			map->empty = 0;

			HidePanel(map->cellinfo_panel);

			break;
	}
	
	return 0;
}

int CVICALLBACK onCellInfoClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	CellMap *map = (CellMap *) callbackData;

	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel(map->cellinfo_panel); 

			break;
	}
	
	return 0;
}


static void gci_cellmap_init(CellMap* map) 
{
	map->projected_cell_array = NULL;
	map->projected_array_invalidated = 1;
	map->cell_list_invalidated = 0;
	map->standard_deviation_x = 0.0;
	map->standard_deviation_y = 0.0;
	
	map->empty = 1;
	
	ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(map), map->parent_panel, 0);   
}


CellMap* gci_cellmap_new(char *title, int top, int left, int width, int height)
{
	CellMap *map = (CellMap*) malloc(sizeof(CellMap));   
	
	ui_module_constructor(UIMODULE_CAST(map), "CellMap");
	
	map->parent_panel = NewPanel (0, title, top, left, height, width); 

	SetPanelAttribute(map->parent_panel, ATTR_SIZABLE, 0);

	map->top_offset = 0;
	map->left_offset = 0;

	map->cell_list = cell_list_new();
	map->cellmap_graph_id = NewCtrl (map->parent_panel, CTRL_GRAPH, "",  0, 0);
	
	gci_cellmap_init(map);
	map->roi = NULL;

	map->cell_map_file = (char *) calloc ( 500, sizeof(char) );
	
	map->cellinfo_panel = ui_module_add_panel(UIMODULE_CAST(map), "cell_finding_ui.uir", CELL_PANEL, 0);
	
	InstallCtrlCallback (map->cellinfo_panel, CELL_PANEL_OK_BUTTON, onCellInfoOk, map); 

	InstallCtrlCallback (map->cellinfo_panel, CELL_PANEL_CLOSE_BUTTON, onCellInfoClose, map);
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(map), "CellFound", VOID_CELL_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(map), "MapClose", VOID_MAP_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(map), "MapClick", VOID_INT_MARSHALLER);  
	
	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_WIDTH, width);
	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_HEIGHT, height);

	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_YGRID_VISIBLE, 0);
	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_XGRID_VISIBLE, 0);

	// Allow the user to change the cusor position
	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_CTRL_MODE, VAL_HOT);

	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_XLABEL_VISIBLE, 0);
	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_YLABEL_VISIBLE, 0);

	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_BORDER_VISIBLE, 0);

	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_PLOT_BGCOLOR, VAL_BLACK);
	
	// Add a cusor to the graph
	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_NUM_CURSORS, 2);
	
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 1, ATTR_CURSOR_MODE, VAL_FREE_FORM);
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 1, ATTR_CURSOR_COLOR, VAL_WHITE);
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 1, ATTR_CROSS_HAIR_STYLE, VAL_NO_CROSS);	
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 1, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
	
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_MODE, VAL_FREE_FORM);
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_POINT_STYLE, VAL_SOLID_SQUARE);
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CROSS_HAIR_STYLE, VAL_NO_CROSS); 
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_ENABLED, 0); 
	
	InstallCtrlCallback (map->parent_panel, map->cellmap_graph_id, onGraphMousePress, map);

	InstallPanelCallback(map->parent_panel, onMapClose, map);

	return map;
}


void gci_cellmap_display(CellMap* map)
{
	DisplayPanel(map->parent_panel);
}

void gci_cellmap_hide(CellMap* map)
{
	HidePanel(map->parent_panel);
}


void gci_cellmap_destroy(CellMap* map)
{
	cell_list_destroy(map->cell_list);
	
	// Clear the cell map file name 
	if(map->cell_map_file != NULL)
		free(map->cell_map_file);
	
	if(map->projected_cell_array != NULL)
		free(map->projected_cell_array);
		
	if(map->parent_panel != -1) {
		ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(map), map->parent_panel, 1); 
		DiscardPanel(map->parent_panel); 
		map->parent_panel = -1;
	}
		
	ui_module_destroy(UIMODULE_CAST(map));

	free(map);
}


void gci_cellmap_close_handler(CellMap* map, MAP_EVENT_HANDLER handler, void *callback_data)
{
	if(GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(map), "MapClose", handler, callback_data) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
}


/*
static void gci_cellmap_setup_properties(CellMap *map)
{
	char buffer[200];
	float region_width, region_height, startx, starty;

	// Get the Region Size
	GCI_ReadStringFromExcelCell(2, 10, 2, buffer);
	
	sscanf(buffer, "%f x %f", &region_width, &region_height);
	
	map->region_width = region_width;
	map->region_height = region_height;
	
	GCI_ReadStringFromExcelCell(2, 11, 2, buffer);
	
	// Get the starting fram x,y
	sscanf(buffer, "%f,%f", &startx, &starty);
	
	gci_cellmap_set_region_size_pos(map, starty, startx, region_width, region_height);
	
	return;
}
*/


void gci_cellmap_clear(CellMap *map)
{
    ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(map), map->parent_panel, 1); 
	
	DeleteGraphPlot (map->parent_panel, map->cellmap_graph_id, -1, VAL_IMMEDIATE_DRAW);

	cell_list_destroy(map->cell_list);

	map->cell_list = cell_list_new();
	
	gci_cellmap_init(map);
	
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);

}


/*
void gci_cellmap_new_file(CellMap *map, char *filepath)
{
	int num_of_entries;
	
	if(filepath != NULL)
		strcpy(map->cell_map_file, filepath);
		
	CellFinding_CreateNewExcelFile(map->cell_map_file, &num_of_entries); 
}
*/


static int gci_cellmap_add_existing_cell(CellMap *map, Cell cell)
{
	//int cell_colour;

	cell_list_add (map->cell_list, cell);

	//cell_colour = gci_cell_get_colour_for_type(&cell);

	map->cell_list_invalidated = 1;
	map->projected_array_invalidated = 1;
	
	return CELLMAP_SUCCESS;
}


/*
int gci_cellmap_load_file(CellMap *map)
{
	int num_of_entries, i, msgpnl;
	char directory_buffer[500];
	double *celltype, *id;
	double *x_position, *y_position;
	double x_min = 999999, y_min = 999999, x_max = -999999, y_max = -999999;
	Cell cell;
	double *area, *perimeter, *shape, *intensity, *std_dev, *dose;
	
	cell_list_destroy(map->cell_list);
	map->cell_list = cell_list_new();
	gci_cellmap_init(map);

	DeleteGraphPlot (map->parent_panel, map->cellmap_graph_id, -1, VAL_IMMEDIATE_DRAW); 

	str_get_path_for_my_documents(directory_buffer);

	if (FileSelectPopup (directory_buffer, "*.xls", "*.xls;*.*", "", VAL_OK_BUTTON, 0, 0, 1, 1, map->cell_map_file) != 1)
		return -1;

	SetWaitCursor(1);
	
	//Display a friendly message to explain the pause
	msgpnl = FindAndLoadUIR(0, "cellmap_ui.uir", XL_LOADPNL);
	DisplayPanel(msgpnl);

	CellFinding_OpenExcelFile(map->cell_map_file, &num_of_entries);
	
	
	gci_cellmap_setup_properties(map);
	
	id = (double *)calloc(num_of_entries, sizeof(double));
	x_position = (double *)calloc(num_of_entries, sizeof(double));
	y_position = (double *)calloc(num_of_entries, sizeof(double));
	area = (double *)calloc(num_of_entries, sizeof(double));
	perimeter = (double *)calloc(num_of_entries, sizeof(double));
	shape = (double *)calloc(num_of_entries, sizeof(double));
	intensity = (double *)calloc(num_of_entries, sizeof(double));
	std_dev = (double *)calloc(num_of_entries, sizeof(double));
	celltype = (double *)calloc(num_of_entries, sizeof(double));
	dose = (double *)calloc(num_of_entries, sizeof(double));
	
	CellFinding_DoubleColumnFromExcel(1, 1, id, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 2, x_position, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 3, y_position, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 4, area, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 5, perimeter, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 6, shape, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 7, intensity, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 8, std_dev, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 9, celltype, num_of_entries);
	CellFinding_DoubleColumnFromExcel(1, 11, dose, num_of_entries);
	
	for(i=0; i < num_of_entries; i++) {
	
		cell = gci_cell_new((int)id[i], (int)celltype[i], x_position[i], y_position[i]);
		cell.area = area[i];
		cell.perimeter = perimeter[i];
		cell.shape = shape[i];
		cell.intensity = intensity[i];
		cell.intensity_stddev = std_dev[i];
		cell.dose = dose[i];
		gci_cellmap_add_existing_cell(map, cell );
	
		x_min = min(x_min, x_position[i]);
		y_min = min(y_min, y_position[i]);
		x_max = max(x_max, x_position[i]);
		y_max = max(y_max, y_position[i]);
	}	
	 
	// The nearest neighbour algoritm needs to know to resort the list.
	map->projected_array_invalidated == 1;
	
	gci_cellmap_set_region_size_pos(map, y_min, x_min, x_max-x_min+1, y_max-y_min+1);

	free(id);
	free(x_position);
	free(y_position);
	free(area);
	free(perimeter);
	free(shape);
	free(intensity);
	free(std_dev);
	free(celltype);
	free(dose);
	
	DiscardPanel(msgpnl);
	SetWaitCursor(0);

	return CELLMAP_SUCCESS;
}


int gci_cellmap_load_file_slow(CellMap *map)
{
	int id, num_of_entries, i, msgpnl;
	char directory_buffer[500];
	int celltype;
	double x_position, y_position;
	double x_min = 999999, y_min = 999999, x_max = -999999, y_max = -999999;
	Cell cell;
	double area, perimeter, shape, intensity, std_dev, dose;
	
	cell_list_destroy(map->cell_list);
	map->cell_list = cell_list_new();

	DeleteGraphPlot (map->parent_panel, map->cellmap_graph_id, -1, VAL_IMMEDIATE_DRAW); 

	str_get_path_for_my_documents(directory_buffer);

	if (FileSelectPopup (directory_buffer, "*.xls", "*.xls;*.*", "", VAL_OK_BUTTON, 0, 0, 1, 1, map->cell_map_file) != 1)
		return -1;

	SetWaitCursor(1);
	//Display a friendly message to explain the pause
	//msgpnl = LoadPanel (0, "Cellmap_ui.uir", XL_LOADPNL);
	msgpnl = FindAndLoadUIR(0, "cellmap_ui.uir", XL_LOADPNL);
	DisplayPanel(msgpnl);

	CellFinding_OpenExcelFile(map->cell_map_file, &num_of_entries);
	
	gci_cellmap_setup_properties(map);
	
	for(i=3; i < num_of_entries + 3; i++) {
	
		CellFinding_IntFromExcel(1, i, 1, &id);
		
		CellFinding_DoubleFromExcel(1, i, 2, &x_position);
		CellFinding_DoubleFromExcel(1, i, 3, &y_position);
		CellFinding_DoubleFromExcel(1, i, 4, &area);
		CellFinding_DoubleFromExcel(1, i, 5, &perimeter);
		CellFinding_DoubleFromExcel(1, i, 6, &shape);
		CellFinding_DoubleFromExcel(1, i, 7, &intensity);
		CellFinding_DoubleFromExcel(1, i, 8, &std_dev);
		
		CellFinding_IntFromExcel(1, i, 9, &celltype);
		CellFinding_DoubleFromExcel(1, i, 11, &dose);
	
		cell = gci_cell_new(id, celltype, x_position, y_position);
		cell.area = area;
		cell.perimeter = perimeter;
		cell.shape = shape;
		cell.intensity = intensity;
		cell.intensity_stddev = std_dev;
		cell.dose = dose;
		gci_cellmap_add_existing_cell(map, cell );
		
		x_min = min(x_min, x_position);
		y_min = min(y_min, y_position);
		x_max = max(x_max, x_position);
		y_max = max(y_max, y_position);
	}
	
	// The nearest neighbour algoritm needs to know to resort the list.
	map->projected_array_invalidated == 1;
	
	gci_cellmap_set_region_size_pos(map, y_min, x_min, x_max-x_min+1, y_max-y_min+1);

	DiscardPanel(msgpnl);
	SetWaitCursor(0);

	return CELLMAP_SUCCESS;
}
*/


int gci_cellmap_get_region_size_pos(CellMap *map, double *top, double *left, double *width, double *height)
{
	*top = map->region_top;
	*left = map->region_left;
	*width = map->region_width;
	*height = map->region_height;

	return CELLMAP_SUCCESS;	
}


int gci_cellmap_set_region_size_pos(CellMap *map, double top, double left, double width, double height)
{
	double aspectRatio;
	int panel_width, panel_height;
	
	map->region_top = top;
	map->region_left = left;
	map->region_width = width;
	map->region_height = height;

	aspectRatio = width/height;

	if (width > height) {
		panel_width = 400;
		panel_height = (int)ceil(400.0/aspectRatio);
	}
	else {
		panel_height = 400;
		panel_width = (int)ceil(400.0*aspectRatio);
	}
	
	SetPanelAttribute (map->parent_panel, ATTR_WIDTH, panel_width);
	SetPanelAttribute (map->parent_panel, ATTR_HEIGHT, panel_height);

	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_WIDTH, panel_width);
	SetCtrlAttribute(map->parent_panel, map->cellmap_graph_id, ATTR_HEIGHT, panel_height);
	
	SetAxisScalingMode (map->parent_panel, map->cellmap_graph_id, VAL_BOTTOM_XAXIS, VAL_MANUAL, left, left + width);
	SetAxisScalingMode (map->parent_panel, map->cellmap_graph_id, VAL_LEFT_YAXIS, VAL_MANUAL, top, top + height);
	
	SetCtrlAttribute (map->parent_panel, map->cellmap_graph_id, ATTR_YREVERSE, 1);
	
	return CELLMAP_SUCCESS;	
}


int gci_cellmap_set_region(CellMap *map, Rect rect)
{
	return gci_cellmap_set_region_size_pos(map, rect.top, rect.left, rect.width, rect.height);
}

int gci_cellmap_add_cell(CellMap *map, Cell cell)
{
	cell_list_add (map->cell_list, cell);

	map->cell_list_invalidated = 1;
	map->projected_array_invalidated = 1;
	
	return CELLMAP_SUCCESS;
}

int gci_cellmap_current_cell(CellMap *map, int *type)
{
	if (&map->current_cell == NULL)
		return -1;
		
	*type = map->current_cell.type;
	
	return map->current_cell.id;
}

int gci_cellmap_first_cell(CellMap *map)
{
	Cell *next_cell; 
	
	//Move stage to first object position
	cell_list_reset(map->cell_list); 
	next_cell = cell_list_get_next(map->cell_list);
	if (next_cell == NULL) return -1;

	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_YELLOW);  
	SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, next_cell->x, next_cell->y); 
	
	map->current_cell = *next_cell;

	return CELLMAP_SUCCESS;
}


double gci_cellmap_pathlength(CellMap *map)
{
	Cell *next_cell; 
	double x1, y1, x2, y2, path = 0.0;
	
	cell_list_reset(map->cell_list); 
	next_cell = cell_list_get_next(map->cell_list);
	if (next_cell == NULL) return -1.0;

	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_YELLOW);  
	SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, next_cell->x, next_cell->y); 
	
	x1 = next_cell->x;
	y1 = next_cell->y;
	
	map->current_cell = *next_cell;

	while (1) {
		
		next_cell = cell_list_get_next(map->cell_list);
		
		if (next_cell == NULL)
			break;

		x2 = next_cell->x;
		y2 = next_cell->y;

		path += sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
		
		map->current_cell = *next_cell;
		
		x1 = x2;
		y1 = y2;
	}
	
	return path;
}


int gci_cellmap_next_cell(CellMap *map)
{
	Cell *next_cell=NULL, *c=NULL; 
	
	next_cell = cell_list_get_next(map->cell_list);
	if (next_cell == NULL) return -1;
	
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_YELLOW);  
	SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, next_cell->x, next_cell->y); 
	
	map->current_cell = *next_cell;
	
	return CELLMAP_SUCCESS;
}

int gci_cellmap_get_cell(CellMap *map, int id)
{
	Cell cell;
	
	cell = cell_list_get_cell_with_id(map->cell_list, id);

	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_YELLOW);  
	SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, cell.x, cell.y); 
	
	map->current_cell = cell;
	
	return CELLMAP_SUCCESS;
}

int gci_cellmap_get_cell_type(CellMap *map, int id, int *type)
{
	Cell cell;
	
	cell = cell_list_get_cell_with_id(map->cell_list, id);

	//SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_YELLOW);  
	//SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, cell.x, cell.y); 
	
	*type = cell.type;
	//map->current_cell = cell;
	
	return CELLMAP_SUCCESS;
}

int gci_cellmap_get_nearest_cell(CellMap *map, int id)
{
	Cell cell, nearest_cell;
	
	cell = cell_list_get_cell_with_id(map->cell_list, id);
	nearest_cell = gci_cellmap_find_nearest_neighbour(map, &cell);

	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_YELLOW);  
	SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, nearest_cell.x, nearest_cell.y); 
	
	map->current_cell = nearest_cell;
	
	return CELLMAP_SUCCESS;
}


/* will have got a separate function to plots cells as
   there are problems plotting cells from a seperate thread 
   Here will can plot all cells from the main ui thread */
void gci_cellmap_plot_cells(CellMap *map)
{
	Cell *next_cell; 
	int cell_colour; 
	double top, left, width, height;
	
	cell_list_reset(map->cell_list); 
	
	gci_cellmap_get_region_size_pos(map, &top, &left, &width, &height); 
	
	while( (next_cell = cell_list_get_next(map->cell_list)) != NULL) {
	
		if (next_cell->plotted < 1) {
		
			cell_colour = gci_cell_get_colour_for_type(next_cell); 
		
			PlotPoint (map->parent_panel, map->cellmap_graph_id, next_cell->x,
				next_cell->y, VAL_SOLID_SQUARE, cell_colour);
		
			next_cell->plotted = 1;
			map->empty = 0;
		}
	}
	
}


static void calculate_standard_deviation(CellMap *map)
{
	Cell *next_cell;
	int i=0, size;
	double x_sumation = 0.0, y_sumation = 0.0, mean_x, mean_y;
	
	if(map->cell_list_invalidated == 0)
		return;
	
	size = cell_list_count(map->cell_list);
	
	if(size <= 1)
		return;
	
	cell_list_reset(map->cell_list); 
	
	while( (next_cell = cell_list_get_next(map->cell_list)) != NULL) {
	
		x_sumation += next_cell->x;
		y_sumation += next_cell->y;
	
		mean_x = x_sumation / size;
		mean_y = y_sumation / size;
		
		i++;
	}
	
	cell_list_reset(map->cell_list);
	
	i=0;
	x_sumation = 0.0;
	y_sumation = 0.0;
	
	while( (next_cell = cell_list_get_next(map->cell_list)) != NULL) {
	
		x_sumation += pow( (next_cell->x - mean_x), 2);
		y_sumation += pow( (next_cell->y - mean_y), 2);
		
		i++;
	}

	map->standard_deviation_x = sqrt(x_sumation / (size - 1));
	map->standard_deviation_y = sqrt(y_sumation / (size - 1)); 
}


static int binary_search (void *array, size_t object_size, int low, int high, const void *key,
						  int (__cdecl *cmp) (const void *, const void *) )
{
	int mid, comp_result;
	char *char_array;
	void *val;

	if (low == high)
		return low;
		
	mid = (low + high) / 2;

	char_array = (char *) array;

	val = char_array + (object_size * mid);

	comp_result = (*cmp) (key, val);

	if ( comp_result > 0 ) {
	
		return binary_search (array, object_size, mid + 1, high, key, cmp);
	}
	else if ( comp_result < 0 ) {
	
		return binary_search (array, object_size, low, mid, key, cmp);
	}
	
	return mid;
}


// This function returns the position of the cell in the projected cell array
// Which is the maximum possible neighbour.
// IE only the area between the search point and range returned here needs to be searched
// for any closer cells.
static int find_nearest_neighbour_pos_in_array(CellMap *map, Cell* projected_cell_array, int array_size,
															  int (__cdecl *cmp) (const void *, const void *), Cell *key)
{
	double range, range_temp, range_min;
	int i, pos_min;
	int pos = binary_search (projected_cell_array, sizeof(Cell), 0, array_size, key, cmp);
		
	// closest points are the one at pos and the one at pos - 1
		
	if(pos == 0) {
		
		// We only have one point 
		// We must find its distance to the key and return that value;
			
		range = gci_cell_get_distance_between_two_cells(key, &(projected_cell_array[pos]));
			
		pos_min = pos;
	}
	else if(pos >= array_size) {
		
		// We only have one point 
		// We must find its distance to the key and return that value;
			
		pos = array_size - 1;
			
		range = gci_cell_get_distance_between_two_cells(key, &(projected_cell_array[pos]));

		pos_min = pos;
	}
	else {
		
		// Find out which of the two closest projected points are closer.
		// We must check using both dimensions.
			
		range = gci_cell_get_distance_between_two_cells(key, &(projected_cell_array[pos]));
			
		range_temp = gci_cell_get_distance_between_two_cells(key, &(projected_cell_array[pos - 1]) );
		
		if(range_temp < range) {
			
			pos_min = pos - 1;
			range = range_temp;
		}
		else {
			
			pos_min = pos;
		}
	}
	
	// We must check the distances of all the point now which lay within reach of range.
	range_min = range;
		
	for(i=0; i < array_size; i++) {
		
		 if( projected_cell_array[i].x < (key->x - range) || projected_cell_array[i].x > (key->x + range) )
		 	continue;
			 	
		 if( projected_cell_array[i].y < (key->y - range) || projected_cell_array[i].y > (key->y + range) )
		 	continue;
		
		 range_temp = gci_cell_get_distance_between_two_cells(key, &(projected_cell_array[i]));
		
		 if(range_temp < range_min) {
			 
		 	range_min = range_temp;
		 	pos_min = i;
		 }
	}
		
	return pos_min;
}


// Finds the two closest values either side to the value key
// The array must be sorted.
// This code uses a projection method to find the nearest neighbour
// See ProjectionMethod2.html in the docs directory.
Cell __cdecl gci_cellmap_find_nearest_neighbour(CellMap *map, Cell *key)
{
	int pos, array_size = cell_list_count(map->cell_list);
	int (__cdecl* cmp_function) (const void *, const void *);
	
	calculate_standard_deviation(map);
	
	cmp_function = (map->standard_deviation_x < map->standard_deviation_y) ?  gci_cell_compare_by_x : gci_cell_compare_by_y;
	
	/* Prepare the sorted projected array */
	
	if(map->projected_array_invalidated == 1) {
	
		if(map->projected_cell_array != NULL)
			free(map->projected_cell_array);
		
		map->projected_cell_array = cell_list_to_array(map->cell_list);
		
		qsort(map->projected_cell_array, array_size, sizeof(Cell), cmp_function); 
		
		map->projected_array_invalidated = 0;
	}
	
	pos =  find_nearest_neighbour_pos_in_array(map, map->projected_cell_array, array_size, cmp_function, key);
	
	// This cusor is transaperent to hide it until we wish to show the nearest neighbour.
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_YELLOW);  
	SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, map->projected_cell_array[pos].x, map->projected_cell_array[pos].y); 

	map->current_cell = map->projected_cell_array[pos];
	
	return map->current_cell;
}


Cell* array_cell_remove(Cell *array, int size, Cell key)
{
	int i;
    Cell *array_ptr, *temp_array;

    temp_array = (Cell *) malloc(sizeof(Cell) * (size - 1));

    array_ptr = temp_array;

    i=0;

    while( i < size ) {

		if(array[i].id != key.id)
	    	*array_ptr++ = array[i];

		i++;
    }

    return temp_array;
}


void gci_cellmap_proprogate_shortest_path(CellMap *map, CELL_EVENT_HANDLER cell_found_handler, void *callback_data)
{
	/*
	int val, i, pos, pos_min, handler_id, neighbour_cell_id, array_size = cell_list_count(map->cell_list);
	double x_sd, y_sd;
	Cell *cell_array, *temp_cell_array, key_cell, temp_cell;
	int (*cmp_function) (const void *, const void *);

	char fname[MAX_PATHNAME_LEN], nn_fname[MAX_PATHNAME_LEN];
	int id=0, dot;
	char title[256]="", dish[30]="", obj_details[50]="", cube_name[20]="";
	int cube, obj, number_of_cells, number_of_singles, number_of_clusters, number_of_grot;
	double roi_left=0, roi_top=0, roi_width=0, roi_height=0;
	double gain, exposure;
	
	//Create new Excel file with the cells in nearest neighbour order
	if( handler_id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(map), "CellFound", cell_found_handler, callback_data) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	key_cell.x = map->region_left;
	key_cell.y = map->region_top;
	key_cell.id = -1;
	key_cell.type = 1;
	
	calculate_standard_deviation(map);
	
	cmp_function = (map->standard_deviation_x < map->standard_deviation_y) ?  gci_cell_compare_by_x : gci_cell_compare_by_y;
	
	cell_array = cell_list_to_array(map->cell_list);
		
	qsort(cell_array, array_size, sizeof(Cell), cmp_function); 
	
	// This cusor is transparent to hide it until we wish to show the nearest neighbour.
	SetCursorAttribute (map->parent_panel, map->cellmap_graph_id, 2, ATTR_CURSOR_COLOR, VAL_YELLOW);
	
	SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, key_cell.x, key_cell.y);


	
	CellFinding_ReadExperimentData(title, dish, obj_details, &obj, &cube, cube_name, &exposure, &gain,
								&roi_width, &roi_height, &roi_left, &roi_top,
								&number_of_cells, &number_of_singles, &number_of_clusters, &number_of_grot); 

	//Make the new filename
	strcpy(fname, map->cell_map_file);
	dot = FindPattern (fname, 0, -1, ".", 0, 1);
	if (dot == -1)
		dot = strlen(fname);
	strncpy(nn_fname, fname, dot);
	nn_fname[dot] = '\0';	   //Doesn't work without this
	strcat(nn_fname, "_nn.xls");
	RemoveFileIfExists (nn_fname);
	
	gci_cellmap_clear(map);
	gci_cellmap_new_file(map, nn_fname);

	CellFinding_SaveExperimentData(title, dish, obj_details, obj, cube, cube_name, exposure, gain,
								roi_width, roi_height, roi_left, roi_top,
								number_of_cells, number_of_singles, number_of_clusters, number_of_grot); 

	while(array_size > 0) {
	
		pos = find_nearest_neighbour_pos_in_array(map, cell_array, array_size, cmp_function, &key_cell);
	
		key_cell = cell_array[pos];
		
		temp_cell_array = array_cell_remove(cell_array, array_size, key_cell);

		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(map), "CellFound", GCI_VOID_POINTER, &key_cell);

		SetGraphCursor (map->parent_panel, map->cellmap_graph_id, 2, key_cell.x, key_cell.y);

		id ++;
		key_cell.id = id;
		gci_cellmap_add_cell(map, key_cell);
		gci_cellmap_plot_cells(map);

		free(cell_array);
		
		cell_array = temp_cell_array;
		
		array_size--;
	}
	
//	GCI_SaveExcelWorkbookNoPrompt();
	free(cell_array);
	
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(map), "CellFound", handler_id);
	
	*/
}
