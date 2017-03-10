/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2006. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  LAMP_PNL                        1
#define  LAMP_PNL_CLOSE                  2       /* callback function: OnLampClose */
#define  LAMP_PNL_LED                    3
#define  LAMP_PNL_ONOFF                  4       /* callback function: OnLampOnOffToggle */
#define  LAMP_PNL_INTENSITY              5       /* callback function: OnLampIntensity */
#define  LAMP_PNL_DECORATION_11          6


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK OnLampClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLampIntensity(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLampOnOffToggle(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
