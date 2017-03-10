/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2005. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  CELL_PANEL                      1
#define  CELL_PANEL_POSITION_TEXT        2
#define  CELL_PANEL_TEXTMSG              3
#define  CELL_PANEL_TYPE_RING            4
#define  CELL_PANEL_OK_BUTTON            5       /* callback function: onCellInfoOk */
#define  CELL_PANEL_CLOSE_BUTTON         6       /* callback function: onCellInfoClose */
#define  CELL_PANEL_TEXTMSG_2            7


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK onCellInfoClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onCellInfoOk(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
