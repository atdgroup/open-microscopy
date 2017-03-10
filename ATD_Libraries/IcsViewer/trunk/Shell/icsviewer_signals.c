#include "icsviewer_signals.h"

#include "signals.h"
#include "gci_utils.h"

int VOID_WINDOW_PTR_VOID_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (IcsViewerWindow *, void* dib, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (IcsViewerWindow *) args[0].void_ptr_data, (void *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int VOID_WINDOW_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (IcsViewerWindow *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (IcsViewerWindow *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int VOID_WINDOW_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (IcsViewerWindow *, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (IcsViewerWindow *) args[0].void_ptr_data, args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int VOID_WINDOW_PTR_POINT_POINT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (IcsViewerWindow *, const Point, const Point, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (IcsViewerWindow *) args[0].void_ptr_data, args[1].point_data, args[2].point_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int VOID_WINDOW_PTR_STRING_STRING_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (IcsViewerWindow *, char *, char *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (IcsViewerWindow *) args[0].void_ptr_data, args[1].string_data, args[2].string_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int VOID_WINDOW_PTR_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (IcsViewerWindow *, int, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (IcsViewerWindow *) args[0].void_ptr_data, args[1].int_data, args[2].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_STRING_STRING_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (const char *message, char *filename, int line, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( args[0].string_data, args[1].string_data, args[2].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_WINDOW_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (IcsViewerWindow *, const int, const int, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (IcsViewerWindow *) args[0].void_ptr_data, args[1].int_data, args[2].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int VOID_WINDOW_PTR_INT_INT_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (IcsViewerWindow *, const int, const int, const int, const int, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (IcsViewerWindow *) args[0].void_ptr_data, args[1].int_data, args[2].int_data, args[3].int_data, args[4].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetErrorHandler( IcsViewerWindow *window,
                                  void (*error_handler) (const char *message, char *filename, int line, void* data), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "Error", error_handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetDestroyEventHandler( IcsViewerWindow *window, void (*destroy_handler) (IcsViewerWindow *window, void *data), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "Destroy", destroy_handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetOnLastWindowDestroyEventHandler(void (*destroy_handler) (int, void *data), void *callback_data)
{
	window_destroyed_callback_data = callback_data;
	window_destroyed_handler = destroy_handler;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetCloseEventHandler( IcsViewerWindow *window, void (*close_handler) (IcsViewerWindow *window, void *data), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "Close", close_handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_SetProfileHandler( IcsViewerWindow *window, PROFILE_HANDLER profile_handler, void *callback_data )
{
	int id;
	
	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "Profile", profile_handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
		
	return id; 
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectProfileHandler( IcsViewerWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "Profile", id);
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_SetCrosshairHandler( IcsViewerWindow *window, CROSSHAIR_HANDLER crosshair_handler, void *callback_data )
{
	int id;
	
	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "Crosshair", crosshair_handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
		
	return id;
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_SetRoiChangedHandler( IcsViewerWindow *window, ROI_CHANGED_HANDLER handler, void *callback_data )
{
	int id;
	
	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "RoiChanged", handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
		
	return id;	
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_SetRoiMoveOrSizeChangeCompletedHandler( IcsViewerWindow *window, ICSVIEWER_EVENT_HANDLER handler, void *callback_data )
{
	int id;
	
	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "RoiMoveOrResizeCompleted", handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
		
	return id;	
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectCrosshairHandler( IcsViewerWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "Crosshair", id);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetLoadHandler( IcsViewerWindow *window, void (*load_handler) (IcsViewerWindow *window, void* data ), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "Load", load_handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetResizedorMovedHandler( IcsViewerWindow *window, void (*resized_handler) (IcsViewerWindow *window, void* data ), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "ResizedorMoved", resized_handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowResizingHandler( IcsViewerWindow *window, void (*resizing_handler) (IcsViewerWindow *window,
										   int left, int top, int right, int bottom, void* data),
										   void *callback_data )
{
	int id;

	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "WindowResizing", resizing_handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");

	return id;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowShowOrHideHandler( IcsViewerWindow *window, void (*handler) (IcsViewerWindow *window,
										   int show, void* data),
										   void *callback_data )
{
	int id;

	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "WindowShowOrHide", handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");

	return id;
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowMinimisedHandler( IcsViewerWindow *window, void (*resizing_handler) (IcsViewerWindow *window, void* data),
										   void *callback_data )
{
	int id;

	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "WindowMinimised", resizing_handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");

	return id;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowMaximisedHandler( IcsViewerWindow *window, void (*resizing_handler) (IcsViewerWindow *window, void* data),
										   void *callback_data )
{
	int id;

	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "WindowMaximised", resizing_handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");

	return id;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowRestoredHandler( IcsViewerWindow *window, void (*resizing_handler) (IcsViewerWindow *window, void* data),
										   void *callback_data )
{
	int id;

	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "WindowRestored", resizing_handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");

	return id;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowMovingHandler( IcsViewerWindow *window, void (*resizing_handler) (IcsViewerWindow *window,
										   int left, int top, int right, int bottom, void* data),
										   void *callback_data )
{
	int id;

	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "WindowMoving", resizing_handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");

	return id;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowResizingHandler( GCIWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "WindowResizing", id);   
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowShowOrHideHandler( GCIWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "WindowShowOrHide", id);   
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowMovingHandler( GCIWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "WindowMoving", id);   
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowRestoredHandler( GCIWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "WindowRestored", id);   
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowMaximisedHandler( GCIWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "WindowMaximised", id);   
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectWindowMinimisedHandler( GCIWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "WindowMinimised", id);   
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetSaveHandler( IcsViewerWindow *window, void (*save_handler) (IcsViewerWindow *window, char *filename, char *ext, void* data ), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "Save", save_handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");	
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_SetMouseDownHandler( GCIWindow *window, CROSSHAIR_HANDLER handler, void *callback_data )
{
	int id;
	
	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "MouseDown", handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
		
	return id;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_SetMouseUpHandler( GCIWindow *window, CROSSHAIR_HANDLER handler, void *callback_data )
{
	int id;
	
	if( (id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "MouseUp", handler, callback_data)) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
		
	return id;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectMouseUpHandler( GCIWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "MouseUp", id);   

}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisconnectMouseDownHandler( GCIWindow *window, int id )
{
	GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(window), "MouseDown", id);   
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetImagePreLoadedHandler( IcsViewerWindow *window, void (*handler) (IcsViewerWindow *window, void* data ), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "ImagePostLoaded", handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetStreamImageGrabPreDisplayedHandler( IcsViewerWindow *window, void (*handler) (IcsViewerWindow *window, void *dib, void* data ), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "ImagePreLoaded", handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetStreamImageSnappedHandler( IcsViewerWindow *window, void (*handler) (IcsViewerWindow *window, void* data ), void *callback_data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(window), "StreamDeviceImageSnapped", handler, callback_data) == SIGNAL_ERROR)
		printf("Error cannot connect signal handler\n");
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_CreateAllSignals( IcsViewerWindow *window) 
{
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "Error", VOID_STRING_STRING_INT_MARSHALLER);  
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "Destroy", VOID_WINDOW_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "Close", VOID_WINDOW_PTR_MARSHALLER);  
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "Profile", VOID_WINDOW_PTR_POINT_POINT_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "Load", VOID_WINDOW_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "Crosshair", VOID_WINDOW_PTR_POINT_POINT_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "ResizedorMoved", VOID_WINDOW_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "WindowResizing", VOID_WINDOW_PTR_INT_INT_INT_INT_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "WindowShowOrHide", VOID_WINDOW_PTR_INT_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "WindowMinimised", VOID_WINDOW_PTR_MARSHALLER);	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "WindowMaximised", VOID_WINDOW_PTR_MARSHALLER);	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "WindowRestored", VOID_WINDOW_PTR_MARSHALLER);	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "WindowMoving", VOID_WINDOW_PTR_INT_INT_INT_INT_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "Save", VOID_WINDOW_PTR_STRING_STRING_MARSHALLER);    
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "MouseDown", VOID_WINDOW_PTR_POINT_POINT_MARSHALLER);  
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "MouseUp", VOID_WINDOW_PTR_POINT_POINT_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "LastWindowDestroyed", VOID_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "RoiChanged", VOID_WINDOW_PTR_POINT_POINT_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "RoiMoveOrResizeCompleted", VOID_WINDOW_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "ImagePreLoaded", VOID_WINDOW_PTR_VOID_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "ImagePostLoaded", VOID_WINDOW_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(window), "StreamDeviceImageSnapped", VOID_WINDOW_PTR_MARSHALLER);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_DestroyAllSignals( IcsViewerWindow *window )
{
	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "Load") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");
		
	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "ResizedorMoved") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");

	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "WindowResizing") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");

	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "WindowShowOrHide") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");

	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "WindowMinimised") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");

	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "WindowMoving") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");

	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "Crosshair") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");
		
	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "Profile") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");
		
	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "Destroy") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");
		
	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "Error") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");
		
	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "ResizedorMoved") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");
	
	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "Save") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");

	if( GCI_Signal_Destroy(UIMODULE_SIGNAL_TABLE(window), "MouseDown") == SIGNAL_ERROR)
		printf("Error cannot destroy signal handler\n");
}
