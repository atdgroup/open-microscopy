#if defined(STANDALONE_APP) || defined(STREAM_DEVICE_PLUGIN)

#include "directshow_wrapper.h"

#include "ISampleGrabber.h"

#include "shellapi.h" 

#include "gci_utils.h"

#include <userint.h>
#include <utility.h>

#define DSHOW_NO_PREVIEW_PIN 262782

#ifndef offsetof
#ifndef offsetof
#define offsetof(t, mem) ((size_t) ((char *)&(((t *)8)->mem) - (char *)8)) 
#endif
#endif

#define IMPL(class, member, pointer) \
    (&((class *)8)->member == pointer, ((class *) (((long) pointer) - offsetof(class, member))))

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(iface, x) if (x) { int count =iface##_Release(x); x = NULL; /* printf("Refcount of %s = %d\n", #x, count);*/ }
#endif

typedef struct _DSCGrabberCB DSCGrabberCB;

struct _DSCGrabberCB
{
    ISampleGrabberCB igrabbercb;
	DSC *dsc;
};

struct _DirectShowCapture
{
	HWND notify_hwnd;
	IGraphBuilder *filterGraphBuilder;
	ICaptureGraphBuilder2		*captureGraph2;
	IBaseFilter	*sample_grabber_basefilter;
	IBaseFilter *captureFilter;
	IBaseFilter *null_renderer;
	IAMStreamConfig *videoCapabilities;
	IVideoWindow  * iVideoWindow;
	IMediaControl * iMediaControl;
	IMediaEventEx * iMediaEvent;
	AM_MEDIA_TYPE mt;
	AM_MEDIA_TYPE capture_filter_mt;

	DSCGrabberCB* grabber_callback;
	
	Crossbar* crossbars;

	volatile int prevent_callback;

	DSW_CAPTURE_MODE current_mode;
	int NumberOfVideoInputs;
	int no_preview_pin;
	int captureGraphBuilt;
	int device_opened;
	int image_snapped;

	DIRECT_DRAW_FRAME_RECIEVED_HANDLER	capture_handler;
	void	*callback_data;
};

int HR_failed(DSC *dsc, HRESULT hr);

static void FreeMediaType(AM_MEDIA_TYPE* mt)
{
    if (mt->cbFormat != 0) {
        CoTaskMemFree((PVOID)mt->pbFormat);

        // Strictly unnecessary but tidier
        mt->cbFormat = 0;
        mt->pbFormat = NULL;
    }
    if (mt->pUnk != NULL) {
		SAFE_RELEASE(IUnknown, mt->pUnk);
        mt->pUnk = NULL;
    }
}

static HRESULT DumpState(DSC *dsc)
{
	HRESULT hr;
	OAFilterState pfs;

	hr = IMediaControl_GetState(dsc->iMediaControl, 1000, &pfs);

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	
	
	if(pfs == State_Stopped)
		printf("State_Stopped\n");
	else if(pfs == State_Paused)
		printf("State_Paused\n");
	else if(pfs == State_Running)
		printf("State_Running\n");

	return S_OK;
}

static void DumpGraph(IFilterGraph *pGraph, DWORD dwLevel)
{
	IEnumFilters *pFilters;
	IBaseFilter *pFilter;
	FILTER_INFO	info;
	ULONG	n;

	if( !pGraph )
		return;

	printf("DumpGraph [%x]\r\n", pGraph);

	if (FAILED(IFilterGraph_EnumFilters(pGraph, &pFilters))) {
		printf("EnumFilters failed!\r\n");
	}

	while (IEnumFilters_Next(pFilters, 1, &pFilter, &n) == S_OK) {

		if (FAILED(IBaseFilter_QueryFilterInfo(pFilter, &info))) {
			printf("    Filter [%p]  -- failed QueryFilterInfo\r\n", pFilter);
		}
		else {

			IEnumPins *pins;
			IPin *pPin;

			if (info.pGraph) {
				SAFE_RELEASE(IFilterGraph, info.pGraph);
			}
			
			// !!! should QueryVendorInfo here!
			printf("    Filter [%p]  '%ls'\r\n", pFilter, info.achName);

			if (FAILED(IBaseFilter_EnumPins(pFilter, &pins))) {
				printf("EnumPins failed!\r\n");
			} 
			else {

				while (IEnumPins_Next(pins, 1, &pPin, &n) == S_OK) {

					PIN_INFO	pinInfo;

					if (FAILED(IPin_QueryPinInfo(pPin, &pinInfo))) {
						printf("          Pin [%x]  -- failed QueryPinInfo\r\n", pPin);
					} 
					else {

						HRESULT hr;
						IPin *pPinConnected = NULL;

						if (pinInfo.pFilter) {
							SAFE_RELEASE(IBaseFilter, pinInfo.pFilter);
						}

						hr = IPin_ConnectedTo(pPin, &pPinConnected);

						if (pPinConnected) {

							printf("          Pin [%p]  '%ls' [%sput]  Connected to pin [%p]\r\n", pPin, pinInfo.achName,
								pinInfo.dir == PINDIR_INPUT ? "In" : "Out",
								pPinConnected);

							SAFE_RELEASE(IPin, pPinConnected);
						
							// perhaps we should really dump the type both ways as a sanity
							// check?
							if (pinInfo.dir == PINDIR_OUTPUT) {

								AM_MEDIA_TYPE mt;

								hr = IPin_ConnectionMediaType(pPin, &mt);

								if (SUCCEEDED(hr)) {
									FreeMediaType(&mt);
								}
							}
						}	
						else {

							printf("          Pin [%x]  '%ls' [%sput]\r\n", pPin, pinInfo.achName,
								pinInfo.dir == PINDIR_INPUT ? "In" : "Out");

						}
					}

					SAFE_RELEASE(IPin, pPin);
				}

				SAFE_RELEASE(IEnumPins, pins);
			}
		}

		SAFE_RELEASE(IBaseFilter, pFilter);
	}

	SAFE_RELEASE(IEnumFilters, pFilters);
}

int HR_failed(DSC *dsc, HRESULT hr)
{
	char szErr[1000] = "";
	DWORD res = AMGetErrorText(hr, szErr, 1000);

	//if(hr == S_OK)
	//	return SOK;

	// Completed without error, but only partial results were obtained.
	//if(hr == S_FALSE)
	//	return S_FALSE;

	if(hr != NOERROR)
	{
		if(hr & 0x80000000) {
			
			// Here we have a real error so we report it
			//MessageBox(0, szErr, "DirectShow Error", MB_OK | MB_ICONERROR);
			printf("DirectShow Error %s\n", szErr);
			//DumpGraph((IFilterGraph*) dsc->filterGraphBuilder, 0);

			return hr;
		}
	}

	return S_OK;
}

static HRESULT __stdcall DSCGrabberCBSampleCB (ISampleGrabberCB *pgrabbercb, double SampleTime,
            IMediaSample *pSample)
{
	DSCGrabberCB *this = IMPL (DSCGrabberCB, igrabbercb, pgrabbercb);
	DSC *dsc = this->dsc;
	BYTE *pBuffer;
	long BufferLen;
	VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*) dsc->mt.pbFormat;

	if(dsc->prevent_callback == 1)
		return S_FALSE;

	BufferLen = IMediaSample_GetSize(pSample);
	IMediaSample_GetPointer(pSample, &pBuffer);

	if(dsc->capture_handler != NULL)
		dsc->capture_handler(dsc, SampleTime, pBuffer, BufferLen, pVih, dsc->callback_data);

	dsc->image_snapped = 1;

	return S_OK;
}
        
static HRESULT __stdcall DSCGrabberCBBufferCB (ISampleGrabberCB *pgrabbercb, double SampleTime,
            BYTE *pBuffer, long BufferLen)
{
	DSCGrabberCB *this = IMPL (DSCGrabberCB, igrabbercb, pgrabbercb);
	DSC *dsc = this->dsc;
	VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*) dsc->mt.pbFormat;

	if(dsc->prevent_callback == 1)
		return S_FALSE;

	if(dsc->capture_handler != NULL)
		dsc->capture_handler(dsc, SampleTime, pBuffer, BufferLen, pVih, dsc->callback_data);

	dsc->image_snapped = 1;

	return S_OK;
}

static HRESULT __stdcall DSCGrabberCBQueryInterface (ISampleGrabberCB *pgrabbercb,
															  REFIID riid, void **ppv)
{
    DSCGrabberCB *this = IMPL (DSCGrabberCB, igrabbercb, pgrabbercb);

	*ppv = (ISampleGrabberCB**) this; 
    return S_OK; 
}

static ULONG __stdcall DSCGrabberCBAddRef (ISampleGrabberCB *pgrabbercb)
{
    return 2;
}

static ULONG __stdcall DSCGrabberCBRelease (ISampleGrabberCB *pgrabbercb)
{
    return 1;
}

static ISampleGrabberCBVtbl vtblSampleGrabberCB =
{
    DSCGrabberCBQueryInterface,
	DSCGrabberCBAddRef,
	DSCGrabberCBRelease,
    DSCGrabberCBSampleCB,
    DSCGrabberCBBufferCB
};

DSCGrabberCB* CreateDSCGrabberCB (DSC *dsc)
{
    DSCGrabberCB *this = (DSCGrabberCB*) malloc(sizeof(DSCGrabberCB));

	this->igrabbercb.lpVtbl = &vtblSampleGrabberCB;
	this->dsc = dsc;

	return this;
}


HRESULT directshow_process_wndproc_event (DSC *dsc)
{
	long evCode, param1, param2;
	HRESULT hr;

	if(dsc->device_opened == 0)
		return S_OK;

	while (hr = IMediaEvent_GetEvent(dsc->iMediaEvent, &evCode, &param1, &param2, 0), SUCCEEDED(hr))
	{
		hr = IMediaEvent_FreeEventParams(dsc->iMediaEvent, evCode, param1, param2);

		if ((EC_COMPLETE == evCode) || (EC_USERABORT == evCode))
		{ 
			//CleanUp();
			break;
		} 
	}

	return S_OK;
}

HRESULT directshow_wait_for_complete_event (DSC *dsc)
{
	HANDLE  hEvent; 
	long    evCode, param1, param2;
	int bDone = 0;
	HRESULT hr = S_OK;

	//long evCode, param1, param2;
	//HRESULT hr;

	if(dsc->device_opened == 0)
		return S_OK;

	//hr = IMediaEvent_GetEvent(dsc->iMediaEvent, &evCode, &param1, &param2, 2000);
	/*
	while (1)
	{
		//hr = IMediaEvent_FreeEventParams(dsc->iMediaEvent, evCode, param1, param2);
		hr = IMediaEvent_GetEvent(dsc->iMediaEvent, &evCode, &param1, &param2, 0.2);

		if ((EC_COMPLETE == evCode) || (EC_USERABORT == evCode))
		{ 
			printf("Recieved EC_COMPLETE or EC_USERABORT\n");
			//CleanUp();
			break;
		} 	
	}


	printf("Recieved %d\n", hr);
	*/



	

	hr = IMediaEvent_GetEventHandle(dsc->iMediaEvent, (OAEVENT*)&hEvent);

	if (FAILED(hr))
	{
		// Insert failure-handling code here. 
	}

	while(!bDone) 
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100))
		{ 
			while (hr = IMediaEvent_GetEvent(dsc->iMediaEvent, &evCode, &param1, &param2, 0), SUCCEEDED(hr)) 
			{
				printf("Event code: %#04x\n Params: %d, %d\n", evCode, param1, param2);

				if ((EC_COMPLETE == evCode) || (EC_USERABORT == evCode))
				{ 
					printf("Recieved EC_COMPLETE or EC_USERABORT\n");
					bDone = 1;
				} 	

				IMediaEvent_FreeEventParams(dsc->iMediaEvent, evCode, param1, param2);
			}
		}
	} 




	return S_OK;
}

DSC * directshow_wrapper_create(HWND hwnd, DIRECT_DRAW_FRAME_RECIEVED_HANDLER callback_handler, void *callback_data)
{
	HRESULT hr;
	DSC *dsc = (DSC*) malloc(sizeof(DSC));

	memset(dsc, 0, sizeof(DSC));

	dsc->notify_hwnd = hwnd;
	dsc->device_opened = 0;
	dsc->current_mode = -1;
	dsc->grabber_callback = CreateDSCGrabberCB (dsc);
	dsc->callback_data = callback_data;
	dsc->capture_handler = callback_handler;
	dsc->no_preview_pin = 0;
	dsc->captureGraphBuilt = 0;

	// Get the interface for DirectShow's filterGraph
    hr=CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC, 
                         &IID_IGraphBuilder, (void **)&(dsc->filterGraphBuilder));

	if(HR_failed(dsc, hr) != NOERROR)
		return NULL;

	hr = CoCreateInstance(&CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
				 &IID_ICaptureGraphBuilder2, (void **)&(dsc->captureGraph2)); 
	
	if(HR_failed(dsc, hr) != NOERROR)
		return NULL;

	hr = CoCreateInstance (&CLSID_NullRenderer , NULL, CLSCTX_INPROC_SERVER,
				&IID_IBaseFilter, (void **) &(dsc->null_renderer));

	if(HR_failed(dsc, hr) != NOERROR)
		return NULL;

	// Obtain interfaces for media control and Video Window
	hr = IGraphBuilder_QueryInterface(dsc->filterGraphBuilder,
		&IID_IMediaControl,(LPVOID *) &(dsc->iMediaControl));
	
	if(HR_failed(dsc, hr) != NOERROR)
		return NULL;	

	hr = IGraphBuilder_QueryInterface(dsc->filterGraphBuilder,
		&IID_IMediaEventEx,(LPVOID *) &(dsc->iMediaEvent));
	
	if(HR_failed(dsc, hr) != NOERROR)
		return NULL;	

	//hr = IMediaEventEx_SetNotifyWindow(dsc->iMediaEvent, (OAHWND) hwnd, WM_GRAPHNOTIFY, 0);
	
	//if(HR_failed(dsc, hr) != NOERROR)
	//	return NULL;	

	// For bitmap get snaps
	 
	hr = CoCreateInstance(&CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                        &IID_IBaseFilter, (LPVOID *)&(dsc->sample_grabber_basefilter));

	if(HR_failed(dsc, hr) != NOERROR)
		return NULL;	

	hr = ICaptureGraphBuilder2_SetFiltergraph(dsc->captureGraph2, dsc->filterGraphBuilder);

	if(HR_failed(dsc, hr) != NOERROR)
		return NULL;	

	return dsc;
}

IBaseFilter* directshow_get_capture_filter(DSC *dsc)
{
	return dsc->captureFilter;
}

HRESULT directshow_wrapper_destroy(DSC *dsc)
{
	directshow_stop_capture(dsc);

	directshow_close_device(dsc);

    IGraphBuilder_RemoveFilter(dsc->filterGraphBuilder, dsc->captureFilter);
                     
	SAFE_RELEASE(IBaseFilter, dsc->captureFilter);

	free(dsc->crossbars);
	dsc->crossbars = NULL;

	free(dsc->grabber_callback);
	dsc->grabber_callback = NULL;

	free(dsc);

	return S_OK;
}

static HRESULT directshow_bind_device(DSC *dsc, GUID device_clsid, int device_id, IBaseFilter **pFilter)
{
	int id = 0;
	char str[2048] = "";
	ULONG cFetched;
	ICreateDevEnum *pCreateDevEnum = NULL;
	IEnumMoniker *pEm = NULL;
	IMoniker *pM = NULL;
	IPropertyBag *pBag = NULL;

    HRESULT hr = CoCreateInstance(&CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  &IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    if (hr != NOERROR)
		return HR_failed(dsc, hr);

	hr = ICreateDevEnum_CreateClassEnumerator(pCreateDevEnum, &device_clsid, &pEm, 0);

    if (hr != NOERROR) 
		return HR_failed(dsc, hr);

	IEnumMoniker_Reset(pEm);
	
    while((hr = IEnumMoniker_Next(pEm, 1, &pM, &cFetched)) == S_OK)
    {	
		hr = IMoniker_BindToStorage(pM, 0, 0, &IID_IPropertyBag, (void **)&pBag);

		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = IPropertyBag_Read(pBag, L"FriendlyName", &var, NULL);

			if (hr == NOERROR) 
			{		
				id++;

				if(id == device_id) {
					hr = IMoniker_BindToObject(pM, 0, 0, &IID_IBaseFilter, (void **)pFilter);
				}

				SysFreeString(var.bstrVal);
			}

			SAFE_RELEASE(IPropertyBag, pBag);
		}
		else {

			return HR_failed(dsc, hr);
		}

		SAFE_RELEASE(IMoniker, pM);
    }

	SAFE_RELEASE(IEnumMoniker, pEm);
	SAFE_RELEASE(ICreateDevEnum, pCreateDevEnum);

	return id;
}


static HRESULT directshow_get_codec_formats(DSC *dsc, IBaseFilter *baseFilter)
{
	HRESULT			hr;
	IAMStreamConfig	*streamConfig = NULL;
	IEnumPins		*pins=0;
	ULONG			n;
	PIN_INFO		pinInfo;
	IPin			*pP	= NULL;
	VIDEOINFOHEADER *pVih;

    if(SUCCEEDED(IBaseFilter_EnumPins(baseFilter, &pins)))
    {
        while((S_OK == IEnumPins_Next(pins, 1, &pP, &n)))
		{
            if(S_OK == IPin_QueryPinInfo(pP, &pinInfo))
            {
                if(pinInfo.dir == PINDIR_OUTPUT)
                {
					// Retrieve the IAMStreamConfig
					hr = IBaseFilter_QueryInterface(pP, &IID_IAMStreamConfig, (void**)&streamConfig);

					if (SUCCEEDED(hr)) {

						int i, nCount = 0, nSize = 0;

						// Get the number of media types
						hr = IAMStreamConfig_GetNumberOfCapabilities(streamConfig, &nCount, &nSize);

						if(HR_failed(dsc, hr) != NOERROR)
							return hr;	

						for (i=0; i<nCount; i++) {

							AM_MEDIA_TYPE *mediaType;
							VIDEO_STREAM_CONFIG_CAPS scc;
					
							// Enumerate the media types; media type and the corresponding capability structure returned
							hr = IAMStreamConfig_GetStreamCaps(streamConfig, i, &mediaType, (BYTE*)&scc);

							if(SUCCEEDED(hr)) {

								pVih = (VIDEOINFOHEADER*) mediaType->pbFormat;

								//StringFromIID(&(mediaType->subtype), &friendly_name);

								//printf("Width %d Height %d subtype %s\n", pVih->bmiHeader.biWidth, pVih->bmiHeader.biHeight,
								//		friendly_name);	
							}		
						}	

						SAFE_RELEASE(IAMStreamConfig, streamConfig);
					}
				}
			}

			SAFE_RELEASE(IPin, pP);
		}

		SAFE_RELEASE(IEnumPins, pins);
	}

	return S_OK;
}


HRESULT directshow_enum_filters(DSC *dsc, GUID device_clsid, DscFilterInfo *dsc_filter_array, int *filter_count)
{
	int id = 0;
	char str[FRIENDLY_DEVICE_NAME_MAX_LEN] = "";
	ULONG cFetched;
	ICreateDevEnum *pCreateDevEnum = NULL;
	IEnumMoniker *pEm = NULL;
	IMoniker *pM = NULL;
	IPropertyBag *pBag = NULL;
	DscFilterInfo *filter;
	HRESULT hr;

	*filter_count = 0;

    hr = CoCreateInstance(&CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  &IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    if (hr != NOERROR)
	{
		return HR_failed(dsc, hr);
	}

	hr = ICreateDevEnum_CreateClassEnumerator(pCreateDevEnum, &device_clsid, &pEm, 0);

    if (hr != NOERROR) 
	{
		return HR_failed(dsc, hr);
    }

	IEnumMoniker_Reset(pEm);
	
	if(dsc_filter_array == NULL) {

		// Return number of filters only
		while((hr = IEnumMoniker_Next(pEm, 1, &pM, &cFetched)) == S_OK) {

			if (hr != NOERROR) {
				goto FINISHED;
			}

			(*filter_count)++;
		}

		return S_OK;
	}

    while((hr = IEnumMoniker_Next(pEm, 1, &pM, &cFetched)) == S_OK)
    {	
		hr = IMoniker_BindToStorage(pM, 0, 0, &IID_IPropertyBag, (void **)&pBag);
	
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = IPropertyBag_Read(pBag, L"FriendlyName", &var, NULL);

			if (hr == NOERROR) 
			{		
				filter = &dsc_filter_array[id]; 
				(*filter_count)++;
				id++;

				WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, str, FRIENDLY_DEVICE_NAME_MAX_LEN - 1, NULL, NULL);
				
				filter->dsc = dsc;
				filter->filter_id = id;
				strncpy(filter->friendly_name, str, FRIENDLY_DEVICE_NAME_MAX_LEN);

				SysFreeString(var.bstrVal);
			}

			SAFE_RELEASE(IPropertyBag, pBag);
		}
		else {

			return HR_failed(dsc, hr);
		}

		SAFE_RELEASE(IMoniker, pM);
    }

FINISHED:

	SAFE_RELEASE(IEnumMoniker, pEm);
	SAFE_RELEASE(ICreateDevEnum, pCreateDevEnum);

	return id;
}

// Tear down everything downstream of a given filter
static void NukeDownstream(DSC *dsc, IBaseFilter *pf, int debug)
{
    IPin *pP=0, *pTo=0;
    ULONG u;
    IEnumPins *pins = NULL;
    PIN_INFO pininfo;
	HRESULT hr;
	FILTER_INFO info, info2;

    if (!pf)
        return;

	if (debug && SUCCEEDED(IBaseFilter_QueryFilterInfo(pf, &info))) {
			printf("\n\nNuking\nTearDown Filter [%p]  '%ls'\r\n", pf, info.achName);
	}
	
    hr = IBaseFilter_EnumPins(pf, &pins);
    IEnumPins_Reset(pins);

    while(hr == NOERROR)
    {
        hr = IEnumPins_Next(pins, 1, &pP, &u);

        if(hr == S_OK && pP)
		{
            IPin_ConnectedTo(pP, &pTo);

            if(pTo)
            {
                hr = IPin_QueryPinInfo(pTo, &pininfo);

                if(hr == NOERROR)
                {
					if(debug) {
					
						if(SUCCEEDED(IBaseFilter_QueryFilterInfo(pininfo.pFilter, &info2))) {
							printf(" -- TearDown Filter [%p]  '%ls' connected to pin '%ls' of filter '%ls'\r\n",
								pf, info.achName, pininfo.achName, pininfo.pFilter);
						}
					}

                    if(pininfo.dir == PINDIR_INPUT)
                    {
						if(debug)
							printf(" -- '%ls' is an input pin -- Looking down stream\r\n", pininfo.achName);

                        NukeDownstream(dsc, pininfo.pFilter, debug);
                        IGraphBuilder_Disconnect(dsc->filterGraphBuilder, pTo);
                        IGraphBuilder_Disconnect(dsc->filterGraphBuilder, pP);
                        IGraphBuilder_RemoveFilter(dsc->filterGraphBuilder, pininfo.pFilter);
                    }
                    
					SAFE_RELEASE(IBaseFilter, pininfo.pFilter);
                }

				SAFE_RELEASE(IPin, pTo);
			}
            
			SAFE_RELEASE(IPin, pP);
        }
    }

	if(pins) {
		SAFE_RELEASE(IEnumPins, pins);
	}
}

// Tear down everything downstream of the capture filters, so we can build
// a different capture graph.  Notice that we never destroy the capture filters
// and WDM filters upstream of them, because then all the capture settings
// we've set would be lost.
//
HRESULT directshow_teardown_graph(DSC *dsc, int debug)
{
    // destroy the graph downstream of our capture filter
	if(dsc->captureFilter) {
        NukeDownstream(dsc, dsc->captureFilter, debug);
      //  gcap.pBuilder->ReleaseFilters();
	}

    dsc->captureGraphBuilt = 0;

	return S_OK;
}

HRESULT directshow_open_device(DSC *dsc, int device_id)
{
	HRESULT hr;

	if(dsc->device_opened)
		return S_FALSE;

	// Bind Capture Device
	directshow_bind_device(dsc, CLSID_VideoInputDeviceCategory, device_id, &(dsc->captureFilter));
	
	if(dsc->captureFilter == NULL)
		return S_FALSE;

	// Add Capture filter to our graph.
	hr = IFilterGraph_AddFilter(dsc->filterGraphBuilder, dsc->captureFilter, L"Video Capture");

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	// Get Video Capabilities
	hr = ICaptureGraphBuilder2_FindInterface(dsc->captureGraph2,
			&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video, dsc->captureFilter,
			&IID_IAMStreamConfig,(void **)&dsc->videoCapabilities);

	if(hr != NOERROR) {
		hr = ICaptureGraphBuilder2_FindInterface(dsc->captureGraph2,
				&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video ,dsc->captureFilter,
				&IID_IAMStreamConfig, (void **)&dsc->videoCapabilities);
	}
		
	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	dsc->device_opened = device_id;

	directshow_set_capture_filter_to_largest_resolution(dsc);

	return S_OK;
}

HRESULT directshow_reopen_device(DSC *dsc)
{
	int device_id = dsc->device_opened;

	directshow_close_device(dsc);

	return directshow_open_device(dsc, device_id);
}

HRESULT directshow_close_device(DSC *dsc)
{
	if(dsc->device_opened == 0)
		return S_OK;

	directshow_stop_capture(dsc);
	directshow_teardown_graph(dsc, 0);

	IGraphBuilder_RemoveFilter(dsc->filterGraphBuilder, dsc->captureFilter);

	//printf("\nClosing Device\n");
	//printf("\nTearDown Start\n");
	//DumpGraph(dsc->filterGraphBuilder, 0);
	//printf("TearDown Done\n");

	dsc->device_opened = 0;

	SAFE_RELEASE(IAMStreamConfig, dsc->videoCapabilities);
	SAFE_RELEASE(IBaseFilter, dsc->captureFilter);

	return S_OK;
}

DSW_CAPTURE_MODE directshow_get_mode(DSC *dsc)
{
	return dsc->current_mode;
}

static HRESULT directshow_setup_bitmap_capture_adv(DSC *dsc, DSW_CAPTURE_MODE mode)
{
	HRESULT hr;
	ISampleGrabber	*sample_grabber;
	IPin *output_pin	= NULL;
	AM_MEDIA_TYPE sample_grabber_mt;

	if(!dsc->device_opened)
		return S_FALSE;

	//if(dsc->current_mode == mode)
	//	return S_OK;

	SAFE_RELEASE(IVideoWindow, dsc->iVideoWindow);

	directshow_stop_capture(dsc);
	directshow_teardown_graph(dsc, 1);

	printf("\n\nTearDown Done\n");
	DumpGraph((IFilterGraph *) dsc->filterGraphBuilder, 0);

	hr = IBaseFilter_QueryInterface(dsc->sample_grabber_basefilter, &IID_ISampleGrabber,
		 (void**)&sample_grabber);

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

//	hr = IFilterGraph_AddFilter(dsc->filterGraphBuilder,dsc->null_renderer, L"Null Renderer");

//	if(HR_failed(dsc, hr) != NOERROR)
//		return hr;

	// Add Capture filter to our graph.
	hr = IFilterGraph_AddFilter(dsc->filterGraphBuilder,
			dsc->sample_grabber_basefilter, L"Sample Grabber");

	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;

	// Add Capture filter to our graph.
	if(mode == DSW_CAPTURE_MODE_SNAP_CALLBACK) {

		hr = ISampleGrabber_SetBufferSamples(sample_grabber, FALSE);

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;

		hr = ISampleGrabber_SetOneShot(sample_grabber, TRUE);

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;

		hr = ISampleGrabber_SetCallback(sample_grabber,
			(ISampleGrabberCB*) dsc->grabber_callback, 1);

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;
	}
	else if(mode == DSW_CAPTURE_MODE_SNAP)
	{
		hr = ISampleGrabber_SetBufferSamples(sample_grabber, TRUE);

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;

		hr = ISampleGrabber_SetOneShot(sample_grabber, TRUE);

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;
	}
	else if(mode == DSW_CAPTURE_MODE_LIVE) {

		hr = ISampleGrabber_SetBufferSamples(sample_grabber, FALSE);

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;

		hr = ISampleGrabber_SetOneShot(sample_grabber, FALSE);

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;

		hr = ISampleGrabber_SetCallback(sample_grabber,
			(ISampleGrabberCB*) dsc->grabber_callback, 1);

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;
	}
	
	// Try to set the mt for the sample grabber to 24 bit
	sample_grabber_mt = dsc->capture_filter_mt;
	sample_grabber_mt.majortype = MEDIATYPE_Video;
	sample_grabber_mt.subtype = MEDIASUBTYPE_RGB24;
	sample_grabber_mt.formattype = FORMAT_VideoInfo; 

	hr = ISampleGrabber_SetMediaType(sample_grabber, &sample_grabber_mt);

	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;

	// Render the preview pin on the video capture filter
	// Use this instead of g_pGraph->RenderFile
	hr = ICaptureGraphBuilder2_RenderStream (dsc->captureGraph2, 
			&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
            (IUnknown*) dsc->captureFilter, NULL, dsc->sample_grabber_basefilter);

			//VFW_S_NOPREVIEWPIN
	if(hr == DSHOW_NO_PREVIEW_PIN) {
		dsc->no_preview_pin = 1;
	}
	else {

		if(HR_failed(dsc, hr) != NOERROR)
			goto Error;
	}

	hr = ISampleGrabber_GetConnectedMediaType(sample_grabber, &(dsc->mt));

	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;

	//DumpGraph((IFilterGraph*) dsc->filterGraphBuilder, 0);

	SAFE_RELEASE(ISampleGrabber, sample_grabber);

	dsc->captureGraphBuilt = 1;
	dsc->prevent_callback = 0;

	dsc->current_mode = mode;

	return S_OK;

Error:

	dsc->current_mode = -1;

	SAFE_RELEASE(ISampleGrabber, sample_grabber);

	return hr;
}

HRESULT directshow_run(DSC *dsc)
{
	HRESULT hr = S_OK;

	DumpState(dsc);

	hr = IMediaControl_Run(dsc->iMediaControl);

    if(HR_failed(dsc, hr) != NOERROR)
		return hr;

	DumpState(dsc);
	DumpGraph((IFilterGraph*) dsc->filterGraphBuilder, 0);

	return S_OK;
}

HRESULT directshow_pause(DSC *dsc)
{
	HRESULT hr = S_OK;

	hr = IMediaControl_Pause(dsc->iMediaControl);

    if(HR_failed(dsc, hr) != NOERROR)
		return hr;

	DumpState(dsc);
	DumpGraph((IFilterGraph*) dsc->filterGraphBuilder, 0);

	return S_OK;
}

/*
HRESULT directshow_stop(DSC *dsc)
{
	HRESULT hr = S_OK;

	hr = IMediaControl_Stop(dsc->iMediaControl);

    if(HR_failed(dsc, hr) != NOERROR)
		return hr;

	DumpState(dsc);
	DumpGraph((IFilterGraph*) dsc->filterGraphBuilder, 0);

	return S_OK;
}
*/

HRESULT directshow_get_buffer_image(DSC *dsc)
{
	ISampleGrabber	*sample_grabber;
	long nBufferSize = 0;
	long *buffer = NULL;
	VIDEOINFOHEADER *pVih;

	HRESULT hr = IBaseFilter_QueryInterface(dsc->sample_grabber_basefilter, &IID_ISampleGrabber,
		 (void**)&sample_grabber);

	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;

	directshow_setup_bitmap_capture_adv(dsc, DSW_CAPTURE_MODE_SNAP);

	directshow_run(dsc);

	/*
	{
		double start_time = Timer();

		while((Timer() - start_time) < 5.0)
			DumpState(dsc);
	}
	*/

//	hr = IMediaEventEx_WaitForCompletion(dsc->iMediaEvent, INFINITE, &evCode);

//	if(HR_failed(dsc, hr) != NOERROR)
//		goto Error;

	

	hr = ISampleGrabber_GetCurrentBuffer(sample_grabber, &nBufferSize, NULL);

	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;

	buffer = (long*) malloc(sizeof(long) * nBufferSize);

	hr = ISampleGrabber_GetCurrentBuffer(sample_grabber, &nBufferSize, buffer);

	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;

	// Examine the format block.
	if (InlineIsEqualGUID(&(dsc->mt.formattype), &FORMAT_VideoInfo) && 
		(dsc->mt.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
		(dsc->mt.pbFormat != NULL) ) 
	{
		pVih = (VIDEOINFOHEADER*) dsc->mt.pbFormat;
	}
	else 
	{
		return VFW_E_INVALIDMEDIATYPE; 
	}

	if(dsc->capture_handler != NULL)
		dsc->capture_handler(dsc, 0.0, (BYTE*) buffer, nBufferSize, pVih, dsc->callback_data);

	free(buffer);

	SAFE_RELEASE(ISampleGrabber, sample_grabber);

	return S_OK;

Error:

	SAFE_RELEASE(ISampleGrabber, sample_grabber);

	return hr;
}


HRESULT directshow_setup_live_capture(DSC *dsc)
{
	return directshow_setup_bitmap_capture_adv(dsc, DSW_CAPTURE_MODE_LIVE);

	//DumpGraph(dsc->filterGraphBuilder, 0);
}

HRESULT directshow_setup_snap_capture(DSC *dsc)
{
	HRESULT hr;

	hr = directshow_setup_bitmap_capture_adv(dsc, DSW_CAPTURE_MODE_SNAP_CALLBACK);

	//DumpGraph(dsc->filterGraphBuilder, 0);

	return S_OK;
}

HRESULT directshow_snap_image(DSC *dsc)
{
	double start_time = 0.0;

	if(!dsc->device_opened)
		return S_FALSE;

	//directshow_pause(dsc);
	directshow_stop_capture(dsc);

	dsc->image_snapped = 0;

	directshow_setup_bitmap_capture_adv(dsc, DSW_CAPTURE_MODE_SNAP);

	directshow_run(dsc);


	/*
	// Begin the operation and continue until it is complete 
// or until the user clicks the mouse or presses a key. 
 
fDone = FALSE; 
while (!fDone) 
{ 
    fDone = DoLengthyOperation(); // application-defined function 
 
    // Remove any messages that may be in the queue. If the 
    // queue contains any mouse or keyboard 
    // messages, end the operation. 
 
    while (PeekMessage(&msg, NULL,  0, 0, PM_NOREMOVE)) 
    { 
        switch(msg.message) 
        { 
            case WM_LBUTTONDOWN: 
            case WM_RBUTTONDOWN: 
            case WM_KEYDOWN: 
                // 
                // Perform any required cleanup. 
                // 
                fDone = TRUE; 
        } 
    } 
} 

*/

	/*
	pMediaControl->Stop();

long evCode;
pEvent->WaitForCompletion(INFINITE, &evCode);
*/

	//directshow_wait_for_complete_event (dsc);

	//DumpState(dsc);

	//directshow_wait_for_complete_event (dsc);

	start_time = Timer();

//	while(dsc->image_snapped == 0 && ((Timer() - start_time) < 3.0)) {

//		ProcessSystemEvents();
//		Delay(0.005);
//	}

	DumpState(dsc);
//	printf("ggg\n");
	//directshow_stop_capture(dsc);

	//{
	//	double start_time = Timer();

	//	while((Timer() - start_time) < 5.0)
	//		DumpState(dsc);
	//}

//	DumpState(dsc);

	/*
	printf("Here\n");
		DumpState(dsc);	

	hr = IMediaEventEx_WaitForCompletion(dsc->iMediaEvent, INFINITE, &evCode);

	if(HR_failed(dsc, hr) != NOERROR) {
		//printf("Here\n");
		DumpState(dsc);	
		return S_FALSE;
	}
*/

	//DumpGraph(dsc->filterGraphBuilder, 0);

	return S_OK;
}

HRESULT directshow_resize_video_window(DSC *dsc, RECT rc)
{
	int hr;

    // Resize the video preview window to match owner window size
	if(dsc->iVideoWindow == NULL)
		return S_FALSE;

	hr = IVideoWindow_SetWindowPosition(dsc->iVideoWindow, 0, 0, rc.right, rc.bottom);
    
	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	return S_FALSE;
}

HRESULT directshow_stop_capture(DSC *dsc)
{
	IMediaControl *pMC = NULL;
	HRESULT hr;
	OAFilterState pfs;

	dsc->prevent_callback = 1;
	
    // Stop the graph   
    hr = IMediaControl_QueryInterface(dsc->filterGraphBuilder,
		&IID_IMediaControl, (void **)&pMC);
    
	DumpGraph((IFilterGraph *) dsc->filterGraphBuilder, 0);

	if(SUCCEEDED(hr))
    {
		hr = IMediaControl_GetState(dsc->iMediaControl, 1000, &pfs);

		if(pfs == State_Running) {
			hr = IMediaControl_Stop(pMC);
			//hr = IMediaControl_StopWhenReady(pMC);

			if(HR_failed(dsc, hr) != NOERROR)
				return hr;		
		}

		SAFE_RELEASE(IMediaControl, pMC);
    }
	else
    {
        GCI_MessagePopup("DirectShow Error", "Error %x: Cannot stop preview graph", hr);
        return FALSE;
    }

	directshow_teardown_graph(dsc, 0);

	return S_OK;
}

HRESULT directshow_setup_fast_live_capture(DSC *dsc, HWND window)
{
	HRESULT hr;

	RECT rc;

	directshow_stop_capture(dsc);
	directshow_teardown_graph(dsc, 0);

	// Render the preview pin on the video capture filter
	// Use this instead of g_pGraph->RenderFile
	hr = ICaptureGraphBuilder2_RenderStream (dsc->captureGraph2, 
				&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                (IUnknown*) dsc->captureFilter, NULL, NULL);

	if(hr == DSHOW_NO_PREVIEW_PIN) {
		dsc->no_preview_pin = 1;
	}
	else {

		if(HR_failed(dsc, hr) != NOERROR)
			return hr;	
	}

	hr = IGraphBuilder_QueryInterface(dsc->filterGraphBuilder,
		&IID_IVideoWindow,(LPVOID *) &(dsc->iVideoWindow));
	
	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	


	// Set the video window to be a child of the main window
	hr = IVideoWindow_put_Owner(dsc->iVideoWindow, (OAHWND) window);
	
	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;
    
    // Set video window style
    hr = IVideoWindow_put_WindowStyle(dsc->iVideoWindow, WS_CHILD | WS_CLIPCHILDREN);
  
	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;	

	 // Make the preview video fill our window
    GetClientRect(window, &rc);

	directshow_resize_video_window(dsc, rc);

	hr = IVideoWindow_put_Visible(dsc->iVideoWindow, OATRUE);

	if(HR_failed(dsc, hr) != NOERROR)
		goto Error;

	dsc->captureGraphBuilt = 1;
	dsc->prevent_callback = 0;

	return S_OK;

Error:

	return hr;
}


HRESULT directshow_setup_save_capture_to_file(DSC *dsc, const char *filepath, int encoder_index, HWND window)
{
	HRESULT hr;
	IBaseFilter *pMux = NULL;
	IBaseFilter *pEncoder = NULL;
	int len;
	BSTR unicodestr;

	directshow_setup_fast_live_capture(dsc, window);
	 
	if(encoder_index > 0) {

		// Create the encoder filter (not shown). 
		directshow_bind_device(dsc, CLSID_VideoCompressorCategory, encoder_index, &pEncoder);
	
		//directshow_get_codec_formats(dsc, pEncoder);

		// Add it to the filter graph.
		hr = IFilterGraph_AddFilter(dsc->filterGraphBuilder, pEncoder, (LPCWSTR) "Encoder");

		if(HR_failed(dsc, hr) != NOERROR)
			return hr;	
	}
	
	len = MultiByteToWideChar(CP_ACP, 0, filepath, strlen(filepath), 0, 0);

	if (len > 0)
	{
		// Check whether conversion was successful
		unicodestr = SysAllocStringLen(0, len);
		MultiByteToWideChar(CP_ACP, 0, filepath, strlen(filepath), unicodestr, len);
	}
	else
	{
		GCI_MessagePopup("Error", "Could not convert ansi string to unicode");
		goto Error;
	}

	hr = ICaptureGraphBuilder2_SetOutputFileName(dsc->captureGraph2,
			&MEDIASUBTYPE_Avi,  // Specifies AVI for the target file.
			unicodestr,	// File name.
			&pMux,              // Receives a pointer to the mux.
			NULL);              // (Optional) Receives a pointer to the file sink.

	SysFreeString(unicodestr);

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	hr = ICaptureGraphBuilder2_RenderStream(dsc->captureGraph2,
			&PIN_CATEGORY_CAPTURE, // Pin category.
			&MEDIATYPE_Video,      // Media type.
			(IUnknown*) dsc->captureFilter,                  // Capture filter.
			pEncoder,                  // Intermediate filter (optional).
			pMux);                 // Mux or file sink filter.

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	// Release the mux filter.
	SAFE_RELEASE(IBaseFilter, pMux);
	SAFE_RELEASE(IBaseFilter, pEncoder);

	dsc->captureGraphBuilt = 1;
	dsc->prevent_callback = 0;

	return S_OK;

Error:

	// Release the mux filter.
	SAFE_RELEASE(IBaseFilter, pMux);
	SAFE_RELEASE(IBaseFilter, pEncoder);

	return hr;
}

static HRESULT crossbar_get_crossbar_ipin_at_index(
   DSC *dsc, 
   IAMCrossbar *pXbar,
   LONG PinIndex,
   BOOL IsInputPin,
   IPin ** ppPin)
{
    LONG         cntInPins, cntOutPins;
    IPin        *pP = 0;
    IBaseFilter *pFilter = NULL;
    IEnumPins   *pins=0;
    ULONG        n;
    HRESULT      hr;
	LONG		 TrueIndex;

    if (!pXbar || !ppPin)
        return E_POINTER;

    *ppPin = 0;

    if(S_OK != IAMCrossbar_get_PinCounts(pXbar, &cntOutPins, &cntInPins))
        return E_FAIL;

    TrueIndex = IsInputPin ? PinIndex : PinIndex + cntInPins;

    hr = IAMCrossbar_QueryInterface(pXbar, &IID_IBaseFilter, (void **)&pFilter);

    if (hr == S_OK) 
    {
        if(SUCCEEDED(IBaseFilter_EnumPins(pFilter, &pins))) 
        {
            LONG i=0;
            while(IEnumPins_Next(pins, 1, &pP, &n) == S_OK) 
            {
                IPin_Release(pP);

                if (i == TrueIndex) 
                {
                    *ppPin = pP;
                    break;
                }
                i++;
            }
           IEnumPins_Release(pins);
        }

		IBaseFilter_Release(pFilter);
    }
    
    return *ppPin ? S_OK : E_FAIL; 
}


//
// Find corresponding index of an IPin on a crossbar
//crossbar_get_crossbar_index_from_ipin
static HRESULT crossbar_get_crossbar_index_from_ipin (
	DSC *dsc, 
    IAMCrossbar * pXbar,
    LONG * PinIndex,
    BOOL IsInputPin,
    IPin * pPin)
{
    LONG         cntInPins, cntOutPins;
    IPin        *pP = 0;
    IBaseFilter *pFilter = NULL;
    IEnumPins   *pins = 0;
    ULONG        n;
    BOOL         fOK = FALSE;
    HRESULT      hr;

    if (!pXbar || !PinIndex || !pPin)
        return E_POINTER;

    if(S_OK != IAMCrossbar_get_PinCounts(pXbar, &cntOutPins, &cntInPins))
        return E_FAIL;

    hr = IAMCrossbar_QueryInterface(pXbar, &IID_IBaseFilter, (void **)&pFilter);

    if (hr == S_OK) 
    {
        if(SUCCEEDED(IBaseFilter_EnumPins(pFilter, &pins))) 
        {
            LONG i=0;
        
            while(IEnumPins_Next(pins, 1, &pP, &n) == S_OK) 
            {
                IPin_Release(pP);
                if (pPin == pP) 
                {
                    *PinIndex = IsInputPin ? i : i - cntInPins;
                    fOK = TRUE;
                    break;
                }
                i++;
            }
            IEnumPins_Release(pins);
        }
        IBaseFilter_Release(pFilter);
    }
    
    return fOK ? S_OK : E_FAIL; 
}

static HRESULT crossbar_get_string_from_pin_type (long lType, char *name)
{
    if (name == NULL)
        return S_FALSE;

    switch (lType) 
    {   
        case PhysConn_Video_Tuner:           strncpy(name, "Video Tuner\0", 20);          break;
        case PhysConn_Video_Composite:       strncpy(name, "Video Composite\0", 20);      break;
        case PhysConn_Video_SVideo:          strncpy(name, "Video SVideo\0", 20);         break;
        case PhysConn_Video_RGB:             strncpy(name, "Video RGB\0", 20);            break;
        case PhysConn_Video_YRYBY:           strncpy(name, "Video YRYBY\0", 20);          break;
        case PhysConn_Video_SerialDigital:   strncpy(name, "Video SerialDigital\0", 20);  break;
        case PhysConn_Video_ParallelDigital: strncpy(name, "Video ParallelDigital\0", 20);break;
        case PhysConn_Video_SCSI:            strncpy(name, "Video SCSI\0", 20);           break;
        case PhysConn_Video_AUX:             strncpy(name, "Video AUX\0", 20);            break;
        case PhysConn_Video_1394:            strncpy(name, "Video 1394\0", 20);           break;
        case PhysConn_Video_USB:             strncpy(name, "Video USB\0", 20);            break;
        case PhysConn_Video_VideoDecoder:    strncpy(name, "Video Decoder\0", 20);        break;
        case PhysConn_Video_VideoEncoder:    strncpy(name, "Video Encoder\0", 20);        break;
    
        case PhysConn_Audio_Tuner:           strncpy(name, "Audio Tuner\0", 20);          break;
        case PhysConn_Audio_Line:            strncpy(name, "Audio Line\0", 20);           break;
        case PhysConn_Audio_Mic:             strncpy(name, "Audio Mic\0", 20);            break;
        case PhysConn_Audio_AESDigital:      strncpy(name, "Audio AESDigital\0", 20);     break;
        case PhysConn_Audio_SPDIFDigital:    strncpy(name, "Audio SPDIFDigital\0", 20);   break;
        case PhysConn_Audio_SCSI:            strncpy(name, "Audio SCSI\0", 20);           break;
        case PhysConn_Audio_AUX:             strncpy(name, "Audio AUX\0", 20);            break;
        case PhysConn_Audio_1394:            strncpy(name, "Audio 1394\0", 20);           break;
        case PhysConn_Audio_USB:             strncpy(name, "Audio USB\0", 20);            break;
        case PhysConn_Audio_AudioDecoder:    strncpy(name, "Audio Decoder\0", 20);        break;
    
        default:
            strncpy(name, "Unknown\0", 20);
            break;
    }
    
    return S_OK;
}

static HRESULT directshow_crossbar_get_video_inputs_for_pin(DSC *dsc, IPin *pStartingInputPin)
{
	HRESULT  hr;
    LONG     InputIndexRelated, OutputIndexRelated;
    LONG     InputPhysicalType, OutputPhysicalType;
    LONG     Inputs, Outputs, InputIndex, OutputIndex;

    IPin    *pPin=0;
    IPin    *pStartingOutputPin=0;
    PIN_INFO pinInfo;
    IAMCrossbar *pXbar=0;
	int i, id = 0;
	char name[50] = "";

	if (pStartingInputPin == NULL)
		return S_FALSE;

	if(dsc->crossbars != NULL) {

		for(i=0; i < dsc->NumberOfVideoInputs; i++) {

			Crossbar *cb = &(dsc->crossbars[i]);
			IAMCrossbar_Release(cb->pin);
			cb->pin = NULL;
		}

		free(dsc->crossbars);
		dsc->crossbars = NULL;
	}

    // If the pin isn't connected, then it's a terminal pin
    hr = IPin_ConnectedTo (pStartingInputPin, &pStartingOutputPin);

	HR_failed(dsc, hr);

    if(HR_failed(dsc, hr) != NOERROR)
		return hr;
	
    // It is connected, so now find out if the filter supports IAMCrossbar
    if (S_OK == IPin_QueryPinInfo(pStartingOutputPin, &pinInfo)) 
    {
        if (pinInfo.dir != PINDIR_OUTPUT)
			return S_FALSE;

        hr = IBaseFilter_QueryInterface(pinInfo.pFilter, &IID_IAMCrossbar, (void **)&pXbar);

        if (hr == S_OK) 
        {
			hr = IAMCrossbar_get_PinCounts(pXbar, &Outputs, &Inputs);

			HR_failed(dsc, hr);

			//DumpGraph((IFilterGraph*) dsc->filterGraphBuilder, 0);

			hr = crossbar_get_crossbar_index_from_ipin(dsc, pXbar, &OutputIndex, FALSE, pStartingOutputPin);

			HR_failed(dsc, hr);

			hr = IAMCrossbar_get_CrossbarPinInfo(pXbar, FALSE, OutputIndex, &OutputIndexRelated, &OutputPhysicalType);

			HR_failed(dsc, hr);

			dsc->crossbars = malloc(sizeof(Crossbar) * Inputs);

            // For all input pins
            for (InputIndex = 0; InputIndex < Inputs; InputIndex++) 
            {		
				hr = IAMCrossbar_get_CrossbarPinInfo(pXbar, TRUE, InputIndex, &InputIndexRelated, &InputPhysicalType);

				HR_failed(dsc, hr);

                // Is the pin a video pin?
                if (InputPhysicalType < PhysConn_Audio_Tuner) 
                {
                    // Can we route it?
                    if (S_OK == IAMCrossbar_CanRoute(pXbar, OutputIndex, InputIndex)) 
                    {
						hr = crossbar_get_crossbar_ipin_at_index(dsc, pXbar, InputIndex, TRUE, &pPin);

						HR_failed(dsc, hr);

						dsc->crossbars[id].pin = pXbar;
						IAMCrossbar_AddRef(pXbar);

						dsc->crossbars[id].dsc = dsc;
						dsc->crossbars[id].type = InputPhysicalType;
						dsc->crossbars[id].videoOutputIndex = OutputIndex;
						dsc->crossbars[id].videoInputIndex = InputIndex;

						crossbar_get_string_from_pin_type (InputPhysicalType, name);
						strncpy(dsc->crossbars[id].friendly_name, name, 49);

						id++;

                    } // if we can route

                } // if its a video pin

            } // for all input pins

            IAMCrossbar_Release(pXbar);
        }
        else 
        {
            // The filter doesn't support IAMCrossbar, so this
            // is a terminal pin
            IBaseFilter_Release(pinInfo.pFilter);
            IPin_Release (pStartingOutputPin);

            return S_FALSE;
        }

		IBaseFilter_Release(pinInfo.pFilter);
    }

    IPin_Release (pStartingOutputPin);

	dsc->NumberOfVideoInputs = id;

    return S_OK;

}


static HRESULT directshow_discover_crossbars(DSC *dsc)
{

	// Use the crossbar class to help us sort out all the possible video inputs
    // The class needs to be given the capture filters ANALOGVIDEO input pin
    IPin        *pP = 0;
    IEnumPins   *pins=0;
    ULONG        n;
    PIN_INFO     pinInfo;
    BOOL         Found = FALSE;
    IKsPropertySet *pKs=0;
    GUID guid;
    DWORD dw;
    BOOL fMatch = FALSE;

	if(dsc->captureFilter == NULL)
		return S_FALSE;

    if(SUCCEEDED(IBaseFilter_EnumPins(dsc->captureFilter,&pins)))
    {
        while(!Found && (S_OK == IEnumPins_Next(pins, 1, &pP, &n)))
        {
            if(S_OK == IPin_QueryPinInfo(pP, &pinInfo))
            {
                if(pinInfo.dir == PINDIR_INPUT)
                {
                    // Is this pin an ANALOGVIDEOIN input pin?
                    if(IPin_QueryInterface(pP, &IID_IKsPropertySet, (void **)&pKs) == S_OK)
                    {
                        if(IKsPropertySet_Get(pKs, &AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0,
                            &guid, sizeof(GUID), &dw) == S_OK)
                        {
							if(InlineIsEqualGUID(&guid, &PIN_CATEGORY_ANALOGVIDEOIN))
                                fMatch = TRUE;
                        }
                            
						SAFE_RELEASE(IKsPropertySet, pKs);
                    }

                    if(fMatch)
                    {
                        HRESULT hrCreate=S_OK;
                  
						directshow_crossbar_get_video_inputs_for_pin(dsc, pP);

                        Found = TRUE;
                    }
                }
					
				SAFE_RELEASE(IBaseFilter, pinInfo.pFilter);
            }

			SAFE_RELEASE(IPin, pP);
        }

		SAFE_RELEASE(IEnumPins, pins);
    }

	if(Found)
		return S_OK;

	return S_FALSE;
}

HRESULT directshow_crossbar_get_video_inputs(DSC *dsc, Crossbar *crossbar_array, int *crossbar_count)
{
	char name[500] = "";

	if(directshow_discover_crossbars(dsc) == S_FALSE)
		return S_FALSE;

	*crossbar_count = dsc->NumberOfVideoInputs;

	if(crossbar_array == NULL) {
		return S_OK;
	}

	memcpy(crossbar_array, dsc->crossbars, sizeof(Crossbar) * dsc->NumberOfVideoInputs);

	return S_OK;
}

HRESULT directshow_crossbar_set_video_input(DSC *dsc, Crossbar *crossbar)
{
	HRESULT hr = E_FAIL;
	IAMCrossbar *pXbar=0;

	hr = IAMCrossbar_Route (crossbar->pin, crossbar->videoOutputIndex, crossbar->videoInputIndex);
    
	if(HR_failed(dsc, hr) != NOERROR)
		return hr;

    return hr;
}

int directshow_get_capture_filter_capabilities(DSC *dsc)
{
	int iFormat, iCount = 0, iSize = 0;
	LPOLESTR friendly_name;

	int hr = ICaptureGraphBuilder2_FindInterface(dsc->captureGraph2,
		&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video, dsc->captureFilter, &IID_IAMStreamConfig,(void **)&dsc->videoCapabilities);

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	// Use IAMStreamConfig to report which formats the device supports

	// Get the number of media types
	hr = IAMStreamConfig_GetNumberOfCapabilities(dsc->videoCapabilities, &iCount, &iSize);

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	// Check the size to make sure we pass in the correct structure.
	if (iSize != sizeof(VIDEO_STREAM_CONFIG_CAPS))
		return S_FALSE;

	// Use the video capabilities structure.
	for (iFormat = 0; iFormat < iCount; iFormat++)
	{
		// Configure the device to use a format returned in the GetStreamCaps
		// Video capabilities are described using the VIDEO_STREAM_CONFIG_CAPS
		VIDEO_STREAM_CONFIG_CAPS scc;
		AM_MEDIA_TYPE *pmtConfig;
		VIDEOINFOHEADER *pVih;

		// enumerate the media types; media type and the corresponding capability structure returned
		hr = IAMStreamConfig_GetStreamCaps(dsc->videoCapabilities, iFormat, &pmtConfig, (BYTE*)&scc);

		if(!SUCCEEDED(hr))
			return S_FALSE;

		pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;

		StringFromIID(&(pmtConfig->subtype), &friendly_name);

		printf("Width %d Height %d subtype %s\n", pVih->bmiHeader.biWidth, pVih->bmiHeader.biHeight,
			friendly_name);	
	}

	return S_OK;
}


HRESULT directshow_set_capture_filter_to_largest_resolution(DSC *dsc)
{
	int iFormat, iCount = 0, iSize = 0;
	int requiredFormat = 0, largest_height = -1;
	LPOLESTR friendly_name;

	// Configure the device to use a format returned in the GetStreamCaps
	// Video capabilities are described using the VIDEO_STREAM_CONFIG_CAPS
	VIDEO_STREAM_CONFIG_CAPS scc;
	AM_MEDIA_TYPE *pmtConfig, *firstPmtConfig;
	VIDEOINFOHEADER *pVih;

	int hr = ICaptureGraphBuilder2_FindInterface(dsc->captureGraph2,
		&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video, dsc->captureFilter, &IID_IAMStreamConfig,(void **)&dsc->videoCapabilities);

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	// Use IAMStreamConfig to report which formats the device supports

	// Get the number of media types
	hr = IAMStreamConfig_GetNumberOfCapabilities(dsc->videoCapabilities, &iCount, &iSize);

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	

	// Check the size to make sure we pass in the correct structure.
	if (iSize != sizeof(VIDEO_STREAM_CONFIG_CAPS))
		return S_FALSE;

	// Get the first format that we will use
	hr = IAMStreamConfig_GetStreamCaps(dsc->videoCapabilities, 0, &firstPmtConfig, (BYTE*)&scc);

	if(!SUCCEEDED(hr))
		return S_FALSE;

	// Use the video capabilities structure.
	for (iFormat = 0; iFormat < iCount; iFormat++)
	{
		// enumerate the media types; media type and the corresponding capability structure returned
		hr = IAMStreamConfig_GetStreamCaps(dsc->videoCapabilities, iFormat, &pmtConfig, (BYTE*)&scc);

		if(!SUCCEEDED(hr))
			return S_FALSE;

		pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;

		StringFromIID(&(pmtConfig->subtype), &friendly_name);

		// Get the format with the highest height with the same subtype as the first
		if(pVih->bmiHeader.biHeight > largest_height && InlineIsEqualGUID(&(pmtConfig->subtype), &(firstPmtConfig->subtype))) {
			largest_height = pVih->bmiHeader.biHeight;
			requiredFormat = iFormat;
		}	
	}

	hr = IAMStreamConfig_GetStreamCaps(dsc->videoCapabilities, requiredFormat, &pmtConfig, (BYTE*)&scc);

	if(!SUCCEEDED(hr))
		return S_FALSE;

	dsc->capture_filter_mt = *pmtConfig;

	pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;

	// Set the best format
	// That resolution is available, now we set the capture format to the resolution we want.
    IAMStreamConfig_SetFormat(dsc->videoCapabilities, pmtConfig);

//	pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;

    //iFrameRate = 10000000/pvi->AvgTimePerFrame;
  //  pVih->AvgTimePerFrame = 100000;  // 30 fps ????????
            //pvi->dwBitRate = scc.MaxBitsPerSecond;
    //        hr = pVSC->SetFormat(pmt);
            
   // }

	//TODO
    //DeleteMediaType(requiredPmtConfig);

	return S_OK;
}

// Show the property pages for a filter.
// This is stolen from the DX9 SDK.
HRESULT directshow_show_capture_filter_property_pages(DSC *dsc, HWND hwnd)
{
	HRESULT hr;
	FILTER_INFO FilterInfo;
	IUnknown *pFilterUnk;
	CAUUID caGUID;

    // Obtain the filter's IBaseFilter interface. (Not shown) 
    ISpecifyPropertyPages *pProp;
  
	if(dsc->captureFilter == NULL)
		return S_FALSE;

	hr = IBaseFilter_QueryInterface(dsc->captureFilter, &IID_ISpecifyPropertyPages,
		 (void**)&pProp);

	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	
	
    // Get the filter's name and IUnknown pointer.
        
	hr = IBaseFilter_QueryFilterInfo(dsc->captureFilter, &FilterInfo);  
 
	if(HR_failed(dsc, hr) != NOERROR)
		return hr;	
    
	IBaseFilter_QueryInterface(dsc->captureFilter, &IID_IUnknown,
		 (void**)&pFilterUnk);

    // Show the page. 
    ISpecifyPropertyPages_GetPages(pProp, &caGUID);
	SAFE_RELEASE(ISpecifyPropertyPages, pProp);

    OleCreatePropertyFrame(
            hwnd,                   // Parent window
            0, 0,                   // Reserved
            FilterInfo.achName,     // Caption for the dialog box
            1,                      // # of objects (just the filter)
            &pFilterUnk,            // Array of object pointers. 
            caGUID.cElems,          // Number of property pages
            caGUID.pElems,          // Array of property page CLSIDs
            0,                      // Locale identifier
            0, NULL                 // Reserved
    );

	SAFE_RELEASE(IUnknown, pFilterUnk);
	SAFE_RELEASE(IFilterGraph, FilterInfo.pGraph);

    CoTaskMemFree(caGUID.pElems);
    
    return hr;
}


HRESULT directshow_show_capture_filter_pin_property_pages(DSC *dsc, HWND hwnd)
{
	//IAMStreamConfig *pSC;
	ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
	HRESULT hr;

	// You can change this pin's output format in these dialogs.
    // If the capture pin is already connected to somebody who's
    // fussy about the connection type, that may prevent using
    // this dialog(!) because the filter it's connected to might not
    // allow reconnecting to a new format. (EG: you switch from RGB
    // to some compressed type, and need to pull in a decoder)
    // I need to tear down the graph downstream of the
    // capture filter before bringing up these dialogs.
    // In any case, the graph must be STOPPED when calling them.
	directshow_stop_capture(dsc);  // make sure graph is stopped

    // The capture pin that we are trying to set the format on is connected if
    // one of these variable is set to TRUE. The pin should be disconnected for
    // the dialog to work properly.
    if(dsc->captureGraphBuilt = 1)
    {
		  directshow_teardown_graph(dsc, 0);    // graph could prevent dialog working
    }
       
	if(dsc->videoCapabilities == NULL)
		return S_FALSE;

    hr = IAMStreamConfig_QueryInterface(dsc->videoCapabilities,
		&IID_ISpecifyPropertyPages, (void **)&pSpec);

    if(hr == S_OK)
    {
		hr = ISpecifyPropertyPages_GetPages(pSpec, &cauuid);
        
		hr = OleCreatePropertyFrame(
				hwnd,
				30,
				30,
				NULL,
				1,
				(IUnknown **)&(dsc->videoCapabilities),
				cauuid.cElems,
                (GUID *)cauuid.pElems,
				0,
				0,
				NULL);

        CoTaskMemFree(cauuid.pElems);

		SAFE_RELEASE(ISpecifyPropertyPages, pSpec);
   }

	return S_OK;
}


static HRESULT directshow_get_crossbars(DSC *dsc, IAMCrossbar **pX, IAMCrossbar **pX2, int *count)
{
	//  The video crossbar, and a possible second crossbar
	IBaseFilter *pXF;
	CAUUID cauuid;
	ISpecifyPropertyPages *pSpec;

	HRESULT hr = ICaptureGraphBuilder2_FindInterface(dsc->captureGraph2,
				&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Interleaved, dsc->captureFilter,
				&IID_IAMCrossbar,(void **)pX);

	if(FAILED(hr)) {

		hr = ICaptureGraphBuilder2_FindInterface(dsc->captureGraph2,
				&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video, dsc->captureFilter,
				&IID_IAMCrossbar,(void **)pX);
	}

    if(SUCCEEDED(hr))
    {
		hr = IAMCrossbar_QueryInterface(*pX, &IID_IBaseFilter, (void **)&pXF); 
    
        if(SUCCEEDED(hr))
        {
            hr = IAMCrossbar_QueryInterface(*pX, &IID_ISpecifyPropertyPages, (void **)&pSpec);

            if(SUCCEEDED(hr))
            {
				hr = ISpecifyPropertyPages_GetPages(pSpec, &cauuid);

				*count = 1;

				CoTaskMemFree(cauuid.pElems);
                SAFE_RELEASE(ISpecifyPropertyPages, pSpec);
			}
			else {

				SAFE_RELEASE(IBaseFilter, pXF);
				return S_FALSE;
			}

			hr = ICaptureGraphBuilder2_FindInterface(dsc->captureGraph2,
					&LOOK_UPSTREAM_ONLY, NULL, pXF, &IID_IAMCrossbar,(void **)pX2);

			if(SUCCEEDED(hr)) {

				hr = ISpecifyPropertyPages_GetPages(pSpec, &cauuid);

				*count = 2;

				CoTaskMemFree(cauuid.pElems);
                SAFE_RELEASE(ISpecifyPropertyPages, pSpec);
			}
 
			SAFE_RELEASE(IBaseFilter, pXF);
        }
    }

	return S_OK;
}

HRESULT directshow_get_capture_filter_property_page_count(DSC *dsc, int *count)
{
	IAMCrossbar *pX = NULL, *pX2 = NULL;
	HRESULT hr = directshow_get_crossbars(dsc, &pX, &pX2, count);

	SAFE_RELEASE(IAMCrossbar, pX);
	SAFE_RELEASE(IAMCrossbar, pX2);

	return hr;
}

HRESULT directshow_show_capture_filter_crossbar_property_pages(DSC *dsc, int crossbar_daialog_number, HWND hwnd)
{
	//  The video crossbar, and a possible second crossbar

    IAMCrossbar *pX = NULL, *pX2 = NULL, *chosen = NULL;
	CAUUID cauuid;
	ISpecifyPropertyPages *pSpec;
	int count;

	HRESULT hr = directshow_get_crossbars(dsc, &pX, &pX2, &count);

	if(crossbar_daialog_number > 1 && count > 1) {

		if(pX2 == NULL)
			return S_FALSE;

		chosen = pX2;
	}
	else {

		if(pX == NULL)
			return S_FALSE;

		chosen = pX;
	}

	if(SUCCEEDED(hr))
    {
		hr = IAMCrossbar_QueryInterface(chosen, &IID_ISpecifyPropertyPages, (void **)&pSpec);

            if(SUCCEEDED(hr))
            {
				hr = ISpecifyPropertyPages_GetPages(pSpec, &cauuid);

				hr = OleCreatePropertyFrame(
							hwnd,
							30,
							30,
							NULL,
							1,
							(IUnknown **)&(chosen),
							cauuid.cElems,
							(GUID *)cauuid.pElems,
							0,
							0,
							NULL);

                    CoTaskMemFree(cauuid.pElems);

                SAFE_RELEASE(ISpecifyPropertyPages, pSpec);
			}
	}

	SAFE_RELEASE(IAMCrossbar, pX);
	SAFE_RELEASE(IAMCrossbar, pX2);

	return S_OK;
}

#endif // #ifdef STANDALONE_APP