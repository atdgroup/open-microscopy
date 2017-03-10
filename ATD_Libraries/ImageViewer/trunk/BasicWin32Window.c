#include "BasicWin32Window.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_HBitmap.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_LinearScale.h"

#include "ImageViewerUtils.h"
#include "ImageViewer.h"

#include <tchar.h>
#include <commctrl.h>

#include <assert.h>
#include <stdio.h>

#define APP_TITLE   _T("BasicWin32Window")

TCHAR		szAppName[] = APP_TITLE;

static int window_count = 0;
static HWND all_windows[100]; 
static HFONT hfont = 0;

static char* IMAGE_TYPES[] =
{
	"FIT_UNKNOWN",
	"FIT_BITMAP",
	"FIT_UINT16",
	"FIT_INT16",
	"FIT_UINT32",
	"FIT_INT32",
	"FIT_FLOAT",
	"FIT_DOUBLE",
	"FIT_COMPLEX",
	"FIT_RGB16",
	"FIT_RGBA16",
	"FIT_RGBF",
	"FIT_RGBAF"
};

typedef struct _BasicWin32WindowCtrl
{
	HWND			hwndMain;
	HWND			hwndCanvas;
	HWND			staticText;
	HWND			dynamicText;

	int				bpp;
	int				width;
	int				height;
	int				pitch;
	FREE_IMAGE_TYPE	type;
	FIBITMAP*		original_dib;

	LONG_PTR		canvas_original_proc_fun_ptr;

} BasicWin32WindowCtrl;

static void ErrorExit(LPTSTR lpszFunction) 
{ 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;

    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    wsprintf((LPTSTR)lpDisplayBuf, 
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

/* Get a state structure from the specified window */
static BasicWin32WindowCtrl*
GetBasicWin32WindowCtrl(HWND hwnd)
{
	#ifdef _WIN64
    return (BasicWin32WindowCtrl*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	#else
	return (BasicWin32WindowCtrl*) LongToPtr( GetWindowLongPtr(hwnd, GWLP_USERDATA));
	#endif
}


/* Set a state structure for the specified window */
static void
SetBasicWin32WindowCtrl(HWND hwnd, BasicWin32WindowCtrl *ctrl)
{
	#ifdef _WIN64
   	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) ctrl);
	#else
	SetWindowLongPtr(hwnd, GWLP_USERDATA, PtrToLong(ctrl));
	#endif

	return;
}

void SetWindowFileName(HWND hwnd, const TCHAR *szFileName)
{
	TCHAR ach[MAX_PATH + sizeof(szAppName) + 4];

	wsprintf(ach, _T("%s"), szFileName);
	SetWindowText(hwnd, ach);
}

static void SetPixelInfoText(BasicWin32WindowCtrl *ctrl, int x, int y)
{
	double val = 0.0;
	char text[200];
	POINT point, image_point;

	point.x = x;
	point.y = y;

	ImageViewer_TranslateWindowPointToImagePoint(ctrl->hwndCanvas, point, &image_point);        

	if(FIA_IsGreyScale(ctrl->original_dib)) {
		FIA_GetPixelValueFromTopLeft(ctrl->original_dib, image_point.x, image_point.y, &val);
		sprintf(text, "(%d, %d) = %.3f", image_point.x, image_point.y, val);
	}
	else {
		RGBQUAD value;

		FIA_GetPixelColourFromTopLeft(ctrl->original_dib, image_point.x, image_point.y, &value);
		sprintf(text, "(X:%d, Y:%d) = (%d,%d,%d)", image_point.x, image_point.y, value.rgbRed, value.rgbGreen, value.rgbBlue);		
	}
						
	SetWindowText(ctrl->dynamicText, text);
}

static void SetControlSizes(BasicWin32WindowCtrl *ctrl, int width, int height)
{
	int canvasBottom =  height - 100;
	char static_text[200];

	SetWindowPos(ctrl->hwndCanvas, NULL, 0, 0,
		width, canvasBottom, SWP_NOZORDER | SWP_NOACTIVATE);

	SetWindowPos(ctrl->staticText, NULL, 1, canvasBottom,
		width - 1, 70, SWP_NOZORDER | SWP_NOACTIVATE);

	SetWindowPos(ctrl->dynamicText, NULL, 1, canvasBottom + 70,
		width - 1, 30, SWP_NOZORDER | SWP_NOACTIVATE);

	sprintf(static_text, "Width: %d\nHeight: %d\nPitch: %d\nType: %s\nBPP: %d",	
							ctrl->width,
							ctrl->height,
							ctrl->pitch,
							IMAGE_TYPES[ctrl->type],
							ctrl->bpp);

	SetWindowText(ctrl->staticText, static_text);
}

LRESULT CALLBACK CanvasWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	BasicWin32WindowCtrl *ctrl = GetBasicWin32WindowCtrl(hwnd);
	
	switch(message) {
		
		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam); 
			int y = HIWORD(lParam);

			SetPixelInfoText(ctrl, x, y);

			break;
		}
		      
        default:
		
        	break;
   	}
   	
	return CallWindowProc ((WNDPROC) ctrl->canvas_original_proc_fun_ptr, hwnd, message, wParam, lParam);
}

//
//	Main window procedure
//
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// retrieve the custom structure POINTER for THIS window
    BasicWin32WindowCtrl *ctrl = GetBasicWin32WindowCtrl(hwnd);

	switch(msg)
	{
		case WM_CLOSE:
			FreeImage_Unload (ctrl->original_dib);
			SetWindowLongPtr (ctrl->hwndCanvas, GWL_WNDPROC, ctrl->canvas_original_proc_fun_ptr);
			free(ctrl);
			ctrl = NULL;
			DestroyWindow(hwnd);
			window_count--;

			//if(window_count <= 0)
			//	PostQuitMessage(0);

			return 0;

		case WM_SIZE:
		{
			int width  = (short)LOWORD(lParam);
			int height = (short)HIWORD(lParam);

			SetControlSizes(ctrl, width, height);
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


//
//	Register main window class
//
static void InitCreateWin32Wnd()
{
	WNDCLASSEX wcx;
	HANDLE hInst = GetModuleHandle(0);

	// Window class for the main application parent window
	wcx.cbSize			= sizeof(wcx);
	wcx.style			= 0;
	wcx.lpfnWndProc		= WndProc;
	wcx.cbClsExtra		= 0;
	wcx.cbWndExtra		= sizeof( BasicWin32WindowCtrl * );
	wcx.hInstance		= (HINSTANCE) hInst;
	wcx.hCursor			= LoadCursor (NULL, IDC_ARROW);
	wcx.hbrBackground	= (HBRUSH)0;
	wcx.lpszMenuName	= 0;
	wcx.lpszClassName	= szAppName;
	wcx.hIcon			= 0;
	wcx.hIconSm			= 0;

	RegisterClassEx(&wcx);
}

//
//	Create a top-level window
//
static HWND CreateWin32Wnd(const char *title, int left, int top, int width, int height)
{
	HWND hwnd = NULL;

	if(width <= 0 || height <= 0)
		return NULL;

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
					szAppName,				// window class name
					"05 - Hello",					// window caption
					WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
					left, top, width, height + 100,
					NULL,					// parent window handle
					NULL,					// use window class menu
					GetModuleHandle(0),		// program instance handle
					NULL);					// creation parameters

	if(hwnd == NULL)
		ErrorExit("CreateWindowEx"); 
	
	return hwnd;
}


HWND IV_DLL_CALLCONV
BasicWin32Window(const char *title, int left, int top, int width, int height, FIBITMAP *dib)
{
	HWND		hwnd;
	HDC			fib_hdc;
	HBITMAP 	fib_bitmap;
    FIBITMAP    *standard_dib;
	double		min, max;

	BasicWin32WindowCtrl *ctrl;
	HANDLE hInst = GetModuleHandle(0);

	// initialize window classes
	InitCreateWin32Wnd();
	ImageViewer_Init();

	if(hfont == 0) {
		long lfHeight;
    
		HDC hdc = GetDC(NULL);
		lfHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(NULL, hdc);

		hfont = CreateFont(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
	}
	
	// Allocate a new ImageViewerCtrl structure for this window. 
    ctrl = (BasicWin32WindowCtrl*) malloc( sizeof(BasicWin32WindowCtrl) );     
		
	// create the main window.
	hwnd = CreateWin32Wnd(title, left, top, width, height);
	
	ctrl->staticText = CreateWindow("STATIC",
											"",
											WS_VISIBLE | WS_CHILD | SS_LEFT,
											10,
											10,
											75,
											35,
											hwnd,
											NULL,
											(HINSTANCE) hInst,
											NULL);

	SendMessage(ctrl->staticText, WM_SETFONT, (WPARAM)hfont, TRUE);
			
	ctrl->dynamicText = CreateWindow("STATIC",
									"",
									WS_VISIBLE | WS_CHILD | SS_LEFT,
									10,
									10,
									75,
									35,
									hwnd,
									NULL,
									(HINSTANCE) hInst,
									NULL);

	SendMessage(ctrl->dynamicText, WM_SETFONT, (WPARAM)hfont, TRUE);

	ctrl->hwndMain = hwnd;
	ctrl->hwndCanvas = ImageViewer_Create(hwnd, 0, 0, 300, 300);

	// Attach custom structure to this window. 
	SetBasicWin32WindowCtrl(hwnd, ctrl);

	// Store the window structure with the window for use in WndProc 
	SetBasicWin32WindowCtrl(ctrl->hwndCanvas, ctrl);	

	// Store the original windows procedure function pointer 
	ctrl->canvas_original_proc_fun_ptr = GetWindowLongPtr (ctrl->hwndCanvas, GWL_WNDPROC);
	
	// Set the new Wnd Proc to be called 
	SetWindowLongPtr (ctrl->hwndCanvas, GWL_WNDPROC, (LONG_PTR) CanvasWndProc);

	if(dib == NULL)
		printf("Not a valid image.");

	fib_hdc = GetDC(ctrl->hwndCanvas);
	assert(fib_hdc != NULL);

	ctrl->original_dib = FreeImage_Clone(dib);

	ctrl->bpp = FreeImage_GetBPP(ctrl->original_dib);
	ctrl->width = FreeImage_GetWidth(ctrl->original_dib);
	ctrl->height = FreeImage_GetHeight(ctrl->original_dib);
	ctrl->pitch = FreeImage_GetPitch(ctrl->original_dib);
	ctrl->type = FreeImage_GetImageType(ctrl->original_dib);

	if(ctrl->bpp == 8 && ctrl->type == FIT_BITMAP) {
		standard_dib = FreeImage_Clone(dib);
	}
	else {

		if(FIA_IsGreyScale(dib))
			standard_dib = FIA_LinearScaleToStandardType(dib, 0.0, 0.0, &min, &max);
		else
			standard_dib = FreeImage_ConvertToStandardType(dib, 1);   	
	}

	fib_bitmap = FIA_GetDibSection(standard_dib, fib_hdc, 0, 0,
								FreeImage_GetWidth(standard_dib),
                                FreeImage_GetHeight(standard_dib));

	ImageViewer_SetImage(ctrl->hwndCanvas, fib_bitmap);

	DeleteObject(fib_bitmap);
	DeleteDC(fib_hdc);

	SetWindowFileName(hwnd, _T(title));

	ShowWindow(hwnd, SW_SHOW);	

    FreeImage_Unload(standard_dib);

	all_windows[window_count++] = hwnd; 

	return hwnd;
}

void IV_DLL_CALLCONV
BasicWin32Window_SetPalette(HWND hwnd, RGBQUAD *palette)
{
	// retrieve the custom structure POINTER for THIS window
    BasicWin32WindowCtrl *ctrl = GetBasicWin32WindowCtrl(hwnd);

	ImageViewer_SetPalette(ctrl->hwndCanvas, palette, 256);

	ImageViewer_Redraw(ctrl->hwndCanvas);
}

void IV_DLL_CALLCONV
RemoveAllBasicWin32Windows(void)
{
	int i = 0;
	int total = window_count;
	BasicWin32WindowCtrl *ctrl;
	HWND hwnd;

	for(i=0; i < total; i++) {

		hwnd = all_windows[i];
		ctrl = GetBasicWin32WindowCtrl(hwnd);
		FreeImage_Unload (ctrl->original_dib);
		SetWindowLongPtr (ctrl->hwndCanvas, GWL_WNDPROC, ctrl->canvas_original_proc_fun_ptr);
		free(ctrl);
		ctrl = NULL;
		DestroyWindow(hwnd);
		window_count--;
	}
}
