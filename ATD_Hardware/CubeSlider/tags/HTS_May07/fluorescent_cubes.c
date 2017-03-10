#include "fluorescent_cubes.h"
#include "fluorescent_cubes_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"

#include "GL_CVIRegistry.h"
#include "toolbox.h"
#include "asynctmr.h"

#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Fluorescent cube control.
////////////////////////////////////////////////////////////////////////////

int send_fluocube_error_text (FluoCubeManager* cube_manager, char fmt[], ...)
{
	char *buf, *buffer_start;
	char tmp_buf[256];
	
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;
	
	if(cube_manager == NULL || cube_manager->_error_handler == NULL)
		return CUBE_MANAGER_ERROR;
	
	buffer_start = (char*) malloc(1024);
	buf = buffer_start;
	
	va_start(ap, fmt);
	
	for (p = fmt; *p; p++) {
	
		if (*p != '%') {
			*buf++ = *p;
			continue;
		}
		
		*buf = '\0';
		
		switch (*++p) {
			case 'd':
			case 'i':
				ival = va_arg(ap, int);
				sprintf(tmp_buf, "%d", ival);
				strcat(buf, tmp_buf);
				buf+=strlen(tmp_buf);
				break;
				
			case 'x':
				ival = va_arg(ap, int);
				sprintf(tmp_buf, "%x", ival);
				strcat(buf, tmp_buf);
				buf+=strlen(tmp_buf);
				break;
				
			case 'f':
				dval = va_arg(ap, double);
				sprintf(tmp_buf, "%f", dval);
				strcat(buf, tmp_buf);
				buf+=strlen(tmp_buf);
				break;
				
			case 's':
				sval = va_arg(ap, char *);
				strcat(buf, sval);
				buf+=strlen(sval);
				break;
				
			default:
				*buf++ = *p;
				break;
		}
		
	}
	
	*buf = '\0';
	va_end(ap);
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	cube_manager->_error_handler(buffer_start, cube_manager);
	free(buffer_start);
	
	return CUBE_MANAGER_SUCCESS;
}


static void error_handler (char *error_string, FluoCubeManager *cube_manager)
{
	MessagePopup("FluoCubeManager Error", error_string); 
}


static void cube_manager_read_or_write_main_panel_registry_settings(FluoCubeManager *cube_manager, int write)
{
	char buffer[500];
	int visible;

	if(cube_manager == NULL || cube_manager->_main_ui_panel == -1)
		return;

	// load or save panel positions
	
	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {
		GetPanelAttribute (cube_manager->_main_ui_panel, ATTR_VISIBLE, &visible);
		if(!visible)
			return;
	
		SetPanelAttribute (cube_manager->_main_ui_panel, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	}
	
	strcpy(buffer, "software\\GCI\\Microscope\\FluoCubeManager\\MainPanel\\");
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer,  "top",    cube_manager->_main_ui_panel, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer,  "left",   cube_manager->_main_ui_panel, ATTR_LEFT);
}


static int CUBE_MANAGER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (FluoCubeManager*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (FluoCubeManager *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int CUBE_MANAGER_PTR_CUBE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (FluoCubeManager*, FluoCube *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (FluoCubeManager *) args[0].void_ptr_data,  (FluoCube *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


void cube_manager_on_change(FluoCubeManager* cube_manager)
{
	int pos;
	static int prev_pos = -1;
	
	CmtGetLock (cube_manager->_lock);
		
	if (cube_manager_get_current_cube_position(cube_manager, &pos) == CUBE_MANAGER_ERROR) {
		CmtReleaseLock(cube_manager->_lock);
		return;
    }
    
	CmtReleaseLock(cube_manager->_lock);
			
	SetCtrlAttribute (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, ATTR_CTRL_INDEX, pos-1);
    
	if (pos != prev_pos) {
    	prev_pos = pos;
		GCI_Signal_Emit(&cube_manager->signal_table, "FluoCubeChanged", GCI_VOID_POINTER, cube_manager); 
    }
	
	cube_manager->_current_position = pos;
	if (pos != cube_manager->_required_position)
		SetCtrlVal (cube_manager->_main_ui_panel, CUBE_INFO_ERROR, 1);
	else
		SetCtrlVal (cube_manager->_main_ui_panel, CUBE_INFO_ERROR, 0);
	
}

int CVICALLBACK OnCubeTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager *cube_manager = (FluoCubeManager *) callbackData;
	
    switch (event)
    {
        case EVENT_TIMER_TICK:
        
        	cube_manager_on_change(cube_manager);
			
            break;
    }
    
    return 0;
}

static int CVICALLBACK sort_by_turret(void *item1, void *item2)
{
	FluoCube *cube1 = (FluoCube *) item1;
	FluoCube *cube2 = (FluoCube *) item2;
	
	return cube1->position - cube2->position;
}

static int cube_manager_load_active_cubes_into_main_list(FluoCubeManager* cube_manager)
{
	int i, size;
	FluoCube cube;
	
	if(ClearListCtrl (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS) < 0)
		return CUBE_MANAGER_ERROR;
		
	for(i=1; i <= CUBE_TURRET_SIZE; i++) 
		InsertListItem (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, -1, "empty", -1);
	
	if(cube_manager->number_of_present_cubes > 1)
		ListQuickSort (cube_manager->_list, sort_by_turret); 
		
	size = ListNumItems (cube_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		ListGetItem (cube_manager->_list, &cube, i);

		if(cube.position > 0)
			ReplaceListItem (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, cube.position-1, cube.name, cube.id);
	}
	
	return CUBE_MANAGER_SUCCESS;
}


static int cube_manager_load_details_panel(FluoCubeManager* cube_manager)
{
	cube_manager->_details_ui_panel = FindAndLoadUIR(0, "fluorescent_cubes_ui.uir", EDIT_PANEL); 
	
	if ( InstallCtrlCallback (cube_manager->_details_ui_panel, EDIT_PANEL_OK_BUTTON, OnCubeAddEditOkClicked, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;	
		
	return CUBE_MANAGER_SUCCESS;
}


static int cube_manager_init (FluoCubeManager* cube_manager)
{
	cube_manager->_timer = -1;
	cube_manager->_i2c_port = 1;
	cube_manager->_data_modified = 0;
	
	GCI_SignalSystem_Create(&(cube_manager->signal_table), 10);
	
	GCI_Signal_New(&(cube_manager->signal_table), "Close", CUBE_MANAGER_PTR_MARSHALLER);
	GCI_Signal_New(&(cube_manager->signal_table), "FluoCubeChanged", CUBE_MANAGER_PTR_MARSHALLER); 

	cube_manager_set_error_handler(cube_manager, error_handler);

    cube_manager->_main_ui_panel = FindAndLoadUIR(0, "fluorescent_cubes_ui.uir", CUBE_INFO);  
    cube_manager->_cube_table_panel = FindAndLoadUIR(0, "fluorescent_cubes_ui.uir", CUBE_CONF);
 
    if ( InstallCtrlCallback (cube_manager->_main_ui_panel, CUBE_INFO_SETUP, OnCubeSetup, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
  	
  	if ( InstallCtrlCallback (cube_manager->_main_ui_panel, CUBE_INFO_CLOSE, OnFluorCubeClose, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, OnCubeChanged, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_ADD, OnCubeDetailsAdd, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_EDIT, OnCubeDetailsEdit, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_REMOVE, OnCubeRemoveClicked, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_EDIT_ACTIVE, OnCubeEditActive, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_RIGHT_ARROW, OnCubeRightArrow, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_LEFT_ARROW, OnCubeLeftArrow, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_UP_ARROW, OnCubeUpArrow, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_DOWN_ARROW, OnCubeDownArrow, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
    if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE, OnCubeTreeValueChanged, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;

	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, OnCubeTreeValueChanged, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_SAVE, OnCubeFileSave, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_LOAD, OnCubeFileRecall, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_cube_table_panel, CUBE_CONF_QUIT, OnCubeConfigCloseClicked, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
	GetCtrlVal(cube_manager->_cube_table_panel, CUBE_CONF_DEFAULT, &cube_manager->_default_pos);
	SetPanelAttribute (cube_manager->_main_ui_panel, ATTR_TITLE, cube_manager->_description);
	
	SetCtrlAttribute (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, ATTR_DIMMED, !cube_manager->_mounted);

	#ifdef ENABLE_CUBE_STATUS_POLLING 
	cube_manager->_timer = NewAsyncTimer (2.0, -1, 1, OnCubeTimerTick, cube_manager);
	SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
	
  	cube_manager_load_cube_file(cube_manager, cube_manager->_data_file);
	SetCtrlVal(cube_manager->_cube_table_panel, CUBE_CONF_DEFAULT, cube_manager->_default_pos);
  	
  	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_signal_close_handler_connect (FluoCubeManager* cube_manager,
	CUBE_MANAGER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(&(cube_manager->signal_table), "Close", handler, callback_data) == SIGNAL_ERROR) {
		send_fluocube_error_text(cube_manager, "Can not connect signal handler for Close signal");
		return CUBE_MANAGER_ERROR;
	}

	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_signal_cube_changed_handler_connect(FluoCubeManager* cube_manager,
	CUBE_MANAGER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(&(cube_manager->signal_table), "FluoCubeChanged", handler, callback_data) == SIGNAL_ERROR) {
		return CUBE_MANAGER_ERROR;
	}
	
	return CUBE_MANAGER_SUCCESS; 
}


static int cube_manager_copy_cube(FluoCubeManager* cube_manager, FluoCube* src, FluoCube* cube)
{
	strcpy(cube->name, src->name);
	
	cube->id = src->id;
	cube->exc_nm = src->exc_nm;
	cube->dichroic_nm = src->dichroic_nm;
	cube->emm_min_nm = src->emm_min_nm;
	cube->emm_max_nm = src->emm_max_nm;

	return CUBE_MANAGER_SUCCESS;
}


FluoCube* cube_manager_get_cube_ptr_for_position(FluoCubeManager* cube_manager, int position)
{
	int i, size;
	FluoCube *cube;
	
	size = ListNumItems (cube_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		cube = ListGetPtrToItem (cube_manager->_list, i);

		if(cube->position == position)
			return cube;	
		
	}
	
	return NULL;
}


FluoCube* cube_manager_get_cube_ptr_for_id(FluoCubeManager* cube_manager, int id)
{
	int i, size;
	FluoCube *cube;
	
	size = ListNumItems (cube_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		cube = ListGetPtrToItem (cube_manager->_list, i);

		if(cube->id == id)
			return cube;	
		
	}
	
	return NULL;
}

		
int cube_manager_get_cube_for_position(FluoCubeManager* cube_manager, int position, FluoCube* dst_cube)
{
	FluoCube *cube;
	
	if((cube = cube_manager_get_cube_ptr_for_position(cube_manager, position)) == NULL)
		return CUBE_MANAGER_ERROR;  
	
	cube_manager_copy_cube(cube_manager, cube, dst_cube);   
	
	return CUBE_MANAGER_SUCCESS;  
}


int CVICALLBACK cube_active_tree_sort_cmp(int panel, int control, int item1, int item2, int keyCol, void *callbackData)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*) callbackData;
	int cube_id1;
	int cube_id2;
	FluoCube *cube1;
	FluoCube *cube2; 
	
	GetValueFromIndex (panel, control, item1, &cube_id1);
	GetValueFromIndex (panel, control, item2, &cube_id2);
	
	cube1 = cube_manager_get_cube_ptr_for_id(cube_manager, cube_id1); 
	cube2 = cube_manager_get_cube_ptr_for_id(cube_manager, cube_id2); 
	
	return cube1->position - cube2->position;
}


static int CVICALLBACK sort_by_position(void *item1, void *item2)
{
	FluoCube *cube1 = (FluoCube *) item1;
	FluoCube *cube2 = (FluoCube *) item2;
	
	return cube1->position - cube2->position;
}



int cube_manager_load_all_possible_cubes_into_ui(FluoCubeManager* cube_manager)
{
	int i, new_index, size;
	FluoCube cube;
	char position[10]; 
	
	if(ClearListCtrl (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE) < 0)
		return CUBE_MANAGER_ERROR;
		
	if(ClearListCtrl (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE) < 0)
		return CUBE_MANAGER_ERROR;
	
	size = ListNumItems (cube_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		ListGetItem (cube_manager->_list, &cube, i);

		if(cube.position) {
		
			sprintf(position, "%d", cube.position); 
			
			new_index = InsertTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE,
						VAL_SIBLING, 0, VAL_NEXT, position, 0, 0, cube.id);

			if(SetTreeCellAttribute (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE,
				new_index, 1, ATTR_LABEL_TEXT, cube.name)) {
				
				return CUBE_MANAGER_ERROR;	
			}
		}
		else {				 
		
			InsertTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE,
						VAL_SIBLING, 0, VAL_NEXT, cube.name, 0, 0, cube.id);
		}
	}
	
	
	if(cube_manager->number_of_present_cubes > 1)
		SortTreeItems (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, 0, 0, 0, 0, cube_active_tree_sort_cmp, cube_manager);
	
	// Update Main UI
	cube_manager_load_active_cubes_into_main_list(cube_manager);
	
	return CUBE_MANAGER_SUCCESS;
}



static int find_first_free_active_pos(FluoCubeManager* cube_manager)
{
	int i, size, active_pos = 1;
	FluoCube *cube;
	
	ListQuickSort (cube_manager->_list, sort_by_position);

	size = ListNumItems (cube_manager->_list);
	
	for(i = 1; i <= size; i++) {     
	
		cube = ListGetPtrToItem (cube_manager->_list, i); 
		
		if(!cube->position)
			continue;
			
		if(cube->position != active_pos)
			break;
			
		active_pos++;
	}
	
	return active_pos;
}

int  cube_manager_move_to_empty_position(FluoCubeManager* cube_manager)
{
	int turret_pos;
	
	turret_pos = find_first_free_active_pos(cube_manager);
	if (turret_pos > CUBE_TURRET_SIZE) return CUBE_MANAGER_ERROR;
	
	return cube_manager_move_to_position(cube_manager, turret_pos);
}

int cube_manager_switch_active_position(FluoCubeManager* cube_manager, int id1, int id2)
{
	int temp_turret_pos;
	FluoCube *cube1, *cube2;
	
	cube1 = cube_manager_get_cube_ptr_for_id(cube_manager, id1);    
	cube2 = cube_manager_get_cube_ptr_for_id(cube_manager, id2);
		
	temp_turret_pos = cube1->position; 
	cube1->position = cube2->position;
	cube2->position = temp_turret_pos;
	
	// Update Main UI
	cube_manager_load_all_possible_cubes_into_ui(cube_manager);    

	return CUBE_MANAGER_SUCCESS;  
}


int cube_manager_change_to_active(FluoCubeManager* cube_manager, int id)
{
	int turret_pos, size, i, panel, pnl, ctrl;
	FluoCube *cube;

	if(cube_manager->number_of_present_cubes >= CUBE_TURRET_SIZE)
		return CUBE_MANAGER_ERROR;	
			  
	panel = FindAndLoadUIR(0, "fluorescent_cubes_ui.uir", INSERT_CUB);

	size = ListNumItems (cube_manager->_list);
	for(i = 1; i <= size; i++) {     
		cube = ListGetPtrToItem (cube_manager->_list, i); 
		
		if(!cube->position>0) continue;
			
		ReplaceListItem (panel, INSERT_CUB_TURRET_POS, cube->position-1, "Occupied", -1);
	}
	
	InstallPopup(panel);
	while (1) {
		GetUserEvent (1, &pnl, &ctrl);
		if (pnl == panel) {
			if (ctrl == INSERT_CUB_CANCEL) {
				DiscardPanel(panel);
				return CUBE_MANAGER_SUCCESS;
			}
			if (ctrl == INSERT_CUB_OK) {
				GetCtrlVal(panel, INSERT_CUB_TURRET_POS, &turret_pos);
				if (turret_pos > 0)
					break;
				MessagePopup("Error", "That position is already occupied. Please try again.");
			}
		}
	}

	DiscardPanel(panel);
	
	cube_manager->number_of_present_cubes++;
	
	//turret_pos = find_first_free_active_pos(cube_manager);

	cube = cube_manager_get_cube_ptr_for_id(cube_manager, id);     

	cube->position = turret_pos;
	
	cube_manager_load_all_possible_cubes_into_ui(cube_manager);
		
	cube_manager->_data_modified = 1;

	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_add_cube(FluoCubeManager* cube_manager)
{
	int size = ListNumItems (cube_manager->_list);
	FluoCube cube;
	
	cube.id = ++size;
	cube.position = 0;
	
	ListInsertItem(cube_manager->_list, &cube, END_OF_LIST);
	
	return cube.id;
}


int cube_manager_get_cube_pos_for_id(FluoCubeManager* cube_manager, int id)
{
	int i, size;
	FluoCube *cube;
	
	size = ListNumItems (cube_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		cube = ListGetPtrToItem (cube_manager->_list, i);

		if(cube->id == id)
			return i;	
		
	}
	
	return -1;
}

int cube_manager_remove_cube(FluoCubeManager* cube_manager, int id)
{
	FluoCube cube;
	int index;
	
	index = cube_manager_get_cube_pos_for_id(cube_manager, id);
	if (index < 1) return CUBE_MANAGER_ERROR;

	ListGetItem (cube_manager->_list, &cube, index);

	if(cube.position > 0)
		cube_manager->number_of_present_cubes--; 
			    
	ListRemoveItem (cube_manager->_list, 0, index);       
			
	cube_manager->_data_modified = 1;

	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_remove_obj_at_active_position(FluoCubeManager* cube_manager, int id)
{
	FluoCube *cube;

	cube = cube_manager_get_cube_ptr_for_id(cube_manager, id);

	cube->position = 0;

	cube_manager->number_of_present_cubes--;
	
	cube_manager_load_all_possible_cubes_into_ui(cube_manager);
		
	cube_manager->_data_modified = 1;
	
	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_edit_cube(FluoCubeManager* cube_manager, int index)
{
	char name[20];
	FluoCube *cube = cube_manager_get_cube_ptr_for_id(cube_manager, index);
	
	GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_NAME, name);
	strcpy(cube->name, name);

	GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCNM, &(cube->exc_nm)); 

	GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_DICHROICNM, &(cube->dichroic_nm));

	GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMIN, &(cube->emm_min_nm));

	GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMAX, &(cube->emm_max_nm));

	DiscardPanel(cube_manager->_details_ui_panel);

	cube_manager_load_all_possible_cubes_into_ui(cube_manager); 
	
	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_edit_cube_ui(FluoCubeManager* cube_manager, int index)
{
	FluoCube *cube;  
	
	cube = cube_manager_get_cube_ptr_for_id(cube_manager, index);     
	
	cube_manager_load_details_panel(cube_manager);

	SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_INDEX, index); 
	
	SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 1);

	SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_NAME, cube->name);

	SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCNM, cube->exc_nm );

	SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_DICHROICNM, cube->dichroic_nm);

	SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMIN, cube->emm_min_nm);

	SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMAX, cube->emm_max_nm);

	DisplayPanel(cube_manager->_details_ui_panel);     
	
	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_add_cube_ui(FluoCubeManager* cube_manager)
{
	cube_manager_load_details_panel(cube_manager);

	SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 0);

	DisplayPanel(cube_manager->_details_ui_panel);

	cube_manager->_data_modified = 1;

	return CUBE_MANAGER_SUCCESS;
}



FluoCubeManager* cube_manager_new(char *name, char *description, const char *data_file, size_t size)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*) malloc(size);
	
	cube_manager->lpVtbl = (FluoCubeManagerVtbl *) malloc(sizeof(FluoCubeManagerVtbl)); 
	
	cube_manager->_list = ListCreate (sizeof(FluoCube)); 
	
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = NULL; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = NULL; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = NULL; 
	
	CmtNewLock (NULL, 0, &(cube_manager->_lock) );  
	
	cube_manager_set_description(cube_manager, description);
    cube_manager_set_name(cube_manager, name);
	cube_manager_set_datafile(cube_manager, data_file);

	cube_manager_init (cube_manager);
	
	return cube_manager;
}

void cube_manager_stop_timer(FluoCubeManager* cube_manager)
{
	#ifdef ENABLE_CUBE_STATUS_POLLING  
	SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void cube_manager_start_timer(FluoCubeManager* cube_manager)
{
	#ifdef ENABLE_CUBE_STATUS_POLLING  
	SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}

int cube_manager_destroy(FluoCubeManager* cube_manager)
{
	CHECK_CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) 
  	
	CALL_CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) 

	#ifdef ENABLE_CUBE_STATUS_POLLING  
	SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
	
	cube_manager_read_or_write_main_panel_registry_settings(cube_manager, 1);

	DiscardPanel(cube_manager->_main_ui_panel);
	if (cube_manager->_cube_table_panel >= 0)
		DiscardPanel(cube_manager->_cube_table_panel);
	
	cube_manager->_main_ui_panel = -1;
	cube_manager->_cube_table_panel = -1;
	
	if(cube_manager->_description != NULL) {
	
  		free(cube_manager->_description);
  		cube_manager->_description = NULL;
  	}
  	
  	if(cube_manager->_name != NULL) {
  	
  		free(cube_manager->_name);
  		cube_manager->_name = NULL;
  	}
  	
  	if(cube_manager->_data_file != NULL) {
	
		free(cube_manager->_data_file);
		cube_manager->_data_file = NULL;
	}
  	
  	CmtDiscardLock (cube_manager->_lock);
  	
  	free(cube_manager->lpVtbl);
  	free(cube_manager);
  	
  	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_set_datafile(FluoCubeManager* cube_manager, const char* data_file)
{
  	cube_manager->_data_file = (char *)malloc(strlen(data_file) + 1);

  	if(cube_manager->_data_file != NULL) {
    	strcpy(cube_manager->_data_file, data_file);
  	}
  	
  	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_set_i2c_port(FluoCubeManager* cube_manager, int port)
{
	cube_manager->_i2c_port = port;
	
	return CUBE_MANAGER_SUCCESS;  
}



void cube_manager_set_error_handler(FluoCubeManager* cube_manager, void (*handler) (char *error_string, FluoCubeManager *cube_manager) )
{
	cube_manager->_error_handler = handler;
}


int  cube_manager_set_description(FluoCubeManager* cube_manager, const char* description)
{
  	cube_manager->_description = (char *) malloc(strlen(description) + 1);

  	if(cube_manager->_description != NULL) {
    	strcpy(cube_manager->_description, description);
  	}
  	
  	return CUBE_MANAGER_SUCCESS;
}


int  cube_manager_get_description(FluoCubeManager* cube_manager, char *description)
{
  	if(cube_manager->_description != NULL) {
    
    	strcpy(description, cube_manager->_description);
    
    	return CUBE_MANAGER_SUCCESS;
  	}
  
  	return CUBE_MANAGER_ERROR;
}


int cube_manager_set_name(FluoCubeManager* cube_manager, char* name)
{
  	cube_manager->_name = (char *)malloc(strlen(name) + 1);

  	if(cube_manager->_name != NULL) {
    	strcpy(cube_manager->_name, name);
  	}
  	
  	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_get_name(FluoCubeManager* cube_manager, char *name)
{
  	if(cube_manager->_name != NULL) {
    
    	strcpy(name, cube_manager->_name);
    
    	return CUBE_MANAGER_SUCCESS;
  	}
  
  	return CUBE_MANAGER_ERROR;
}


int cube_manager_goto_default_position(FluoCubeManager* cube_manager)
{
	int position;

	cube_manager_get_current_cube_position(cube_manager, &position);
	if (position != cube_manager->_default_pos)
		cube_manager_move_to_position(cube_manager, cube_manager->_default_pos); 
	
	cube_manager->_current_position = cube_manager->_default_pos;

	return CUBE_MANAGER_SUCCESS;
}

int cube_manager_get_number_of_cubes(FluoCubeManager* cube_manager, int *number_of_cubes)
{
	*number_of_cubes = cube_manager->number_of_present_cubes;
	
	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_get_current_cube_position(FluoCubeManager* cube_manager, int *position)
{
	CHECK_CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) 

	if( (*cube_manager->lpVtbl->get_current_cube_position)(cube_manager, position) == CUBE_MANAGER_ERROR ) {
		send_fluocube_error_text(cube_manager, "cube_manager_get_current_cube_position failed");
		return CUBE_MANAGER_ERROR;
	}

  	return CUBE_MANAGER_SUCCESS;
}

int cube_manager_current_cube_position(FluoCubeManager* cube_manager, int *position)
{
	*position = cube_manager->_current_position;
	
  	return CUBE_MANAGER_SUCCESS;
}

int cube_manager_move_to_position(FluoCubeManager* cube_manager, int position)
{
	CHECK_CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) 

	if( (*cube_manager->lpVtbl->move_to_cube_position)(cube_manager, position) == CUBE_MANAGER_ERROR ) {
		send_fluocube_error_text(cube_manager, "cube_manager_move_to_position failed");
		return CUBE_MANAGER_ERROR;
	}

	cube_manager->_required_position = position;

  	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_get_cube(FluoCubeManager* cube_manager, int cube_number, FluoCube *cube)
{
	FluoCube *temp_cube; 
	
	if(cube_number < 1 || cube_number > cube_manager->number_of_present_cubes) 
		return CUBE_MANAGER_ERROR;
		
	temp_cube = cube_manager_get_cube_ptr_for_position(cube_manager, cube_number);

	cube_manager_copy_cube(cube_manager, temp_cube, cube);          
	
  	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_get_current_cube(FluoCubeManager* cube_manager, FluoCube *cube)
{
	int pos;
	
	if (cube_manager_get_current_cube_position(cube_manager, &pos) == CUBE_MANAGER_ERROR)
		return CUBE_MANAGER_ERROR;
		
	return cube_manager_get_cube_for_position(cube_manager, pos, cube);
}


int cube_manager_display_main_ui(FluoCubeManager* cube_manager)
{
	if(cube_manager->_main_ui_panel != -1) {
	
		cube_manager_read_or_write_main_panel_registry_settings(cube_manager, 0);
		
		DisplayPanel(cube_manager->_main_ui_panel);
		
		SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  1);
	}
	
	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_hide_main_ui(FluoCubeManager* cube_manager)
{
	if(cube_manager->_main_ui_panel != -1) {
		cube_manager_read_or_write_main_panel_registry_settings(cube_manager, 1);
		HidePanel(cube_manager->_main_ui_panel);
	}
	
	GCI_Signal_Emit(&cube_manager->signal_table, "Close", GCI_VOID_POINTER, cube_manager); 
	
	//SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	
	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_is_main_ui_visible(FluoCubeManager* cube_manager)
{
	int visible;
	
	GetPanelAttribute(cube_manager->_main_ui_panel, ATTR_VISIBLE, &visible);
	
	return visible;
}



int cube_manager_display_config_ui(FluoCubeManager* cube_manager)
{
	if(cube_manager->_cube_table_panel != -1) {
	
		GCI_ShowPasswordProtectedPanel(cube_manager->_cube_table_panel);
	}
	
	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_hide_config_ui(FluoCubeManager* cube_manager)
{
	if(cube_manager->_cube_table_panel != -1)
		HidePanel(cube_manager->_cube_table_panel);
	
	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_is_config_ui_visible(FluoCubeManager* cube_manager)
{
	int visible;
	
	GetPanelAttribute(cube_manager->_cube_table_panel, ATTR_VISIBLE, &visible);
	
	return visible;
}
