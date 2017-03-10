#ifndef __IMAGING_WINDOW_PRIVATE__
#define __IMAGING_WINDOW_PRIVATE__

#include "icsviewer_window.h"

void Debug(IcsViewerWindow *window, const char *fmt, ...) ; 

/**
 * GetInternalMetaData:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Get metadata value for key.
 **/
int
GetInternalMetaData(IcsViewerWindow *window, char *key, char *value);


/**
 * ModifyInternalMetaData:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Replace the metdata value for an existing key.
 * This is not save to file until the user saves the image to an ics file.
 **/
int 
ModifyInternalMetaData(IcsViewerWindow *window, char *key, char *value);

/**
 * Save Metadata to image
 * Only works for ics files.
**/
void
SaveMetaDataToImage(IcsViewerWindow *window, const char *filepath);

int
GCI_ImagingWindow_LoadImageAdvanced(IcsViewerWindow *window, FIBITMAP *dib,
	int reset_multidimensional_data);

void DestroyIdandWindowPtrLists(void);

void DisplayImage(IcsViewerWindow *window, FIBITMAP *dib);

int Window_Destroy(IcsViewerWindow *window);

void CreateButtonBar(IcsViewerWindow *window);

void RegisterDropWindow(IcsViewerWindow *window);

void UnregisterDropWindow(IcsViewerWindow *window);

void format_intensity_string(IcsViewerWindow *window, double value, char *string);

void SetStatusbarText(IcsViewerWindow *window, int part, char *fmt, ...);

FIAPOINT GdiToFiaPoint(POINT pt);

LRESULT CALLBACK GCI_WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  	
LRESULT CALLBACK ScrolledWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  	
LRESULT CALLBACK GCI_WindowCanvasProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT IcsViewerWindow_OnSizeEvent(IcsViewerWindow *window, WPARAM wParam, LPARAM lParam);

LRESULT GCICanvas_OnLMousePressEvent(IcsViewerWindow *window, int x, int y); 

LRESULT GCICanvas_OnRMousePressEvent(IcsViewerWindow *window, int x, int y); 

LRESULT GCICanvas_OnLMouseReleaseEvent(IcsViewerWindow *window, int x, int y);

void ics_viewer_registry_read_string(IcsViewerWindow *window, const char *key, char *value,
  unsigned int *realStringSize);
  
void ics_viewer_registry_save_panel_position(IcsViewerWindow *window, int panel_id);
void ics_viewer_registry_save_panel_size_position(IcsViewerWindow *window, int panel_id);

void ics_viewer_registry_read_panel_position(IcsViewerWindow *window, int panel_id);
void ics_viewer_registry_read_panel_size_position(IcsViewerWindow *window, int panel_id);
void ics_viewer_set_panel_to_top_left_of_window(IcsViewerWindow *window, int panel_id);

char* GetDefaultDirectoryPath(IcsViewerWindow *window, char *path);

#endif
