#ifndef __IMAGING_WINDOW_TOOLS__
#define __IMAGING_WINDOW_TOOLS__

#include "icsviewer_plugin.h"

typedef void (*ToolCallbackPtr)(Tool *tool, int main_ctrl_changed, int active, int locked, void *callback_data);

struct _Tool
{
	ImageWindowPlugin parent;   
	
	int button_id;
	int locked;
	int lock_id;
	
	ToolCallbackPtr eventFunction;
	void *callback_data;
};


ImageWindowPlugin* Plugin_NewToolPluginType(IcsViewerWindow *window, char *name, size_t size,
	char *label, char *icon_name, int create_lock, ToolCallbackPtr eventFunction, void *callback_data);


void DimTool(Tool *tool);

void UnDimTool(Tool *tool);

void ActivateTool(Tool *tool);

void DeactivateTool(Tool *tool);

void HideTool(Tool *tool);

void ShowTool(Tool *tool);

void LockTool(Tool *tool);

void UnlockTool(Tool *tool);

int IsToolActive(Tool *tool);

int IsToolLocked(Tool *tool);

void SetMinOnPaletteBar(IcsViewerWindow *window, double value);

void SetMaxOnPaletteBar(IcsViewerWindow *window, double value);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableLineTool(IcsViewerWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableLineTool(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableCrossHair(IcsViewerWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableCrossHair(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableZoomTool(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableZoomTool(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableRoiTool(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableRoiTool(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_LockProfileButton(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockProfileButton(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_LockCrossHairButton(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockCrossHairButton(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_LockRoiButton(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockRoiButton(GCIWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DimRoiTool(IcsViewerWindow *window, int dim);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DimZoomTool(IcsViewerWindow *window, int dim);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DimCrossHairTool(IcsViewerWindow *window, int dim);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DimProfileTool(IcsViewerWindow *window, int dim);

#endif
