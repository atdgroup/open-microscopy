#ifndef __ICS_VIEWER_WINDOW__
#define __ICS_VIEWER_WINDOW__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#include <userint.h>
#include <ansi_c.h>
#include <windows.h>

#include "gci_ui_module.h"

#include "FreeImage.h"
#include "gci_utils.h"
#include "signals.h"
#include "toolbox.h" 

#include "FreeImageIcs_IO.h"
#include "FreeImageIcs_MetaData.h"
#include "libics.h"

#include "FreeImageAlgorithms_Filters.h"

#include "dictionary.h"

#include <oleidl.h>

#ifdef STREAM_DEVICE_PLUGIN

typedef struct		_DirectShowCapture DirectShowCapture;
typedef struct		_DirectShowCapture DSC;
typedef struct		_Routing Routing;
typedef struct		_Crossbar Crossbar;
typedef struct		_DscFilterInfo DscFilterInfo;

#include <strmif.h>
#endif

#define IW_DLL_CALLCONV __stdcall

// defined with this macro as being exported.

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IMAGEVIEWER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IM_DLL_API functions as being imported from a DLL, wheras this DLL sees symbols

#ifdef IMAGEWINDOW_EXPORTS
#define IW_DLL_API __declspec(dllexport)
#elif IMAGEWINDOW_IMPORTS
#define IW_DLL_API __declspec(dllimport)
#else
#define IW_DLL_API
#endif

#ifdef STANDALONE_APP 
#define RESTORE_MSG_TEXT "IcsViewer_Restore"
#define OPEN_IN_SAME_WINDOW 
#define ENABLE_TWAIN
#endif

#define PANEL_MIN_WIDTH 400
#define PANEL_MIN_HEIGHT 400
#define IMAGE_VIEW_BORDER 10	
#define PALETTEBAR_WIDTH 40

#define VIEWER_TOP 35
#define VIEWER_LEFT_WITHOUT_TOOLBAR 10
#define VIEWER_LEFT_WITH_TOOLBAR 80

#define VIEWER_DIST_FROM_RIGHT_WITHOUT_PALETTEBAR IMAGE_VIEW_BORDER     
#define VIEWER_DIST_FROM_RIGHT_WITH_PALETTEBAR (IMAGE_VIEW_BORDER * 2 + PALETTEBAR_WIDTH)

#define GCI_IMAGING_ERROR -1
#define GCI_IMAGING_SUCCESS 0

#define MAX_NUMBER_OF_PLUGINS 25

#define MAX_NUMBER_OF_MENUS_PER_PLUGIN 6

#define MAX_NUMBER_OF_MENU_ITEMS (MAX_NUMBER_OF_PLUGINS * MAX_NUMBER_OF_MENUS_PER_PLUGIN)

#define IMAGE_PATH_SIZE 500

#define U12BIT_MAX 4096
#define U14BIT_MAX 16384

#define BIT12_MAX 2047
#define BIT12_MIN (-BIT12_MAX - 1)  

#define BIT14_MAX 8191
#define BIT14_MIN (-BIT14_MAX - 1)

#define REGISTRY_SUBKEY "software\\GCI\\IcsViewer\\"

#define ICSVIEWER_BASE         			(WM_USER + 10)
#define ICSVIEWER_IMAGE_LOAD 			(ICSVIEWER_BASE + 1)
#define ICSVIEWER_UPDATE_TITLEBAR		(ICSVIEWER_BASE + 2)

#ifdef _WIN32
#define STATUSCLASSNAMEW        L"msctls_statusbar32"
#define STATUSCLASSNAMEA        "msctls_statusbar32"

#ifdef UNICODE
#define STATUSCLASSNAME         STATUSCLASSNAMEW
#else
#define STATUSCLASSNAME         STATUSCLASSNAMEA
#endif

#else
#define STATUSCLASSNAME         "msctls_statusbar"
#endif

#define SB_SETTEXTA             (WM_USER+1)
#define SB_SETTEXTW             (WM_USER+11)
#define SB_GETTEXTA             (WM_USER+2)
#define SB_GETTEXTW             (WM_USER+13)
#define SB_GETTEXTLENGTHA       (WM_USER+3)
#define SB_GETTEXTLENGTHW       (WM_USER+12)

#ifdef UNICODE
#define SB_GETTEXT              SB_GETTEXTW
#define SB_SETTEXT              SB_SETTEXTW
#define SB_GETTEXTLENGTH        SB_GETTEXTLENGTHW
#if (_WIN32_IE >= 0x0400)
#define SB_SETTIPTEXT           SB_SETTIPTEXTW
#define SB_GETTIPTEXT           SB_GETTIPTEXTW
#endif
#else
#define SB_GETTEXT              SB_GETTEXTA
#define SB_SETTEXT              SB_SETTEXTA
#define SB_GETTEXTLENGTH        SB_GETTEXTLENGTHA
#if (_WIN32_IE >= 0x0400)
#define SB_SETTIPTEXT           SB_SETTIPTEXTA
#define SB_GETTIPTEXT           SB_GETTIPTEXTA
#endif
#endif

#define SB_SETPARTS             (WM_USER+4)
#define SB_GETPARTS             (WM_USER+6)

#define SB_SIMPLE               (WM_USER+9)

#define FRAMES_PER_SECOND 10
#define MIN_TIME_PER_FRAME (1.0 / FRAMES_PER_SECOND)

void *window_destroyed_callback_data;
void (*window_destroyed_handler) (int window_id, void *data);

#define RESOURCE_DIR_SIZE 500

char		    resource_dir[RESOURCE_DIR_SIZE];
char		    uir_file_path[RESOURCE_DIR_SIZE];
char			icon_file_path[RESOURCE_DIR_SIZE];
	
int RESTORE_MSG;

typedef struct IcsViewerWindow_ IcsViewerWindow;
typedef struct IcsViewerWindow_ GCIWindow; 		   // Backward Compatibility.

typedef struct IcsViewerWindowMenu_ IcsViewerWindowMenu;

typedef struct _ImageWindowPlugin ImageWindowPlugin;

typedef struct _ImageWindowMenuPlugin ImageWindowMenuPlugin; 

typedef struct _ImageWindowMenuDialogPlugin ImageWindowMenuDialogPlugin; 

typedef struct _Manipulater3d Manipulater3d;

typedef struct _Tool Tool;

typedef enum  { GREYSCALE_PALETTE = 1,
				OVERLOAD_PALETTE,
				LOG_PALETTE,
				RAINBOW_PALETTE,
				REVERSE_RAINBOW_PALETTE,
				PILEUP_PALETTE,
				SEISMIC_PALETTE,
				TEMPARATURE_PALETTE,
				FALSE_COLOUR_PALETTE
			  } PALETTE_TYPE;


typedef enum {
	
	CURSOR_NORMAL,
	CURSOR_CROSS,
	CURSOR_ZOOM,
	CURSOR_PANNING,
	CURSOR_SIZE_ALL,
	CURSOR_NS,
	CURSOR_WE,
	CURSOR_NWSE,
	CURSOR_NESW
	
} CursorType;


struct IcsViewerWindowMenu_
{
	int	menubar_id;
	int open_menu_item_id;
	int exit_menu_item_id;
	int file_menu_id;
	int view_menu_id;
	int edit_menu_id;
	int option_menu_id;
	int show_toolbar_menu_item_id;
	int	help_menu_id;
	int	about_menu_item_id;
	int multidimensional_menu_item_id;
 	
};

struct IcsViewerWindow_
{
	UIModule        parent; 
	
	IDropTarget 	iDropTarget;
	
	int				id;
    int 			cRef;
	
	HWND      		panel_window_handle;
	HWND			canvas_window;
	HWND 			hWndStatus;
	HDC				canvas_window_hdc;
	
	HWND			canvas_palettebar;
	
	LONG_PTR  		panel_original_proc_fun_ptr;
	LONG_PTR		canvas_original_proc_fun_ptr;
	
	/* This is a reference to the freeimage before conversion and display ie maybe float image type
	 * This is mainly for functions such as the histogram */
	/* Standard display dib */  

	RGBQUAD*		current_palette;
	
	FIBITMAP*		panel_dib;
	
	FIBITMAP*		temp_dib;   
	
	HBITMAP 		hbitmap; 
	
	Manipulater3d*		importer3d;
	IcsViewerWindowMenu 	*panel_menu;
	
	// These refs are needed for plugins that depend on one another.
	ImageWindowPlugin  *zoom_tool;
	ImageWindowPlugin  *line_tool;
	ImageWindowPlugin  *roi_tool;
	ImageWindowPlugin  *palettebar_tool;
	ImageWindowPlugin  *crosshair_tool;
	ImageWindowPlugin  *linear_scale_plugin;
	ImageWindowPlugin  *palette_plugin;
	ImageWindowPlugin  *metadata_plugin;  
	ImageWindowPlugin  *save_plugin;
	ImageWindowPlugin  *profile_plugin;
	ImageWindowPlugin  *stream_plugin;

	//ProgressDialog	   *progress_dialog;
	
	int				number_of_plugins; 
	int				signal_on_exit;
	
	ListType		program_plugins;

	// Contains the history for ics files
	//ListType	 	history;

	ICS *ics;
	
	/* Lab window control id's */
	int       		panel_id;
	int				is_moving;
	int				can_close;
	int				just_display;

	int				live_mode;
	int				prevent_image_load;
	int 			cropping;

	int				about_panel_id;
	
	int				multidimension_panel_id;

	int				image_view_left;
	int 			image_view_top;
	
	int				panel_toolbar_width;
	int				panel_palettebar_status;

	double			microns_per_pixel;
	
	int				panel_window_visible;
	
	int 			binning_size;
						 
	int				load_timer;
	
	int			    lock;
	
	int				finished_loaded;
						  
	int				tool_button_top;
	
	int				no_crosshair_tool;
	int				no_roi_tool;
	
	int					binning_enabled;
	FIA_BINNING_TYPE	binning_type;
	int					binning_radius;

	CursorType 		cursor_type;
						  
	double			panel_max_pixel_value;
	double			panel_min_pixel_value;
	
	double 			last_display_time;
	double 			last_time_fps_updated; 
	
	double 			fps;
	int				fps_count;
	
	int				last_3d_dimension1_shown;
	int				last_3d_dimension2_shown;
	int 			last_3d_slice_shown;
	int				last_dimension_moved;
	int 			mutidimensional_data;
	int				interpret_3_dims_as_colour;	// Hack to multi dimension data with
										// 3 dims as colour if it does not have the corect metadata

	char 			panel_user_title[100];
	char 			panel_tmp_title[100];

	char 			default_directory[500];
	char			window_name[500];
	char			temp_dir_path[GCI_MAX_PATHNAME_LEN];
	char 			filename[GCI_MAX_PATHNAME_LEN];
	char            _unique_id[50];
	int             _unique_id_set;

	dictionary		*metadata;
};


typedef int (*METADATA_SAVE_FUNCTION) (char *ics_filepath, IcsViewerWindow *window, void* callback); 

typedef void (*APP_PROVIDED_METADATA_CALLBACK) (IcsViewerWindow* window, int panel, int ctrl, void* callback);  

typedef void (*APP_PROVIDED_SAVE_CALLBACK) (IcsViewerWindow* window, void* callback);  

/* Public Methods */

/**
 * GCI_CreateImageWindow:
 * @dest_width: Width of destination area.
 * @dest_height: Height of destination area.
 * @src_width: Width of source image.
 * @src_height: Height of source image.
 * @width: Return value for image width.
 * @height: Return value for image height.
 *
 * Computes the final dimensions of an image that is to be scaled to fit to a
 * certain size.
 **/
IW_DLL_API IcsViewerWindow * IW_DLL_CALLCONV
GCI_ImagingWindow_CreateAdvanced(const char *name, const char * title, int left, int top, int width, int height, int override, int monitor);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetPosition(IcsViewerWindow* window, int left, int top, int width, int height, int override);

IW_DLL_API IcsViewerWindow * IW_DLL_CALLCONV
GCI_ImagingWindow_CreateAdvanced2(const char *name, const char * title, int left, int top, int width, int height, int override, int monitor, int can_close);

IW_DLL_API HWND IW_DLL_CALLCONV
GCI_ImagingWindow_GetImageViewHandle(IcsViewerWindow *window);

IW_DLL_API HWND IW_DLL_CALLCONV
GCI_ImagingWindow_GetWindowHandle(IcsViewerWindow *window);

IW_DLL_API char* IW_DLL_CALLCONV
ics_viewer_get_registry_subkey(IcsViewerWindow *window, char *key);

IW_DLL_API IcsViewerWindow* IW_DLL_CALLCONV
GCI_ImagingWindow_Create(const char *name, const char * title);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetFramesPerSecondIndicator(IcsViewerWindow *window, double fps);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_Initialise(IcsViewerWindow* window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_RemoveCrosshairTool(IcsViewerWindow* window);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_LoadImageFile(IcsViewerWindow *window, char *imagePath);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetLiveStatus(IcsViewerWindow *window, int status);  

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_GetLiveStatus(IcsViewerWindow *window);

IW_DLL_API double IW_DLL_CALLCONV
GCI_ImageWindow_GetMaxPixelValueInDisplayedImage(IcsViewerWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetBinningSize(IcsViewerWindow *window, int size);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImageWindow_SetFalseColourWavelength(IcsViewerWindow *window, double wavelength);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImageWindow_SetLinearScale(IcsViewerWindow *window, int bitmode);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetLinearScaleLimits(IcsViewerWindow *window, double min, double max);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetMetaDataFromDictionary(IcsViewerWindow *window, dictionary *d);

/**
 * GCI_ImagingWindow_GetPanelID:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Gets the labwindow panel id.
 **/
IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_GetPanelID(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_GetOriginalFIB:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Gets the original freeimage bitmap diplayed in the window.
 **/
IW_DLL_API FIBITMAP * IW_DLL_CALLCONV
GCI_ImagingWindow_GetOriginalFIB( IcsViewerWindow *window);


IW_DLL_API FIBITMAP * IW_DLL_CALLCONV
GCI_ImagingWindow_GetDisplayedFIB( IcsViewerWindow *window);


/**
 * GCI_ImageWindow_SetFalseColourWavelength:
 * @window: Imaging window in which the image is to be loaded.
 * @wavelength: wavelength to use.
 *
 * Specifies the wavelength to use when constructing the false colour palette.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImageWindow_SetFalseColourWavelength(IcsViewerWindow *window, double wavelength); 


/**
 * GCI_ImagingWindow_SetPixelToMicronFactor:
 * @window: Imaging window in which the image is to be loaded.
 * @factor: scale factor.
 *
 * Specifies the factor used to convert pixels to microns.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetMicronsPerPixelFactor(IcsViewerWindow *window, double factor);


/**
 * GCI_ImagingWindow_PlaceCrossHair:
 * @window: Imaging window in which the image is to be loaded.
 * @point: point.
 *
 * Places and draws a crosshair at the specified point.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_PlaceCrossHair(IcsViewerWindow *window, const Point point);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetBackgroundColour(IcsViewerWindow *window, COLORREF colour);

/**
 * GCI_ImagingWindow_ActivateCrossHairTool:
 * @window: Imaging window in which the image is to be loaded.
 * @point: point.
 *
 * Activates the crosshair tool from code.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_ActivateCrossHairTool(IcsViewerWindow *window);

/**
 * GCI_ImagingWindow_PlaceCrossHairAtImagePoint:
 * @window: Imaging window in which the image is to be loaded.
 * @point: point.
 *
 * Places and draws a crosshair at the specified point on the original image.
 **/
IW_DLL_API  void IW_DLL_CALLCONV
GCI_ImagingWindow_PlaceCrossHairAtImagePoint(IcsViewerWindow *window, const Point point);

/**
 * GCI_ImagingWindow_GetMicronsPerPixelFactor:
 * @window: Imaging window in which the image is to be loaded..
 *
 * Gets the factor used to convert pixels to microns.
 **/
IW_DLL_API double IW_DLL_CALLCONV
GCI_ImagingWindow_GetMicronsPerPixelFactor(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_Show:
 *
 * Shows a imaging window
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_Show(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_BringToFront:
 *
 * Brings a window to the front of other windows.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_BringToFront(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_Hide:
 *
 * Hides a imaging window
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_Hide(IcsViewerWindow *window);


IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_IsVisible(IcsViewerWindow *window);

/**
 * GCI_CloseImageWindow:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Closes and cleans up an imaging window
 **/
IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_Close(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_LoadFreeImageBitmap:
 * @window: Imaging window in which the image is to be loaded.
 * @dib: FIBITMAP image to load into the imaging window.
 *
 * Load a FIBITMAP into the given imaging window.
 **/
IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_LoadImage(IcsViewerWindow *window, FIBITMAP *dib);


IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SaveImage(IcsViewerWindow *window, const char *filepath);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetDefaultDirectoryPathForAllWindows(const char *path);

/**
 * GCI_ImagingWindow_SetDefaultDirectoryPath:
 * @window: Imaging window in which the path is to be set for.
 *
 * Sets the default directory to save images too.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetDefaultDirectoryPath(IcsViewerWindow *window, const char *path);

IW_DLL_API char* IW_DLL_CALLCONV
GCI_ImagingWindow_GetDefaultDirectoryPath(IcsViewerWindow *window, char *path);


/**
 * GCI_ImagingWindow_SetWindowTitle:
 * @window: Imaging window in which the image is to be loaded.
 * @title: String to set the window title.
 *
 * Sets the windows title string.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowTitle(IcsViewerWindow *window, char *title);


/**
 * GCI_ImagingWindow_ShowToolBar:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Show the toolbar containing items such as zoom, profile and crosshair.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_ShowToolBar(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_HideToolBar:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Hide the toolbar containing items such as zoom, profile and crosshair.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_HideToolBar(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_EnableAllActions:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Enables all the tools and plugins from the menu and toolbar
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableAllActions(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_DisableAllActions:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Disables all the tools and plugins from the menu and toolbar
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableAllActions(IcsViewerWindow *window);

/**
 * GCI_ImagingWindow_HidePaletteBar:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Hide the palettebar.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_ShowPaletteBar(IcsViewerWindow *window);

/**
 * GCI_ImagingWindow_HidePaletteBar:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Hide the palettebar.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_HidePaletteBar(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_SetProfileEnable:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Enables the drawing of profiles.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableLineTool(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_DisableLineTool:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Disables the drawing of profiles.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableLineTool(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_EnableProfile:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Enables the drawing of profiles.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableProfile(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_DisableProfile:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Disables the drawing of profiles.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableProfile(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_EnableCrossHair:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Enable the drawing of the cross hair events still occur.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableCrossHair(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_EnableCrossHair:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Disable the drawing of the cross hairs from code.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableCrossHair(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_EnableZoomTool:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Enable the zoom from mouse.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableZoomTool(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_DisableZoomTool:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Disable the zoom from mouse.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableZoomTool(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_EnableRoiTool:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Enable the roi from mouse.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableRoiTool(IcsViewerWindow *window);


/**
 * GCI_ImagingWindow_PreventRoiResize:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Prevent the ROI from being resized.
 **/
int	IW_DLL_CALLCONV
GCI_ImagingWindow_PreventRoiResize(IcsViewerWindow *window);

/**
 * GCI_ImagingWindow_AllowRoiResize:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Allow the ROI to be resized.
 **/
int	IW_DLL_CALLCONV
GCI_ImagingWindow_AllowRoiResize(IcsViewerWindow *window);

/**
 * GCI_ImagingWindow_DisableRoiTool:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Disable the roi from mouse.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableRoiTool(IcsViewerWindow *window);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_LockProfileButton(IcsViewerWindow *window);     


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockProfileButton(IcsViewerWindow *window);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_LockCrossHairButton(IcsViewerWindow *window);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockCrossHairButton(IcsViewerWindow *window);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_LockRoiButton(IcsViewerWindow *window);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockRoiButton(IcsViewerWindow *window);


//int GCI_ImagingWindow_PlaceCrossHair(IcsViewerWindow *window, int x, int y); 

/**
 * GCI_ImagingWindow_SetZoomFactor:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Sets the zoom factor of the current loaded image.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetZoomFactor(IcsViewerWindow *window, double zoom);


/**
 * GCI_ImagingWindow_SetResizeFitStyle:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Resizes any loaded images to fit the size of the window.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetResizeFitStyle(IcsViewerWindow *window); 


/**
 * GCI_ImagingWindow_SetMetaDataProviderCallback:
 * @window: Imaging window in which the image is to be loaded.
 *
 * Sets the function that we be called when the metadata to be shown is needed.
 **/
IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetMetaDataProviderCallback(IcsViewerWindow *window, APP_PROVIDED_METADATA_CALLBACK callback, void *callback_data);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetSaveFileProviderCallback(IcsViewerWindow *window, APP_PROVIDED_SAVE_CALLBACK callback, void *callback_data);

IW_DLL_API void	IW_DLL_CALLCONV
GCI_ImagingWindow_RemoveRoiTool(IcsViewerWindow *window);

IW_DLL_API int	IW_DLL_CALLCONV
GCI_ImagingWindow_GetROIImageRECT(IcsViewerWindow *window, RECT *rect);

IW_DLL_API int	IW_DLL_CALLCONV
GCI_ImagingWindow_GetROICanvasRECT(IcsViewerWindow *window, RECT *rect);


IW_DLL_API void IW_DLL_CALLCONV
ImageWindow_SetPalette(IcsViewerWindow *window);    

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_GetImageBPP(IcsViewerWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_GetMaxPossibleValuesForDataType(IcsViewerWindow *window, double *min, double *max);


IW_DLL_API int	IW_DLL_CALLCONV
GCI_ImagingWindow_SetROIImageRECT(IcsViewerWindow *window, RECT *rect);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SignalOnlyOnCloseOrExit(IcsViewerWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DestroyWindow(IcsViewerWindow *window);


IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetDefaultPalette(IcsViewerWindow *window, PALETTE_TYPE palette);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetFalseColourPaletteAsDefault(IcsViewerWindow *window, double wavelength);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetPileUpPaletteAsDefault(IcsViewerWindow *window, RGBQUAD colour1, RGBQUAD colour2, RGBQUAD colour3, BYTE *sizes);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DrawText(IcsViewerWindow *window, COLORREF colour, POINT pt, char *text);


#ifdef STREAM_DEVICE_PLUGIN

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceSetFakeImagePath(IcsViewerWindow *window, const char* path);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceForce8bit(IcsViewerWindow *window, int force);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_UseStreamDeviceWithNameLike(IcsViewerWindow *window, const char *name, int count);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_UseStreamDeviceWithId(IcsViewerWindow *window, int id);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_PrintStreamDevices(IcsViewerWindow *window);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceSnap(IcsViewerWindow *window);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDevicePause(IcsViewerWindow *window);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceLive(IcsViewerWindow *window);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceStopCapture(IcsViewerWindow *window);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_UseStreamDeviceWithId(IcsViewerWindow *window, int id);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_CloseStreamDevice(IcsViewerWindow *window);

IW_DLL_API IBaseFilter* IW_DLL_CALLCONV
GCI_ImagingWindow_GetCurrentStreamDeviceCaptureFilter(IcsViewerWindow *window);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceSetFastDisplay(IcsViewerWindow *window, int enabled);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_SetFastDisplay(IcsViewerWindow *window, int fast);

IW_DLL_API DirectShowCapture* IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceDirectShowInterface(IcsViewerWindow *window);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceShowCapturePropertyPages(IcsViewerWindow *window);

#endif

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_DiableSoftwareBinning(IcsViewerWindow *window, FIA_BINNING_TYPE binning_type, int radius);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetSoftwareBinning(IcsViewerWindow *window, FIA_BINNING_TYPE binning_type, int radius);

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_EmptyMetaData(IcsViewerWindow *window);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetMetaData(IcsViewerWindow *window, dictionary *metadata);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SetMetaDataKey(IcsViewerWindow *window, const char *key, const char *value);

IW_DLL_API char* IW_DLL_CALLCONV
GCI_ImagingWindow_GetMetaDataKey(IcsViewerWindow *window, const char *key, char *value, char *def);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataDictionaryToTextFile(GCIWindow *window, dictionary* d, const char *filename);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataToTextFile(GCIWindow *window, const char *filename);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataDictionaryToIcsFilePath(GCIWindow *window, dictionary* d, const char *filepath);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataToIcsFilePath(GCIWindow *window, const char *filepath);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataDictionaryToIcsFile(GCIWindow *window, dictionary* d, ICS *ics);

IW_DLL_API void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataToIcsFile(GCIWindow *window, ICS *ics);

#endif
