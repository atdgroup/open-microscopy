#ifndef __MICROSCOPE_COMMON__
#define __MICROSCOPE_COMMON__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#include "Microscope.h"      

#include "gci_ui_module.h"  
#include "icsviewer_window.h"
#include "icsviewer_signals.h" 

#ifdef MICROSCOPE_PYTHON_AUTOMATION
#include "pyconfig.h"
#include "Python.h"
#include "gci_python_wrappers.h" 
#include "microscope_python_wrappers.h"
#endif

// Experiments
#include "cell_finding.h"
#include "AutoFocus.h" 
#include "RegionOfInterest.h"    
#include "timelapse.h"
#include "RegionScan.h" 
//#include "WellPlateDefiner.h"
#include "realtime_overview.h"

#include "camera\gci_camera.h" 
#include "focus.h"    
#include "background_correction.h"
#include "optical_calibration.h"
#include "AutoFocus.h" 
#include "ZDrive.h"

#include "utility.h"

#ifdef MICROSCOPE_HUYGENS
#include "HuygensMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_ABBE
#include "AbbeMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_FABER
#include "FaberMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_GALILEO
#include "GalileoMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_HOOKE
#include "HookeMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_HUYGENS
#include "HuygensMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_LISTER
#include "ListerMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_COMETS
#include "CometsMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_JANSSEN
#include "JanssenMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_SURREY_ENDSTATION
#include "SurreyEndstationMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_ZERNIKE
#include "ZernikeMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_KOHLER
#include "KohlerMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_KOSSEL
#include "KosselMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_LINACENDSTATION
#include "LinacEndstationMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_FOCUS
#include "FocusMicroscopeUI.h"
#endif

#ifdef MICROSCOPE_DUMMY
#include "OpenMicroscopeUI.h"
#endif

void CVICALLBACK OnMicroscopeSaveSettingsAsDefaultForModeClicked (int menuBar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeSaveSettingsAsClicked (int menuBar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeResetToDefaultSettingsClicked (int menuBar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeMenuItemReInitialiseDevicesClicked (int menuBar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeLoadSettingsFromFileClicked (int menuBar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK OnMicroscopeCameraBinningChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeCameraExposureChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeCameraGainChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeGotoXYZDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeJoystickEnable(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeLEDIntensity(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeLEDState(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeLive(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK OnMicroscopeMenuItemClicked(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeMenuItemLogClicked(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeMenuSaveLoadSettingsClicked(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK OnMicroscopeSetCube(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeSetObjective(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeSetDatumXYZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeSetIlluminationMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeSetOpticalPath(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMicroscopeSnap(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK OnMicroscopeMenuHelpAbout(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeMenuHelpHelp(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeRealTimeOverviewClicked (int menuBar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnMicroscopeMenuItemAutoSnapClicked (int menuBar, int menuItem, void *callbackData, int panel);

void microscope_disable_failed_devices(Microscope *microscope);
void microscope_add_common_module_menu_entries (Microscope* microscope);
int microscope_disable_all_common_module_panels(Microscope* microscope, int disable);

#ifdef __cplusplus
}	// extern "C" 
#endif

#endif

 
