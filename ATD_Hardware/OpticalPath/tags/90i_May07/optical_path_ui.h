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

#define  ADD_PATH                        1
#define  ADD_PATH_TURRET_POS             2
#define  ADD_PATH_CANCEL                 3
#define  ADD_PATH_OK                     4

#define  OPTIC_PATH                      2
#define  OPTIC_PATH_CLOSE                2       /* callback function: OnOpticalPathClose */
#define  OPTIC_PATH_SETUP                3       /* callback function: OnOpticalPathSetup */
#define  OPTIC_PATH_TURRET_POS           4       /* callback function: OnOpticalPathChanged */

#define  PATH_CONF                       3
#define  PATH_CONF_QUIT                  2       /* callback function: OnOpticalPathConfigCloseClicked */
#define  PATH_CONF_SAVE                  3       /* callback function: OnOpticalPathFileSave */
#define  PATH_CONF_ADD                   4       /* callback function: OnOpticalPathDetailsAdd */
#define  PATH_CONF_REMOVE                5       /* callback function: OnOpticalPathRemoveClicked */
#define  PATH_CONF_EDIT                  6       /* callback function: OnOpticalPathDetailsEdit */
#define  PATH_CONF_EDIT_ACTIVE           7       /* callback function: OnOpticalPathEditActive */
#define  PATH_CONF_LOAD                  8       /* callback function: OnOpticalPathFileRecall */
#define  PATH_CONF_DOWN_ARROW            9       /* callback function: OnOpticalPathDownArrow */
#define  PATH_CONF_UP_ARROW              10      /* callback function: OnOpticalPathUpArrow */
#define  PATH_CONF_RIGHT_ARROW           11      /* callback function: OnOpticalPathRightArrow */
#define  PATH_CONF_LEFT_ARROW            12      /* callback function: OnOpticalPathLeftArrow */
#define  PATH_CONF_FNAME                 13
#define  PATH_CONF_ALL_TREE              14      /* callback function: OnOpticalPathTreeValueChanged */
#define  PATH_CONF_ACTIVE_TREE           15      /* callback function: OnOpticalPathTreeValueChanged */

#define  PATH_DETS                       4
#define  PATH_DETS_OK_BUTTON             2       /* callback function: OnOpticalPathAddEditOkClicked */
#define  PATH_DETS_NAME                  3
#define  PATH_DETS_ADDOREDIT             4
#define  PATH_DETS_INDEX                 5


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK OnOpticalPathAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathConfigCloseClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathDetailsAdd(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathDetailsEdit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathDownArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathEditActive(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathFileRecall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathFileSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathLeftArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathRemoveClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathRightArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathTreeValueChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathUpArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
