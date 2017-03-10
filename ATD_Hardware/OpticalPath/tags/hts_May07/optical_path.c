#include "optical_path.h"
#include "optical_path_ui.h"
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
//Optical Path control.
////////////////////////////////////////////////////////////////////////////

int send_optical_path_error_text (OpticalPathManager* optical_path_manager, char fmt[], ...)
{
	char *buf, *buffer_start;
	char tmp_buf[256];
	
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;
	
	if(optical_path_manager == NULL || optical_path_manager->_error_handler == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;
	
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
	optical_path_manager->_error_handler(buffer_start, optical_path_manager);
	free(buffer_start);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


static void error_handler (char *error_string, OpticalPathManager *optical_path_manager)
{
	MessagePopup("OpticalPathManager Error", error_string); 
}


static void optical_path_manager_read_or_write_main_panel_registry_settings(OpticalPathManager *optical_path_manager, int write)
{
	char buffer[500];
	int visible;

	if(optical_path_manager == NULL || optical_path_manager->_main_ui_panel == -1)
		return;

	// load or save panel positions
	
	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {
		GetPanelAttribute (optical_path_manager->_main_ui_panel, ATTR_VISIBLE, &visible);
		if(!visible)
			return;
	
		SetPanelAttribute (optical_path_manager->_main_ui_panel, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	}
	
	strcpy(buffer, "software\\GCI\\Microscope\\OpticalPathManager\\MainPanel\\");
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer,  "top",    optical_path_manager->_main_ui_panel, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer,  "left",   optical_path_manager->_main_ui_panel, ATTR_LEFT);
}


static int OPTICAL_PATH_MANAGER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (OpticalPathManager*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (OpticalPathManager *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int OPTICAL_PATH_MANAGER_PTR_OPTICAL_PATH_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (OpticalPathManager*, OpticalPath *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (OpticalPathManager *) args[0].void_ptr_data,  (OpticalPath *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


void optical_path_manager_on_change(OpticalPathManager* optical_path_manager)
{
	int pos;
	static int prev_pos = -1;

	CmtGetLock (optical_path_manager->_lock);
	
	if (optical_path_manager_get_current_optical_path_position(optical_path_manager, &pos) == OPTICAL_PATH_MANAGER_ERROR) {
		CmtReleaseLock(optical_path_manager->_lock);
		return;
    }
    
	CmtReleaseLock(optical_path_manager->_lock);

	SetCtrlAttribute (optical_path_manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, ATTR_CTRL_INDEX, pos-1);
    
	if (pos != prev_pos) {
    	prev_pos = pos;
        GCI_Signal_Emit(&optical_path_manager->signal_table, "OpticalPathChanged", GCI_VOID_POINTER, optical_path_manager); 
	}
}

int CVICALLBACK OnOpticalPathTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager *optical_path_manager = (OpticalPathManager *) callbackData;
	
    switch (event)
    {
        case EVENT_TIMER_TICK:
        
        	optical_path_manager_on_change(optical_path_manager);
			
            break;
    }
    
    return 0;
}

static int CVICALLBACK sort_by_turret(void *item1, void *item2)
{
	OpticalPath *optical_path1 = (OpticalPath *) item1;
	OpticalPath *optical_path2 = (OpticalPath *) item2;
	
	return optical_path1->position - optical_path2->position;
}

static int optical_path_manager_load_active_optical_paths_into_main_list(OpticalPathManager* optical_path_manager)
{
	int i, size;
	OpticalPath optical_path;
	
	if(ClearListCtrl (optical_path_manager->_main_ui_panel, OPTIC_PATH_TURRET_POS) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
	for(i=1; i <= OPTICAL_PATH_TURRET_SIZE; i++) 
		InsertListItem (optical_path_manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, -1, "empty", -1);
	
	if(optical_path_manager->number_of_present_optical_paths > 1)
		ListQuickSort (optical_path_manager->_list, sort_by_turret); 
		
	size = ListNumItems (optical_path_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		ListGetItem (optical_path_manager->_list, &optical_path, i);

		if(optical_path.position > 0)
			ReplaceListItem (optical_path_manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, optical_path.position-1, optical_path.name, optical_path.id);
	}
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


static int optical_path_manager_load_details_panel(OpticalPathManager* optical_path_manager)
{
	optical_path_manager->_details_ui_panel = FindAndLoadUIR(0, "optical_path_ui.uir", PATH_DETS); 
	
	if ( InstallCtrlCallback (optical_path_manager->_details_ui_panel, PATH_DETS_OK_BUTTON, OnOpticalPathAddEditOkClicked, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;	
		
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


static int optical_path_manager_init (OpticalPathManager* optical_path_manager)
{
	optical_path_manager->_timer = -1;
	optical_path_manager->_i2c_port = 1;
	optical_path_manager->_data_modified = 0;
	
	GCI_SignalSystem_Create(&(optical_path_manager->signal_table), 10);
	
	GCI_Signal_New(&(optical_path_manager->signal_table), "Close", OPTICAL_PATH_MANAGER_PTR_MARSHALLER);
	GCI_Signal_New(&(optical_path_manager->signal_table), "OpticalPathChanged", OPTICAL_PATH_MANAGER_PTR_MARSHALLER); 

	optical_path_manager_set_error_handler(optical_path_manager, error_handler);

    optical_path_manager->_main_ui_panel = FindAndLoadUIR(0, "optical_path_ui.uir", OPTIC_PATH);  
    optical_path_manager->_optical_path_table_panel = FindAndLoadUIR(0, "optical_path_ui.uir", PATH_CONF);
 
    if ( InstallCtrlCallback (optical_path_manager->_main_ui_panel, OPTIC_PATH_SETUP, OnOpticalPathSetup, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
  	
    if ( InstallCtrlCallback (optical_path_manager->_main_ui_panel, OPTIC_PATH_CALIBRATE, OnOpticalPathCalibrate, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
  	
  	if ( InstallCtrlCallback (optical_path_manager->_main_ui_panel, OPTIC_PATH_CLOSE, OnOpticalPathClose, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, OnOpticalPathChanged, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_ADD, OnOpticalPathDetailsAdd, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_EDIT, OnOpticalPathDetailsEdit, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_REMOVE, OnOpticalPathRemoveClicked, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_EDIT_ACTIVE, OnOpticalPathEditActive, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_RIGHT_ARROW, OnOpticalPathRightArrow, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_LEFT_ARROW, OnOpticalPathLeftArrow, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_UP_ARROW, OnOpticalPathUpArrow, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_DOWN_ARROW, OnOpticalPathDownArrow, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
    if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE, OnOpticalPathTreeValueChanged, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;

	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, OnOpticalPathTreeValueChanged, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_SAVE, OnOpticalPathFileSave, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_LOAD, OnOpticalPathFileRecall, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (optical_path_manager->_optical_path_table_panel, PATH_CONF_QUIT, OnOpticalPathConfigCloseClicked, optical_path_manager) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
	SetPanelAttribute (optical_path_manager->_main_ui_panel, ATTR_TITLE, optical_path_manager->_description);
	
	SetCtrlAttribute (optical_path_manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, ATTR_DIMMED, !optical_path_manager->_mounted);

	#ifdef ENABLE_OPTICAL_PATH_POLLING
	optical_path_manager->_timer = NewAsyncTimer (2.0, -1, 1, OnOpticalPathTimerTick, optical_path_manager);
	SetAsyncTimerAttribute (optical_path_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
	
  	optical_path_manager_load_optical_path_file(optical_path_manager, optical_path_manager->_data_file);
  	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}

void optical_path_manager_stop_timer(OpticalPathManager* optical_path_manager)
{
	#ifdef ENABLE_OPTICAL_PATH_POLLING  
	SetAsyncTimerAttribute (optical_path_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif 
}

void optical_path_manager_start_timer(OpticalPathManager* optical_path_manager)
{
	#ifdef ENABLE_OPTICAL_PATH_POLLING  
	SetAsyncTimerAttribute (optical_path_manager->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif 
}


int optical_path_manager_signal_close_handler_connect (OpticalPathManager* optical_path_manager,
	OPTICAL_PATH_MANAGER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(&(optical_path_manager->signal_table), "Close", handler, callback_data) == SIGNAL_ERROR) {
		send_optical_path_error_text(optical_path_manager, "Can not connect signal handler for Close signal");
		return OPTICAL_PATH_MANAGER_ERROR;
	}

	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_signal_optical_path_changed_handler_connect(OpticalPathManager* optical_path_manager,
	OPTICAL_PATH_MANAGER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(&(optical_path_manager->signal_table), "OpticalPathChanged", handler, callback_data) == SIGNAL_ERROR) {
		return OPTICAL_PATH_MANAGER_ERROR;
	}
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}


static int optical_path_manager_copy_optical_path(OpticalPathManager* optical_path_manager, OpticalPath* src, OpticalPath* optical_path)
{
	strcpy(optical_path->name, src->name);
	optical_path->id = src->id;
	optical_path->position = src->position;
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


OpticalPath* optical_path_manager_get_optical_path_ptr_for_position(OpticalPathManager* optical_path_manager, int position)
{
	int i, size;
	OpticalPath *optical_path;
	
	size = ListNumItems (optical_path_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		optical_path = ListGetPtrToItem (optical_path_manager->_list, i);

		if(optical_path->position == position)
			return optical_path;	
		
	}
	
	return NULL;
}


OpticalPath* optical_path_manager_get_optical_path_ptr_for_id(OpticalPathManager* optical_path_manager, int id)
{
	int i, size;
	OpticalPath *optical_path;
	
	size = ListNumItems (optical_path_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		optical_path = ListGetPtrToItem (optical_path_manager->_list, i);

		if(optical_path->id == id)
			return optical_path;	
		
	}
	
	return NULL;
}

		
int optical_path_manager_get_optical_path_for_position(OpticalPathManager* optical_path_manager, int position, OpticalPath* dst_optical_path)
{
	OpticalPath *optical_path;
	
	if((optical_path = optical_path_manager_get_optical_path_ptr_for_position(optical_path_manager, position)) == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;  
	
	optical_path_manager_copy_optical_path(optical_path_manager, optical_path, dst_optical_path);   
	
	return OPTICAL_PATH_MANAGER_SUCCESS;  
}


int CVICALLBACK optical_path_active_tree_sort_cmp(int panel, int control, int item1, int item2, int keyCol, void *callbackData)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;
	int optical_path_id1;
	int optical_path_id2;
	OpticalPath *optical_path1;
	OpticalPath *optical_path2; 
	
	GetValueFromIndex (panel, control, item1, &optical_path_id1);
	GetValueFromIndex (panel, control, item2, &optical_path_id2);
	
	optical_path1 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, optical_path_id1); 
	optical_path2 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, optical_path_id2); 
	
	return optical_path1->position - optical_path2->position;
}


static int CVICALLBACK sort_by_position(void *item1, void *item2)
{
	OpticalPath *optical_path1 = (OpticalPath *) item1;
	OpticalPath *optical_path2 = (OpticalPath *) item2;
	
	return optical_path1->position - optical_path2->position;
}



int optical_path_manager_load_all_possible_optical_paths_into_ui(OpticalPathManager* optical_path_manager)
{
	int i, new_index, size;
	OpticalPath optical_path;
	char position[10]; 
	
	if(ClearListCtrl (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
	if(ClearListCtrl (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	size = ListNumItems (optical_path_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		ListGetItem (optical_path_manager->_list, &optical_path, i);

		if(optical_path.position) {
		
			sprintf(position, "%d", optical_path.position); 
			
			new_index = InsertTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE,
						VAL_SIBLING, 0, VAL_NEXT, position, 0, 0, optical_path.id);

			if(SetTreeCellAttribute (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE,
				new_index, 1, ATTR_LABEL_TEXT, optical_path.name)) {
				
				return OPTICAL_PATH_MANAGER_ERROR;	
			}
		}
		else {				 
		
			InsertTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE,
						VAL_SIBLING, 0, VAL_NEXT, optical_path.name, 0, 0, optical_path.id);
		}
	}
	
	
	if(optical_path_manager->number_of_present_optical_paths > 1)
		SortTreeItems (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, 0, 0, 0, 0, optical_path_active_tree_sort_cmp, optical_path_manager);
	
	// Update Main UI
	optical_path_manager_load_active_optical_paths_into_main_list(optical_path_manager);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}



static int find_first_free_active_pos(OpticalPathManager* optical_path_manager)
{
	int i, size, active_pos = 1;
	OpticalPath *optical_path;
	
	ListQuickSort (optical_path_manager->_list, sort_by_position);

	size = ListNumItems (optical_path_manager->_list);
	
	for(i = 1; i <= size; i++) {     
	
		optical_path = ListGetPtrToItem (optical_path_manager->_list, i); 
		
		if(!optical_path->position)
			continue;
			
		if(optical_path->position != active_pos)
			break;
			
		active_pos++;
	}
	
	return active_pos;
}


int optical_path_manager_switch_active_position(OpticalPathManager* optical_path_manager, int id1, int id2)
{
	int temp_turret_pos;
	OpticalPath *optical_path1, *optical_path2;
	
	optical_path1 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, id1);    
	optical_path2 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, id2);
		
	temp_turret_pos = optical_path1->position; 
	optical_path1->position = optical_path2->position;
	optical_path2->position = temp_turret_pos;
	
	// Update Main UI
	optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager);    

	return OPTICAL_PATH_MANAGER_SUCCESS;  
}


int optical_path_manager_change_to_active(OpticalPathManager* optical_path_manager, int id)
{
	int turret_pos, size, i, panel, pnl, ctrl;
	OpticalPath *optical_path;

	if(optical_path_manager->number_of_present_optical_paths >= OPTICAL_PATH_TURRET_SIZE)
		return OPTICAL_PATH_MANAGER_ERROR;	
			  
	panel = FindAndLoadUIR(0, "optical_path_ui.uir", ADD_PATH);

	size = ListNumItems (optical_path_manager->_list);
	for(i = 1; i <= size; i++) {     
		optical_path = ListGetPtrToItem (optical_path_manager->_list, i); 
		
		if(!optical_path->position>0) continue;
			
		ReplaceListItem (panel, ADD_PATH_TURRET_POS, optical_path->position-1, "Occupied", -1);
	}
	
	InstallPopup(panel);
	while (1) {
		GetUserEvent (1, &pnl, &ctrl);
		if (pnl == panel) {
			if (ctrl == ADD_PATH_CANCEL) {
				DiscardPanel(panel);
				return OPTICAL_PATH_MANAGER_SUCCESS;
			}
			if (ctrl == ADD_PATH_OK) {
				GetCtrlVal(panel, ADD_PATH_TURRET_POS, &turret_pos);
				if (turret_pos > 0)
					break;
				MessagePopup("Error", "That position is already occupied. Please try again.");
			}
		}
	}

	DiscardPanel(panel);
	
	optical_path_manager->number_of_present_optical_paths++;
	
	//turret_pos = find_first_free_active_pos(optical_path_manager);

	optical_path = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, id);     

	optical_path->position = turret_pos;
	
	optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager);
		
	optical_path_manager->_data_modified = 1;

	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_add_optical_path(OpticalPathManager* optical_path_manager)
{
	int size = ListNumItems (optical_path_manager->_list);
	OpticalPath optical_path;
	
	optical_path.id = ++size;
	optical_path.position = 0;
	
	ListInsertItem(optical_path_manager->_list, &optical_path, END_OF_LIST);
	
	return optical_path.id;
}


int optical_path_manager_get_optical_path_pos_for_id(OpticalPathManager* optical_path_manager, int id)
{
	int i, size;
	OpticalPath *optical_path;
	
	size = ListNumItems (optical_path_manager->_list);
	
	for(i=1; i <= size; i++) {
	
		optical_path = ListGetPtrToItem (optical_path_manager->_list, i);

		if(optical_path->id == id)
			return i;	
		
	}
	
	return -1;
}

int optical_path_manager_remove_optical_path(OpticalPathManager* optical_path_manager, int id)
{
	OpticalPath optical_path;
	int index;
	
	index = optical_path_manager_get_optical_path_pos_for_id(optical_path_manager, id);
	if (index < 1) return OPTICAL_PATH_MANAGER_ERROR;

	ListGetItem (optical_path_manager->_list, &optical_path, index);

	if(optical_path.position > 0)
		optical_path_manager->number_of_present_optical_paths--; 
			    
	ListRemoveItem (optical_path_manager->_list, 0, index);       
			
	optical_path_manager->_data_modified = 1;

	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_remove_optical_path_at_active_position(OpticalPathManager* optical_path_manager, int id)
{
	OpticalPath *optical_path;

	optical_path = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, id);

	optical_path->position = 0;

	optical_path_manager->number_of_present_optical_paths--;
	
	optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager);
		
	optical_path_manager->_data_modified = 1;
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_edit_optical_path(OpticalPathManager* optical_path_manager, int index)
{
	char name[20];
	OpticalPath *optical_path = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, index);
	
	GetCtrlVal(optical_path_manager->_details_ui_panel, PATH_DETS_NAME, name);
	strcpy(optical_path->name, name);

	DiscardPanel(optical_path_manager->_details_ui_panel);

	optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager); 
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_edit_optical_path_ui(OpticalPathManager* optical_path_manager, int index)
{
	OpticalPath *optical_path;  
	
	optical_path = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, index);     
	
	optical_path_manager_load_details_panel(optical_path_manager);

	SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_DETS_INDEX, index); 
	
	SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_DETS_ADDOREDIT, 1);

	SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_DETS_NAME, optical_path->name);

	DisplayPanel(optical_path_manager->_details_ui_panel);     
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_add_optical_path_ui(OpticalPathManager* optical_path_manager)
{
	optical_path_manager_load_details_panel(optical_path_manager);

	SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_DETS_ADDOREDIT, 0);

	DisplayPanel(optical_path_manager->_details_ui_panel);

	optical_path_manager->_data_modified = 1;

	return OPTICAL_PATH_MANAGER_SUCCESS;
}



OpticalPathManager* optical_path_manager_new(char *name, char *description, const char *data_file, size_t size)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*) malloc(size);
	
	optical_path_manager->lpVtbl = (OpticalPathManagerVtbl *) malloc(sizeof(OpticalPathManagerVtbl)); 
	
	optical_path_manager->_list = ListCreate (sizeof(OpticalPath)); 
	
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, destroy) = NULL; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, move_to_optical_path_position) = NULL; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, get_current_optical_path_position) = NULL; 
	
	CmtNewLock (NULL, 0, &(optical_path_manager->_lock) );  
	
	optical_path_manager_set_description(optical_path_manager, description);
    optical_path_manager_set_name(optical_path_manager, name);
	optical_path_manager_set_datafile(optical_path_manager, data_file);

	optical_path_manager_init (optical_path_manager);
	
	return optical_path_manager;
}


int optical_path_manager_destroy(OpticalPathManager* optical_path_manager)
{
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, destroy) 
  	
	CALL_OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, destroy) 

	SetAsyncTimerAttribute (optical_path_manager->_timer, ASYNC_ATTR_ENABLED,  0);

	optical_path_manager_read_or_write_main_panel_registry_settings(optical_path_manager, 1);

	DiscardPanel(optical_path_manager->_main_ui_panel);
	if (optical_path_manager->_optical_path_table_panel >= 0)
		DiscardPanel(optical_path_manager->_optical_path_table_panel);
	
	optical_path_manager->_main_ui_panel = -1;
	optical_path_manager->_optical_path_table_panel = -1;
	
	if(optical_path_manager->_description != NULL) {
	
  		free(optical_path_manager->_description);
  		optical_path_manager->_description = NULL;
  	}
  	
  	if(optical_path_manager->_name != NULL) {
  	
  		free(optical_path_manager->_name);
  		optical_path_manager->_name = NULL;
  	}
  	
  	if(optical_path_manager->_data_file != NULL) {
	
		free(optical_path_manager->_data_file);
		optical_path_manager->_data_file = NULL;
	}
  	
  	CmtDiscardLock (optical_path_manager->_lock);
  	
  	free(optical_path_manager->lpVtbl);
  	free(optical_path_manager);
  	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_set_datafile(OpticalPathManager* optical_path_manager, const char* data_file)
{
  	optical_path_manager->_data_file = (char *)malloc(strlen(data_file) + 1);

  	if(optical_path_manager->_data_file != NULL) {
    	strcpy(optical_path_manager->_data_file, data_file);
  	}
  	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_set_i2c_port(OpticalPathManager* optical_path_manager, int port)
{
	optical_path_manager->_i2c_port = port;
	
	return OPTICAL_PATH_MANAGER_SUCCESS;  
}



void optical_path_manager_set_error_handler(OpticalPathManager* optical_path_manager, void (*handler) (char *error_string, OpticalPathManager *optical_path_manager) )
{
	optical_path_manager->_error_handler = handler;
}


int  optical_path_manager_set_description(OpticalPathManager* optical_path_manager, const char* description)
{
  	optical_path_manager->_description = (char *) malloc(strlen(description) + 1);

  	if(optical_path_manager->_description != NULL) {
    	strcpy(optical_path_manager->_description, description);
  	}
  	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int  optical_path_manager_get_description(OpticalPathManager* optical_path_manager, char *description)
{
  	if(optical_path_manager->_description != NULL) {
    
    	strcpy(description, optical_path_manager->_description);
    
    	return OPTICAL_PATH_MANAGER_SUCCESS;
  	}
  
  	return OPTICAL_PATH_MANAGER_ERROR;
}


int optical_path_manager_set_name(OpticalPathManager* optical_path_manager, char* name)
{
  	optical_path_manager->_name = (char *)malloc(strlen(name) + 1);

  	if(optical_path_manager->_name != NULL) {
    	strcpy(optical_path_manager->_name, name);
  	}
  	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_get_name(OpticalPathManager* optical_path_manager, char *name)
{
  	if(optical_path_manager->_name != NULL) {
    
    	strcpy(name, optical_path_manager->_name);
    
    	return OPTICAL_PATH_MANAGER_SUCCESS;
  	}
  
  	return OPTICAL_PATH_MANAGER_ERROR;
}


int optical_path_manager_get_number_of_optical_paths(OpticalPathManager* optical_path_manager, int *number_of_optical_paths)
{
	*number_of_optical_paths = optical_path_manager->number_of_present_optical_paths;
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_get_current_optical_path_position(OpticalPathManager* optical_path_manager, int *position)
{
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, get_current_optical_path_position) 

	if( (*optical_path_manager->lpVtbl->get_current_optical_path_position)(optical_path_manager, position) == OPTICAL_PATH_MANAGER_ERROR ) {
		send_optical_path_error_text(optical_path_manager, "optical_path_manager_get_current_optical_path_position failed");
		return OPTICAL_PATH_MANAGER_ERROR;
	}

  	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int optical_path_manager_move_to_position(OpticalPathManager* optical_path_manager, int position)
{
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, move_to_optical_path_position) 

	if( (*optical_path_manager->lpVtbl->move_to_optical_path_position)(optical_path_manager, position) == OPTICAL_PATH_MANAGER_ERROR ) {
		send_optical_path_error_text(optical_path_manager, "optical_path_manager_move_to_position failed");
		return OPTICAL_PATH_MANAGER_ERROR;
	}

  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_get_optical_path(OpticalPathManager* optical_path_manager, int optical_path_number, OpticalPath *optical_path)
{
	OpticalPath *temp_optical_path; 
	
	if(optical_path_number < 1 || optical_path_number > optical_path_manager->number_of_present_optical_paths) 
		return OPTICAL_PATH_MANAGER_ERROR;
		
	temp_optical_path = optical_path_manager_get_optical_path_ptr_for_position(optical_path_manager, optical_path_number);

	optical_path_manager_copy_optical_path(optical_path_manager, temp_optical_path, optical_path);          
	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_get_current_optical_path(OpticalPathManager* optical_path_manager, OpticalPath *optical_path)
{
	int pos;
	
	if (optical_path_manager_get_current_optical_path_position(optical_path_manager, &pos) == OPTICAL_PATH_MANAGER_ERROR)
		return OPTICAL_PATH_MANAGER_ERROR;
		
	return optical_path_manager_get_optical_path_for_position(optical_path_manager, pos, optical_path);            
}


int optical_path_manager_display_main_ui(OpticalPathManager* optical_path_manager)
{
	if(optical_path_manager->_main_ui_panel != -1) {
	
		optical_path_manager_read_or_write_main_panel_registry_settings(optical_path_manager, 0);
		
		DisplayPanel(optical_path_manager->_main_ui_panel);
		
		SetAsyncTimerAttribute (optical_path_manager->_timer, ASYNC_ATTR_ENABLED,  1);
	}
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_hide_main_ui(OpticalPathManager* optical_path_manager)
{
	if(optical_path_manager->_main_ui_panel != -1) {
		optical_path_manager_read_or_write_main_panel_registry_settings(optical_path_manager, 1);
		HidePanel(optical_path_manager->_main_ui_panel);
	}
	
	GCI_Signal_Emit(&optical_path_manager->signal_table, "Close", GCI_VOID_POINTER, optical_path_manager); 
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_is_main_ui_visible(OpticalPathManager* optical_path_manager)
{
	int visible;
	
	GetPanelAttribute(optical_path_manager->_main_ui_panel, ATTR_VISIBLE, &visible);
	
	return visible;
}



int optical_path_manager_display_config_ui(OpticalPathManager* optical_path_manager)
{
	if(optical_path_manager->_optical_path_table_panel != -1) {
	
		GCI_ShowPasswordProtectedPanel(optical_path_manager->_optical_path_table_panel);
	}
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_hide_config_ui(OpticalPathManager* optical_path_manager)
{
	if(optical_path_manager->_optical_path_table_panel != -1)
		HidePanel(optical_path_manager->_optical_path_table_panel);
	
	//GCI_Signal_Emit(&optical_path_manager->signal_table, "Hide", GCI_VOID_POINTER, optical_path_manager); 
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_is_config_ui_visible(OpticalPathManager* optical_path_manager)
{
	int visible;
	
	GetPanelAttribute(optical_path_manager->_optical_path_table_panel, ATTR_VISIBLE, &visible);
	
	return visible;
}

int optical_path_manager_display_calib_ui(OpticalPathManager* optical_path_manager)
{
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, setup_optical_path) 

	if( (*optical_path_manager->lpVtbl->setup_optical_path)(optical_path_manager) == OPTICAL_PATH_MANAGER_ERROR ) {
		send_optical_path_error_text(optical_path_manager, "setup_optical_path failed");
		return OPTICAL_PATH_MANAGER_ERROR;
	}

  	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int optical_path_manager_hide_calib_ui(OpticalPathManager* optical_path_manager)
{
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(optical_path_manager, hide_optical_path_calib) 

	if( (*optical_path_manager->lpVtbl->hide_optical_path_calib)(optical_path_manager) == OPTICAL_PATH_MANAGER_ERROR ) {
		send_optical_path_error_text(optical_path_manager, "hide_optical_path_calib failed");
		return OPTICAL_PATH_MANAGER_ERROR;
	}

  	return OPTICAL_PATH_MANAGER_SUCCESS;
}

