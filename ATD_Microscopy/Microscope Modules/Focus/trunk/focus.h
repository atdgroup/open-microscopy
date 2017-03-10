#ifndef _FOCUS_INDICATOR_
#define _FOCUS_INDICATOR_

#include "gci_ui_module.h"
#include "FreeImageAlgorithms.h"

#define IS_TYPE(class, instance) (&((class *)8) == instance

#define FOCUS_IS_INSTANCE(obj) IS_TYPE(focus, obj)	
#define FOCUS_MODULE_CAST(obj) ((focus *) (obj))

typedef struct _GciCamera GciCamera;

typedef enum {FOCUS_SETTINGS_SAMPLE_CUSTOM=0, 
			  FOCUS_SETTINGS_SAMPLE_FUZZY=1,
			  FOCUS_SETTINGS_SAMPLE_DETAILED=2,
			 } FOCUS_SETTINGS_SAMPLE_TYPE;

typedef enum {FOCUS_SETTINGS_CROP_NONE=1, 
			  FOCUS_SETTINGS_CROP_2=2,
			  FOCUS_SETTINGS_CROP_4=4,
			  FOCUS_SETTINGS_CROP_8=8
			 } FOCUS_SETTINGS_CROP_TYPE;

typedef enum {FOCUS_SETTINGS_RESAMPLE_NONE=1, 
			  FOCUS_SETTINGS_RESAMPLE_2=2,
			  FOCUS_SETTINGS_RESAMPLE_4=4,
			  FOCUS_SETTINGS_RESAMPLE_8=8,
			 } FOCUS_SETTINGS_RESAMPLE_TYPE;

typedef struct 
{
	FOCUS_SETTINGS_SAMPLE_TYPE sample_type; 
	FOCUS_SETTINGS_CROP_TYPE crop_type;
	FOCUS_SETTINGS_RESAMPLE_TYPE resample_type;

} FocusSettings;

typedef struct _focus
{
	UIModule parent;

	GciCamera *camera;
	
	int _focus_on;
	int _panel_id;
	
	int _resizing_handler_id;
	int _moving_handler_id;
	int _minimise_handler_id;
	int _restored_handler_id;

	int prev_rescaled_width;
	int prev_rescaled_height;
	
	FIBITMAP *full_mask;
	FIBITMAP *half_mask; 
	
} focus;


focus* focus_new(GciCamera *camera);

void focus_set_camera (focus*, GciCamera *camera);

void focus_show_all_panels(focus* foc);

void focus_hide_all_panels(focus* foc);

void focus_destroy(focus*);

void focus_set_on(focus* foc);

void focus_set_off(focus* foc);

int focus_is_on(focus* foc); 

void focus_get_image_focus(focus* foc, FIBITMAP *dib, double* fval);

void focus_update_focus_ui(focus* foc, FIBITMAP *dib, unsigned char average_number_of_frames);

int  CVICALLBACK cbFocusCustom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbFocusMax(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbFocusOnOff(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif  
