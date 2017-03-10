#ifndef _SOFTWARE_AUTOFOCUS_
#define _SOFTWARE_AUTOFOCUS_

#include "HardwareTypes.h"
#include "gci_ui_module.h"
#include "FreeImageAlgorithms.h"

#define IS_TYPE(class, instance) (&((class *)8) == instance

#define FOCUS_IS_INSTANCE(obj) IS_TYPE(focus, obj)	
#define FOCUS_MODULE_CAST(obj) ((focus *) (obj))

typedef struct _SWAutoFocus SWAutoFocus;
	
struct _SWAutoFocus
{
	UIModule parent;

	Microscope *ms;
	int _panel_id;
	int _msg_panel_id;
	int _abort;

	FIBITMAP **freeimage_dib_array;    
	
};


SWAutoFocus* sw_autofocus_new(void);

void sw_autofocus_autofocus(SWAutoFocus *af);
void sw_autofocus_autofocus_abort(SWAutoFocus *af);

void sw_autofocus_destroy(SWAutoFocus *af);

int  CVICALLBACK OnAutoFocusGoButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCloseButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
