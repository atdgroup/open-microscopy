#ifndef __MICROSCOPY__
#define __MICROSCOPY__

#include "Config.h"
#include "Microscope.h"
#include "AutomationEditor.h"

typedef struct _Microscopy
{
	UIModule parent;   
  
	HINSTANCE hInstance;
	
	Microscope *microscope;
	AutomationEditor* automation_editor;

} Microscopy;



Microscopy* microscopy_app_new(HINSTANCE hInstance);

void microscopy_app_close(Microscopy* app); 

void microscopy_get_user_folder(char *path); 

#endif
