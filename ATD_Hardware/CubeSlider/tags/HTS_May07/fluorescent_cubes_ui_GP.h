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

#define  CUBE_CONF                       1
#define  CUBE_CONF_QUIT                  2       /* callback function: OnConfigCloseClicked */
#define  CUBE_CONF_SAVE                  3       /* callback function: OnFileSave */
#define  CUBE_CONF_ADD                   4       /* callback function: OnDetailsAdd */
#define  CUBE_CONF_REMOVE                5       /* callback function: OnRemoveClicked */
#define  CUBE_CONF_EDIT                  6       /* callback function: OnDetailsEdit */
#define  CUBE_CONF_EDIT_ACTIVE           7       /* callback function: OnEditActive */
#define  CUBE_CONF_LOAD                  8       /* callback function: OnFileRecall */
#define  CUBE_CONF_DOWN_ARROW            9       /* callback function: OnDownArrow */
#define  CUBE_CONF_UP_ARROW              10      /* callback function: OnUpArrow */
#define  CUBE_CONF_RIGHT_ARROW           11      /* callback function: OnRightArrow */
#define  CUBE_CONF_LEFT_ARROW            12      /* callback function: OnLeftArrow */
#define  CUBE_CONF_FNAME                 13
#define  CUBE_CONF_ALL_TREE              14      /* callback function: OnTreeValueChanged */
#define  CUBE_CONF_ACTIVE_TREE           15      /* callback function: OnTreeValueChanged */

#define  CUBE_INFO                       2
#define  CUBE_INFO_CLOSE                 2       /* callback function: OnFluorCubeClose */
#define  CUBE_INFO_SETUP                 3       /* callback function: OnCubeSetup */
#define  CUBE_INFO_TEXTMSG               4
#define  CUBE_INFO_DECORATION_18         5
#define  CUBE_INFO_TEXTMSG_2             6
#define  CUBE_INFO_TEXTMSG_3             7
#define  CUBE_INFO_TEXTMSG_4             8
#define  CUBE_INFO_TEXTMSG_5             9
#define  CUBE_INFO_TEXTMSG_6             10
#define  CUBE_INFO_CUBE_NAME             11
#define  CUBE_INFO_CUBE_EXC              12
#define  CUBE_INFO_CUBE_DICHROIC         13
#define  CUBE_INFO_CUBE_EM_MIN           14
#define  CUBE_INFO_CUBE_EM_MAX           15

#define  EDIT_PANEL                      3
#define  EDIT_PANEL_OK_BUTTON            2       /* callback function: OnAddEditOkClicked */
#define  EDIT_PANEL_NAME                 3
#define  EDIT_PANEL_TEXTMSG              4
#define  EDIT_PANEL_TEXTMSG_4            5
#define  EDIT_PANEL_TEXTMSG_5            6
#define  EDIT_PANEL_TEXTMSG_6            7
#define  EDIT_PANEL_TEXTMSG_8            8
#define  EDIT_PANEL_ADDOREDIT            9
#define  EDIT_PANEL_INDEX                10
#define  EDIT_PANEL_EMMAX                11
#define  EDIT_PANEL_EMMIN                12
#define  EDIT_PANEL_DICHROICNM           13
#define  EDIT_PANEL_EXCNM                14


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK OnAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnConfigCloseClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDetailsAdd(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDetailsEdit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDownArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnEditActive(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFileRecall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFileSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFluorCubeClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLeftArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnRemoveClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnRightArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTreeValueChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnUpArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
