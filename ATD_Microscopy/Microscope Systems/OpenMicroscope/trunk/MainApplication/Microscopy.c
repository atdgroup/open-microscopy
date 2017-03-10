#include "microscopy.h"
#include "Microscopy_ui.h"

#include "OpenMicroscope.h"  
#include "FreeImageAlgorithms_IO.h"

// Experiments
#ifdef BUILD_MODULE_SPC 
#include "Spc.h"  
#endif

#include "Timelapse.h"
#include "RegionScan.h"  
#include "string_utils.h"
#include "gci_utils.h"
#include "ExceptionHandler.h"

#include <utility.h>
#include <cvirte.h>		
#include <userint.h>
#include <toolbox.h>

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "asynctmr.h"

static int CVICALLBACK OnMicroscopeControl (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			
			Microscopy* app = (Microscopy*) callbackData;        
	
			microscope_display_main_ui(app->microscope);   
			
		}break;
	}
	
	return 0;
}

/*
static int CVICALLBACK OnMicroscopySpcFlim (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			Microscopy* app = (Microscopy*) callbackData;  
			
			Spc* spc = microscope_get_spc(app->microscope);  
		
			spc_display_main_ui(spc);  
				
			break;
		}
	}
	
	return 0; 
}
*/

static int CVICALLBACK OnMicroscopyStageScan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			Microscopy* app = (Microscopy*) callbackData;  
			stage_scan* ss = microscope_get_stage_scan(app->microscope);
			
//			stage_scan_display_ui(ss);  
			stage_scan_display_overview_ui(ss);  

		}break;
	}
	
	return 0; 
}

#ifdef MICROSCOPE_PYTHON_AUTOMATION

static int CVICALLBACK OnMicroscopyTimeLapse (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			
			Microscopy* app = (Microscopy*) callbackData;  
			timelapse* tl = microscope_get_timelapse(app->microscope);  
		
			timelapse_display(tl);  
			
		}break;
	}
	
	return 0; 
}

#endif // MICROSCOPE_PYTHON_AUTOMATION

static int CVICALLBACK OnMicroscopyRegionScan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			
			Microscopy* app = (Microscopy*) callbackData;  
			region_scan* rs = microscope_get_region_scan(app->microscope);  
		
			region_scan_display(rs);  
			
		}break;
	}
	
	return 0; 
}


static int CVICALLBACK OnMicroscopyAutomation (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			
			Microscopy* app = (Microscopy*) callbackData;  

			app->automation_editor = Get_AutomationEditor(app->hInstance);   
			
			AutomationEditor_Display(app->automation_editor);
			
		}break;
	}
	
	return 0; 
}


static int CVICALLBACK OnMicroscopyClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			Microscopy* app = (Microscopy*) callbackData;   
	
			if (!GCI_ConfirmPopup ("Exit", IDI_WARNING, "Are you sure?"))
				return 0;
			
			microscopy_app_close(app);     
			
			QuitUserInterface(0); 

		}break;
	}
	
	return 0; 
}

void OnMicroscopeDimChanged (Microscope* microscope, int state, void *data)
{
	Microscopy* app = (Microscopy*) data;

	if(state == 1)
		ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(app), 1, MICROSCOPY_CLOSE);
	else
		ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(app), 0);
}

Microscopy* microscopy_app_new(HINSTANCE hInstance)
{
	int panel_id;
	Microscopy* app = (Microscopy*) malloc(sizeof(Microscopy));
	
	FIBITMAP *input = NULL;
	//LensDistortion* ld = NULL;

	ui_module_constructor(UIMODULE_CAST(app), "Microscope");
	ui_module_set_description(UIMODULE_CAST(app), "Microscopy");
	
	panel_id = ui_module_add_panel(UIMODULE_CAST(app), "Microscopy_ui.uir", MICROSCOPY, 1);  
	
	GCI_SetPanelsMonitorAsDefaultForDialogs(panel_id);

	if ( InstallCtrlCallback (panel_id, MICROSCOPY_MICROSCOPE, OnMicroscopeControl, app) < 0)
		return NULL;

	//if ( InstallCtrlCallback (panel_id, MICROSCOPY_SPC, OnMicroscopySpcFlim, app) < 0)
	//	return NULL;     

	#ifdef MICROSCOPE_PYTHON_AUTOMATION

	if ( InstallCtrlCallback (panel_id, MICROSCOPY_TIMELAPSE, OnMicroscopyTimeLapse, app) < 0)
		return NULL;     
	
	#endif // MICROSCOPE_PYTHON_AUTOMATION

	if ( InstallCtrlCallback (panel_id, MICROSCOPY_REGION_SCAN, OnMicroscopyRegionScan, app) < 0)
		return NULL;    
	
	if ( InstallCtrlCallback (panel_id, MICROSCOPY_STAGE_SCAN, OnMicroscopyStageScan, app) < 0)
		return NULL;    
	
	if ( InstallCtrlCallback (panel_id, MICROSCOPY_CLOSE, OnMicroscopyClose, app) < 0)
		return NULL;     
	
	if ( InstallCtrlCallback (panel_id, MICROSCOPY_AUTOMATION, OnMicroscopyAutomation, app) < 0)
		return NULL;

	app->hInstance = hInstance;
	
	SetPanelAttribute (panel_id, ATTR_DIMMED, 1);
	
	app->microscope = microscope_new(hInstance);

	if(app->microscope == NULL) {
	
		GCI_MessagePopup("Fatal Error", "Some part of the hardware communication has failed.");
		return NULL;
	}
	
	microscope_initialise(app->microscope);      
	microscope_dimmed_handler_connect(app->microscope, OnMicroscopeDimChanged, app);

	// We always want the microscope panel to be visiable at startup
	ui_module_display_all_panels(UIMODULE_CAST(app));
	ui_module_display_main_panel_without_activation(UIMODULE_CAST(app->microscope));
		
	app->automation_editor = NULL;
	
	SetPanelAttribute (panel_id, ATTR_DIMMED, 0);
	
	//ld = lens_distortion_new(app->microscope, "");

	//input = FIA_LoadFIBFromFile("C:\\Devel\\ATD\\ATD_Microscopy\\Microscope\ Modules\\LensDistortionCorrection\\grid-pin-cushion.bmp");
	//lens_distortion_fit_polynomial(ld, input);

	return app;	
	
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	Microscopy* app;

	#ifdef VERBOSE_DEBUG

	HANDLE handle_out, handle_in;
	FILE* hf_out, *hf_in;
	int hCrt;

	AllocConsole();
	
    handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
    hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;

	#endif

	if (InitCVIRTE (hInstance, 0, 0) == 0)
		return -1;	// out of memory 

//	#ifdef _MSC_VER
//	__try
//	{
//	#endif

		app = microscopy_app_new(hInstance);  
	
		if (app==NULL) return 0;
	
		RunUserInterface ();

		free(app);

//	#ifdef _MSC_VER
//	}
//	__except(CreateMiniDump(GetExceptionInformation(), GetExceptionCode()))
//	{
		// Do nothing here - CreateMiniDump() has already done
		// everything that is needed. Actually this code won't even
		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
		// the __except clause.
//	}
	//__finally {

    //     return QuitUserInterface(-1);
    //} 

//	#endif

	return 0;
}


#ifdef _DEBUG
int main ()
{
	Microscopy* app;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	if (InitCVIRTE (hInstance, 0, 0) == 0)
		return -1;	// out of memory 

	#ifdef FORTIFY   
	Fortify_EnterScope(); 
	#endif  
	
	app = microscopy_app_new(hInstance);  
	
	if (app==NULL) return 0;
	
	RunUserInterface ();

	free(app);
	
	#ifdef FORTIFY
	Fortify_LeaveScope();
	#endif
	
	#ifndef NO_PROFILE
	PROFILE_PRINT();
	#endif

	while(1);  // in debug mode, wait here forever so we can read the terminal

	return 0;
}
#endif

void microscopy_app_close(Microscopy* app)
{
	if(app->automation_editor != NULL) {
		AutomationEditor_Destroy();
		app->automation_editor = NULL;		
	}
	
	if(app->microscope != NULL)
		microscope_destroy(app->microscope);
	
	ui_module_destroy(UIMODULE_CAST(app)); 
}

