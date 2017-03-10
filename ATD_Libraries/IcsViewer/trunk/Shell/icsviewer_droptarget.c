#include "icsviewer_window.h" 
#include "icsviewer_com_utils.h"

#include "gci_utils.h"

#include <shellapi.h>

#include "FreeImageAlgorithms_IO.h"

int vcObjects = 0;

static HRESULT __stdcall DropTargetQueryInterface (IDropTarget *pDropTarget, REFIID riid, void **ppv)
{
    IcsViewerWindow *this = IMPL (IcsViewerWindow, iDropTarget, pDropTarget);

    if (IsEqualIID (riid, &IID_IUnknown) || IsEqualIID (riid, &IID_IDropTarget))
        *ppv = &this->iDropTarget;
    else
    {
        *ppv = 0;
        return E_NOINTERFACE;
    }    

    AddRef ((IUnknown *)*ppv);
    
    return NOERROR;
}


static ULONG __stdcall DropTargetAddRef (IDropTarget *pDropTarget)
{
	IcsViewerWindow *this = IMPL (IcsViewerWindow, iDropTarget, pDropTarget);
    
    return ++this->cRef;
}



static ULONG __stdcall DropTargetRelease (IDropTarget *pDropTarget)
{
    IcsViewerWindow *this = IMPL (IcsViewerWindow, iDropTarget, pDropTarget);

    if (--this->cRef == 0)
    {
        --vcObjects;

        free (this);
        return 0;
    }
    
    return this->cRef;
}


static HRESULT __stdcall DropTargetDragEnter(IDropTarget *pDropTarget, IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	#ifndef NO_ICSVIEWER_DRAG_DROP

  	FORMATETC fmte = { CF_HDROP, (DVTARGETDEVICE FAR *)NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  	
    // CVI uses windows files that are now incompatible with vs
    #ifdef _CVI_
 
	STGMEDIUM StgMed;
	
    if(S_OK == IDataObject_GetData (pDataObject, &fmte, &StgMed))  
  	{
   		HDROP hdrop = (HDROP)StgMed.u.hGlobal;
   		UINT cFiles = DragQueryFile(hdrop, (UINT)-1, NULL, 0);

		if(cFiles > 1)
			return S_FALSE;	
	}  

	#else

	STGMEDIUM StgMed;
	
    if(S_OK == IDataObject_GetData (pDataObject, &fmte, &StgMed))  
  	{
   		HDROP hdrop = (HDROP)StgMed.hGlobal;
   		UINT cFiles = DragQueryFile(hdrop, (UINT)-1, NULL, 0);

		if(cFiles > 1)
			return S_FALSE;	
	}  

    #endif
	#endif

	return S_OK;
}


static HRESULT __stdcall DropTargetDragOver(IDropTarget *pDropTarget, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	return S_OK;
}

static HRESULT __stdcall DropTargetDragLeave(IDropTarget *pDropTarget)
{
	return S_OK;

}

static HRESULT __stdcall DropTargetDrop(IDropTarget *pDropTarget, IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	#ifndef NO_ICSVIEWER_DRAG_DROP

	IcsViewerWindow *this = IMPL (IcsViewerWindow, iDropTarget, pDropTarget);    
  
    STGMEDIUM StgMed;
  	FORMATETC fmte = { CF_HDROP, (DVTARGETDEVICE FAR *)NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  	
  	if(S_OK == IDataObject_GetData (pDataObject, &fmte, &StgMed))  
  	{
        // CVI uses windows files that are now incompatible with vs
        #ifdef _CVI_

		char filename[MAX_FILENAME_LEN]; 
		char directory[MAX_FILENAME_LEN];

        HDROP hdrop = (HDROP)StgMed.u.hGlobal;
   	
  		DragQueryFile(hdrop, 0, filename, sizeof(filename));

   		if (StgMed.pUnkForRelease)
 			Release (StgMed.pUnkForRelease); 
   		else
   			GlobalFree(StgMed.u.hGlobal);
   			
		if(!filename)
			return S_FALSE;
		
		GetDirectoryForFile(filename, directory);
		AddToFilePopupDirHistory (directory);
		
		if(!GCI_ImagingWindow_LoadImageFile(this, filename))
			return S_FALSE;	
        
		#else

		char filename[MAX_FILENAME_LEN]; 
		char directory[MAX_FILENAME_LEN];

        HDROP hdrop = (HDROP)StgMed.hGlobal;
   	
  		DragQueryFile(hdrop, 0, filename, sizeof(filename));

   		if (StgMed.pUnkForRelease)
 			Release (StgMed.pUnkForRelease); 
   		else
   			GlobalFree(StgMed.hGlobal);
   			
		if(!filename)
			return S_FALSE;
		
		GetDirectoryForFile(filename, directory);
		AddToFilePopupDirHistory (directory);
		
		if(!GCI_ImagingWindow_LoadImageFile(this, filename))
			return S_FALSE;	

        #endif
        }

	#endif

	return S_OK;
}
    

static IDropTargetVtbl vtblDropTarget =
{
    DropTargetQueryInterface, DropTargetAddRef, DropTargetRelease,
    DropTargetDragEnter, DropTargetDragOver, DropTargetDragLeave, DropTargetDrop
};



void RegisterDropWindow(IcsViewerWindow *window)
{
	window->iDropTarget.lpVtbl = &vtblDropTarget;

	// acquire a strong lock
	//CoLockObjectExternal(pDropTarget, TRUE, FALSE);

	// tell OLE that the window is a drop target
	RegisterDragDrop(window->canvas_window, &(window->iDropTarget));
}


void UnregisterDropWindow(IcsViewerWindow *window)
{
	// remove drag+drop
	RevokeDragDrop(window->canvas_window);

	// remove the strong lock
	//CoLockObjectExternal(pDropTarget, FALSE, TRUE);

	// release our own reference
	Release (&window->iDropTarget); 
}

