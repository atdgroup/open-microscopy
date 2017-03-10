#include "icsviewer_window.h"
#include "icsviewer_signals.h"

#include "string_utils.h"
#include "gci_utils.h"

#include "FreeImage.h" 
#include "FreeImageIcs_IO.h"
#include "FreeImageAlgorithms_IO.h" 
#include "FreeImageAlgorithms_Drawing.h"
#include "FreeImageAlgorithms_Utilities.h"

#include "xgetopt.h"

//#include "wdm_capture.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include <utility.h>
	
static IcsViewerWindow *image_window = NULL;
static int enable_one_instance = 0;
static HANDLE gMutex;
static char filepath[500];

#ifdef LIVE_TEST

static int info_panel_id;
static int fps_label_id;
static int panel_id;
static int timer_id;

FIBITMAP *dib;
RECT rect;
POINT p1, p2; 
ICS *ics;

static char *filename = "C:\\Documents and Settings\\Pierce\\My Documents\\Test Images\\12bitoverload.ics"; 


int CVICALLBACK OnTimerTick (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_TIMER_TICK:

			if(image_window == NULL)
				return 0;

				dib = FreeImageIcs_LoadFIBFromIcsFile (ics);  
	
				GCI_ImagingWindow_LoadImage(image_window, dib);       
			
			break;
		}
		
	return 0;
}


#endif


static void on_last_window_destroy_handler (int id, void *data)
{
	QuitUserInterface(0);
}


static PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = strlen(CmdLine);
	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

	argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
	i + (len+2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while( a = CmdLine[i] ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }

		i++;
	}

	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return argv;
}

static int process_commandline(IcsViewerWindow *window, int argc, PCHAR *argv)
{
	int c;

	while ((c = getopt(argc, argv, "i:s:t:m:p:")) != EOF)
	{
		switch (c)
		{
			case 'i':
				// Load an image from given path
				strncpy(filepath, optarg, 500);
	 			GCI_ImagingWindow_LoadImageFile(window, filepath); 
                break;

			case 's':
				// Specify a default Stream Device ID, i.e. on Linac we use "1"
				GCI_ImagingWindow_UseStreamDeviceWithId(window, atoi(optarg));
				break;

			case 't':
				// Specify a window title, Linac uses this to find this instance and close it etc.
				GCI_ImagingWindow_SetWindowTitle(window, optarg);
				break;

			case 'm':
				// Force the Stream device to return an 8bit image
				// Hack: I (GP) want the mode to always be forced to 8bit, not just for the stream device
				// Stuart needs this for now on Linac
				if(atoi(optarg) == 8)
					GCI_ImagingWindow_StreamDeviceForce8bit(window, 1);

			break;

			case 'p':
			{	
				// Specify a palette to start with
				int palette_id = atoi(optarg);

				if(palette_id >= GREYSCALE_PALETTE && palette_id <= FALSE_COLOUR_PALETTE)
					GCI_ImagingWindow_SetDefaultPalette(window, palette_id);

				break;
			}

			//case '1':
			//	enable_one_instance = 1;
			//	break;

            default:
				GCI_MessagePopup("Warning", "WARNING: no handler for option %c\n", c);
                return 0;
        }
	}

    //
    // check for non-option args here
    //
    return 1;
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	int argc, last_err;
	PCHAR *lpszArgv;
	//char buffer[MAX_FILENAME_LEN] = "";
	//char fileName[MAX_FILENAME_LEN] = "";
	//char workingDir[MAX_PATH] = "";

	#ifdef VERBOSE_DEBUG 

	HANDLE handle_out, handle_in;
	FILE* hf_out, *hf_in;
	int hCrt;
	HWND hwnd;
	UINT myTimer;

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
		return -1;

	lpszArgv = CommandLineToArgvA(GetCommandLineA(), &argc);

	/*
	if(enable_one_instance) {
	
		if((RESTORE_MSG = RegisterWindowMessage(RESTORE_MSG_TEXT)) == 0) {
			MessagePopup("Error","Can not create global message");  
			return -1;
		}
		
		gMutex = CreateMutex(NULL, FALSE, TEXT("Global\\{9F19711A-C403-4f2b-A7D8-402825E80505}")); 
		
		last_err = GetLastError();

		// Nasty hack to pass info to other process ! Not sure how to do it properly.
		if(argc > 1)
			RegWriteString(REGKEY_HKCU, REGISTRY_SUBKEY, "LastArgvArg", filepath);

		if(last_err == ERROR_ALREADY_EXISTS)
		{
			PostMessage(HWND_BROADCAST, RESTORE_MSG, 0, 0);  
			
			return 0;
		}
	
	}
	*/

	if (!SUCCEEDED(OleInitialize(NULL)))
	{
		return -1;
	}

	#ifndef LIVE_TEST

	if ( (image_window = GCI_ImagingWindow_Create("IcsViewer", "IcsViewer")) == NULL ) {
			
		MessagePopup("Error", "Can not create window");
		
		return -1;
	}

	GCI_ImagingWindow_Initialise(image_window);     
	
	process_commandline(image_window, argc, lpszArgv);

	/*
	MessageBoxA(NULL, lpszArgv[1], NULL, MB_OK);

	GetCurrentDirectory(sizeof(workingDir) - 1, workingDir);
	
	SplitPath(lpszArgv[1], NULL, NULL, fileName);
	sprintf(buffer, "%s\\%s", workingDir, fileName);

	MessageBoxA(NULL, buffer, NULL, MB_OK);
	*/

	GCI_ImagingWindow_Show(image_window);
	
	#else

	if ( (image_window = GCI_ImagingWindow_Create("IcsViewer")) == NULL ) {
			
		MessagePopup("Error", "Can not create window");
		
		return -1;
	}
	
	GCI_ImagingWindow_Initialise(image_window); 
	
	if(FreeImageIcs_IcsOpen (&ics, filename, "r") != IcsErr_Ok)
		return -1;

	info_panel_id = NewPanel(0, "fps", 10, 10, 200,100);
	
	fps_label_id = NewCtrl(info_panel_id, CTRL_TEXT_MSG, "", 10, 10);
	
	panel_id = GCI_ImagingWindow_GetPanelID(image_window);
	
	timer_id = NewCtrl(panel_id, CTRL_TIMER, "", 0,0);
	
	GCI_ImagingWindow_LoadImageFile(image_window, filename);

	GCI_ImagingWindow_Show(image_window);   
	
	InstallCtrlCallback(panel_id, timer_id, OnTimerTick, image_window);
	
	SetCtrlAttribute(panel_id, timer_id, ATTR_INTERVAL, 0.001);
	SetCtrlAttribute(panel_id, timer_id, ATTR_ENABLED, 1);

	DisplayPanel(info_panel_id);
	
	#endif
	
	GCI_ImagingWindow_SetOnLastWindowDestroyEventHandler(on_last_window_destroy_handler, NULL);

	RunUserInterface();
	
	OleUninitialize();
	
	GlobalFree(lpszArgv);

	/*
	if(enable_one_instance) {
	
		CloseHandle(gMutex); 
		gMutex = NULL; 
	
	}
	*/

	PROFILE_PRINT();
	
	return 0;
}