/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2008. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  TCAM_PNL                        1
#define  TCAM_PNL_CAPTURE                2       /* callback function: TwainCameraOnCapturePressed */
#define  TCAM_PNL_CLOSE                  3       /* callback function: TwainCameraOnClosePressed */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK TwainCameraOnCapturePressed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TwainCameraOnClosePressed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
