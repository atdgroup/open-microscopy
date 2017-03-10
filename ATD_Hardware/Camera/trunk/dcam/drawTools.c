#include "toolbox.h"
#include <utility.h>
#include <cvirte.h>
#include <userint.h>
#include <ansi_c.h>
#include "drawTools.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FILE:    drawTools.c                                                      */
/*          RJL April 2001                                                   */
/*          PRB May 2002                                                     */
/*          PRB June 2002                                                    */
/*                                                                           */
/* PURPOSE: Tool for defining the size and position of a box, oval or triangle. */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/* RJL March 2004                                                   		 */
/* Mods for use with multi-photon systems.		 							 */
/* 1. Add routine to programmatically set a fixed aspect ratio  		     */
/* 2. Remove drag points at top, bottom, right and left           		     */
/* 3. Don't allow dragging outside of the canvas                    		 */
/* 4. Don't call the callback routine on initialisation of the box  		 */
/* 5. If user clicks anywhere inside the canvas make this the new centre	 */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Include files                                                             */
/* drawTools.h                                                               */
/*---------------------------------------------------------------------------*/

#define HANDLE_SIZE 8
#define CROSS_SIZE 5
#define DELAY 0.01
#define MOUSE_INTERVAL 0.1

static Rect  rectVal, rectOut, rectL, rectT, rectR, rectB, rectTL, rectTR, rectBL, rectBR;
static int rectHeight, rectWidth, canvasWidth, canvasHeight;
static int colBg, col1, panel=-1, canvas=-1, bgCanvas, tool, gFixRatio=0;
static float gRatio=-1.0;
static Point centre, offset;
static void (*comitCallback)(Rect, void* callback_data);
static void *comitData;

static void ReDrawRect(int x, int y);
static int CVICALLBACK canvasCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int GCI_setupCanvasDrawTools(int panelIn, int controlIn, int toolIn, int boxColour, int bgColour,
							 void (*externalFn)(Rect, void* data), void* callback_data)
{   // tool, 0=rectangle, 1=oval, 2=right triangle, 3=square
	//		 4=horizontal line, 5=vertical line, 6=point
	int t, l;
	
	panel = panelIn;
	bgCanvas=controlIn;
	comitCallback = externalFn;
	comitData = callback_data;
	tool = toolIn;
	
	GCI_fixShapeAspectRatio(-1.0);
	gFixRatio = 0;
	if (tool == 3) GCI_fixShapeAspectRatio(1.0);   //square
	
	//Set up colours
	colBg = bgColour;
	col1  = boxColour;

	//Set up canvas
	GetCtrlAttribute (panel, bgCanvas, ATTR_WIDTH, &canvasWidth);
	GetCtrlAttribute (panel, bgCanvas, ATTR_HEIGHT, &canvasHeight);
	GetCtrlAttribute (panel, bgCanvas, ATTR_TOP,  &t);
	GetCtrlAttribute (panel, bgCanvas, ATTR_LEFT, &l);

	if (canvas==-1) 
	{
		canvas = NewCtrl (panel, CTRL_CANVAS, "", t, l);
		SetCtrlAttribute (panel, canvas, ATTR_HEIGHT, canvasHeight);
		SetCtrlAttribute (panel, canvas, ATTR_WIDTH, canvasWidth);
		SetCtrlAttribute (panel, canvas, ATTR_PICT_BGCOLOR, VAL_TRANSPARENT);
		SetCtrlAttribute (panel, canvas, ATTR_PEN_COLOR, col1);
		SetCtrlAttribute (panel, canvas, ATTR_CTRL_MODE, VAL_INDICATOR);
		SetCtrlAttribute (panel, canvas, ATTR_CALLBACK_FUNCTION_POINTER, canvasCallback);
		SetCtrlAttribute (panel, canvas, ATTR_OVERLAPPED_POLICY, VAL_DRAW_ON_TOP);
		SetCtrlAttribute (panel, canvas, ATTR_DRAW_POLICY, VAL_UPDATE_IMMEDIATELY);

		//Initial box
		rectWidth = canvasWidth;
		rectHeight = canvasHeight;
		ReDrawRect(canvasWidth/2, canvasHeight/2);
	}
	else {
		ReDrawRect(rectVal.left+rectVal.width/2, rectVal.top+rectVal.height/2);
	}
	
	//(*comitCallback)(rectVal);
	EnableExtendedMouseEvents (panel, canvas, MOUSE_INTERVAL);

	return(0);
}

int GCI_stopCanvasDrawTools(void)
{
	if(panel == -1 || canvas == -1)
		return 0;

	CanvasClear (panel, canvas, rectOut);
	DisableExtendedMouseEvents (panel, canvas);
	DiscardCtrl (panel, canvas);
	SetMouseCursor (VAL_DEFAULT_CURSOR);
	canvas=-1;
	
	return(0);
}

static int bitmapDrawTriangle (IPIImageRef image, Rect rect, int drawMode, int side)
{   // side is 0,1,2 or 3, for 4 orientations
	float val=1.0;
	
	if (side<0 || side>4) return(-1);

	if (side==0 || side==2)
	{
		IPI_DrawLine (image, image, MakePoint(rect.left, rect.top), 
									MakePoint(rect.left+rect.width, rect.top+rect.height), drawMode, val);
		if (side==0)
		{
			IPI_DrawLine (image, image, MakePoint(rect.left+rect.width, rect.top+rect.height), 
										MakePoint(rect.left, rect.top+rect.height), drawMode, val);
			IPI_DrawLine (image, image, MakePoint(rect.left, rect.top+rect.height), 
										MakePoint(rect.left, rect.top), drawMode, val);
		}
		else
		{
			IPI_DrawLine (image, image, MakePoint(rect.left+rect.width, rect.top+rect.height), 
										MakePoint(rect.left+rect.width, rect.top), drawMode, val);
			IPI_DrawLine (image, image, MakePoint(rect.left+rect.width, rect.top), 
										MakePoint(rect.left, rect.top), drawMode, val);
		}
	}
	else
	{
		IPI_DrawLine (image, image, MakePoint(rect.left+rect.width, rect.top), 
									MakePoint(rect.left, rect.top+rect.height), drawMode, val);
		if (side==1)
		{
			IPI_DrawLine (image, image, MakePoint(rect.left, rect.top+rect.height), 
										MakePoint(rect.left, rect.top), drawMode, val);
			IPI_DrawLine (image, image, MakePoint(rect.left, rect.top), 
										MakePoint(rect.left+rect.width, rect.top), drawMode, val);
		}
		else
		{
			IPI_DrawLine (image, image, MakePoint(rect.left, rect.top+rect.height), 
										MakePoint(rect.left+rect.width, rect.top+rect.height), drawMode, val);
			IPI_DrawLine (image, image, MakePoint(rect.left+rect.width, rect.top+rect.height), 
										MakePoint(rect.left+rect.width, rect.top), drawMode, val);
		}
	}
	
	if (drawMode==IPI_DRAW_PAINT || drawMode==IPI_INVERT_PAINT)
			IPI_FillHole (image, image, TRUE);
		    
	return(0);
}

static void bitmapDrawCross(IPIImageRef image, int drawMode, int x, int y)
{
	IPI_DrawLine (image, image, MakePoint(x-CROSS_SIZE, y), MakePoint(x+CROSS_SIZE, y), drawMode, 1.0); 
	IPI_DrawLine (image, image, MakePoint(x, y-CROSS_SIZE), MakePoint(x, y+CROSS_SIZE), drawMode, 1.0); 
}
int GCI_getDrawToolsMask(IPIImageRef image, int drawMode)
{
	int error=0, midx, midy, right, bottom;
	
	error = IPI_SetImageSize (image, canvasWidth, canvasHeight);
	if (error==0)
		error = IPI_DrawRect (image, image, IPI_FULL_RECT, IPI_DRAW_PAINT, 0.0);
	
	if (error==0)
	{
		midx = rectVal.left + rectVal.width/2;
		midy = rectVal.top + rectVal.height/2;
		right = rectVal.left + rectVal.width;
		bottom = rectVal.top + rectVal.height;
		
		if      (tool==1) IPI_DrawOval (image, image, rectVal, drawMode, 1.0);
		else if (tool==2) bitmapDrawTriangle (image, rectVal, drawMode, tool-2);
		else if (tool==4) IPI_DrawLine (image, image, MakePoint(rectVal.left, midy), MakePoint(right, midy), drawMode, 1.0);
		else if (tool==5) IPI_DrawLine (image, image, MakePoint(midx, rectVal.top), MakePoint(midx, bottom), drawMode, 1.0);
		else if (tool==6) bitmapDrawCross (image, drawMode, rectVal.left, rectVal.top);
		else              IPI_DrawRect (image, image, rectVal, drawMode, 1.0);
						  
	}
	
	return(error);
}

static int canvasDrawTriangle (int panel, int canvas, Rect rect, int side)
{   // side is 0,1,2 or 3, for 4 orientations
	if (side<0 || side>4) return(-1);

	if (side==0 || side==2)
	{
		CanvasSetPenPosition (panel, canvas, MakePoint(rect.left, rect.top));
		CanvasDrawLineTo (panel, canvas, MakePoint(rect.left+rect.width, rect.top+rect.height));
		if (side==0)
			CanvasDrawLineTo (panel, canvas, MakePoint(rect.left, rect.top+rect.height));
		else
			CanvasDrawLineTo (panel, canvas, MakePoint(rect.left+rect.width, rect.top));
		CanvasDrawLineTo (panel, canvas, MakePoint(rect.left, rect.top));
	}
	else
	{
		CanvasSetPenPosition (panel, canvas, MakePoint(rect.left+rect.width, rect.top));
		CanvasDrawLineTo (panel, canvas, MakePoint(rect.left, rect.top+rect.height));
		if (side==1)
			CanvasDrawLineTo (panel, canvas, MakePoint(rect.left, rect.top));
		else
			CanvasDrawLineTo (panel, canvas, MakePoint(rect.left+rect.width, rect.top+rect.height));
		CanvasDrawLineTo (panel, canvas, MakePoint(rect.left+rect.width, rect.top));
	}
	return(0);
}
static void canvasDrawCross(int panel, int canvas, int x, int y)
{
	CanvasDrawLine (panel, canvas, MakePoint(x-CROSS_SIZE, y), MakePoint(x+CROSS_SIZE, y)); 
	CanvasDrawLine (panel, canvas, MakePoint(x, y-CROSS_SIZE), MakePoint(x, y+CROSS_SIZE)); 
}

static void ReDrawRect(int x, int y)
{
	int l, t, r, b, midv, midh, h, hb2;
	int midx, midy, right, bottom;
	
	CanvasClear (panel, canvas, rectOut);
	
	if (rectWidth < 0) rectWidth = -rectWidth; 
	if (rectHeight < 0) rectHeight = -rectHeight;

	// keep bounding rect on the canvas
	if (x<rectWidth/2) x=rectWidth/2;
	if ((x+rectWidth/2) > canvasWidth) x = canvasWidth - rectWidth/2;
	if (y<rectHeight/2) y=rectHeight/2;
	if ((y+rectHeight/2) > canvasHeight) y = canvasHeight - rectHeight/2;
	
    rectVal = MakeRect (y-rectHeight/2, x-rectWidth/2, rectHeight, rectWidth);

    if (tool==6) {	  //Point tool
    	rectOut = MakeRect (y-(CROSS_SIZE+1),x-(CROSS_SIZE+1),(CROSS_SIZE+1)*2,(CROSS_SIZE+1)*2);
    	canvasDrawCross(panel, canvas, x, y);
    	return;
    }

	h   = HANDLE_SIZE;
   	hb2 = HANDLE_SIZE/2;
    l    = rectVal.left                  - hb2;
   	t    = rectVal.top                   - hb2;
    r    = rectVal.left + rectVal.width  - hb2;
   	b    = rectVal.top  + rectVal.height - hb2;
	midh = rectVal.left + (rectVal.width  - h) / 2;
   	midv = rectVal.top  + (rectVal.height - h) / 2;
                         
   	rectOut = MakeRect (t, l, rectVal.height + h, rectVal.width + h);
	rectTL  = MakeRect (t, l, HANDLE_SIZE, HANDLE_SIZE);
	rectTR  = MakeRect (t, r, HANDLE_SIZE, HANDLE_SIZE);
	rectBL  = MakeRect (b, l, HANDLE_SIZE, HANDLE_SIZE);
	rectBR  = MakeRect (b, r, HANDLE_SIZE, HANDLE_SIZE);
	rectT   = MakeRect (t, midh, HANDLE_SIZE, HANDLE_SIZE);
	rectL   = MakeRect (midv, l, HANDLE_SIZE, HANDLE_SIZE);
	rectB   = MakeRect (b, midh, HANDLE_SIZE, HANDLE_SIZE);
	rectR   = MakeRect (midv, r, HANDLE_SIZE, HANDLE_SIZE);

    CanvasStartBatchDraw (panel, canvas);
   	SetCtrlAttribute (panel, canvas, ATTR_PEN_WIDTH, 1);
    SetCtrlAttribute (panel, canvas, ATTR_PEN_COLOR, col1);
	SetCtrlAttribute (panel, canvas, ATTR_PEN_FILL_COLOR, colBg);
	
	midx = rectVal.left + rectVal.width/2;
	midy = rectVal.top + rectVal.height/2;
	right = rectVal.left + rectVal.width;
	bottom = rectVal.top + rectVal.height;
	if      (tool==1) CanvasDrawOval (panel, canvas, rectVal, VAL_DRAW_FRAME);
	else if (tool==2) canvasDrawTriangle (panel, canvas, rectVal, tool-2);
	else if (tool==4) CanvasDrawLine (panel, canvas, MakePoint(rectVal.left, midy), MakePoint(right, midy));
	else if (tool==5) CanvasDrawLine (panel, canvas, MakePoint(midx, rectVal.top), MakePoint(midx, bottom));
	else              CanvasDrawRect (panel, canvas, rectVal, VAL_DRAW_FRAME);
					  
	if (tool < 4) {  //corner boxes, but not for lines nor points
		CanvasDrawRect (panel, canvas, rectTL, VAL_DRAW_FRAME_AND_INTERIOR);
		CanvasDrawRect (panel, canvas, rectTR, VAL_DRAW_FRAME_AND_INTERIOR);
		CanvasDrawRect (panel, canvas, rectBL, VAL_DRAW_FRAME_AND_INTERIOR);
		CanvasDrawRect (panel, canvas, rectBR, VAL_DRAW_FRAME_AND_INTERIOR);
	}
	if (gFixRatio == 0) {	//not square add side boxes if appropriate
		if ((tool == 5) || ((tool != 4) && (rectVal.width>3*HANDLE_SIZE)))
		{
			CanvasDrawRect (panel, canvas, rectT, VAL_DRAW_FRAME_AND_INTERIOR);
			CanvasDrawRect (panel, canvas, rectB, VAL_DRAW_FRAME_AND_INTERIOR);
		}		
		if ((tool == 4) || ((tool != 5) && (rectVal.height>3*HANDLE_SIZE)))
		{
			CanvasDrawRect (panel, canvas, rectL, VAL_DRAW_FRAME_AND_INTERIOR);
			CanvasDrawRect (panel, canvas, rectR, VAL_DRAW_FRAME_AND_INTERIOR);
		}
	}
   	CanvasEndBatchDraw (panel, canvas);
}

void GCI_externalReDrawShapeTool (Rect box)
{
	rectWidth  = box.width;
	rectHeight = box.height;
	
	ReDrawRect(box.left+box.width/2, box.top+box.height/2);
}

void GCI_fixShapeAspectRatio (float ratio)
{
	gFixRatio = 1;
	gRatio = ratio;
}
static void keepAspectRatio(int *x, int *y)
{
	int ref;
	
	if ((float)rectHeight/((float)rectWidth+1e-10) < gRatio)
	{
		ref = *y+rectHeight;
		rectHeight = gRatio*rectWidth;
		if (rectHeight > canvasHeight) {
			rectHeight = canvasHeight;
			rectWidth = canvasHeight/gRatio;
		}
		*y = ref-rectHeight;
	}
	else
	{
		ref = *x+rectWidth;
		rectWidth = rectHeight/gRatio;
		if (rectWidth > canvasWidth) {
			rectWidth = canvasWidth;
			rectHeight = gRatio*rectWidth;
		}
		*x = ref-rectWidth;
	}
	
	return;
}
	
static void checkXY (int *x, int *y)
{
	if (*x<0) *x=0;
	else if (*x>canvasWidth) *x=canvasWidth;
	if (*y<0) *y=0;
	else if (*y>canvasHeight) *y=canvasHeight;
}

static void TopLeft(int x, int y, int modifier)
{
	checkXY(&x, &y);
	rectWidth += rectVal.left - x;
	rectHeight += rectVal.top - y;
	if (gFixRatio) keepAspectRatio(&x, &y);
	if (modifier & VAL_SHIFT_MODIFIER) keepAspectRatio(&x, &y);
	if (modifier & VAL_MENUKEY_MODIFIER)
		ReDrawRect(centre.x, centre.y);	
	else
		ReDrawRect(x+rectWidth/2, y+rectHeight/2);	
}
static void TopRight(int x, int y, int modifier)
{
	int dummy=0;
	checkXY(&x, &y);
	rectWidth += x - (rectVal.left+rectWidth);
	rectHeight += rectVal.top - y;
	if (gFixRatio) keepAspectRatio(&dummy, &y);
	if (modifier & VAL_SHIFT_MODIFIER) keepAspectRatio(&dummy, &y);
	if (modifier & VAL_MENUKEY_MODIFIER)
		ReDrawRect(centre.x, centre.y);	
	else
		ReDrawRect(rectVal.left+rectWidth/2, y+rectHeight/2);	
}
static void BottomRight(int x, int y, int modifier)
{
	int dummy=0;
	checkXY(&x, &y);
	rectWidth += x - (rectVal.left+rectWidth);
	rectHeight += y - (rectVal.top+rectHeight);
	if (gFixRatio) keepAspectRatio(&dummy, &dummy);
	if (modifier & VAL_SHIFT_MODIFIER) keepAspectRatio(&dummy, &dummy);
	if (modifier & VAL_MENUKEY_MODIFIER)
		ReDrawRect(centre.x, centre.y);	
	else
		ReDrawRect(rectVal.left+rectWidth/2, rectVal.top+rectHeight/2);	
}
static void BottomLeft(int x, int y, int modifier)
{
	int dummy=0;
	checkXY(&x, &y);
	rectWidth += rectVal.left - x;
	rectHeight += y - (rectVal.top+rectHeight);
	if (gFixRatio) keepAspectRatio(&x, &dummy);
	if (modifier & VAL_SHIFT_MODIFIER) keepAspectRatio(&x, &dummy);
	if (modifier & VAL_MENUKEY_MODIFIER)
		ReDrawRect(centre.x, centre.y);	
	else
		ReDrawRect(x+rectWidth/2, rectVal.top+rectHeight/2);	
}
static void Bottom(int x, int y, int modifier)
{
	int dummy=0;

	checkXY(&x, &y);
	rectHeight += y - (rectVal.top+rectHeight);
	if (gFixRatio) keepAspectRatio(&dummy, &dummy);
	if (modifier & VAL_MENUKEY_MODIFIER)
		ReDrawRect(centre.x, centre.y);	
	else
		ReDrawRect(rectVal.left+rectWidth/2, rectVal.top+rectHeight/2);	
}
static void Left(int x, int y, int modifier)
{
	checkXY(&x, &y);
	rectWidth += rectVal.left - x;
	if (gFixRatio) keepAspectRatio(&x, &y);
	if (modifier & VAL_MENUKEY_MODIFIER)
		ReDrawRect(centre.x, centre.y);	
	else
		ReDrawRect(x+rectWidth/2, rectVal.top+rectHeight/2);	
}
static void Top(int x, int y, int modifier)
{
	checkXY(&x, &y);
	rectHeight += rectVal.top - y;
	if (gFixRatio) keepAspectRatio(&x, &y);
	if (modifier & VAL_MENUKEY_MODIFIER)
		ReDrawRect(centre.x, centre.y);	
	else
		ReDrawRect(rectVal.left+rectWidth/2, y+rectHeight/2);	
}
static void Right(int x, int y, int modifier)
{
	int dummy=0;

	checkXY(&x, &y);
	rectWidth += x - (rectVal.left+rectWidth);
	if (gFixRatio) keepAspectRatio(&dummy, &dummy);
	if (modifier & VAL_MENUKEY_MODIFIER)
		ReDrawRect(centre.x, centre.y);	
	else
		ReDrawRect(rectVal.left+rectWidth/2, rectVal.top+rectHeight/2);	
}

static int CVICALLBACK canvasCallback (int panel, int control, int event, 
                            void *callbackData, int eventData1, int eventData2)
{
   int mouseX, mouseY, leftDown, modifier, activePanel;
   
   if (event == EVENT_LEFT_CLICK) 
    {
		GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
   		// Get aspect ratio
   		gRatio = (float)rectHeight / (float)rectWidth;
   		
   		// Get Centre
   		centre.x = rectVal.left + rectVal.width/2;
   		centre.y = rectVal.top  + rectVal.height/2;
   		// Get offset to centre
   		offset.x = centre.x - mouseX;
   		offset.y = centre.y - mouseY;
       		
		if ((tool != 6) && RectContainsPoint (rectOut, MakePoint(mouseX,mouseY))) 
        {
        	if      ((tool<4) && RectContainsPoint (rectTL, MakePoint(mouseX,mouseY)))
        		while (leftDown) {
        			TopLeft(mouseX, mouseY, modifier);
			        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
					Delay (DELAY);
			    }
        	else if ((tool<4) && RectContainsPoint (rectBR, MakePoint(mouseX,mouseY)))
        		while (leftDown) {
	        		BottomRight(mouseX, mouseY, modifier);
			        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
					Delay (DELAY);
			    }
        	else if ((tool<4) && RectContainsPoint (rectTR, MakePoint(mouseX,mouseY)))
        		while (leftDown) {
	        		TopRight(mouseX, mouseY, modifier);
			        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
					Delay (DELAY);
			    }
        	else if ((tool<4) && RectContainsPoint (rectBL, MakePoint(mouseX,mouseY)))
        		while (leftDown) {
	        		BottomLeft(mouseX, mouseY, modifier);
			        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
					Delay (DELAY);
			    }
      	 	else if (((tool<3) || (tool==4)) && RectContainsPoint (rectL, MakePoint(mouseX,mouseY)))
        		while (leftDown) {
	        		Left(mouseX, mouseY, modifier);
			        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
					Delay (DELAY);
			    }
     	 	else if (((tool<3) || (tool==5)) && RectContainsPoint (rectT, MakePoint(mouseX,mouseY)))
        		while (leftDown) {
	        		Top(mouseX, mouseY, modifier);
			        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
					Delay (DELAY);
			    }
     	 	else if (((tool<3) || (tool==4)) && RectContainsPoint (rectR, MakePoint(mouseX,mouseY)))
        		while (leftDown) {
	        		Right(mouseX, mouseY, modifier);
			        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
					Delay (DELAY);
			    }
     	 	else if (((tool<3) || (tool==5)) && RectContainsPoint (rectB, MakePoint(mouseX,mouseY)))
        		while (leftDown) {
	        		Bottom(mouseX, mouseY, modifier);
			        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
					Delay (DELAY);
			    }
		}
       	while (leftDown) {  //just move it
       		ReDrawRect(mouseX, mouseY);
	        GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
			Delay (DELAY);
	    }
		
		(*comitCallback)(rectVal, comitData);
    }

    else if (event == EVENT_MOUSE_MOVE) 
    {
		if (tool == 6) return 1;	//point tool
		
		activePanel = GetActivePanel ();
		if (panel==activePanel)
		{
			GetRelativeMouseState (panel, canvas, &mouseX, &mouseY, &leftDown, 0, &modifier);
			if (RectContainsPoint (rectOut, MakePoint(mouseX,mouseY)))
			{
   	    	 	if      ((tool<4) && RectContainsPoint (rectTL, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_SIZE_NW_SE_CURSOR);
				else if ((tool<4) && RectContainsPoint (rectBR, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_SIZE_NW_SE_CURSOR);
				else if ((tool<4) && RectContainsPoint (rectTR, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_SIZE_NE_SW_CURSOR);
				else if ((tool<4) && RectContainsPoint (rectBL, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_SIZE_NE_SW_CURSOR);
				else if (((tool<3) || (tool==4)) && RectContainsPoint (rectL, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_SIZE_EW_CURSOR);
				else if (((tool<3) || (tool==4)) && RectContainsPoint (rectR, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_SIZE_EW_CURSOR);
				else if (((tool<3) || (tool==5)) && RectContainsPoint (rectT, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_SIZE_NS_CURSOR);
				else if (((tool<3) || (tool==5)) && RectContainsPoint (rectB, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_SIZE_NS_CURSOR);
				else if (RectContainsPoint (rectVal, MakePoint(mouseX,mouseY)))
					SetMouseCursor (VAL_OPEN_HAND_CURSOR);
				else 
					SetMouseCursor (VAL_DEFAULT_CURSOR);
			}
			else 
				SetMouseCursor (VAL_DEFAULT_CURSOR);
		}
		else 
			SetMouseCursor (VAL_DEFAULT_CURSOR);
	}
//	return 0;
	return 1; // swallow the event
}
