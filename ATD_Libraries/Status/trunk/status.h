#ifndef __STATUS__
#define __STATUS__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#include "gci_ui_module.h" 

typedef struct
{
	UIModule parent;   
    LONG_PTR old_wndproc;

	int panel_id;
	int textbox_id;
	int hide_button_id;
	
} Feedback;

void feedback_new(void);  

void feedback_destroy(void);

void feedback_show(char *title, char *message);

#endif
