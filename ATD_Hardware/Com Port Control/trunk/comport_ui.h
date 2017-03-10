/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2007. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  COM_PANEL                       1
#define  COM_PANEL_DECORATION            2
#define  COM_PANEL_OK_BUTTON             3       /* callback function: onOkButton */
#define  COM_PANEL_TEXTMSG               4
#define  COM_PANEL_TEXTMSG_2             5
#define  COM_PANEL_TEXTMSG_3             6
#define  COM_PANEL_TEXTMSG_4             7
#define  COM_PANEL_APPLY_BUTTON          8       /* callback function: onApplyButton */
#define  COM_PANEL_STOP_BITS_RING        9
#define  COM_PANEL_PARITY_RING           10
#define  COM_PANEL_INPUT_QUEUE_NUMERIC   11
#define  COM_PANEL_TEXTMSG_6             12
#define  COM_PANEL_OUTPUT_QUEUE_NUMERIC  13
#define  COM_PANEL_TEXTMSG_5             14
#define  COM_PANEL_TEXTMSG_7             15
#define  COM_PANEL_TEXTMSG_8             16
#define  COM_PANEL_DEVICE_TEXT           17
#define  COM_PANEL_DEVICE_LISTBOX        18      /* callback function: onListSelection */
#define  COM_PANEL_PORT_RING             19
#define  COM_PANEL_BAUD_RING             20
#define  COM_PANEL_DATA_BITS_RING        21


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK onApplyButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onListSelection(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onOkButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
