#include <userint.h>
#include "icsviewer_window.h"

#ifdef ENABLE_TWAIN  

#include "twain_plugin.h"

#include "TWAIN.h"
#include "EZTWAIN.H"  

#include "gci_utils.h"
#include <userint.h>
#include "toolbox.h"

#include "FreeImageAlgorithms.h" 
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_HBitmap.h"

#include "icsviewer_uir.h"
#include "string_utils.h"

	
static HWND hMainWnd;
static ListType sourceList;

int openDSM (void)
{
// must open DSM with correct hwnd or messages from DS will not be recieved by our window procedure
	//hMainWnd = (HWND)GetCVIWindowHandle ();
	hMainWnd = (HWND)GetCVIWindowHandleForCurrThread ();
	
	
	if (!TWAIN_OpenSourceManager((HWND)hMainWnd)) 
	{
   		TWAIN_ErrorBox("Unable to open Source Manager");
   		return(-1);
   	}

 // 	if ((panelHandle = LoadPanel (0, "TwainDriver_UI.uir", PANEL_2)) < 0)
//	return (-1);

   	return (0);
}
		

int getTwainSourceList (void)
{
	TW_IDENTITY source;
	int error=0;
	
	if (openDSM()<0) {error=-1; goto Error;}
	
	if (sourceList!=0)
	{
		ListDispose (sourceList);
		sourceList = 0;
	}
	
	sourceList = ListCreate (sizeof(TW_IDENTITY));
	if (sourceList==0) {error=-2; goto Error;}

	if (TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &source))
		ListInsertItem (sourceList, (void *)&source, 1);
	else
		{error=-3; goto Error;}

	while (TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &source))
		ListInsertItem (sourceList, (void *)&source, END_OF_LIST);

Error:
	return(error);
}

int openTwainSource (char *nameStr)
{
	TW_IDENTITY source;
	int n, i;

	if (getTwainSourceList()<0)
	{
   		TWAIN_ErrorBox("Unable to get Source list");
   		return(-2);
   	}
   	
	n = ListNumItems (sourceList);
   	for (i=1; i<=n; i++)
   	{
		ListGetItem (sourceList, (void *)&source, i);
		if (strcmp (source.ProductName, nameStr)==0) break;
   	}
   	
   	if (i<=n) // We have a match
   	{
		if (!TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &source)) return(-4);
   	}
   	else return(-3);
 
	return(0);
}

void closeTwain (void)
{
	TWAIN_DisableSource();
	TWAIN_CloseSource();
	TWAIN_CloseSourceManager(hMainWnd);
	TWAIN_UnloadSourceManager();
}

int selectTwainSource (void)
{
	return (!TWAIN_SelectImageSource(hMainWnd));
}

int openTwainDefaultSource (void)
{
	if (openDSM()<0)
		return(-1);
	
	return (!TWAIN_OpenDefaultSource ());
}

static FIBITMAP* GCI_TwainAcquireFIB (IcsViewerWindow *window, int useTwainDeviceUI)
{
    HANDLE hdib;
	//HBITMAP hbitmap;
    //TWAIN_LogFile(1);
    FIBITMAP *fib;
	int bpp, width, height;
		
	//TWAIN_SetHideUI(TRUE); 
	TWAIN_SetHideUI(FALSE); 
	
    if (TWAIN_OpenDefaultSource()) {
        
		TWAIN_NegotiateXferCount(1);
		//TWAIN_NegotiatePixelTypes(TWAIN_GRAY | TWAIN_RGB);
		
      //  if (!TWAIN_SetAutoScan(FALSE)) {
      //      MessageBox(NULL, "SetAutoScan(FALSE) failed - scanner cannot single-scan?", "TWAIN Alert", MB_ICONINFORMATION | MB_OK);
      //  }
		
        // If you can't get a Window handle, use NULL:
        hdib = TWAIN_AcquireNative(NULL, TWAIN_GRAY | TWAIN_RGB);
		//hdib = TWAIN_WaitForNativeXfer(NULL);
		
        if (hdib) {

			LPBITMAPINFOHEADER pbi = (LPBITMAPINFOHEADER) GlobalLock(hdib);
  
			width = TWAIN_DibWidth(hdib);
			height = TWAIN_DibHeight(hdib); 
			bpp = TWAIN_DibDepth(hdib);
			
			fib = FreeImage_Allocate(width, height, bpp, 0, 0, 0);    
			
			memcpy(FreeImage_GetBits(fib), (const void*) DibBits(pbi), pbi->biSizeImage);     
	
			GlobalUnlock(hdib);    
			
            // <your image processing here>
            TWAIN_FreeNative(hdib);
			  
			return fib;    
        }
    }
	
	return NULL;
}

static void TwainAcquire (TwainPlugin* twain_plugin)
{
	FIBITMAP *dib;
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) twain_plugin;   
	
	if(openTwainDefaultSource () != 0)  {
		GCI_MessagePopup("Error", "Unable to open twain source"); 	
		return;
	}
		
	if( (dib = GCI_TwainAcquireFIB (twain_plugin->parent.window, 1)) == NULL) {
		GCI_MessagePopup("Error", "Unable to aquire data from twain source");
		return;
	}

	GCI_ImagingWindow_LoadImage(plugin->window, dib);
	
	GCI_ImagingWindow_SetWindowTitle(plugin->window, "Epson Scan Aquired Image");
	
	GCI_ImagingWindow_Show(plugin->window); 
}


static void CVICALLBACK
	on_twain_select_source_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	if(selectTwainSource () != 0)
		GCI_MessagePopup("Error", "Unable to select twain source");
}


static void CVICALLBACK
	on_twain_acquire_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	TwainPlugin* twain_plugin = (TwainPlugin*) callbackData;    
	
	TwainAcquire(twain_plugin);       
}

							 
static int on_validate_plugin(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}


static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	TwainPlugin* twain_plugin = (TwainPlugin*) plugin; 
}

ImageWindowPlugin* twain_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "TwainPlugin", sizeof(TwainPlugin)); 

	TwainPlugin* twain_plugin = (TwainPlugin*) plugin; 

	twain_plugin->select_menu_item_id = Plugin_AddMenuItem(plugin, "File//Twain//Select Twain Source", 0,
		on_twain_select_source_menu_clicked, plugin);

	twain_plugin->acquire_menu_item_id = Plugin_AddMenuItem(plugin, "File//Twain//Twain Acquire", 0,
		on_twain_acquire_menu_clicked, plugin);

	SetMenuBarAttribute (MENUBAR_ID(plugin), twain_plugin->select_menu_item_id, ATTR_DIMMED, 0);
	SetMenuBarAttribute (MENUBAR_ID(plugin), twain_plugin->acquire_menu_item_id, ATTR_DIMMED, 0);

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}

#endif // ENABLE_TWAIN
