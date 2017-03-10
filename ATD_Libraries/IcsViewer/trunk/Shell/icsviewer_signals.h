#ifndef __IMAGING_WINDOW_SIGNALS__
#define __IMAGING_WINDOW_SIGNALS__

#include "icsviewer_window.h"
#include "icsviewer_signals.h"

#include "signals.h"

int VOID_WINDOW_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_WINDOW_PTR_POINT_POINT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_WINDOW_PTR_STRING_STRING_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_STRING_STRING_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_WINDOW_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

typedef void (*ICSVIEWER_EVENT_HANDLER) (IcsViewerWindow *window, void* data );

typedef void (*PROFILE_HANDLER) (IcsViewerWindow *window, const Point p1, const Point p2, void* data );

typedef void (*CROSSHAIR_HANDLER) (IcsViewerWindow *window, const Point image_point, const Point viewer_point, void* data);

typedef void (*ROI_CHANGED_HANDLER) (IcsViewerWindow *window, const Point image_point, const Point viewer_point, void* data);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_CreateAllSignals( IcsViewerWindow *window);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetOnLastWindowDestroyEventHandler(void (*destroy_handler) (int, void *data), void *callback_data);


/**
 * GCI_SetImageWindowErrorHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @error_handler: pointer to a function that handles errors.
 *
 * Specifies a function for handling errors
 **/
IW_DLL_API void IW_DLL_CALLCONV 
GCI_ImagingWindow_SetErrorHandler( IcsViewerWindow *window,
	void (*error_handler) (const char *message, char *filename, int line, void* data ), void *callback_data );



/**
 * GCI_ImagingWindow_SetProfileHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @profile_handler: pointer to a function that is call when a user has drawn a profile.
 *
 * Specifies a function for handling profile events
 **/
IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetProfileHandler( IcsViewerWindow *window, PROFILE_HANDLER profile_handler, void *callback_data );


/**
 * GCI_ImagingWindow_DisconnectProfileHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @id: id of the callback return from SetProfileHandler.
 *
 * Disconnects a profile signal handler
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectProfileHandler( IcsViewerWindow *window, int id );


/**
 * GCI_ImagingWindow_SetCrosshairHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @crosshair_handler: pointer to a function that is called when a user has drawn a crosshair.
 * @x: x coord on the image.
 * @y: y coord on the image.
 *
 * Specifies a function for handling crosshair events
 **/
IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetCrosshairHandler( IcsViewerWindow *window, CROSSHAIR_HANDLER crosshair_handler, void *callback_data );


IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetRoiChangedHandler( IcsViewerWindow *window, ROI_CHANGED_HANDLER handler, void *callback_data );

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetRoiMoveOrSizeChangeCompletedHandler( IcsViewerWindow *window, ICSVIEWER_EVENT_HANDLER handler, void *callback_data );

/**
 * GCI_ImagingWindow_DisconnectCrosshairHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @id: id of the callback return from SetProfileHandler.
 *
 * Disconnects a crosshair signal handler
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectCrosshairHandler( IcsViewerWindow *window, int id );


/**
 * GCI_ImagingWindow_SetLoadHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @load_handler: pointer to a function that is call when an image has been loaded.
 *
 * Specifies a function for handling load events
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetLoadHandler( IcsViewerWindow *window, void (*load_handler) (IcsViewerWindow *window, void* data ), void* callback_data  );



/**
 * GCI_ImagingWindow_SetResizedHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @resized_handler: pointer to a function that is called when the window has been resized.
 *
 * Specifies a function for handling resized events
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetResizedorMovedHandler( IcsViewerWindow *window, void (*resized_handler) (IcsViewerWindow *window, void* data ), void *callback_data );

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowResizingHandler( IcsViewerWindow *window,
		void (*resizing_handler) (IcsViewerWindow *window, int left, int top, int right, int bottom, void* data ),
		void *callback_data );

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowShowOrHideHandler( IcsViewerWindow *window, void (*handler) (IcsViewerWindow *window,
										   int show, void* data),
										   void *callback_data );

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowMovingHandler( IcsViewerWindow *window,
		void (*resizing_handler) (IcsViewerWindow *window, int left, int top, int right, int bottom, void* data ),
		void *callback_data );

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowMinimisedHandler( IcsViewerWindow *window, void (*resizing_handler) (IcsViewerWindow *window, void* data),
										   void *callback_data );

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowMaximisedHandler( IcsViewerWindow *window, void (*resizing_handler) (IcsViewerWindow *window, void* data),
										   void *callback_data );

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowRestoredHandler( IcsViewerWindow *window, void (*resizing_handler) (IcsViewerWindow *window, void* data),
										   void *callback_data );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowResizingHandler( GCIWindow *window, int id );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowShowOrHideHandler( GCIWindow *window, int id );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowMovingHandler( GCIWindow *window, int id );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowRestoredHandler( GCIWindow *window, int id );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowMaximisedHandler( GCIWindow *window, int id );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowMinimisedHandler( GCIWindow *window, int id );

/**
 * GCI_ImagingWindow_DestroyAllSignals:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Destroy all of a windows signals.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DestroyAllSignals( IcsViewerWindow *window );


/**
 * GCI_ImagingWindow_SetDestroyEventHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @destroy_handler: function that is called.
 *
 * Set a function to be called when the window is destroyed.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetDestroyEventHandler( IcsViewerWindow *window, void (*destroy_handler) (IcsViewerWindow *window, void *data), void *callback_data );


/**
 * GCI_ImagingWindow_SetCloseEventHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @close_handler: function that is called when the user closes the window.
 *
 * Set a function to be called when the window is destroyed..
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetCloseEventHandler( IcsViewerWindow *window, void (*close_handler) (IcsViewerWindow *window, void *data), void *callback_data );


/**
 * GCI_ImagingWindow_SetSaveHandler:
 * @window: Imaging window in which the image is to be loaded.
 * @save_handler: pointer to a function that is called when the windows image has been saved.
 *
 * Specifies a function for handling save events
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetSaveHandler( IcsViewerWindow *window, void (*save_handler) (IcsViewerWindow *window, char *filename, char *ext, void* data ), void *callback_data );


IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetMouseDownHandler( GCIWindow *window, CROSSHAIR_HANDLER handler, void *callback_data );


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectMouseDownHandler( GCIWindow *window, int id );

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetMouseUpHandler( GCIWindow *window, CROSSHAIR_HANDLER handler, void *callback_data );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectMouseUpHandler( GCIWindow *window, int id );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetImagePreLoadedHandler(
	IcsViewerWindow *window, void (*handler) (IcsViewerWindow *window, void* data ), void *callback_data );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetStreamImageGrabPreDisplayedHandler( IcsViewerWindow *window,
	void (*handler) (IcsViewerWindow *window, void *dib, void* data ), void *callback_data );

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetStreamImageSnappedHandler(
	IcsViewerWindow *window, void (*handler) (IcsViewerWindow *window, void* data ), void *callback_data );

#endif
