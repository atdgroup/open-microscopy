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

#define  TEST_PNL                        1
#define  TEST_PNL_STRING                 2
#define  TEST_PNL_NUMERIC                3       /* callback function: OnNumericChanged */
#define  TEST_PNL_COMMANDBUTTON          4       /* callback function: OnOkClicked */
#define  TEST_PNL_RING_2                 5       /* callback function: OnRingChanged */
#define  TEST_PNL_RING                   6       /* callback function: OnRingChanged */
#define  TEST_PNL_LISTBOX                7       /* callback function: OnListChanged */
#define  TEST_PNL_TAB                    8
#define  TEST_PNL_TOGGLEBUTTON           9       /* callback function: OnToggleButtonChange */
#define  TEST_PNL_NUMERICSLIDE           10

#define  TEST_PNL2                       2
#define  TEST_PNL2_NUMERIC               2

#define  TABPANEL_NUMERIC                2       /* callback function: OnNumericChanged */

#define  TABPANEL_2_NUMERIC              2       /* callback function: OnNumericChanged */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                         1
#define  MENUBAR_FILE                    2
#define  MENUBAR_FILE_ITEM1              3       /* callback function: OnFileOpen */
#define  MENUBAR_MENU2                   4
#define  MENUBAR_MENU2_ITEM3             5       /* callback function: OnViewClicked */


     /* Callback Prototypes: */ 

void CVICALLBACK OnFileOpen(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK OnListChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnNumericChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnRingChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnToggleButtonChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK OnViewClicked(int menubar, int menuItem, void *callbackData, int panel);


#ifdef __cplusplus
    }
#endif
