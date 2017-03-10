#include "WizardUI.h"
#include "Wizard.h"
#include "timelapse.h"
#include "timelapse_ui.h"  
#include "TimeLapse-PointWizardImportUI.h"
#include "StagePlate.h"
#include "stage\stage.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "microscope.h"
#include "file_prefix_dialog.h"
#include "gci_menu_utils.h"

#include <userint.h>  
#include <utility.h> 

void wizard_set_next_button_dimmed(wizard *wiz, int dimmed)
{
	SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_NEXT, ATTR_DIMMED, dimmed);
}

void wizard_set_prev_button_dimmed(wizard *wiz, int dimmed)
{
	SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_PREV, ATTR_DIMMED, dimmed);
}

static void DisplayPage(wizard *wiz, int panel_id, int panel_or_resorce_id, const char *title, const char *description)
{
	int left, top, width, height;

	// We need sto find the next step and display the panel
	if(wiz->_last_visible_panel_id != 0)
		HidePanel(wiz->_last_visible_panel_id);

	GetCtrlAttribute(wiz->_panel_id, WIZ_PNL_SW_BORDER, ATTR_LEFT, &left);
	GetCtrlAttribute(wiz->_panel_id, WIZ_PNL_SW_BORDER, ATTR_TOP, &top);
	GetCtrlAttribute(wiz->_panel_id, WIZ_PNL_SW_BORDER, ATTR_WIDTH, &width);
	GetCtrlAttribute(wiz->_panel_id, WIZ_PNL_SW_BORDER, ATTR_HEIGHT, &height);

	SetPanelAttribute(panel_id, ATTR_TITLEBAR_VISIBLE, 0);
	SetPanelAttribute(panel_id, ATTR_BORDER_VISIBLE, 0);

	SetPanelAttribute(panel_id, ATTR_LEFT, left);
	SetPanelAttribute(panel_id, ATTR_TOP, top);
	SetPanelAttribute(panel_id, ATTR_WIDTH, width);
	SetPanelAttribute(panel_id, ATTR_HEIGHT, height);

	SetCtrlVal(wiz->_panel_id, WIZ_PNL_TITLE, title);
	SetCtrlVal(wiz->_panel_id, WIZ_PNL_DESCRIPTION, description);

	DisplayPanel(panel_id);
}

static void MoveButtonOverOther(int panel_id, int button1, int button2)
{
	int left, top;

	GetCtrlAttribute(panel_id, button2, ATTR_LEFT, &left);
	GetCtrlAttribute(panel_id, button2, ATTR_TOP, &top);

	SetCtrlAttribute(panel_id, button1, ATTR_LEFT, left);
	SetCtrlAttribute(panel_id, button1, ATTR_TOP, top);
}

static void DisplayStepPanel(wizard *wiz, wizard_step *step, int next)
{
	DisplayPage(wiz, step->panel_id, step->panel_resource_id, step->title, step->description);	
	
	wiz->_last_visible_panel_id = step->panel_id;
	wiz->_last_visible_panel_resource_id = step->panel_resource_id;
	
	if(next == 0) {
		// We are going to a previous page lets replace the last page in the steps taken list with the new one
		if(ListNumItems(wiz->steps_taken) > 0)
			ListRemoveItem(wiz->steps_taken, NULL, END_OF_LIST);

		return;
	}

	ListInsertItem(wiz->steps_taken, step, END_OF_LIST);
}

static void PrintDebugStepList(wizard *wiz)
{
	int i;
	wizard_step *step = NULL;
	int number_of_steps = ListNumItems(wiz->steps_taken);
	
	printf("---------------------------------------------------------------------------------------\n\n");

	for(i=1; i <= number_of_steps; i++) {
		
		step = (wizard_step *) ListGetPtrToItem(wiz->steps_taken, i);
		
		printf("name:%s description:%s panel_id:%d panel_resource_id:%d\n", step->name, step->description, step->panel_id, step->panel_resource_id);
	}
}

static int CVICALLBACK OnNextClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			wizard *wiz = (wizard *) callbackData;  

			int next_panel_or_resorce_id;
			wizard_step *step = wizard_get_step(wiz, wiz->_last_visible_panel_resource_id);
			
			// This is called so the client can deal with the step that has just completed.
			if(wiz->on_step_done != NULL) {

				next_panel_or_resorce_id = wiz->on_step_done(step, step->panel_id, step->wiz->on_step_done_user_data);

				if(next_panel_or_resorce_id == 0)
				{
					// Hide next and Prev buttons and change cancel button to OK
					
//					MoveButtonOverOther(wiz->_panel_id, WIZ_PNL_OK, WIZ_PNL_CANCEL);
					SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_NEXT, ATTR_VISIBLE, 0);
//					SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_PREV, ATTR_VISIBLE, 0);
					SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_CANCEL, ATTR_VISIBLE, 0);
					SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_OK, ATTR_VISIBLE, 1);

					// We seem to be finish lets set the ok button to finished
					DisplayPage(wiz, wiz->_finished_panel_id, FINISHED, "Finished", "That completes the Wizard!");	
				
					// Just add something to the list in case back is pressed, then it is removed ;-), repeat last item
					ListInsertItem(wiz->steps_taken, step, END_OF_LIST);

					wizard_set_next_button_dimmed(wiz, 1);

					return 0;
				}

				step = wizard_get_step(wiz, next_panel_or_resorce_id);

				if(step == NULL)
					return 0;
				
				wizard_set_prev_button_dimmed(wiz, 0);
				
				DisplayStepPanel(wiz, step, 1);

				printf("Adding\n");
				PrintDebugStepList(wiz);
			}

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnPrevClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{		
			wizard *wiz = (wizard *) callbackData;  
			wizard_step step;
	
			int size = ListNumItems(wiz->steps_taken);

			if (size <= 2) { // we are going to the beginning, coming back from the second panel
				wizard_set_prev_button_dimmed(wiz, 1);
			}
			
			if (size <= 1) { // we are at the beginning, go no further
				return 0;
			}

			ListGetItem(wiz->steps_taken, &step, size - 1);

			SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_NEXT, ATTR_VISIBLE, 1);
			SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_CANCEL, ATTR_VISIBLE, 1);
			SetCtrlAttribute(wiz->_panel_id, WIZ_PNL_OK, ATTR_VISIBLE, 0);

			DisplayStepPanel(wiz, &step, 0);

			// If someone has gone back then they should be able to go forward again.
			wizard_set_next_button_dimmed(wiz, 0);

			printf("Removing\n");
			PrintDebugStepList(wiz);

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnCancelClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{		
			wizard *wiz = (wizard *) callbackData;  
			
			if(wiz->on_wizard_canceled != NULL)
				wiz->on_wizard_canceled(wiz, wiz->on_wizard_cancelled_user_data);

			wizard_destroy(wiz);

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{		
			wizard *wiz = (wizard *) callbackData;  
			
			if(wiz->on_wizard_finished != NULL)
				wiz->on_wizard_finished(wiz, wiz->on_wizard_finished_user_data);

			wizard_destroy(wiz);

			break;
		}
	}
	
	return 0;
}

void wizard_destroy(wizard *wiz)
{
	ListDispose(wiz->steps);
	ListDispose(wiz->steps_taken);
	ui_module_destroy(UIMODULE_CAST(wiz));
}

wizard * wizard_new(const char* description, const char *uir_filepath)
{
	wizard *wiz = (wizard *) malloc (sizeof(wizard));		
	
	memset(wiz, 0, sizeof(wizard));

	ui_module_constructor(UIMODULE_CAST(wiz), "Wizard");  
	
	strcpy(wiz->uir_filepath, uir_filepath);
	wiz->_last_visible_panel_id = 0;
	wiz->steps = ListCreate(sizeof(wizard_step));
	wiz->steps_taken = ListCreate(sizeof(wizard_step));
	
	wiz->_panel_id = ui_module_add_panel(UIMODULE_CAST(wiz), "WizardUI.uir", WIZ_PNL, 1);   
	wiz->_finished_panel_id = ui_module_add_panel_as_child(UIMODULE_CAST(wiz), wiz->uir_filepath, FINISHED, wiz->_panel_id);  
	
	SetPanelAttribute(wiz->_panel_id, ATTR_TITLE, description);

	if ( InstallCtrlCallback (wiz->_panel_id, WIZ_PNL_NEXT, OnNextClicked, wiz) < 0)
		return NULL;

	if ( InstallCtrlCallback (wiz->_panel_id, WIZ_PNL_PREV, OnPrevClicked, wiz) < 0)
		return NULL;

	if ( InstallCtrlCallback (wiz->_panel_id, WIZ_PNL_CANCEL, OnCancelClicked, wiz) < 0)
		return NULL;

	if ( InstallCtrlCallback (wiz->_panel_id, WIZ_PNL_OK, OnOkClicked, wiz) < 0)
		return NULL;

	return wiz;
}

void wizard_set_on_step_shown_callback(wizard *wiz, WIZARD_STEP_EVENT_HANDLER handler, void *data)
{
	wiz->on_step_done = handler;
	wiz->on_step_done_user_data = data;
}

void wizard_set_on_finished_callback(wizard *wiz, WIZARD_EVENT_HANDLER handler, void *data)
{
	wiz->on_wizard_finished = handler;
	wiz->on_wizard_finished_user_data = data;
}

void wizard_set_on_cancelled_callback(wizard *wiz, WIZARD_EVENT_HANDLER handler, void *data)
{
	wiz->on_wizard_canceled = handler;
	wiz->on_wizard_cancelled_user_data = data;
}

int wizard_list_add_step(wizard *wiz, const char *name, const char* title,
	const char* description, int panel_resource_id)
{
	wizard_step step;

	step.wiz = wiz;
	strcpy(step.name, name);
	strcpy(step.title, title);
	strcpy(step.description, description);
	step.panel_resource_id = panel_resource_id;
	
	step.panel_id = ui_module_add_panel_as_child(UIMODULE_CAST(wiz), wiz->uir_filepath, panel_resource_id, wiz->_panel_id);  
	
	ListInsertItem (wiz->steps, &step, END_OF_LIST);
	
	return step.panel_id;
}

int wizard_get_step_index_from_panel_resource(wizard *wiz, int panel_resource_id)
{
	int i, number_of_steps;
	wizard_step * step = NULL;

	number_of_steps = ListNumItems(wiz->steps);
	
	for(i=1; i <= number_of_steps; i++) {
		
		step = (wizard_step *) ListGetPtrToItem(wiz->steps, i);
		
		if(step->panel_resource_id == panel_resource_id)
			return i;
	}

	GCI_MessagePopup("Error", "Panel with resource id %d has not been added with wizard_list_add_step", panel_resource_id);
	return -1;
}

wizard_step* wizard_get_step_from_index(wizard *wiz, int index)
{
	int i, number_of_steps;
	wizard_step * step = NULL;

	number_of_steps = ListNumItems(wiz->steps);
	
	for(i=1; i <= number_of_steps; i++) {
		
		step = (wizard_step *) ListGetPtrToItem(wiz->steps, i);
		
		if(i == index)
			return step;
	}

	GCI_MessagePopup("Error", "Step with index %d not found", index);

	return NULL;
}

wizard_step* wizard_get_step(wizard *wiz, int panel_resource_id)
{
	int i, number_of_steps;
	wizard_step * step = NULL;

	number_of_steps = ListNumItems(wiz->steps);
	
	for(i=1; i <= number_of_steps; i++) {
		
		step = (wizard_step *) ListGetPtrToItem(wiz->steps, i);
		
		if(step->panel_resource_id == panel_resource_id)
			return step;
	}

	GCI_MessagePopup("Error", "Panel with resource id %d has not been added with wizard_list_add_step", panel_resource_id);
	return NULL;
}

int wizard_start(wizard *wiz, int panel_or_resource_id)
{
	wizard_step *step = wizard_get_step(wiz, panel_or_resource_id);

	wizard_set_prev_button_dimmed(wiz, 1); 

	DisplayStepPanel(wiz, step, 1);

	ui_module_display_panel(UIMODULE_CAST(wiz), wiz->_panel_id);

	return 0;
}