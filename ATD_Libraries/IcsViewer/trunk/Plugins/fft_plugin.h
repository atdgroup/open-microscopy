#include "icsviewer_plugin.h" 

#include "FreeImageAlgorithms_FFT.h" 

typedef struct
{
	ImageWindowPlugin parent;
	
	HBITMAP hbitmap;
	
	int	activated;
	int panel_id;
	int ok_button;
	int handle;
	
	HWND canvas;
	HDC  canvas_hdc;
	
} FFTPlugin;


ImageWindowPlugin* fft_plugin_constructor(IcsViewerWindow *window);
