#ifndef __WIZARD__
#define __WIZARD__

#include "gci_ui_module.h" 

#define WIZARD_SUCCESS 1
#define WIZARD_ERROR   0

#define WIZARD_NAME_LENGTH  50
#define WIZARD_TITLE_LENGTH 100
#define WIZARD_DESC_LENGTH  500

typedef struct _wizard_step wizard_step;
typedef struct _wizard wizard;

typedef int (*WIZARD_EVENT_HANDLER) (wizard *wiz, void *data); 
typedef int (*WIZARD_STEP_EVENT_HANDLER) (wizard_step *step, int panel_id, void *data); 

struct _wizard_step
{
	wizard *wiz;
	int panel_id;
	int panel_resource_id;
	char name[WIZARD_NAME_LENGTH];
	char title[WIZARD_TITLE_LENGTH];
	char description[WIZARD_DESC_LENGTH];
};

struct _wizard
{
	UIModule parent;

	char uir_filepath[GCI_MAX_PATHNAME_LEN];
	int _panel_id;
	int _finished_panel_id;
	int _last_visible_panel_id;
	int _last_visible_panel_resource_id;	// Panel Resource
	int current_wizard_list;
	ListType steps;
	ListType steps_taken;

	int current_step;

	WIZARD_STEP_EVENT_HANDLER on_step_done;
	void *on_step_done_user_data;

	WIZARD_EVENT_HANDLER on_wizard_finished;
	void *on_wizard_finished_user_data;

	WIZARD_EVENT_HANDLER on_wizard_canceled;
	void *on_wizard_cancelled_user_data;
};

wizard * wizard_new(const char* description, const char *uir_filepath);

void wizard_destroy(wizard *wiz);

void wizard_set_on_step_shown_callback(wizard *wiz, WIZARD_STEP_EVENT_HANDLER handler, void *data);

void wizard_set_on_finished_callback(wizard *wiz, WIZARD_EVENT_HANDLER handler, void *data);

void wizard_set_on_cancelled_callback(wizard *wiz, WIZARD_EVENT_HANDLER handler, void *data);

int wizard_step_list_new(wizard *wiz, const char* name);

int wizard_list_add_step(wizard *wiz, const char *name, const char* title,
	const char* description, int panel_id);

int wizard_get_step_index_from_panel_resource(wizard *wiz, int panel_resource_id);

wizard_step* wizard_get_current_step(wizard *wiz);

wizard_step* wizard_get_step(wizard *wiz, int panel_resource_id);

wizard_step* wizard_get_step_from_index(wizard *wiz, int index);

int wizard_start(wizard *wiz, int panel_or_resource_id);

void wizard_set_next_button_dimmed(wizard *wiz, int dimmed);

/* Example

wizard = wizard_new();

wizard_step_list* manual_or_stage_plate_wizard_list = wizard_step_list_new();

wizard_step_list_step(manual_or_stage_plate_wizard_list, "Step1", STEP1);

*/

#endif