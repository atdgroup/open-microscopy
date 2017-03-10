#ifndef __IMAGEVIEWER_UTILS__
#define __IMAGEVIEWER_UTILS__

#ifdef __cplusplus
extern "C" {
#endif

#include "ImageViewer.h"

ImageViewerCtrl*
GetImageViewerCtrl(HWND hwnd);

void
SetImageViewerCtrl(HWND hwnd, ImageViewerCtrl *viewer);

LONG
GetTrackPos32(HWND hwnd, int nBar);

int
GetClosestIntMultipleOfZoomFactor(ImageViewerCtrl *viewer, int value);

void
ImageViewer_SetupScrollbars(ImageViewerCtrl *viewer);

void
RedrawScreenImmediately(ImageViewerCtrl *viewer,  RECT *updateRect);

int
GetViewWidth(ImageViewerCtrl *viewer);

int
GetViewHeight(ImageViewerCtrl *viewer);

void
PaintBackGround(ImageViewerCtrl *viewer, HDC hdc);

double
CalculateZoomSizeForWindow(ImageViewerCtrl *viewer);

FSIZE
CalculateRealZoomedSizeForImage(ImageViewerCtrl *viewer);

SIZE
CalculateZoomedSizeForImage(ImageViewerCtrl *viewer);

FPOINT
CalculateRealTopLeftForZoomedImage(ImageViewerCtrl *viewer);

POINT
CalculateTopLeftForZoomedImage(ImageViewerCtrl *viewer);

int
IsZoomedImageSmallerThanView(ImageViewerCtrl *viewer);

int
ImageViewer_Destroy(ImageViewerCtrl *viewer);

#ifdef __cplusplus
}
#endif

#endif
