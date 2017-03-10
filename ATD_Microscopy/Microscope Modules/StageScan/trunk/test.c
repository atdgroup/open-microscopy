#include "gci_dummy_camera.h"
#include "microscopeFacade.h"
#include "StageScan.h"
#include <utility.h>

static GciCamera *camera;

stage_scan *ss=NULL;


static int Load ()
{
	if( (camera = gci_dummy_camera_new()) == NULL)
		return -1;

	if( (gci_camera_init(camera)) == CAMERA_ERROR)
		return -1;

	if (GCI_MicroscopeInit() == MICROSCOPE_ERROR)
		return -1;

	ss = stage_scan_new(camera); 
	stage_scan_display_ui(ss);
	GCI_MicroscopeStageDisplayMainUI();
	//GCI_MicroscopeDisplayPanel();

	return 0;
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)  	
{
	char path[GCI_MAX_PATHNAME_LEN];

	if (InitCVIRTE (hInstance, 0, 0) == 0)
		return -1;	// out of memory        
	
	if (Load ()) return 0;
	
	RunUserInterface(); 
	
	GCI_MicroscopeStopAllTimers();

	gci_camera_destroy(camera);

	GCI_MicroscopeClose();

	return 0;
}
