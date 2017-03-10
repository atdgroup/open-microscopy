#ifndef __LOGGER__
#define __LOGGER__

#include "gci_ui_module.h" 
#include "gci_utils.h"

#include "toolbox.h"

typedef enum
{
    LOGGER_INFORMATIONAL,
    LOGGER_WARNING,
    LOGGER_ERROR

} LOGGER_PRIORITY;

typedef struct _Logger
{
	int panel_id;
	int enable_toggle_id;   
	int	logging_enabled;
	int lock;
	int number_of_errors;
	int number_of_errors_label;
	int file_id;

	HWND 		window_handle_hwnd;
	HWND		list;
	LONG_PTR  	panel_original_proc_fun_ptr;
	
	FILE		*fp;

    char save_dir[GCI_MAX_PATHNAME_LEN];
    
} Logger;

Logger* logger_new(const char* save_dir);  

void logger_set_data_dir(Logger *logger, const char *path);

void logger_destroy(Logger *logger);

void logger_log_ap(Logger *logger, LOGGER_PRIORITY priority, char *fmt, va_list ap);

void logger_log(Logger *logger, LOGGER_PRIORITY priority, char *message, ...);

void logger_get_log_filepath(Logger *logger, char *path);

void logger_show(Logger *logger);

void logger_hide(Logger *logger);

#endif
