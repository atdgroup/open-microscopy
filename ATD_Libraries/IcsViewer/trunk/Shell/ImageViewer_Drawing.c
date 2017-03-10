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

#include "ImageViewer.h"
#include <ansi_c.h>
#include "ImageViewer_Drawing.h"

int
ImageViewer_DrawLine(HWND viewer_window, const POINT p1, const POINT p2, int width, COLORREF colour, int style, HDC hdc)
{
	HPEN colour_pen, old_pen;
	HDC drawing_hdc;
	
	if(hdc == NULL)
		drawing_hdc = ImageViewer_GetBufferHdc(viewer_window);
	else
		drawing_hdc = hdc;

	colour_pen = CreatePen(style, width, colour); 
	old_pen = (HPEN) SelectObject(drawing_hdc, colour_pen);
	
	MoveToEx(drawing_hdc, p1.x, p1.y, NULL);
	LineTo(drawing_hdc, p2.x, p2.y);
	
	DeleteObject(colour_pen);
	SelectObject(drawing_hdc, old_pen); 

	return IMAGEVIEWER_SUCCESS;
}


int
ImageViewer_DrawRectangle(HWND viewer_window, RECT rect, int width, COLORREF colour)
{
	POINT p1, p2;

	p1.x = rect.left;
	p1.y = rect.top;
	p2.x = rect.right;
	p2.y = rect.top;
	
	ImageViewer_DrawLine(viewer_window, p1, p2, width, colour, PS_SOLID, NULL);
	
	
	p1.x = rect.left;
	p1.y = rect.bottom;
	p2.x = rect.right;
	p2.y = rect.bottom;
	
	ImageViewer_DrawLine(viewer_window, p1, p2, width, colour, PS_SOLID, NULL);
	
	
	p1.x = rect.left;
	p1.y = rect.top;
	p2.x = rect.left;
	p2.y = rect.bottom;
	
	ImageViewer_DrawLine(viewer_window, p1, p2, width, colour, PS_SOLID, NULL);
	
	p1.x = rect.right;
	p1.y = rect.top;
	p2.x = rect.right;
	p2.y = rect.bottom;
	
	ImageViewer_DrawLine(viewer_window, p1, p2, width, colour, PS_SOLID, NULL);
	
	return IMAGEVIEWER_SUCCESS;
}



void ImageViewer_DrawCross(HWND viewer_window, POINT point, int width, COLORREF colour)
{
	// Draw vertical line 
	POINT p1, p2;
	
	p1.x = p2.x = point.x + 1;

	p1.y = (point.y - width / 2) + 1;
	p2.y = point.y + 2 + width / 2;
	
	ImageViewer_DrawLine(viewer_window, p1, p2, 1, colour, PS_SOLID, NULL);

	// Draw horizontal line 
	p1.y = p2.y = point.y + 1;
	
	p1.x = (point.x - width / 2) + 1;
	p2.x = point.x + 2 + width / 2;
	
	ImageViewer_DrawLine(viewer_window, p1, p2, 1, colour, PS_SOLID, NULL);
}


int
ImageViewer_DrawRectangleFromPoints(HWND viewer_window, POINT p1, POINT p2, int width, COLORREF colour)
{
	RECT rect;
	
	rect.left = p1.x;
	rect.top = p1.y;
	rect.right = p2.x;
	rect.bottom = p2.y;

	ImageViewer_DrawRectangle(viewer_window, rect, width, colour);  
    
    return IMAGEVIEWER_SUCCESS;
}



int
ImageViewer_DrawSolidRect(HWND viewer_window, RECT rect, COLORREF colour)
{
	HBRUSH colour_brush, old_brush;     
	
	HDC drawing_hdc = ImageViewer_GetBufferHdc(viewer_window);      

	colour_brush = CreateSolidBrush(colour);
	old_brush = (HBRUSH) SelectObject(drawing_hdc, colour_brush);    
	
    FillRect( drawing_hdc, &rect, colour_brush);   
        
    DeleteObject(colour_brush);
    
    SelectObject(drawing_hdc, old_brush);
	
    return IMAGEVIEWER_SUCCESS;
}


int
ImageViewer_DrawCircle(HWND viewer_window, POINT center, int radius, COLORREF colour)
{
	RECT rect;
	HPEN color_pen, oldPen;
	
	HDC drawing_hdc = ImageViewer_GetBufferHdc(viewer_window);      

	color_pen = CreatePen(PS_SOLID, 2, colour); 
	oldPen = (HPEN) SelectObject(drawing_hdc, color_pen);

	rect.left = center.x - radius;
	rect.top = center.y - radius;
	rect.right = center.x + radius;
	rect.bottom = center.y + radius;

    Ellipse(drawing_hdc, rect.left, rect.top, rect.right, rect.bottom);
        
    DeleteObject(color_pen);
    
    SelectObject(drawing_hdc, oldPen);

    return IMAGEVIEWER_SUCCESS;
}


void
ImageViewer_DrawText(HWND viewer_window, COLORREF colour, POINT point, char *text)
{
	HDC drawing_hdc = ImageViewer_GetBufferHdc(viewer_window);    
	
	SetTextColor(drawing_hdc, colour);
	SetBkMode(drawing_hdc, TRANSPARENT);
	TextOut(drawing_hdc, point.x, point.y, text, (int) strlen(text));
}


void 
ImageViewer_TranslateWindowPointToFreeImagePoint(HWND viewer_window, POINT canvas_point, POINT *image_point)
{
	SIZE image_size;
	POINT p;

	ImageViewer_TranslateWindowPointToImagePoint(viewer_window, canvas_point, &p); 

	ImageViewer_GetImageSize(viewer_window, &image_size);    
	
	image_point->x = p.x;
	image_point->y = image_size.cy - p.y;
}

 
void 
ImageViewer_TranslateImagePointToFreeImagePoint(HWND viewer_window, POINT image_point, POINT *freeimage_point)
{
	SIZE image_size;

	ImageViewer_GetImageSize(viewer_window, &image_size);    
	
	freeimage_point->x = image_point.x;
	freeimage_point->y = image_size.cy - image_point.y - 1;
}
 
 
POINT ImageViewer_GetClosetPosOnZoomedImageFromCursorPos(HWND viewer_window)
{
	RECT image_rect, window_rect;
	POINT cursor_position, temp_pos;  
		
	GetCursorPos(&cursor_position); 
		
	ImageViewer_ScreenPointToWindowPoint(viewer_window, cursor_position, &temp_pos ); 
		
	ImageViewer_GetDisplayedImageRect(viewer_window, &image_rect);       
		
	ImageViewer_GetDisplayedImageScreenRect(viewer_window, &window_rect);    

	// The cursor is outside of the imageviewer window - so this timer is active.
	// we calculate the closest position on the window to the cursor.
	if(cursor_position.x < window_rect.left)
		temp_pos.x = image_rect.left;	 
	else if(cursor_position.x > window_rect.right)
		temp_pos.x = image_rect.right;
		
	if(cursor_position.y < window_rect.top)
		temp_pos.y = image_rect.top;		
	else if(cursor_position.y > window_rect.bottom)
		temp_pos.y = image_rect.bottom;

	return temp_pos;
}
