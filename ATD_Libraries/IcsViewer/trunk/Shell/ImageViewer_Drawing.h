// ImageWindow - Displays images with zooming and panning
// Copyright (C) 2005  Glenn Pierce glennpierce@gmail.com
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __IMAGEVIEWER_DRAWING__
#define __IMAGEVIEWER_DRAWING__

#include "ImageViewer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ImageViewer_DrawLine:
 * @viewer_window: Canvas Window.
 *
 * Draw a line on the ImageView internal buffer.
 **/
int
ImageViewer_DrawLine(HWND viewer_window, const POINT p1, const POINT p2, int width, COLORREF colour, int style, HDC hdc);



/**
 * ImageViewer_DrawRectangle:
 * @viewer_window: Canvas Window.
 *
 * Draw a rectangle on the ImageView internal buffer.
 **/
int
ImageViewer_DrawRectangle(HWND viewer_window, RECT rect, int width, COLORREF colour);


/**
 * ImageViewer_DrawRectangleFromPoints:
 * @viewer_window: Canvas Window.
 *
 * Draw a rectangle on the ImageView internal buffer.
 **/
int
ImageViewer_DrawRectangleFromPoints(HWND viewer_window, POINT p1, POINT p2, int width, COLORREF colour);


void
ImageViewer_DrawCross(HWND viewer_window, POINT point, int width, COLORREF colour);


/* 
 * Calculates image point for canvas point.
 * Takes account of freeimage bitmaps being vertically flipped
 *
 **/
void 
ImageViewer_TranslateWindowPointToFreeImagePoint(HWND viewer_window, POINT canvas_point, POINT *image_point);


/* 
 * Calculates freeimage point from normal image where the top left is 0,0
 *
 **/
void 
ImageViewer_TranslateImagePointToFreeImagePoint(HWND viewer_window, POINT image_point, POINT *freeimage_point);


/**
 * ImageViewer_DrawCircle:
 * @viewer_window: Canvas Window.
 *
 * Draw a circle on the ImageView internal buffer.
 **/
int
ImageViewer_DrawCircle(HWND viewer_window, POINT center, int radius, COLORREF colour);


/**
 * ImageViewer_DrawText:
 * @viewer_window: Canvas Window.
 *
 * Draw text on the ImageView internal buffer.
 **/
void
ImageViewer_DrawText(HWND viewer_window, COLORREF colour, POINT point, char *text);


/**
 * ImageViewer_DrawSolidRect:
 * @viewer_window: Canvas Window.
 *
 * Draw a solid colour rectangle.
 **/
int
ImageViewer_DrawSolidRect(HWND viewer_window, RECT rect, COLORREF colour);


POINT ImageViewer_GetClosetPosOnZoomedImageFromCursorPos(HWND viewer_window);


#ifdef __cplusplus
}
#endif

#endif
