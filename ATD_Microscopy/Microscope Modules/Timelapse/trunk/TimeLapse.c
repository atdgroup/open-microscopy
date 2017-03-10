#include "timelapse.h"
#include "timelapse_ui.h"  
#include "gci_ui_module.h" 
#include "camera\gci_camera.h"
#include "StagePlate.h"
#include "stage\stage.h"
#include "string_utils.h"
#include "stage\stage.h"
#include "CubeSlider.h"
#include "gci_utils.h"
#include "microscope.h"
#include "file_prefix_dialog.h"
#include "gci_menu_utils.h"

#include <userint.h>  
#include <utility.h> 

#ifdef MICROSCOPE_PYTHON_AUTOMATION

void enable_mosaic_region(timelapse *tl)
{
	stage_scan *ss=NULL;
	realtime_overview *rto=NULL;

	// set this to NULL to test attaching to ss or rto
	if (tl->mosaic_window != NULL) {
		GCI_ImagingWindow_DisableRoiTool(tl->mosaic_window->window);
		GCI_ImagingWindow_DisableCrossHair(tl->mosaic_window->window);
		GCI_ImagingWindow_DisconnectCrosshairHandler(tl->mosaic_window->window, tl->_crosshair_signal_id); 
		tl->mosaic_window = NULL;
	}

	// try to attach to the stage scan window
	ss = microscope_get_stage_scan(tl->ms);
	if (ss!=NULL)
		if (ss->rto!=NULL)
			tl->mosaic_window = ss->rto->mosaic_window;

	// if no joy, try the microscope rto
	if (tl->mosaic_window == NULL){
		rto = microscope_get_realtime_overview(tl->ms);
		if (rto!=NULL)
			tl->mosaic_window = rto->mosaic_window;
	}

	if (tl->mosaic_window != NULL) {
		GCI_ImagingWindow_EnableRoiTool(tl->mosaic_window->window);
	}
	else
		GCI_MessagePopup("Error", "A Stage Scan or Realtime Overview is required.");
}

void disable_mosaic_region(timelapse *tl)
{
	if (tl->mosaic_window != NULL) {
		GCI_ImagingWindow_DisableRoiTool(tl->mosaic_window->window);
		GCI_ImagingWindow_EnableCrossHair(tl->mosaic_window->window);  // enable the cros ahir again for stage pos pointing
		tl->mosaic_window = NULL;
	}
}

void check_mosaic_region_needed(timelapse *tl)
{
	int val;

	GetCtrlVal(tl->regions_panel, TLREGIONS_VARIABLE, &val);

	if (tl->region_mode && val) {
		enable_mosaic_region(tl);
	}
	else {
		disable_mosaic_region(tl);
	}
}

void timelapse_region_mode_on(timelapse *tl)
{
	ui_module_display_panel(UIMODULE_CAST(tl), tl->regions_panel);
	SetCtrlAttribute (tl->panel_id, TIMELAPSE_TABLE, ATTR_NUM_VISIBLE_COLUMNS, 5);
	SetCtrlAttribute (tl->panel_id, TIMELAPSE_REGION_TEXT, ATTR_VISIBLE, 1);
	SetCtrlAttribute (tl->panel_id, TIMELAPSE_DUMMY_AREA, ATTR_LEFT, 350);
	SetCtrlVal(tl->panel_id, TIMELAPSE_USECROSSHAIR, 0); 
	SetCtrlVal(tl->panel_id, TIMELAPSE_USECROSSHAIR_2, 0); 
	CallCtrlCallback(tl->panel_id, TIMELAPSE_USECROSSHAIR, EVENT_COMMIT, 0, 0, 0);
	CallCtrlCallback(tl->panel_id, TIMELAPSE_USECROSSHAIR_2, EVENT_COMMIT, 0, 0, 0);
	tl->region_mode = 1;
	timelapse_setup_action_scripts(tl);	
	timelapse_draw_points(tl);
}

void timelapse_region_mode_off(timelapse *tl)
{
	ui_module_hide_panel(UIMODULE_CAST(tl), tl->regions_panel);
	SetCtrlAttribute (tl->panel_id, TIMELAPSE_TABLE, ATTR_NUM_VISIBLE_COLUMNS, 3);
	SetCtrlAttribute (tl->panel_id, TIMELAPSE_REGION_TEXT, ATTR_VISIBLE, 0);
	SetCtrlAttribute (tl->panel_id, TIMELAPSE_DUMMY_AREA, ATTR_LEFT, 230);
	tl->region_mode = 0;
	timelapse_setup_action_scripts(tl);
	timelapse_draw_points(tl);
}

void timelapse_hide(timelapse *tl)
{
	SetCtrlAttribute(tl->panel_id, TIMELAPSE_TIMER, ATTR_ENABLED, 0);  
			
	// reset this as image window may be used for other things 
	SetCtrlVal(tl->panel_id, TIMELAPSE_USECROSSHAIR, 0); 
	if (tl->window!=NULL) {
		GCI_ImagingWindow_DisconnectCrosshairHandler(tl->window, tl->_crosshair_signal_id); 
	}
	
	SetCtrlVal(tl->panel_id, TIMELAPSE_USECROSSHAIR_2, 0); 
	if (tl->mosaic_window!=NULL) {
		GCI_ImagingWindow_DisconnectCrosshairHandler(tl->mosaic_window->window, tl->_crosshair_signal_id); 
	}
	
	ui_module_hide_all_panels(UIMODULE_CAST(tl));
	ui_module_clear_attached_panels(UIMODULE_CAST(tl));
}


void timelapse_destroy(timelapse *tl)
{
	timelapse_clear_auto_save_data(tl);

	dictionary_del(tl->script_details_index);
	ListDispose(tl->script_details);

	if (tl->_microscope_master_camera_changed_signal_id >= 0)
		microscope_master_camera_changed_handler_disconnect(tl->ms, tl->_microscope_master_camera_changed_signal_id);

	if (tl->_stage_plate_signal_plate_changed_signal_id >= 0 && tl->stage_plate_module!=NULL)
		stage_plate_signal_plate_changed_handler_disconnect(tl->stage_plate_module, tl->_stage_plate_signal_plate_changed_signal_id);

	ui_module_destroy(UIMODULE_CAST(tl));
	
	free(tl);
}


static int get_first_line_of_file_ptr(timelapse *tl, FILE *fp, char *line)
{
	memset(line, 0, 1);
	
	// Get the first line
	fgets(line, 500, fp);
		
	str_remove_char(line, '\n'); 
	
	return TIMELAPSE_SUCCESS;      
}

static int get_first_line_of_file(timelapse *tl, const char* filepath, char *line)
{
	FILE *fp = fopen (filepath, "r");
	
	get_first_line_of_file_ptr(tl, fp, line); 
	
	fclose(fp);
	
	return TIMELAPSE_SUCCESS;      
}

static char* trim_start(char *str)
{
	// loop through whitespace
	while(*str == ' ')
		str++;

	return str;
}


static int remove_comment(timelapse *tl, char *old_line, char *line)
{
	memset(line, 0, 1);   
		
	// Increment pass comment operator. 
	if(*old_line == '#')
		old_line++;
	
	old_line = trim_start(old_line);
	
	strcpy(line, old_line);
	
	return 0;
}

static int parse_comment_lines(timelapse *tl, FILE *fp, const char* filepath)
{
	char line[500];
	char *key_raw = NULL;
	char *val = NULL;
	char key[100], lower_key[100];

	TimelapseScriptDetails details;

	while(1) {

		fgets(line, 500, fp);
		
		 if(line[0] != '#')
			 break;
		
		str_remove_char(line, '\n'); 

		key_raw = strtok (line, ":");
		remove_comment(tl, key_raw, key);  
		val = strtok (NULL, ":");

		if(val == NULL) {
			// If script has old format just copy the single comment line into name
			strcpy(details.name, key);
			strcpy(details.filepath, filepath);
			details.new_format = 0;
			ListInsertItem(tl->script_details, &details, END_OF_LIST);
			dictionary_setint(tl->script_details_index, details.name, ListNumItems(tl->script_details));
			break;
		}

		val = trim_start(val);

		details.new_format = 1;

		strtolwr(key, lower_key);

		if(strcmp(lower_key, "name") == 0) {
			strcpy(details.name, val);
		}

		if(strcmp(lower_key, "description") == 0) {
			strcpy(details.description, val);
		}

		if(strcmp(lower_key, "author") == 0) {
			strcpy(details.author, val);
		}

		if(strcmp(lower_key, "category") == 0) {
			strcpy(details.category, val);
		}

		strcpy(details.filepath, filepath);

		ListInsertItem(tl->script_details, &details, END_OF_LIST);
		dictionary_setint(tl->script_details_index, details.name, ListNumItems(tl->script_details));
	}
	
	return TIMELAPSE_SUCCESS;      
}

static int get_details_of_file(timelapse *tl, const char* filepath, char *name, char *description, char *author, char* category)
{
	FILE *fp = fopen (filepath, "r");
	int index;
	static int first_line = 0;

	TimelapseScriptDetails details;

	parse_comment_lines(tl, fp, filepath);

	index =  ListNumItems(tl->script_details);

	ListGetItem(tl->script_details, &details, index);

	strcpy(name, details.name);
	
	if(details.new_format > 0) {
		strcpy(description, details.description);
		strcpy(author, details.author);
		strcpy(category, details.category);
	}
	else {
		strcpy(description, "unknown");
		strcpy(author, "unknown");
		strcpy(category, "unknown");
	}

	fclose(fp);
	
	return index;      
}

int InsertFilesInToList(timelapse *tl)
{
	char name[500] = "", des[500] = "", author[500] = "", category[500] = "";
	int index, sub_menu, author_item, script_item;
	TimelapseScriptDetails details;

	char dir[GCI_MAX_PATHNAME_LEN], search_str[500], filename[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];

	microscope_get_data_subdirectory(tl->ms, "TimeLapse Scripts", dir);
	
	dictionary_del(tl->script_details_index);
	tl->script_details_index = dictionary_new(50);
	ListClear(tl->script_details);
	
	// Lets loop throught the script dir and get the first line of the script as that is the name.
	
	sprintf(search_str, "%s\\*.py", dir);

	EmptyMenu  (tl->action_menubar, tl->action_menu);
	
	if(GetFirstFile (search_str, 1, 1, 0, 0, 0, 0, filename) < 0)
		return TIMELAPSE_ERROR;

	sprintf(filepath, "%s\\%s", dir, filename);

	index = get_details_of_file(tl, filepath, name, des, author, category);

	if (tl->region_mode && strcmp("Regions", category)) {  // If in region mode and category is not Regions (strcmp returns non-zero), skip this file
		// empty, do nothing
	}
	else {
		author_item = NewMenuItem(tl->action_menubar, tl->action_menu, author, -1, 0, MenuItemChanged, tl);   
		sub_menu = NewSubMenu(tl->action_menubar, author_item);    
		script_item = NewMenuItem(tl->action_menubar, sub_menu, name, -1, 0, MenuItemChanged, tl);       
	}

	while (!GetNextFile (filename))
	{
		sprintf(filepath, "%s\\%s", dir, filename); 
   
		get_details_of_file(tl, filepath, name, des, author, category);

		if (tl->region_mode && strcmp("Regions", category)) {  // If in region mode and category is not Regions (strcmp returns non-zero), skip this file
			continue;
		}

		author_item = FindMenuItemIdFromNameInMenu(tl->action_menubar, tl->action_menu, author, 0);

		// Create new author menu or get the id of the last one
		if(author_item < 0) {
			author_item = NewMenuItem(tl->action_menubar, tl->action_menu, author, -1, 0, MenuItemChanged, tl);   		
		}
		
		GetMenuBarAttribute (tl->action_menubar, author_item, ATTR_SUBMENU_ID, &sub_menu);  	

		if(sub_menu == 0)
			sub_menu = NewSubMenu(tl->action_menubar, author_item);

		script_item = NewMenuItem(tl->action_menubar, sub_menu, name, -1, 0, MenuItemChanged, tl);   
	}

	ListGetItem(tl->script_details, &details, tl->last_script_index);

	ClearListCtrl(tl->panel_id, TIMELAPSE_ACTIONS);
	InsertListItem(tl->panel_id, TIMELAPSE_ACTIONS, -1, details.name, details.filepath);

	return TIMELAPSE_SUCCESS;
}

int timelapse_setup_action_scripts(timelapse *tl)
{
	int curr_val = 0;

	GetCtrlIndex(tl->panel_id, TIMELAPSE_ACTIONS, &curr_val);
	SetCtrlIndex(tl->panel_id, TIMELAPSE_ACTIONS, 0);
	ClearListCtrl(tl->panel_id, TIMELAPSE_ACTIONS);
	
	InsertFilesInToList(tl);

	if (curr_val > 0) 
		SetCtrlIndex(tl->panel_id, TIMELAPSE_ACTIONS, curr_val);
	
	return TIMELAPSE_SUCCESS;
}


int timelapse_import_script(timelapse *tl)
{
	int count = 0;
	char filepath[GCI_MAX_PATHNAME_LEN]="", name[500] = "", name_without_ext[500] = "";  
	PyObject *pDict;

	PyObject *pName;

	// Are there any installed scripts.
	GetNumListItems(tl->panel_id, TIMELAPSE_ACTIONS, &count);
	
	if(count == 0)
		goto FAIL;
	
	// Get the path to the selected script
	GetCtrlVal(tl->panel_id, TIMELAPSE_ACTIONS, filepath);
	
	if(tl->pModule != NULL)
		Py_DECREF(tl->pModule);

	SplitPath (filepath, NULL, NULL, name);

	if(strcmp(name, tl->last_script_filename) != 0) {

		get_file_without_extension(name, name_without_ext);
		pName = PyString_FromString(name_without_ext);

		tl->pModule = PyImport_Import(pName);

		// Hack to make sure module is recompiled 
		if(tl->pModule != NULL) {
			Py_DECREF(tl->pModule);
			tl->pModule = PyImport_ReloadModule(tl->pModule);
		}

		Py_DECREF(pName);
	}
	else {
		
		 if (tl->pModule == NULL) {
			goto FAIL;
		 }

		 tl->pModule = PyImport_ReloadModule(tl->pModule);
	}

	if (tl->pModule == NULL) {
		PyErr_Print();
		strcpy(tl->last_script_filename, "");
		goto FAIL;
	}

	// pDict is a borrowed reference don't dec ref count
	pDict = PyModule_GetDict(tl->pModule);

	tl->on_start_callable = PyDict_GetItemString(pDict, "OnStart");

	if(!PyCallable_Check(tl->on_start_callable)) {
		GCI_MessagePopup("Python Script Error", "OnStart Not callable!");	
		goto FAIL;
	}

	tl->on_abort_callable = PyDict_GetItemString(pDict, "OnAbort");

	if(!PyCallable_Check(tl->on_abort_callable)) {
		GCI_MessagePopup("Python Script Error", "OnAbort Not callable!");	
		goto FAIL;
	}

	tl->on_cycle_start_callable = PyDict_GetItemString(pDict, "OnCycleStart");   

	if(!PyCallable_Check(tl->on_cycle_start_callable)) {
		GCI_MessagePopup("Python Script Error", "OnCycleStart Not callable!");	
		goto FAIL;
	}

    tl->on_point_changed_callable = PyDict_GetItemString(pDict, "OnNewPoint");

	if(!PyCallable_Check(tl->on_point_changed_callable)) {
		GCI_MessagePopup("Python Script Error", "OnNewPoint Not callable!");	
		goto FAIL;
	}

	strcpy(tl->last_script_filename, name);

	return TIMELAPSE_SUCCESS;

FAIL:

	return TIMELAPSE_ERROR;
}

int timelapse_call_python_on_point_changed(timelapse *tl, GCI_FPOINT pt, int current_point)
{
	PyObject* ArgsTuple = PyTuple_New(4);
	PyObject *pValue;

	logger_log(UIMODULE_LOGGER(tl), LOGGER_INFORMATIONAL, "Timelapse call script OnNewPoint");   

	PyTuple_SetItem(ArgsTuple, 0, PyFloat_FromDouble(pt.x));
	PyTuple_SetItem(ArgsTuple, 1, PyFloat_FromDouble(pt.y));
	PyTuple_SetItem(ArgsTuple, 2, PyFloat_FromDouble(pt.z));    
	PyTuple_SetItem(ArgsTuple, 3, PyInt_FromLong(current_point));
	
	// Make a call to the function referenced
    // by "expression"
	pValue = PyObject_CallObject(tl->on_point_changed_callable, ArgsTuple);

    Py_DECREF(ArgsTuple);

	if(pValue != NULL) {
		Py_DECREF(pValue);
	}
	else {
		PyErr_Print();
		return TIMELAPSE_SUCCESS;
	}

	return TIMELAPSE_SUCCESS; 

}

int timelapse_call_python_on_cycle_start(timelapse *tl)
{
	PyObject *pValue;

	if(tl->on_cycle_start_callable == NULL) {
		GCI_MessagePopup("Timelapse Error", "Error calling cycle start script function");
		logger_log(UIMODULE_LOGGER(tl), LOGGER_WARNING, "Timelapse call script OnCycleStart Failed");   
		return TIMELAPSE_ERROR;
	}

	logger_log(UIMODULE_LOGGER(tl), LOGGER_INFORMATIONAL, "Timelapse call script OnCycleStart");   

	// Make a call to the function referenced
    // by "expression"
	pValue = PyObject_CallObject(tl->on_cycle_start_callable, NULL);

	if(pValue != NULL) {
		Py_DECREF(pValue);
	}
	else {
		PyErr_Print();
		return TIMELAPSE_SUCCESS;
	}

	return TIMELAPSE_SUCCESS; 
	
}	


int timelapse_call_python_on_start(timelapse *tl)
{
	PyObject *pValue;

	if(tl->on_start_callable == NULL)
		return TIMELAPSE_ERROR;

	logger_log(UIMODULE_LOGGER(tl), LOGGER_INFORMATIONAL, "Timelapse call script OnStart");   

	// Make a call to the function referenced
    // by "expression"
    pValue = PyObject_CallObject(tl->on_start_callable, NULL);

	if(pValue != NULL) {
		Py_DECREF(pValue);
	}
	else {
		PyErr_Print();
		return TIMELAPSE_SUCCESS;
	}

	return TIMELAPSE_SUCCESS; 
	
}	

static void timelapse_plot_points(timelapse *tl)
{
	int i, size = ListNumItems (tl->points);
	TimelapseTableEntry pt;
	
	for(i=1; i <= size; i++) {
	
		ListGetItem (tl->points, &pt, i);

		PlotPoint (tl->revisit_panel, REVISIT_XY_DISP, pt.centre.x, pt.centre.y, VAL_SMALL_SOLID_SQUARE, VAL_DK_YELLOW);

		if (tl->region_mode && pt.hasRegion)
			PlotRectangle (tl->revisit_panel, REVISIT_XY_DISP, pt.centre.x - pt.regionSize.x/2.0, pt.centre.y - pt.regionSize.y/2.0, 
									pt.centre.x + pt.regionSize.x/2.0, pt.centre.y + pt.regionSize.y/2.0, VAL_SMALL_SOLID_SQUARE, VAL_YELLOW);
	}
}


static int timelapse_get_extents(timelapse *tl, double *xmin, double *xmax,
										 double *ymin, double *ymax,
										 double *zmin, double *zmax)
{
	int i, size = ListNumItems (tl->points);
	TimelapseTableEntry pt; ;
	
	// Get first list item to initialise values
	ListGetItem (tl->points, &pt, 1);

	// Some large numbers to start the algorithm
	*xmin = 999999.0;
	*xmax = -999999.0;
	*ymin = 999999.0;
	*ymax = -999999.0;
	*zmin = 999999.0;
	*zmax = -999999.0;
	
	if(size == 1) {
		
		*xmin = -999999;
		*xmax = 999999;
		*ymin = -999999;
		*ymax = 999999;
		*zmin = -999999;
		*zmax = 999999;
		
		return TIMELAPSE_SUCCESS;      	
	}
		
	for(i=1; i <= size; i++) {
	
		ListGetItem (tl->points, &pt, i);

		*xmin = MIN(*xmin, pt.centre.x-pt.regionSize.x/2);
		*xmax = MAX(*xmax, pt.centre.x+pt.regionSize.x/2);
		*ymin = MIN(*ymin, pt.centre.y-pt.regionSize.y/2);
		*ymax = MAX(*ymax, pt.centre.y+pt.regionSize.y/2);
		*zmin = MIN(*zmin, pt.centre.z-pt.regionSize.z/2);
		*zmax = MAX(*zmax, pt.centre.z+pt.regionSize.z/2);
	}
	
	// Make ten percent larger
	*xmin -= (fabs(*xmax-*xmin) * 0.1);
	*xmax += (fabs(*xmax-*xmin) * 0.1);
	*ymin -= (fabs(*ymax-*ymin) * 0.1);
	*ymax += (fabs(*ymax-*ymin) * 0.1);
	*zmin -= (fabs(*zmax-*zmin) * 0.1);
	*zmax += (fabs(*zmax-*zmin) * 0.1);
	
	
	// Maybe some silly point like Zeros are being added
	if(*xmin == *xmax) {
		*xmin -= 1000;
		*xmax += 1000;
	}
	
	if(*ymin == *ymax) {
		*ymin -= 1000;
		*ymax += 1000;
	}
	
	if(*zmin == *zmax) {
		*zmin -= 1000;
		*zmax += 1000;
	}
	
	return TIMELAPSE_SUCCESS;
}

void timelapse_draw_points(timelapse *tl) 
{
	TimelapseTableEntry pt;     
	double xmin, xmax, ymin, ymax, zmin, zmax;
	int number_of_points, ctrl_wd, ctrl_ht;

	DeleteGraphPlot (tl->revisit_panel, REVISIT_XY_DISP, -1, VAL_IMMEDIATE_DRAW);  
	
	timelapse_get_extents(tl, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	
	GetCtrlAttribute(tl->revisit_panel, REVISIT_XY_DISP, ATTR_WIDTH, &ctrl_wd);
	GetCtrlAttribute(tl->revisit_panel, REVISIT_XY_DISP, ATTR_HEIGHT, &ctrl_ht);

	if ((ymax-ymin)/(xmax-xmin) > (float)ctrl_ht/(float)ctrl_wd) { // fit height
		xmax = (ymax-ymin)/(float)ctrl_ht*(float)ctrl_wd + xmin;
	}
	else { // fit width
		ymax = (xmax-xmin)/(float)ctrl_wd*(float)ctrl_ht + ymin;
	}
	
	SetAxisScalingMode (tl->revisit_panel, REVISIT_XY_DISP, VAL_BOTTOM_XAXIS, VAL_MANUAL, xmin, xmax);
	SetAxisScalingMode (tl->revisit_panel, REVISIT_XY_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, ymin, ymax);

	SetCtrlAttribute (tl->revisit_panel, REVISIT_XY_DISP, ATTR_YREVERSE, 1);

	//horzDir = stage_get_axis_dir(tl->stage, XAXIS);  
	//vertDir = stage_get_axis_dir(tl->stage, YAXIS); 
	/*
	if (horzDir == STAGE_NEGATIVE_TO_POSITIVE)  
		SetCtrlAttribute (tl->revisit_panel, REVISIT_XY_DISP, ATTR_XREVERSE, 1);  
	else
		SetCtrlAttribute (tl->revisit_panel, REVISIT_XY_DISP, ATTR_XREVERSE, 0);  
		
	SetAxisScalingMode (tl->revisit_panel, REVISIT_XY_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, ymin, ymax);
	
	if (vertDir == STAGE_NEGATIVE_TO_POSITIVE)  
		SetCtrlAttribute (tl->revisit_panel, REVISIT_XY_DISP, ATTR_YREVERSE, 0);  
	else
		SetCtrlAttribute (tl->revisit_panel, REVISIT_XY_DISP, ATTR_YREVERSE, 1);
	*/

	timelapse_plot_points(tl);     
	
	// Plot the current point with different colour
	ListGetItem (tl->points, &pt, tl->current_point);     
	
	tl->plot = PlotPoint (tl->revisit_panel, REVISIT_XY_DISP, pt.centre.x, pt.centre.y, VAL_SMALL_SOLID_SQUARE, VAL_RED);
	
	number_of_points = ListNumItems(tl->points);
	
	// Update the count of points on the reisit panel
	SetCtrlVal (tl->revisit_panel, REVISIT_NPTS_2, number_of_points);
	SetCtrlVal (tl->revisit_panel, REVISIT_THISPOINT, tl->current_point);

	if (number_of_points == 1) {
	
		SetCtrlAttribute (tl->revisit_panel, REVISIT_PREV, ATTR_DIMMED, 1);
		SetCtrlAttribute (tl->revisit_panel, REVISIT_NEXT, ATTR_DIMMED, 1);
	}
	else if(number_of_points < 1)
		SetCtrlAttribute (tl->revisit_panel, REVISIT_DELETE, ATTR_DIMMED, 1);
	
	timelapse_update_revisit_buttons(tl);

	SetTableSelection (tl->panel_id, TIMELAPSE_TABLE, MakeRect(tl->current_point, 1, 1, TABLE_COL_NUMBER));
}

void OnCrosshairClicked (IcsViewerWindow *window, const Point p1, const Point p2, void* data)
{
	timelapse *tl = (timelapse*) data;

	TimelapseTableEntry pt;	
	double microns_per_pixel;
	FIBITMAP *image;
	int width, height;
	double stage_x, stage_y;
	//image coordinates
	int x = p1.x;
	int y = p1.y;
	
	// convert image coordinates into central stage co-ordinates.
	StageDirection xdir = stage_get_axis_dir(tl->stage, XAXIS);
	StageDirection ydir = stage_get_axis_dir(tl->stage, YAXIS);

	microns_per_pixel = gci_camera_get_true_microns_per_pixel(tl->camera); 

	// get a pointer to the displayed FIB, no need to free it after
	image = GCI_ImagingWindow_GetDisplayedFIB(tl->window);
	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);

	stage_get_xy_position (tl->stage, &stage_x, &stage_y);

	// calc image coords relative to image centre
	x = p1.x - width / 2;
	y = p1.y - height / 2;

	// Calculate stage x,y
	pt.centre.x = stage_x + xdir*(x * microns_per_pixel);
	pt.centre.y = stage_y + ydir*(y * microns_per_pixel);

	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), &(pt.centre.z));

	pt.hasRegion = 0;
	
	timelapse_add_point(tl, pt);     
	
	timelapse_draw_points(tl); 
}

void OnCrosshairMosaicClicked (IcsViewerWindow *window, const Point p1, const Point p2, void* data)
{
	timelapse *tl = (timelapse*) data;
	TimelapseTableEntry pt;	
	double startx, starty, roi_width, roi_height;
	int width, height;

	if (tl->mosaic_window != NULL) {

		if(p1.x < 0 || p1.y < 0)
			return;

		startx =		tl->mosaic_window->region.left;
		starty =		tl->mosaic_window->region.top; 
		roi_width =		tl->mosaic_window->region.width; 
		roi_height =	tl->mosaic_window->region.height;
		
		width = FreeImage_GetWidth(tl->mosaic_window->mosaic_image);
		height = FreeImage_GetHeight(tl->mosaic_window->mosaic_image);

		pt.centre.x = startx + (((float) p1.x / width) * roi_width);
		pt.centre.y = starty + (((float) p1.y / height) * roi_height);      

		z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), &(pt.centre.z));

		pt.hasRegion = 0;
		
		timelapse_add_point(tl, pt);     
		
		timelapse_draw_points(tl); 
	}
}

static void GetTableCell(timelapse *tl, int row, int col, double *val)
{
	GetTableCellAttribute (tl->panel_id, TIMELAPSE_TABLE, MakePoint(row, col), ATTR_CTRL_VAL, val);
}

static void SetTableCell(timelapse *tl, int row, int col, double val)
{
	SetTableCellRangeAttribute (tl->panel_id, TIMELAPSE_TABLE, MakeRect(row,col,1,1), ATTR_CELL_TYPE, VAL_CELL_NUMERIC);
	SetTableCellAttribute (tl->panel_id, TIMELAPSE_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, val);
}

static void addPointToTable (timelapse *tl, TimelapseTableEntry pt)
{
	int size, number_of_rows = 0;

	GetNumTableRows (tl->panel_id, TIMELAPSE_TABLE, &number_of_rows);   
	
	size = ListNumItems(tl->points);
		
	if (size > number_of_rows)
		InsertTableRows (tl->panel_id, TIMELAPSE_TABLE, -1, 1, VAL_USE_MASTER_CELL_TYPE);
	
	SetTableCell(tl, size, 1, pt.centre.x);
	SetTableCell(tl, size, 2, pt.centre.y);
	SetTableCell(tl, size, 3, pt.centre.z);
	SetTableCell(tl, size, 4, pt.regionSize.x);
	SetTableCell(tl, size, 5, pt.regionSize.y);
/*
	if (pt.hasRegion) {
		SetTableCellAttribute (tl->panel_id, TIMELAPSE_TABLE, MakePoint(size, 4), ATTR_DIMMED, 0);
		SetTableCellAttribute (tl->panel_id, TIMELAPSE_TABLE, MakePoint(size, 5), ATTR_DIMMED, 0);
	}
	else {
		SetTableCellAttribute (tl->panel_id, TIMELAPSE_TABLE, MakePoint(size, 4), ATTR_DIMMED, 1);
		SetTableCellAttribute (tl->panel_id, TIMELAPSE_TABLE, MakePoint(size, 5), ATTR_DIMMED, 1);
	}
*/

}

int GenerateStagePlatePointsWithDirInfo(timelapse *tl,
										int selected_points, int is_horizontal,
										StagePlateHorizontalStartPosition horz_start,
										StagePlateVerticalStartPosition vert_start,
										int shortest_path, int region, double xsize, double ysize)
{
    int i, number_of_wells;
	GCI_FPOINT pt;
	TimelapseTableEntry tte;
	StagePlate plate;
	Well *wells = NULL;

	StagePlateModule* plate_module = microscope_get_stage_plate_module(tl->ms);

	if (plate_module == NULL){
		GCI_MessagePopup("Error", "Stage Plate software module required.");
		return TIMELAPSE_ERROR;
	}

	if (stage_plate_get_current_plate(plate_module, &plate) == STAGE_PLATE_MODULE_ERROR){
		GCI_MessagePopup("Error", "No valid Stage Plate selected.");
		return TIMELAPSE_ERROR;
	}

	// the maximum number of wells
	number_of_wells = plate.rows * plate.cols;

	wells = (Well*) malloc(sizeof(Well) * number_of_wells);

	// Get the actual wells that are selected
	number_of_wells = stage_plate_get_well_positions(plate_module, wells);

	plate_points_set_sorting_params(is_horizontal, horz_start, vert_start, shortest_path);
	qsort(wells, number_of_wells, sizeof(Well), plate_point_sort); 

	timelapse_clear_points(tl);

	if (region) 
		timelapse_region_mode_on(tl);
	
	InsertTableRows (tl->panel_id, TIMELAPSE_TABLE, -1, number_of_wells, VAL_USE_MASTER_CELL_TYPE); 
	
	for (i=0; i<number_of_wells; i++) {     
		
		calculate_well_z_position_for_xy(tl->wpd, wells[i].region.cx, wells[i].region.cy, &pt);
				
		tte.centre.x      = pt.x;
		tte.centre.y      = pt.y;
		tte.centre.z      = pt.z;
		tte.hasRegion     = region;
		tte.regionSize.x  = xsize;
		tte.regionSize.y  = ysize;
		tte.hasFocalPlane = 0;

		timelapse_add_point(tl, tte);

//		timelapse_add_point_xyz(tl, pt.x, pt.y, pt.z);
	}
		
	SetCtrlVal (tl->revisit_panel, REVISIT_NPTS_2, number_of_wells);

	SetCtrlAttribute (tl->revisit_panel, REVISIT_PREV, ATTR_DIMMED, 0);
	SetCtrlAttribute (tl->revisit_panel, REVISIT_NEXT, ATTR_DIMMED, 0);
	SetCtrlAttribute (tl->revisit_panel, REVISIT_DELETE, ATTR_DIMMED, 0);
	
	timelapse_draw_points(tl);

	free(wells);

	return TIMELAPSE_SUCCESS;
}

static void timelapse_on_stage_plate_changed (StagePlateModule* stage_plate_module, int pos, void *data) 
{
	timelapse *tl = (timelapse*) data;

	// Dim or undeim the menu if plate has has changed type
	if(tl->stage_plate_module != NULL) {  // how can this be NULL? It should equal stage_plate_module. PB May 2012
		
		StagePlate plate;

		stage_plate_get_current_plate(tl->stage_plate_module, &plate);

		SetCtrlMenuAttribute (tl->panel_id, TIMELAPSE_TABLE, tl->stage_plate_menu_item,
			ATTR_DIMMED, plate.type != PLATE_WELLPLATE);
	}

//	if(!ui_module_main_panel_is_visible(UIMODULE_CAST(tl)))
//		return;

//	ret = GCI_ConfirmPopup("Info", IDI_INFORMATION, "The stage plate has been changed.\n"
//		"Would you like to populate the timelapse with new points");

//	if(ret > 0) 
//		GenerateStagePlatePoints(tl);
}

static void OnMasterCameraChanged (Microscope* microscope, void *data)
{
	timelapse *tl = (timelapse*) data;  

    tl->camera = MICROSCOPE_MASTER_CAMERA(tl->ms); 	
}

int CVICALLBACK cube_options_CompareFunction(void *item1, void *item2)
{
	TimelapseCubeOptions *c1 = (TimelapseCubeOptions *)item1;
	TimelapseCubeOptions *c2 = (TimelapseCubeOptions *)item2;

	if (c1->position < c2->position)
		return -1;
	else if  (c2->position < c1->position)
		return 1;

	return 0;
}

void timelapse_setup_cube_options(timelapse *tl)
{
	// get the available cubes and reset the cube options to some sensible values
	int n, i;
	double gain, exposure;
	FluoCubeManager *cm;
	FluoCube *cubes;
	TimelapseCubeOptions cube_opt;

	gci_camera_get_gain(tl->camera, CAMERA_ALL_CHANNELS, &gain); 
	exposure = gci_camera_get_exposure_time(tl->camera); 

	cm = microscope_get_cube_manager(tl->ms);
	cube_manager_get_number_of_cubes(cm, &n);
	cubes = cube_manager_get_active_cubes(cm);

	ListClear(tl->cube_options);
	ClearListCtrl(tl->panel_id, TIMELAPSE_CUBE);

	// make a list of the devices returned from device manager
	for (i=0; i<n; i++) {

		strncpy(&(cube_opt.name), cubes[i].name, 499);
		cube_opt.position = cubes[i].position;
		cube_opt.exposure = exposure;
		cube_opt.gain = gain;
		cube_opt.offset = 0.0;

		ListInsertItem(tl->cube_options, &cube_opt, END_OF_LIST);
	}

	// devices may not be in any order, sort them
	ListInsertionSort(tl->cube_options, cube_options_CompareFunction);

	// then insert into the UI in the correct order
	for (i=1; i<=n; i++) {
		ListGetItem(tl->cube_options, &cube_opt, i);
		InsertListItem(tl->panel_id, TIMELAPSE_CUBE, i-1, cube_opt.name, i);
	}

	// update UI by calling the callback
	CallCtrlCallback(tl->panel_id, TIMELAPSE_CUBE, EVENT_COMMIT, 0, 0, 0);

	free(cubes);
}

timelapse* timelapse_new(Microscope *ms)
{
	char script_dir[GCI_MAX_PATHNAME_LEN];
	
	timelapse *tl = (timelapse *) malloc (sizeof(timelapse));		
	
	StagePlateModule* plate_module = NULL;

	Rect cellselection;

	memset(tl, 0, sizeof(timelapse));

	tl->ms = ms;
  	tl->camera = microscope_get_camera(ms);
    tl->stage = microscope_get_stage(ms);
	tl->z_drive = microscope_get_master_zdrive (ms);
	tl->points = ListCreate(sizeof(TimelapseTableEntry));
	tl->cube_options = ListCreate(sizeof(TimelapseCubeOptions));
	tl->has_run = 0;
	tl->plot = -1;
	tl->current_point = 1;
	tl->active = 0;
	tl->pModule = NULL;
	tl->region_mode = 0;  // default to coincide with UI
	tl->focus_mode = TIMELAPSE_USECONSTFOCALPLANE;  // default value to coincide with UI
	tl->has_global_focalPlane = 0;
	tl->global_focalPlane_a = 0.0;
	tl->global_focalPlane_b = 0.0;
	tl->global_focalPlane_c = 0.0;

	ui_module_constructor(UIMODULE_CAST(tl), "TimeLapse");  
	
	tl->last_script_index = 0;
	tl->action_menubar = NewMenuBar(0); 
	tl->action_menu = NewMenu(tl->action_menubar, "Menu Name", -1);    
	tl->script_details_index = dictionary_new(50);
	tl->script_details = ListCreate(sizeof(TimelapseScriptDetails));

	tl->panel_id = ui_module_add_panel(UIMODULE_CAST(tl), "TimeLapse_ui.uir", TIMELAPSE, 1);  
	tl->revisit_panel = ui_module_add_panel(UIMODULE_CAST(tl), "TimeLapse_ui.uir", REVISIT, 0);   
	tl->regions_panel = ui_module_add_panel(UIMODULE_CAST(tl), "TimeLapse_ui.uir", TLREGIONS, 0);   

	// Make script Directory
	microscope_get_data_subdirectory(tl->ms, "TimeLapse Scripts", script_dir);      
	
	if (!FileExists(script_dir, 0))
		MakeDir (script_dir);   
	
	HideBuiltInCtrlMenuItem (tl->panel_id,  TIMELAPSE_TABLE, VAL_GOTO);
	HideBuiltInCtrlMenuItem (tl->panel_id,  TIMELAPSE_TABLE, VAL_SEARCH); 
	HideBuiltInCtrlMenuItem (tl->panel_id,  TIMELAPSE_TABLE, VAL_SORT); 

	SetActiveCtrl(tl->panel_id, TIMELAPSE_NEW_POINT); 
	SetCtrlAttribute(tl->panel_id, TIMELAPSE_TABLE, ATTR_HILITE_ONLY_WHEN_PANEL_ACTIVE, 0);
	SetCtrlAttribute(tl->panel_id, TIMELAPSE_TABLE, ATTR_ENABLE_ROW_SIZING, 0);
	SetCtrlAttribute(tl->panel_id, TIMELAPSE_TABLE, ATTR_ENABLE_COLUMN_SIZING, 0);

	cellselection = MakeRect(1, 1, 1, 3);
		
	SetTableSelection (tl->panel_id, TIMELAPSE_TABLE, cellselection);

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_DEF_PNTS, OnDefinePointsClicked, tl) < 0)
		return NULL;	

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_NEW_POINT, OnNewPointClicked, tl) < 0)
		return NULL;	

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_UPDATE_POINT, OnUpdatePointClicked, tl) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_TABLE, OnTimeLapseTableEdited, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_TIMER, OnTimeLapseTimerTick, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_CLEAR, OnClearAllClicked, tl) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_USECROSSHAIR, OnUseCrossHairClicked, tl) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_USECROSSHAIR_2, OnUseCrossHairClicked_2, tl) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_GOTO, OnGotoPointClicked, tl) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_DELETE, OnDeleteClicked2, tl) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_CLOSE, OnTimeLapseCloseClicked, tl) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_LOAD, OnTimeLapseLoadClicked, tl) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_SAVE, OnTimeLapseSaveClicked, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_REVISIT, OnRevisitClicked, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_START, OnStartClicked, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_STOP, OnStopClicked, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_EDIT, OnTimeLapseEdit, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_HELP, OnTimeLapseHelp, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_USE_REGIONS, OnUseRegionsClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_DEFAULTCUBEOPTIONS, OnUseDefaultCubeOptions, tl) < 0)
		return NULL;
	
	//if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_ACTIONS, OnTimeLapseActionChanged, tl) < 0)
	//	return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_ACTIONS, OnTimeLapseMenuRingChanged, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_CUBE, OnCubeSelect, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_ACQEXPOSURE, OnCubeOptionsChanged, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_ACQGAIN, OnCubeOptionsChanged, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_ACQFOCUSOFFSET, OnCubeOptionsChanged, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_GETEXPGAIN, OnGetCameraValsClicked, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_SETEXPGAIN, OnSetCameraValsClicked, tl) < 0)
		return NULL;
	
	tl->_microscope_master_camera_changed_signal_id = microscope_master_camera_changed_handler_connect(tl->ms, OnMasterCameraChanged, tl);

	/*
	if(tl->ms->_stage_plate_module != NULL) {
		
		StagePlate plate;

		//tl->stage_plate_module_menu_item = NewCtrlMenuItem (tl->panel_id, TIMELAPSE_TABLE,
		//	"Generate points from stage insert", -1, OnGenerateStagePlatePoints, tl);

		stage_plate_get_current_plate(tl->ms->_stage_plate_module, &plate);

		if(plate.type != PLATE_WELLPLATE) {
			SetCtrlMenuAttribute (tl->panel_id, TIMELAPSE_TABLE, tl->stage_plate_module_menu_item,
				ATTR_DIMMED, 1);
		}
	}
	*/

	// Revisit Panel
	
	if ( InstallCtrlCallback (tl->revisit_panel, REVISIT_PREV, OnPrevClicked, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->revisit_panel, REVISIT_NEXT, OnNextClicked, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->revisit_panel, REVISIT_DELETE, OnDeleteClicked, tl) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (tl->revisit_panel, REVISIT_CLOSE, OnRevisitCloseClicked, tl) < 0)
		return NULL;

	// region panel

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_FIXED_SIZE, OnRegionTypeClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_VARIABLE, OnRegionTypeClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_USEGLOBALFOCALPLANE, OnFocusTypeClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_USEINDFOCALPLANE, OnFocusTypeClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_USECONSTFOCALPLANE, OnFocusTypeClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_USEAUTOEVERY, OnFocusTypeClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_NEW_REGION, OnNewRegionClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_CLOSE, OnRegionsCloseClicked, tl) < 0)
		return NULL;

	if ( InstallCtrlCallback (tl->regions_panel, TLREGIONS_FOCALPLANES, OnFocalPlanesClicked, tl) < 0)
		return NULL;
		
	timelapse_setup_action_scripts(tl);

	tl->stage_plate_module = microscope_get_stage_plate_module(tl->ms);

	if(tl->stage_plate_module != NULL) {
		tl->_stage_plate_signal_plate_changed_signal_id = stage_plate_signal_plate_changed_handler_connect(tl->stage_plate_module,
														 timelapse_on_stage_plate_changed, tl);  // Signal: StagePlateChanged
	}

	FilePrefixSave_EraseLastUsedEntries(UIMODULE_MAIN_PANEL_ID(tl));	

	return tl;
}


void timelapse_add_point(timelapse *tl, TimelapseTableEntry pt)
{
	// make sure unused values are zero
	if (pt.hasRegion == 0) {
		pt.regionSize.x = 0.0;
		pt.regionSize.y = 0.0;
		pt.hasFocalPlane = 0;
	}

	if (pt.hasFocalPlane == 0) {
		pt.focalPlane_a = 0.0;
		pt.focalPlane_b = 0.0;
		pt.focalPlane_c = 0.0;
	}
	
	ListInsertItem(tl->points, &pt, END_OF_LIST);
	
	addPointToTable(tl, pt); 

	// scroll the table to the last possible row
	SetCtrlAttribute(tl->panel_id, TIMELAPSE_TABLE, ATTR_VSCROLL_OFFSET, 32767);
}

void timelapse_add_point_xyz(timelapse *tl, double x, double y, double z)
{
	TimelapseTableEntry pt;

	pt.centre.x = x;
	pt.centre.y = y;
	pt.centre.z = z;
	pt.hasRegion = 0;

	timelapse_add_point(tl, pt);
}

int timelapse_edit_centre_point(timelapse *tl, int pos, GCI_FPOINT pt)
{
	int size, number_of_rows = 0;
	TimelapseTableEntry * point;
	
	GetNumTableRows (tl->panel_id, TIMELAPSE_TABLE, &number_of_rows);   
	
	size = ListNumItems(tl->points);
		
	if (pos > size)
		return TIMELAPSE_ERROR;
	
	point = (TimelapseTableEntry*) ListGetPtrToItem(tl->points, pos);
	
	point->centre.x = pt.x;
	point->centre.y = pt.y; 
	point->centre.z = pt.z; 
	
	SetTableCell(tl, pos, 1, point->centre.x);
	SetTableCell(tl, pos, 2, point->centre.y);
	SetTableCell(tl, pos, 3, point->centre.z);

	return TIMELAPSE_SUCCESS;    
}

int timelapse_edit_ROI_size(timelapse *tl, int pos, GCI_FPOINT pt)
{
	int size, number_of_rows = 0;
	TimelapseTableEntry * point;
	
	GetNumTableRows (tl->panel_id, TIMELAPSE_TABLE, &number_of_rows);   
	
	size = ListNumItems(tl->points);
		
	if (pos > size)
		return TIMELAPSE_ERROR;
	
	point = (TimelapseTableEntry*) ListGetPtrToItem(tl->points, pos);
	
	point->regionSize.x = pt.x;
	point->regionSize.y = pt.y;
	point->hasRegion = 1;
	
	SetTableCell(tl, pos, 4, point->regionSize.x);
	SetTableCell(tl, pos, 5, point->regionSize.y);

	return TIMELAPSE_SUCCESS;    
}

void timelapse_get_point(timelapse *tl, int position, TimelapseTableEntry *pt)
{
	ListGetItem(tl->points, pt, position);
}

void timelapse_get_cube_options(timelapse *tl, int position, TimelapseCubeOptions *pt)
{
	ListGetItem(tl->cube_options, pt, position);
}

ListType timelapse_get_point_list(timelapse *tl)
{
	return tl->points;
}

void timelapse_update_revisit_buttons(timelapse *tl)
{
	int size = ListNumItems (tl->points); 

	switch(size)
	{
		case 0:
			SetCtrlAttribute(tl->revisit_panel, REVISIT_DELETE, ATTR_DIMMED, 1); 	
			// Fall through
		case 1:
			SetCtrlAttribute(tl->revisit_panel, REVISIT_PREV, ATTR_DIMMED, 1);
			SetCtrlAttribute(tl->revisit_panel, REVISIT_NEXT, ATTR_DIMMED, 1);
			break;
		default:
			SetCtrlAttribute(tl->revisit_panel, REVISIT_PREV, ATTR_DIMMED, 0);
			SetCtrlAttribute(tl->revisit_panel, REVISIT_NEXT, ATTR_DIMMED, 0);
			SetCtrlAttribute(tl->revisit_panel, REVISIT_DELETE, ATTR_DIMMED, 0);
	}
}

void timelapse_load_data_from_file(timelapse *tl, const char *filepath)
{
	TimelapseTableEntry pt;
	FILE *fp=NULL;
	int i, number_of_points = 0, n=0;
	float version = 0.0f;
	char microscope_version[500] = "";

	timelapse_clear_points(tl);
	
	fp = fopen (filepath, "r");
	
	// Get the version of the file being loaded
	// We don't use this yet but it may be useful for the future.
	fscanf(fp, "#Version %f\n", &version);  
	fscanf(fp, "#Microscope %[^\n]", microscope_version); 

	if(strcmp(microscope_version, UIMODULE_GET_NAME(tl->ms)) != 0) {
		// This file is not meant for this microscope
		GCI_MessagePopup("Warning", "This file was created on a different microscope. It may have problems loading");
	}

	// Get the number of points
	fscanf(fp, "%d\n", &number_of_points);   
	InsertTableRows (tl->panel_id, TIMELAPSE_TABLE, -1, number_of_points, VAL_USE_MASTER_CELL_TYPE); 
	
	for (i=1; i<=number_of_points; i++) {     
		
		if (version == 1.0) {
			if (fscanf(fp, "%lf\t%lf\t%lf\n", &(pt.centre.x), &(pt.centre.y), &(pt.centre.z)) < 3)
				break;

			pt.hasRegion = 0;
		}
		else {// cope with the new version with regions
			char buffer[512];
			fgets(buffer, 511, fp);  // reads to end of line (NB fscanf ignores line ends)
			n = sscanf(buffer, "%lf\t%lf\t%lf%d\t%lf\t%lf\t%d\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t\n", 
				&(pt.centre.x), &(pt.centre.y), &(pt.centre.z),
				&(pt.hasRegion),
				&(pt.regionSize.x), &(pt.regionSize.y), 
				&(pt.hasFocalPlane),
				&(pt.focalPlane_a),
				&(pt.focalPlane_b),
				&(pt.focalPlane_c));

			// n = number of items read from buffer
			if (n<3)  // does not have a enough data for a point, file must be corrupt or incorrect formatting
				break;
			else if (n<6) // does not have complete data for a region
				pt.hasRegion = 0;
			else if (n<16) // does not have complete data for a focal plane
				pt.hasFocalPlane = 0;
		}

		timelapse_add_point(tl, pt);      
	}
	
	fclose(fp);
	
	SetCtrlVal (tl->revisit_panel, REVISIT_NPTS_2, number_of_points);

	SetCtrlAttribute (tl->revisit_panel, REVISIT_PREV, ATTR_DIMMED, 0);
	SetCtrlAttribute (tl->revisit_panel, REVISIT_NEXT, ATTR_DIMMED, 0);
	SetCtrlAttribute (tl->revisit_panel, REVISIT_DELETE, ATTR_DIMMED, 0);
	
	timelapse_draw_points(tl);
}


void timelapse_load_data_from_selected_filepath(timelapse *tl) 
{
	char path[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];
	
	microscope_get_user_data_directory(tl->ms, path);              
	
	if (FileSelectPopup (path, "*.pts", "*.pts", "Load Points", VAL_LOAD_BUTTON, 0, 0, 1, 0, filepath) != 1)
		return;
	
	timelapse_load_data_from_file(tl, filepath);         
}



void timelapse_save_data(timelapse *tl, const char *filepath)
{
	FILE *fp=NULL;
	int i, number_of_points;
	TimelapseTableEntry point;
	
	fp = fopen (filepath, "w");
	
	number_of_points = ListNumItems(tl->points);
	
	fprintf(fp, "#Version 2.0\n");
	fprintf(fp, "#Microscope %s\n", UIMODULE_GET_NAME(tl->ms));

	// Save number of points first
	fprintf(fp, "%d\n", number_of_points); 
	
	for (i=1; i<=number_of_points; i++) {
	
		ListGetItem (tl->points, &point, i);    
		
		fprintf(fp, "%.2f\t%.2f\t%.2f\t%d", point.centre.x, point.centre.y, point.centre.z, point.hasRegion);

		if (point.hasRegion) {
			fprintf(fp, "\t%.2f\t%.2f", point.regionSize.x, point.regionSize.y);

			if (point.hasFocalPlane) {
				fprintf(fp, "\t%.2f\t%.2f\t%.2f", point.focalPlane_a, point.focalPlane_b, point.focalPlane_c);
			}
		}

		fprintf(fp, "\n");
	}
	
	fclose(fp);
}


void timelapse_save_data_to_selected_filepath(timelapse *tl)
{
	char path[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];             
	
	//Save positions to a text file. Save relative to the zero limit switch.
	microscope_get_user_data_directory(tl->ms, path);         

	if (FileSelectPopup (path, "*.pts", "*.pts", "Save Points", VAL_SAVE_BUTTON, 0, 0, 1, 1, filepath) < 1)
		return;
	
	timelapse_save_data(tl, filepath);   

	// Don't need autosaved data anymore.
	timelapse_clear_auto_save_data(tl);
}

static void timelapse_get_autosave_filepath(timelapse *tl, char *filepath)
{
	char path[GCI_MAX_PATHNAME_LEN];               
	
	//Save positions to a text file. Save relative to the zero limit switch.
	microscope_get_user_data_directory(tl->ms, path);         

	sprintf(filepath, "%s\\TimelapseAutoSavePoints.pts", path);
}

void timelapse_auto_save_data(timelapse *tl)
{
	char filepath[GCI_MAX_PATHNAME_LEN];               
	
	timelapse_get_autosave_filepath(tl, filepath);
	
	timelapse_save_data(tl, filepath);    
}

void timelapse_clear_auto_save_data(timelapse *tl)
{
	char filepath[GCI_MAX_PATHNAME_LEN];               
	
	timelapse_get_autosave_filepath(tl, filepath);
	
	DeleteFile(filepath);   
}

static void timelapse_reload_old_autosaved_data(timelapse *tl)
{
	int file_size;
	char filepath[GCI_MAX_PATHNAME_LEN];               
	
	timelapse_get_autosave_filepath(tl, filepath);

	if(FileExists(filepath, &file_size)) {

		if(GCI_ConfirmPopup("Warning", IDI_INFORMATION,
			"There appears to be some point data present from a previous run.\n"
			"Would you like to load this data")) {

			timelapse_load_data_from_file(tl, filepath);    

			return;
		}
		else {

			return;
		}
	}
}

void timelapse_display(timelapse *tl)         
{
	static int first_display = 1;

	// Load any previous autosaved data.
	if(first_display > 0) {
		timelapse_reload_old_autosaved_data(tl);
		first_display = 0;
	}

	SetCtrlAttribute(tl->panel_id, TIMELAPSE_TIMER, ATTR_ENABLED, 1);

	SetTableSelection (tl->panel_id, TIMELAPSE_TABLE, MakeRect(tl->current_point, 1, 1, TABLE_COL_NUMBER));

	check_mosaic_region_needed(tl);

	ui_module_attach_panel_to_panel(UIMODULE_CAST(tl), tl->regions_panel, UI_MODULE_REL_TOP_RIGHT, 5, 0);
	ui_module_attach_panel_to_panel(UIMODULE_CAST(tl), tl->revisit_panel, UI_MODULE_REL_BOTTOM_LEFT, 0, 5);

	ui_module_display_main_panel(UIMODULE_CAST(tl));
	ui_module_hide_panel(UIMODULE_CAST(tl), tl->revisit_panel);
	timelapse_region_mode_off(tl);

	timelapse_setup_cube_options(tl);
}

int timelapse_move_to_xyz_position (timelapse *tl, double x, double y, double z)
{
	if(z_drive_is_part_of_stage(MICROSCOPE_MASTER_ZDRIVE(tl->ms))) {

		// Ok the stage and z drive are one of the same.
		// So we move them at the same time to give a potential speed up.
		if (stage_goto_xyz_position (tl->stage, x, y, z) == STAGE_ERROR){
			return TIMELAPSE_ERROR;
		}
		z_drive_update_current_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), z);
	}
	else {
		z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), z);   
	
		if (stage_goto_xy_position (tl->stage, x, y) == STAGE_ERROR){
			return TIMELAPSE_ERROR;
		}

		// We need to wait here for the z drive to stop moving 
//		z_drive_wait_for_stop_moving (MICROSCOPE_MASTER_ZDRIVE(roi->ms), 2.0);
	}

	return TIMELAPSE_SUCCESS;
}

int timelapse_move_to_point(timelapse *tl, int pos) 
{
	int size;
	TimelapseTableEntry pt;
	
	size = ListNumItems(tl->points);
		
	if (pos > size)
		return TIMELAPSE_ERROR;
	
	timelapse_get_point(tl, pos, &pt);
	
	// Update current point on display
	SetCtrlVal (tl->revisit_panel, REVISIT_THISPOINT, pos);   
	SetTableSelection (tl->panel_id, TIMELAPSE_TABLE, MakeRect(pos, 1, 1, TABLE_COL_NUMBER));

	timelapse_move_to_xyz_position (tl, pt.centre.x, pt.centre.y, pt.centre.z);

//	stage_async_goto_xy_position(tl->stage, pt.centre.x, pt.centre.y);    
//	z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), pt.centre.z);

	tl->current_point = pos;
	
	timelapse_draw_points(tl); 
	
	return TIMELAPSE_SUCCESS;
}

int timelapse_move_to_next_point(timelapse *tl) 
{
	TimelapseTableEntry pt;
	
	int number_of_points = ListNumItems(tl->points);  
	
	// Revert back to the begiining of the list
	if (++(tl->current_point) > number_of_points)
		tl->current_point = 1;
	
	timelapse_get_point(tl, tl->current_point, &pt); 
	
	// Update current point on display
	SetCtrlVal (tl->revisit_panel, REVISIT_THISPOINT, tl->current_point);   
	SetTableSelection (tl->panel_id, TIMELAPSE_TABLE, MakeRect(tl->current_point, 1, 1, TABLE_COL_NUMBER));

	timelapse_move_to_xyz_position (tl, pt.centre.x, pt.centre.y, pt.centre.z);

//	stage_async_goto_xy_position(tl->stage, pt.centre.x, pt.centre.y);    
//	z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), pt.centre.z);
	
	timelapse_draw_points(tl); 
	
	return TIMELAPSE_SUCCESS;
}

int timelapse_move_to_previous_point(timelapse *tl) 
{
	TimelapseTableEntry pt;
	
	// Revert back to the end of the list
	if (--(tl->current_point) < 1)
		tl->current_point = ListNumItems(tl->points);
	
	timelapse_get_point(tl, tl->current_point, &pt); 
	
	// Update current point on display
	SetCtrlVal (tl->revisit_panel, REVISIT_THISPOINT, tl->current_point);   
	SetTableSelection (tl->panel_id, TIMELAPSE_TABLE, MakeRect(tl->current_point, 1, 1, TABLE_COL_NUMBER));

	timelapse_move_to_xyz_position (tl, pt.centre.x, pt.centre.y, pt.centre.z);

//	stage_async_goto_xy_position(tl->stage, pt.centre.x, pt.centre.y);  
//	z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), pt.centre.z);
	
	timelapse_draw_points(tl);
	
	return TIMELAPSE_SUCCESS;     
}


int timelapse_remove_point(timelapse *tl, int pos)    
{
	char msg[50] = "";
	int delete_overide_checked = 0;

	sprintf(msg,"Are you sure you want to remove point %d?", pos);
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	GetCtrlVal(tl->revisit_panel,REVISIT_DEL_WARN_OVERIDE, &delete_overide_checked);

	if (!delete_overide_checked) {
		if (!ConfirmPopup("", msg))
			return TIMELAPSE_SUCCESS;
	}

	ListRemoveItem(tl->points, NULL, pos);
	DeleteTableRows (tl->panel_id, TIMELAPSE_TABLE, pos, 1);

	// If we have remove the current point change the current point.
	if(pos == tl->current_point)
		tl->current_point--;
	
	timelapse_draw_points(tl);
	
	return TIMELAPSE_SUCCESS; 
}



int timelapse_clear_points(timelapse *tl) 
{
	int number_of_points = ListNumItems(tl->points);
	int numRows;
	
	DeleteGraphPlot (tl->revisit_panel, REVISIT_XY_DISP, -1, VAL_IMMEDIATE_DRAW);
	
	if(number_of_points > 0) {
		SetTableCellRangeAttribute (tl->panel_id, TIMELAPSE_TABLE, MakeRect(1,1,number_of_points,TABLE_COL_NUMBER), ATTR_CELL_TYPE, VAL_CELL_STRING);
		FillTableCellRange (tl->panel_id, TIMELAPSE_TABLE, MakeRect (1, 1, number_of_points, TABLE_COL_NUMBER), "");
	}
	
	GetNumTableRows (tl->panel_id, TIMELAPSE_TABLE, &numRows);
	DeleteTableRows (tl->panel_id, TIMELAPSE_TABLE, 1, numRows);

	ListClear(tl->points);
	
	tl->current_point = 0;
	
	SetCtrlVal (tl->revisit_panel, REVISIT_NPTS_2, 0);
	SetCtrlVal (tl->revisit_panel, REVISIT_THISPOINT, tl->current_point);

	SetCtrlAttribute (tl->revisit_panel, REVISIT_PREV, ATTR_DIMMED, 1);
	SetCtrlAttribute (tl->revisit_panel, REVISIT_NEXT, ATTR_DIMMED, 1);
	SetCtrlAttribute (tl->revisit_panel, REVISIT_DELETE, ATTR_DIMMED, 1);
	
	return TIMELAPSE_SUCCESS; 
}

int timelapse_update_centre_point(timelapse *tl, int pos)   
{
	GCI_FPOINT pt;	
	
	stage_get_xy_position (tl->stage, &(pt.x), &(pt.y));  
	
	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), &(pt.z));
	
	if(timelapse_edit_centre_point(tl, pos, pt) == TIMELAPSE_ERROR)
		return TIMELAPSE_ERROR;
	
	timelapse_draw_points(tl); 
	
	return TIMELAPSE_SUCCESS; 
}

int timelapse_new_point(timelapse *tl)   
{
	TimelapseTableEntry pt;	

	stage_get_xy_position (tl->stage, &(pt.centre.x), &(pt.centre.y));  

	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), &(pt.centre.z));

	pt.hasRegion = 0;
	
	timelapse_add_point(tl, pt);     
	
	timelapse_draw_points(tl);
	
	return TIMELAPSE_SUCCESS; 
}

int timelapse_new_region_WH(timelapse *tl, double width, double height)   
{   // supplied w and h, but uses stage XYZ
	TimelapseTableEntry pt;	

	stage_get_xy_position (tl->stage, &(pt.centre.x), &(pt.centre.y));  

	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(tl->ms), &(pt.centre.z));

	pt.hasRegion = 1;
	pt.regionSize.x = width;
	pt.regionSize.y = height;
	pt.hasFocalPlane = 0;
	
	timelapse_add_point(tl, pt);     
	
	timelapse_draw_points(tl);
	
	return TIMELAPSE_SUCCESS; 
}

void timelapse_set_vals_to_camera(timelapse *tl)
{
	double exposure, gain;
	int cube_pos;
	FluoCubeManager *cm;

	// get cube and set
	GetCtrlVal(tl->panel_id, TIMELAPSE_CUBE, &cube_pos);
	cm = microscope_get_cube_manager(tl->ms);
	cube_manager_move_to_position(cm, cube_pos);

	GetCtrlVal(tl->panel_id, TIMELAPSE_ACQGAIN, &gain);
	GetCtrlVal(tl->panel_id, TIMELAPSE_ACQEXPOSURE, &exposure);
	gci_camera_set_gain(tl->camera, CAMERA_ALL_CHANNELS, gain); 
	gci_camera_set_exposure_time(tl->camera, exposure); 
	gci_camera_snap_image(tl->camera);
}

void timelapse_get_vals_from_camera(timelapse *tl)
{
	double exposure, gain;
	FluoCubeManager *cm;

	// get cube and set menu
	cm = microscope_get_cube_manager(tl->ms);
	SetCtrlVal(tl->panel_id, TIMELAPSE_CUBE, cm->_current_pos);
	CallCtrlCallback(tl->panel_id, TIMELAPSE_CUBE, EVENT_COMMIT, 0, 0, 0);

	gci_camera_get_gain(tl->camera, CAMERA_ALL_CHANNELS, &gain); 
	exposure = gci_camera_get_exposure_time(tl->camera); 

	SetCtrlVal(tl->panel_id, TIMELAPSE_ACQEXPOSURE, exposure);
	SetCtrlVal(tl->panel_id, TIMELAPSE_ACQGAIN, gain);

	// update list by calling the callback
	CallCtrlCallback(tl->panel_id, TIMELAPSE_ACQEXPOSURE, EVENT_COMMIT, 0, 0, 0);
}

int timelapse_new_region_XYZWH(timelapse *tl, double X, double Y, double Z, double width, double height)   
{   // supplied XYZ, W and H
	TimelapseTableEntry pt;	

	pt.centre.x = X;
	pt.centre.y = Y;
	pt.centre.z = Z;
	pt.hasRegion = 1;
	pt.regionSize.x = width;
	pt.regionSize.y = height;
	pt.hasFocalPlane = 0;
	
	timelapse_add_point(tl, pt);     
	
	timelapse_draw_points(tl);
	
	return TIMELAPSE_SUCCESS; 
}

void timelapse_status(timelapse *tl, int *has_run, int *active, char *start_time, char *end_time)
{
	*active = tl->active;
   
	*has_run = tl->has_run;

	strncpy(start_time, tl->start_date_time, 199);
	strncpy(end_time, tl->end_date_time, 199);
}

int timelapse_perform_points(timelapse *tl)
{
	int current_point = 1, size = ListNumItems (tl->points);
	GCI_FPOINT pt;
	TimelapseTableEntry point;

	for (current_point=1; current_point<=size; current_point++)
	{
		// Get the point and move the stage
		ListGetItem (tl->points, &point, current_point);    
		
		tl->current_point = current_point;

		timelapse_draw_points(tl);

		// Run the script for this point
		pt.x = point.centre.x;
		pt.y = point.centre.y;
		pt.z = point.centre.z;
		timelapse_call_python_on_point_changed(tl, pt, current_point);   
					
		ProcessSystemEvents();
	
		if (!tl->active)
			return 1;
	}
	
	return 0;
}

int timelapse_perform_sequence(timelapse *tl)
{
	int stopped = 1, prog=-1, repeat_type = 0, repeat_val = 0, count = 0;
	int interval = 0 ; // Time to wait before moving on to next cycle.  
	double pct=0.0, time = 0.0;
	char text[512];
	
	tl->start_time = Timer();

	GetCtrlVal(tl->panel_id, TIMELAPSE_REPEAT_RING, &repeat_type);
	GetCtrlVal(tl->panel_id, TIMELAPSE_REPEAT_VAL, &repeat_val);          
	GetCtrlVal (tl->panel_id, TIMELAPSE_INTERVAL, &interval);  
		
	if(timelapse_import_script(tl) == TIMELAPSE_ERROR)
		return TIMELAPSE_ERROR;
		
	gci_camera_set_snap_mode(tl->camera); 
	stage_set_joystick_off(tl->stage);
	microscope_stop_all_timers(tl->ms);
	microscope_disable_all_panels(tl->ms, 1);     
	
	focus_set_off(tl->ms->_focus);
	
	// This leaves the abort / stop button enabled.
	ui_module_disable_panel(tl->panel_id, 2, TIMELAPSE_STOP, TIMELAPSE_REVISIT);           
	ui_module_disable_panel(tl->revisit_panel, 0);           
	ui_module_disable_panel(tl->regions_panel, 0);           
		
	// Hack to re enable controls
	// New dynamic menu work means all panels and controls
	// are dimmed when calling microscope_disable_all_panels
	SetCtrlAttribute(tl->panel_id, TIMELAPSE_STOP, ATTR_DIMMED, 0);
	SetCtrlAttribute(tl->panel_id, TIMELAPSE_REVISIT, ATTR_DIMMED, 0);

	tl->active = 1;
	tl->has_run = 1;

	get_time_string(tl->start_date_time);

	timelapse_call_python_on_start(tl); 

	while(tl->active)
	{
		time = Timer();	    
		
		// report progress
		if(repeat_type == REPEAT_CYCLES) {
			// check that we should not stop
			if (count >= repeat_val)
				goto FINISHED; 

			sprintf (text, "Status: Cycle %d of %d", count+1, repeat_val);
			SetCtrlVal(tl->panel_id, TIMELAPSE_PROGRESS, text);
		}
		else if(repeat_type == REPEAT_HOURS) {
			// check that we should not stop
			if (time - tl->start_time >= repeat_val * 3600)
				goto FINISHED; 

			sprintf (text, "Status: Cycle Started at %.4f of %d hours", (float)(time - tl->start_time)/3600.0, repeat_val);
			SetCtrlVal(tl->panel_id, TIMELAPSE_PROGRESS, text);
		}
		else {
			SetCtrlVal(tl->panel_id, TIMELAPSE_PROGRESS, "Status: running forever");
		}
	
		// start this cycle, this will probably call timelapse_perform_points() at least once 
		timelapse_call_python_on_cycle_start(tl); 
		
		if (!tl->active)
			goto FINISHED;
		
		// Wait (if neccessary) before beginning the next cycle
		if(repeat_type == REPEAT_CYCLES && count == repeat_val-1)
			goto FINISHED; 

		while((Timer() - time) < interval) 
		{
		
			if (prog<0)
				prog = CreateProgressDialog ("Time till next cycle", "Percent Complete", 0, VAL_NO_INNER_MARKERS, "__Cancel");

			pct = (Timer()-time)*100.0/interval;
				
			stopped = UpdateProgressDialog (prog, RoundRealToNearestInteger(pct), 1);
			
			if(stopped == 1) {
				tl->active = 0;
				goto FINISHED;
			}
			
			Delay(0.05);
		}
		
		if (prog>=0)
			DiscardProgressDialog (prog);

		prog = -1;
		
		count++;
	}
	
	FINISHED: 

	get_time_string(tl->end_date_time);
	SetCtrlVal(tl->panel_id, TIMELAPSE_PROGRESS, "Status: Idle");
		
	focus_set_on(tl->ms->_focus);       
	
	if (prog>=0) 
		DiscardProgressDialog (prog);
	
	stage_set_joystick_on(tl->stage);
	microscope_start_all_timers(tl->ms);
	microscope_disable_all_panels(tl->ms, 0);     
	
	ui_module_enable_panel(tl->panel_id, 0);   
	ui_module_enable_panel(tl->revisit_panel, 0);   
	ui_module_enable_panel(tl->regions_panel, 0);   
	
	return TIMELAPSE_SUCCESS; 
}

void timelapse_set_repeat_ring(timelapse *tl, int value)
{
	SetCtrlVal(tl->panel_id, TIMELAPSE_REPEAT_RING, value);
}

void timelapse_set_repeat_val(timelapse *tl, int value)
{
	SetCtrlVal(tl->panel_id, TIMELAPSE_REPEAT_VAL, value);
}

void timelapse_set_interval(timelapse *tl, int value)
{
	SetCtrlVal(tl->panel_id, TIMELAPSE_INTERVAL, value);
}


#endif // MICROSCOPE_PYTHON_AUTOMATION