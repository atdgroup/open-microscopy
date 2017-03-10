#ifndef __BASIC_WIN32_WINDOW__
#define __BASIC_WIN32_WINDOW__

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "ImageViewer.h"
#include "FreeImage.h"

#ifdef __cplusplus
extern "C" {
#endif

IV_DLL_API HWND IV_DLL_CALLCONV
BasicWin32Window(const char *title, int left, int top, int width, int height, FIBITMAP *dib);

IV_DLL_API void IV_DLL_CALLCONV
BasicWin32Window_SetPalette(HWND hwnd, RGBQUAD *palette);

IV_DLL_API void IV_DLL_CALLCONV
RemoveAllBasicWin32Windows(void);

#ifdef __cplusplus
}
#endif

#endif