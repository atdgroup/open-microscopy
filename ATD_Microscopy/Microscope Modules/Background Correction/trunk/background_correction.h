#ifndef _REF_IMAGES_
#define _REF_IMAGES_

#include "gci_ui_module.h"
#include "FreeImageAlgorithms.h" 

typedef struct _Microscope Microscope;  

typedef struct _ref_images ref_images;

ref_images* ref_images_new(Microscope *microscope);

void ref_images_destroy(ref_images*);

int ref_images_get_black_image(ref_images* ri);

int ref_images_get_blank_image(ref_images* ri);

void ref_images_recall_images(ref_images* ri);

int ref_images_can_process(ref_images* ri);

int ref_images_should_process(ref_images* ri);

int ref_images_in_place_process(ref_images* ri, FIBITMAP** image);

int ref_images_in_place_process_forced(ref_images* ri, FIBITMAP** image);

void ref_images_enable(ref_images* ri);

void ref_images_disable(ref_images* ri);

FIBITMAP* ref_images_process(ref_images* ri, FIBITMAP* image);

int  CVICALLBACK OnBackgroundCorrectionEnabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnBackgroundCorrectionClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnGetBlackImage(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnGetWhiteImage(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnProcess(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#endif
