#ifndef __DIRECTSHOW_CAPTURE_WRAPPER__
#define __DIRECTSHOW_CAPTURE_WRAPPER__

#include <strmif.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>

#define WM_GRAPHNOTIFY					WM_APP + 1

#define FRIENDLY_DEVICE_NAME_MAX_LEN 500

typedef struct		_DirectShowCapture DirectShowCapture;
typedef struct		_DirectShowCapture DSC;
typedef struct		_Routing Routing;
typedef struct		_Crossbar Crossbar;
typedef struct		_DscFilterInfo DscFilterInfo;

struct _DscFilterInfo
{
	DSC *dsc;
	int  filter_id;
	char friendly_name[FRIENDLY_DEVICE_NAME_MAX_LEN];
};

struct _Crossbar
{
	DSC						*dsc;
	IAMCrossbar				*pin;
	int						type;
	int						videoOutputIndex;		
	int						videoInputIndex;
	char					friendly_name[FRIENDLY_DEVICE_NAME_MAX_LEN];
};

typedef enum {DSW_CAPTURE_MODE_LIVE, DSW_CAPTURE_MODE_SNAP_CALLBACK, DSW_CAPTURE_MODE_SNAP} DSW_CAPTURE_MODE;

typedef void (*DIRECT_DRAW_FRAME_RECIEVED_HANDLER) (DSC* dsc, double SampleTime,
            BYTE *pBuffer, long BufferLen, VIDEOINFOHEADER *pVih, void *callback_data);


DSC * directshow_wrapper_create(HWND hwnd, DIRECT_DRAW_FRAME_RECIEVED_HANDLER callback_handler, void *callback_data);
HRESULT directshow_wrapper_destroy(DSC *dsc);

IBaseFilter* directshow_get_capture_filter(DSC *dsc);

HRESULT directshow_process_wndproc_event (DSC *dsc);
HRESULT directshow_enum_filters(DSC *dsc, GUID device_clsid, DscFilterInfo *dsc_filter_array, int *filter_count);
HRESULT directshow_open_device(DSC *dsc, int device_id);
HRESULT directshow_reopen_device(DSC *dsc);
HRESULT directshow_close_device(DSC *dsc);
HRESULT directshow_set_capture_filter_to_largest_resolution(DSC *dsc);

HRESULT directshow_teardown_graph(DSC *dsc, int debug);

HRESULT directshow_resize_video_window(DSC *dsc, RECT rc);

HRESULT directshow_setup_fast_live_capture(DSC *dsc, HWND window);

DSW_CAPTURE_MODE directshow_get_mode(DSC *dsc);

HRESULT directshow_setup_live_capture(DSC *dsc);
HRESULT directshow_setup_snap_capture(DSC *dsc);
HRESULT directshow_setup_save_capture_to_file(DSC *dsc, const char *filepath, int encoder_index, HWND window);

HRESULT directshow_run(DSC *dsc);
HRESULT directshow_pause(DSC *dsc);
HRESULT directshow_snap_image(DSC *dsc);

int directshow_get_capture_filter_capabilities(DSC *dsc);
HRESULT directshow_show_capture_filter_property_pages(DSC *dsc, HWND hwnd);
HRESULT directshow_show_capture_filter_pin_property_pages(DSC *dsc, HWND hwnd);
HRESULT directshow_get_capture_filter_property_page_count(DSC *dsc, int *count);
HRESULT directshow_show_capture_filter_crossbar_property_pages(DSC *dsc, int crossbar_daialog_number, HWND hwnd);
HRESULT directshow_stop_capture(DSC *dsc);
HRESULT directshow_get_buffer_image(DSC *dsc);
HRESULT directshow_crossbar_get_video_inputs(DSC *dsc, Crossbar *crossbar_array, int *crossbar_count);
HRESULT directshow_crossbar_set_video_input(DSC *dsc, Crossbar *crossbar);

#endif