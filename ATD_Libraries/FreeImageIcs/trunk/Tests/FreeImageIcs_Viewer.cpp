#include "FreeImageAlgorithms.h"
#include "FreeImageIcs_Viewer.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_HBitmap.h"
#include "FreeImageAlgorithms_Palettes.h"
#include "FreeImageAlgorithms_LinearScale.h"
#include "FreeImageAlgorithms_Statistics.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Utils.h"

#include <iostream>
#include <assert.h>

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>

#define APP_TITLE   _T("CanvasTest")

HWND		hwndMain;
HDC			hdc;
HDC			hbitmap_hdc;
HBITMAP		hbitmap;

TCHAR szFileName[MAX_PATH];
TCHAR szFileTitle[MAX_PATH];
TCHAR		szAppName[] = APP_TITLE;

static void PaintRect(HWND hwnd, HDC hdc, COLORREF colour)
{
	RECT rect;
    COLORREF oldcr = SetBkColor(hdc, colour);
    
	GetClientRect(hwnd, &rect);
	
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, "", 0, 0);
    SetBkColor(hdc, oldcr);
}


//
//	Main window procedure
//
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int width, height;

	switch(msg)
	{
		case WM_PAINT:
		{
			//PaintRect(hwnd, hdc, RGB(255,0,0));
			HDC          hdc;
			PAINTSTRUCT  ps;
	
			// Get a device context for this window
			hdc = BeginPaint(hwnd, &ps);

			PaintRect(hwnd, hdc, RGB(255,0,0) );

			SetStretchBltMode(hdc, COLORONCOLOR);

			BitBlt(hdc, 0, 0, 800, 600, hbitmap_hdc, 0, 0, SRCCOPY); 

			// Release the device context
			EndPaint(hwnd, &ps);

			return 0;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//
//	Register main window class
//
static void InitMainWnd()
{
	WNDCLASSEX wcx;
	HANDLE hInst = GetModuleHandle(0);

	// Window class for the main application parent window
	wcx.cbSize			= sizeof(wcx);
	wcx.style			= 0;
	wcx.lpfnWndProc		= WndProc;
	wcx.cbClsExtra		= 0;
	wcx.cbWndExtra		= 0;
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
static HWND CreateMainWnd()
{
	return CreateWindowEx(0,
				szAppName,				// window class name
				szAppName,				// window caption
				WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
				CW_USEDEFAULT,			// initial x position
				CW_USEDEFAULT,			// initial y position
				800,					// initial x size
				600,					// initial y size
				NULL,					// parent window handle
				NULL,					// use window class menu
				GetModuleHandle(0),		// program instance handle
				NULL);					// creation parameters
}




static HBITMAP 
GetDibSection(FIBITMAP *src, HDC hdc, int left, int top, int right, int bottom)
{
	if(!src) 
		return NULL;

	unsigned bpp = FreeImage_GetBPP(src);

	if(bpp <=4)
		return NULL;

	// normalize the rectangle
	if(right < left)
		INPLACESWAP(left, right);

	if(bottom < top)
		INPLACESWAP(top, bottom);

	// check the size of the sub image
	int src_width  = FreeImage_GetWidth(src);
	int src_height = FreeImage_GetHeight(src);
	int src_pitch = FreeImage_GetPitch(src);

	if((left < 0) || (right > src_width) || (top < 0) || (bottom > src_height))
		return NULL;

	// allocate the sub image
	int dst_width = (right - left);
	int dst_height = (bottom - top);

	BITMAPINFO *info = (BITMAPINFO *) malloc(sizeof(BITMAPINFO) + (FreeImage_GetColorsUsed(src) * sizeof(RGBQUAD)));
	BITMAPINFOHEADER *bih = (BITMAPINFOHEADER *)info;

	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = dst_width;
	bih->biHeight = dst_height;
	bih->biPlanes = 1;
	bih->biBitCount = bpp;
	bih->biCompression = BI_RGB;
	bih->biSizeImage = 0;
    bih->biXPelsPerMeter = 0;
    bih->biYPelsPerMeter = 0;
   	bih->biClrUsed = 0;           // Always use the whole palette.
    bih->biClrImportant = 0;

	// copy the palette
	if(bpp < 16)
		memcpy(info->bmiColors, FreeImage_GetPalette(src), FreeImage_GetColorsUsed(src) * sizeof(RGBQUAD));

	BYTE *dst_bits;

	HBITMAP hbitmap = CreateDIBSection(hdc, info, DIB_RGB_COLORS, (void **) &dst_bits, NULL, 0);

	free(info);

	if(hbitmap == NULL || dst_bits == NULL)
		return NULL;

	// get the pointers to the bits and such
	BYTE *src_bits = FreeImage_GetScanLine(src, src_height - top - dst_height);

	// calculate the number of bytes per pixel
	unsigned bytespp = FreeImage_GetLine(src) / FreeImage_GetWidth(src);
	
	// point to x = left
	src_bits += (left * bytespp);	

	int dst_line = (dst_width * bpp + 7) / 8;
	int dst_pitch = (dst_line + 3) & ~3;

	for(int y = 0; y < dst_height; y++)
		memcpy(dst_bits + (y * dst_pitch), src_bits + (y * src_pitch), dst_line);

	return hbitmap;
}



void
ShowImage(FIBITMAP *src)
{
	MSG msg;
	FIBITMAP *src2;

	// initialize window classes
	InitMainWnd();

	hwndMain = CreateMainWnd();
	hdc = GetDC(hwndMain);

	assert(hdc != NULL);

	src2 = FreeImage_ConvertToStandardType(src);

	int width = FreeImage_GetWidth(src2);
	int height = FreeImage_GetHeight(src2);

	hbitmap = GetDibSection(src2, hdc, 0, 0, width, height);
	
	assert(hbitmap != NULL);

	hbitmap_hdc = CreateCompatibleDC(hdc);

	// Associate the new DC with the bitmap for drawing 
	SelectObject( hbitmap_hdc, hbitmap );

	ShowWindow(hwndMain, 1);

	// message-loop
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}
}

void
ShowImageFromFile(char *filepath)
{
	FIBITMAP* dib = FreeImageAlgorithms_LoadFIBFromFile(filepath);

	assert(dib != NULL);

	ShowImage(dib);

	FreeImage_Unload(dib);
}