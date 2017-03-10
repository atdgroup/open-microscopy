#ifndef __GCI_UIMODULE__
#define __GCI_UIMODULE__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define INITGUID 
#include <ole2.h>
#include <objbase.h>
#include <initguid.h>
#include <windows.h>

#include "signals.h"
#include "gci_ui_logger.h"
#include "stdarg.h"
#include "gci_utils.h"
#include "Toolbox.h"

#define UI_MODULE_SUCCESS 1
#define UI_MODULE_ERROR  0

#define UIMODULE_CAST(obj) ((UIModule *) (obj))    
#define UIMODULE_SIGNAL_TABLE(obj) &(UIMODULE_CAST(obj)->_signal_table) 
  
#define UIMODULE_MAIN_PANEL_ID(obj) (UIMODULE_CAST(obj)->_main_panel_id) 
#define UIMODULE_LOGGER(obj) (UIMODULE_CAST(obj)->_logger)
#define UIMODULE_WND_HWND(obj) ((HWND)(UIMODULE_CAST(obj)->window_handle_hwnd)) 
//#define UIMODULE_ORIG_WNDPROC(obj) (UIMODULE_CAST(obj)->panel_original_proc_fun_ptr) 

#define UIMODULE_GET_NAME(obj) (UIMODULE_CAST(obj)->_name)
#define UIMODULE_GET_DESCRIPTION(obj) (UIMODULE_CAST(obj)->_description) 
#define UIMODULE_GET_DATA_DIR(obj) (UIMODULE_CAST(obj)->_data_dir) 

#define UIMODULE_NAME_LEN 200

#define UIMODULE_ERROR_NONE 0
#define UIMODULE_ERROR_IGNORE 1
#define UIMODULE_ERROR_RETRY 2

#define MICROSCOPE_BLUE MakeColor(30, 87, 174)
#define MICROSCOPE_GRAY MakeColor(192, 192, 192)
#define MICROSCOPE_GRAY_COLORREF RGB(192, 192, 192)

typedef struct _UIModule UIModule;
typedef struct _Logger Logger;

// Signals
typedef void (*UI_MODULE_EVENT_HANDLER) (UIModule *module, void *data); 

typedef void (*UI_MODULE_EVENT_INT_HANDLER) (UIModule *module, int panel_id,  void *data); 

typedef int (*UI_MODULE_ERROR_HANDLER) (UIModule *module, const char *title, const char *error_string, void *callback_data);       

#define UI_MOD_MAX_PANELS 10
#define UI_MOD_MAX_ATTACHEE_PANELS 5

typedef enum {UI_MODULE_REL_TOP_LEFT,
			  UI_MODULE_REL_TOP_RIGHT,
			  UI_MODULE_REL_BOTTOM_LEFT,
			  UI_MODULE_REL_BOTTOM_RIGHT,
			  UI_MODULE_REL_CENTRE} UIModuleRelativePoint;

typedef struct _AttacheePanelInfo
{
	int _panel_id;
	int _attachee_panel_id;
	int _offset_x;
	int _offset_y;
	int _old_style;
	int _old_titlebar_visible;
	int _old_sizeable;
	UIModuleRelativePoint _relative;

} AttacheePanelInfo;

struct _UIModule
{
	UI_MODULE_ERROR_HANDLER _error_handler;    
	
	Logger* _logger;
	void *callback_data;

	//int _number_of_child_panels;
	int _id;
	int _main_panel_id;
	int _module_list_id;
	int _number_of_panels;
	int _panel_ids[UI_MOD_MAX_PANELS];
	int _child_panel_ids[10];
	int _prevent_mouse_activation;
	int _number_of_attachee_panels;
	AttacheePanelInfo _attachee_panels[UI_MOD_MAX_ATTACHEE_PANELS];
	char _name[200];
	char _description[GCI_MAX_PATHNAME_LEN];
	char _data_dir[GCI_MAX_PATHNAME_LEN];		
	
	signal_table _signal_table;
};

void ui_module_constructor(UIModule *module, const char *name);

LONG_PTR ui_module_get_original_wndproc_ptr(UIModule *module);

int ui_module_number_of_modules(void);
ListType* ui_module_get_ui_module_list(void);
UIModule* ui_module_get_module_at_index(int index);

void ui_module_prevent_mouse_activation(UIModule *module);

void ui_module_enable_mouse_activation(UIModule *module);

void ui_module_force_send_sizing_message(UIModule *module);

int ui_module_add_panel(UIModule *module, const char *filename, int panel_id, int is_main);

int ui_module_add_panel_as_child(UIModule *module, const char *filename, int panel_id, int parent_panel_id);

LONG_PTR ui_module_set_window_proc(UIModule *module, int panel_id, LONG_PTR proc);   

void ui_module_hide_logger(UIModule *module);

void ui_module_destroy_panel(UIModule *module, int panel_id);

void ui_module_restore_cvi_wnd_proc(int panel_id);

void ui_module_destroy(UIModule *module);

void ui_module_display_main_panel_without_registry(UIModule *module);

void ui_module_display_main_panel(UIModule *module);

void ui_module_display_main_panel_without_activation(UIModule *module);

void ui_module_display_panel_without_activation(UIModule *module, int panel_id);

void ui_module_display_panel(UIModule *module, int panel_id);

void ui_module_display_all_panels(UIModule *module);

void ui_module_hide_panel(UIModule *module, int panel_id);

void ui_module_hide_main_panel(UIModule *module);

void ui_module_hide_all_panels(UIModule *module);

void ui_module_set_error_handler(UIModule *module, UI_MODULE_ERROR_HANDLER handler, void *callback_data);

int ui_module_send_valist_error(UIModule *module, const char *title, const char *fmt, va_list ap);

int ui_module_send_error(UIModule *module, const char *title, const char *fmt, ...);

int ui_module_dialog_feedback(UIModule *module, const char *title, const char *fmt, ...);

int ui_module_panel_are_any_panels_visible(UIModule *module);

int ui_module_panel_is_visible(UIModule *module, int panel_id);

int ui_module_main_panel_is_visible(UIModule *module);

int ui_module_set_name(UIModule *module, const char* name);

int ui_module_get_name(UIModule *module, char* name);

void ui_module_set_logger(UIModule *module, Logger *log);

Logger* ui_module_get_logger(UIModule *module);

int ui_module_set_description(UIModule *module, const char* description);

int ui_module_get_description(UIModule *module, char* description);

int ui_module_set_data_dir(UIModule *module, const char* path);

int ui_module_get_data_dir(UIModule *module, char* path);

void ui_module_name_get_registry_store_for_name(const char *name, int panel_id, char *path);

void ui_module_name_read_or_write_registry_settings(const char *name, int panel_id, int write);

void ui_module_panel_read_or_write_registry_settings(UIModule *module, int panel_id, int write);

void ui_module_read_or_write_registry_settings(UIModule *module, int write);

int ui_module_panel_saved_visibility_status (UIModule *module, int panel_id);

int ui_module_disable_all_panels_and_controls(UIModule *module, int disable);

int ui_module_disable_panel(int panel_id, int number_of_exceptions, ...);

int ui_module_disable_all_panel_controls(UIModule *module, int dimmed);

int ui_module_set_panel_dimmed_attribute(int panel_id, int dimmed, int number_of_exceptions, ...);

int ui_module_enable_panel(int panel_id, int number_of_exceptions, ...);

int ui_module_move_control_pixels_from_right(int panel_id, int ctrl_id, int pixels);

int ui_module_move_control_pixels_from_bottom(int panel_id, int ctrl_id, int pixels);

int ui_module_anchor_control_to_panel_edge(int panel_id, int ctrl_id, int left, int right, int top, int bottom);

void ui_module_save_settings(UIModule *module, const char *filepath, const char *flags);

void ui_module_restore_settings(UIModule *module, const char *filepath);

UI_MODULE_ERROR_HANDLER ui_module_get_error_handler(UIModule *module); 

void ui_module_set_main_panel_title (UIModule *module);

// Event Connection
int ui_module_main_panel_hide_or_close_handler_connect (UIModule *module, UI_MODULE_EVENT_HANDLER handler,
	void *callback_data);

int ui_module_main_panel_show_mainpanel_handler_connect (UIModule *module, UI_MODULE_EVENT_HANDLER handler,
	void *callback_data);

int ui_module_panel_show_handler_connect (UIModule *module, UI_MODULE_EVENT_INT_HANDLER handler,
	void *callback_data);

void ui_module_clear_attached_panels(UIModule *module);
void ui_module_clear_attached_panel(UIModule *module, int panel_id); // Written but UNTESTED!

void ui_module_attach_panel_to_panel_advanced(UIModule *module, int parent_panel, int attchee_panel, UIModuleRelativePoint relative,
													  int offset_x, int offset_y);

void ui_module_attach_panel_to_panel(UIModule *module, int attchee_panel, UIModuleRelativePoint relative,
													   int offset_x, int offset_y);

#ifdef __cplusplus
}
#endif

#endif
