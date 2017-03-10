// ImageWindow - Displays images with zooming and panning

#ifndef __IMAGEVIEWER_CONTROL__
#define __IMAGEVIEWER_CONTROL__

#define SZ_CLASSNAME ImageViewerCtrl

#define _WIN32_WINNT 0x0400 // This adds support for mouse wheel scrolling

#include <windows.h>

#ifdef IMAGEVIEWER_STDCALL
#define IV_DLL_CALLCONV __stdcall  
#elif  IMAGEVIEWER_CDECL
#define IV_DLL_CALLCONV __cdecl 
#else
#define IV_DLL_CALLCONV
#endif

// defined with this macro as being exported.

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IMAGEVIEWER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IV_DLL_API functions as being imported from a DLL, wheras this DLL sees symbols

#ifdef IMAGEVIEWER_EXPORTS
#define IV_DLL_API __declspec(dllexport)
#elif  IMAGEVIEWER_IMPORTS
#define IV_DLL_API __declspec(dllimport)
#else
#define IV_DLL_API
#endif // IMAGEVIEWER_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

#define IMAGEVIEWER_ERROR -1
#define IMAGEVIEWER_SUCCESS 1

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

typedef struct ImageViewer_  ImageViewerCtrl;

#define BUFFER_SIZE 3000 

#define ZOOM_MIN 0.1
#define ZOOM_MAX 100.0
#define ZOOM_INC 2
#define ZOOM_DEC 0.5

typedef enum {NO_INTERACT_MODE, PANNING_MODE, ZOOM_MODE} InteractMode;

#define IMAGEVIEWER_BASE         			(WM_USER + 200)
#define IMAGEVIEWER_ZOOM_CHANGED 			(IMAGEVIEWER_BASE + 0)
#define IMAGEVIEWER_BUFFER_PAINTED			(IMAGEVIEWER_BASE + 1)
#define IMAGEVIEWER_SCREEN_PAINTED			(IMAGEVIEWER_BASE + 2)

typedef struct _FPOINT
{
    float  x;
    float  y;

} FPOINT;

typedef struct _FSIZE
{
    float        cx;
    float        cy;

} FSIZE;

/* Canvas Control state structure: */
struct ImageViewer_
{
    HWND     		hwnd;

	HDC				screen_hdc;
	HDC 			hbitmap_hdc;
	HBITMAP 		hbitmap;
	HDC 			temp_hbitmap_hdc;
	HBITMAP 		temp_hbitmap;
	HDC 			buffer_hdc;
	HBITMAP 		buffer_bitmap;

	COLORREF		background_colour;

	InteractMode	viewer_interact_mode;

	UINT_PTR		timer_id_ptr;

	HCURSOR			zoom_cursor;
	WPARAM			last_mouse_move_wparam;
	
	double 			zoom_factor;
	int				bitmap_width;
	int				bitmap_height;
	int				image_size_changed;
	int				fit_to_viewer;
	int 			last_mouse_x_pos;
	int 			last_mouse_y_pos;
	int				horizontal_scroll_position;
	int				vertical_scroll_position;
};


IV_DLL_API HINSTANCE IV_DLL_CALLCONV
ImageViewer_GetHINSTANCE(void);

/**
 * ImageViewer_Init:
 *
 * This function initialises the viewer control.
 * The function must be called before creating an actual instance. 
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_Init(void);


/**
 * ImageViewer_Create:
 * @hwndParent: Parent Window
 * @left: x coordinate of the top left corner.
 * @top: y coordinate of the top left corner.
 * @width: width of the viewer control.
 * @height: height of the viewer control.
 *
 * Creates a new viewer instance
 **/
IV_DLL_API HWND IV_DLL_CALLCONV
ImageViewer_Create(HWND hwndParent, int left, int top, int width, int height);


IV_DLL_API HWND IV_DLL_CALLCONV
ImageViewer_CreateAdvanced (HWND hwndParent, int window_edge_type, int left, int top, int width, int height);

/**
 * ImageViewer_GetZoomedWidth:
 * @viewer_window: Canvas Window.
 *
 * Gets the width that the image is zoomed to.
 **/
IV_DLL_API int IV_DLL_CALLCONV
ImageViewer_GetZoomedWidth(HWND viewer_window);


/**
 * ImageViewer_GetZoomedWidth:
 * @viewer_window: Canvas Window.
 *
 * Gets the width that the image is zoomed to.
 **/
IV_DLL_API int IV_DLL_CALLCONV
ImageViewer_GetZoomedHeight(HWND viewer_window);


/**
 * ImageViewer_SetBackgroundColour:
 * @viewer_window: Canvas Window.
 *
 * Sets the background colour of the viewer.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_SetBackgroundColour(HWND viewer_window, COLORREF colour);


IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_PaintBackground(HWND viewer_window);

/**
 * ImageViewer_IsWindowPointWithinImage:
 * @viewer_window: Canvas Window.
 *
 * Detect if a point in window coordinates is within the displayed image.
 **/
IV_DLL_API int IV_DLL_CALLCONV
ImageViewer_IsWindowPointWithinImage(HWND viewer_window, POINT viewer_point);

/**
 * ImageViewer_TranslateRealImagePointToWindowPoint:
 * @viewer_window: Canvas Window.
 *
 * Translate a point on the control to the equivilent point on the image,
 * Taking acount of zoom, scroll and position.
 **/
void IV_DLL_CALLCONV
ImageViewer_TranslateRealImagePointToWindowPoint(HWND viewer_window, FPOINT image_point, FPOINT *viewer_point);


/**
 * ImageViewer_TranslateRealWindowPointToImagePoint:
 * @viewer_window: Canvas Window.
 *
 * Translate a point on the image to the equivilent point on the Viewer,
 * Taking acount of zoom, scroll and position.
 **/
void IV_DLL_CALLCONV
ImageViewer_TranslateRealWindowPointToImagePoint(HWND viewer_window, FPOINT viewer_point, FPOINT *image_point);


/**
 * ImageViewer_TranslateCanvasPointToImagePoint:
 * @viewer_window: Canvas Window.
 *
 * Translate a point on the control to the equivilent point on the image,
 * Taking acount of zoom, scroll and position.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_TranslateWindowPointToImagePoint(HWND viewer_window, POINT viewer_point, POINT *image_point);


/**
 * ImageViewer_TranslateImagePointToCanvasPoint:
 * @viewer_window: Canvas Window.
 *
 * Translate a point on the image to the equivilent point on the Viewer,
 * Taking acount of zoom, scroll and position.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_TranslateImagePointToWindowPoint(HWND viewer_window, POINT image_point, POINT *viewer_point);


/**
 * ImageViewer_GetImageSize:
 * @viewer_window: Canvas Window.
 *
 * Returns the size of the original image.
 **/
IV_DLL_API void IV_DLL_CALLCONV 
ImageViewer_GetImageSize(HWND viewer_window, SIZE *size);


/**
 * ImageViewer_GetDisplayedImageRect:
 * @viewer_window: Canvas Window.
 *
 * Returns the rectangle the surrounds the displayed image.
 * If the image is larger than the view the client area is returned.
 **/
IV_DLL_API void IV_DLL_CALLCONV 
ImageViewer_GetDisplayedImageRect(HWND viewer_window, RECT *rect);


/**
 * ImageViewer_GetDisplayedImageScreenRect:
 * @viewer_window: Canvas Window.
 *
 * Returns ImageViewer_GetDisplayedImageRect in screen coords.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_GetDisplayedImageScreenRect(HWND window, RECT *screen_rect);


/**
 * ImageViewer_ScreenPointToWindowPoint:
 * @viewer_window: Canvas Window.
 *
 * Converts a POINT in screen coords to a POINT on the imageviewer window.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_ScreenPointToWindowPoint(HWND window, POINT screen_point, POINT *window_point);


/**
 * ImageViewer_IsPointWithinImage:
 * @viewer_window: Canvas Window.
 *
 * Returns > 0 if the point is within the displayed image rect.
 **/
IV_DLL_API int IV_DLL_CALLCONV 
ImageViewer_IsPointWithinDisplayedImageRect(HWND viewer_window, POINT point);


/**
 * ImageViewer_GetBufferHdc:
 * @viewer_window: Canvas Window.
 *
 * Get the HDC for the buffer, so the user can draw before the paint event.
 **/
IV_DLL_API HDC IV_DLL_CALLCONV
ImageViewer_GetBufferHdc(HWND viewer_window); 

/**
 * ImageViewer_GetBufferHdc:
 * @viewer_window: Canvas Window.
 *
 * Get the HDC for the used with the hbitmap passed in with SetImage.
 **/
IV_DLL_API HDC IV_DLL_CALLCONV
ImageViewer_GetBitmapHdc(HWND viewer_window); 

/**
 * ImageViewer_GetBufferHdc:
 * @viewer_window: Canvas Window.
 *
 * Sets the palette for the passed in HBitmap.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_SetPalette(HWND viewer_window, RGBQUAD *palette, int size); 


/**
 * ImageViewer_Redraw:
 * @viewer_window: Canvas Window.
 *
 * Erase any drawing done on the buffer since the image was loaded and invalidates viewer.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_Redraw(HWND viewer_window); 


/**
 * ImageViewer_EnableZoomToFit:
 * @viewer_window: Canvas Window.
 *
 * Keeps the image zoomed to the size of the window while maintaining aspect ratio.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_EnableZoomToFit(HWND viewer_window);


/**
 * ImageViewer_DisableZoomToFit:
 * @viewer_window: Canvas Window.
 *
 * Disable zooming the image to fit the window.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_DisableZoomToFit(HWND viewer_window);


/**
 * ImageViewer_IsZoomToFit:
 * @viewer_window: Canvas Window.
 *
 * Returns 1 if the ImageViewer is set to ZoomToFit.
 **/
IV_DLL_API int IV_DLL_CALLCONV
ImageViewer_IsZoomToFit(HWND viewer_window);


/**
 * ImageViewer_SetZoom:
 * @viewer_window: Canvas Window.
 *
 * Display the image at a specified zoom level.
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_SetZoom(HWND viewer_window, double zoom);


/**
 * ImageViewer_GetZoomFactor:
 * @viewer_window: Canvas Window.
 *
 * Get the current zoom level.
 **/
IV_DLL_API double IV_DLL_CALLCONV
ImageViewer_GetZoomFactor(HWND viewer_window);


/**
 * ImageViewer_GetHorizontalScrollPosition:
 * @viewer_window: Canvas Window.
 *
 * Get the current horzontal scroll position.
 **/
IV_DLL_API int IV_DLL_CALLCONV
ImageViewer_GetHorizontalScrollPosition(HWND viewer_window);


/**
 * ImageViewer_GetVerticalScrollPosition:
 * @viewer_window: Canvas Window.
 *
 * Get the current vertical scroll position.
 **/
IV_DLL_API int IV_DLL_CALLCONV
ImageViewer_GetVerticalScrollPosition(HWND viewer_window);


/**
 * ImageViewer_SetImage:
 * @viewer_window: Canvas Window.
 *
 * Set the FreeImage to display
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_SetImage(HWND viewer_window, HBITMAP bitmap);


/**
 * ImageViewer_SetInteractMode:
 * @viewer_window: Canvas Window.
 *
 * Sets the ImageViewer_SetInteractMode.
 * This can enable zoom on mouse clicks or panning or scroll the image when the mouse button
 * in held don outside the ImageViewer - (Useful for scroll while drawing).
 **/
IV_DLL_API void IV_DLL_CALLCONV
ImageViewer_SetInteractMode(HWND viewer_window, InteractMode mode);


/**
 * ImageViewer_GetInteractMode:
 * @viewer_window: Canvas Window.
 *
 * Gets the ImageViewer_SetInteractMode.
 **/
IV_DLL_API InteractMode IV_DLL_CALLCONV
ImageViewer_GetInteractMode(HWND viewer_window);


#ifdef __cplusplus
}
#endif

#endif
