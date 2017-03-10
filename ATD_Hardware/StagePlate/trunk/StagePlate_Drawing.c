#include "HardWareTypes.h"

#include "StagePlate.h"
#include "StagePlateUI.h"
#include "device_list_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"
#include "stage/stage.h"

#include "toolbox.h"
#include "asynctmr.h"

#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 

#define DRAW_COLOUR			VAL_BLACK
#define HIGHLIGHT_COLOUR    VAL_RED      // highlighted 1, well also filled

ScaledCanvas* scaled_canvas_new(StagePlateModule* stage_plate_module, int panel, const char *label, int top, int left)
{
	ScaledCanvas *canvas = (ScaledCanvas*) malloc(sizeof(ScaledCanvas));
	
	canvas->panel_id = panel;
	canvas->canvas = NewCtrl(panel, CTRL_CANVAS, label, top, left);
	canvas->stage_plate_module = stage_plate_module;

	SetCtrlAttribute(panel, canvas->canvas, VAL_MARK_FOR_UPDATE, 1);
	scaled_canvas_set_scale_rect(canvas, 0.0, 0.0, 100.0, 100.0);

	return canvas;
}

void scaled_canvas_set_scale_rect(ScaledCanvas* canvas, double left, double top, double width, double height)
{
	canvas->scale_left = (float) left;
	canvas->scale_top = (float) top;
	canvas->scale_width = (float) width;
	canvas->scale_height = (float) height;
}

void scaled_canvas_set_draw_rect(ScaledCanvas* canvas, Rect rect)
{
	canvas->draw_rect = rect;
}

static Point translate_point_to_canvas_point(ScaledCanvas* canvas, double x, double y)
{
	Point pt;
	int w, h;

	GetCtrlAttribute(canvas->panel_id, canvas->canvas, ATTR_WIDTH, &w);
    GetCtrlAttribute(canvas->panel_id, canvas->canvas, ATTR_HEIGHT, &h);

	pt.x = (int)(((x - canvas->scale_left) / canvas->scale_width) * w);
	pt.y = (int)(((y - canvas->scale_top) / canvas->scale_height) * h);

	//pt.x += canvas->draw_rect.left;
	//pt.y += canvas->draw_rect.top;

	return pt;
}

static Rect translate_rect_to_canvas_rect(ScaledCanvas* canvas,
										  double left, double top, double width, double height)
{
	Rect rect;

	Point top_left = translate_point_to_canvas_point(canvas, left, top);
	Point right_bottom = translate_point_to_canvas_point(canvas, left + width - 1, top + height - 1);

	rect.left = top_left.x;
	rect.top = top_left.y;
	rect.width = right_bottom.x - top_left.x + 1;
	rect.height = right_bottom.y - top_left.y + 1;

	return rect;
}

static void scaled_canvas_draw_rect(ScaledCanvas* canvas,
						 double left, double top, double width, double height,
						 int drawMode, int colour)
{
	Rect rect = translate_rect_to_canvas_rect(canvas, left, top, width, height);

	SetCtrlAttribute(canvas->panel_id, canvas->canvas, ATTR_PEN_COLOR,  colour);
    SetCtrlAttribute(canvas->panel_id, canvas->canvas, ATTR_PEN_FILL_COLOR,  colour);

	CanvasDrawRect (canvas->panel_id, canvas->canvas, rect, drawMode);
}

int scaled_canvas_draw_circle_around_center(ScaledCanvas* canvas,
						 double cx, double cy, double radius,
						 int drawMode, int colour)
{
	Rect rect;

	Point top_left = translate_point_to_canvas_point(canvas, cx - radius, cy - radius);
	Point right_bottom = translate_point_to_canvas_point(canvas, cx + radius, cy + radius);

	rect.left = top_left.x;
	rect.top = top_left.y;
	rect.width = right_bottom.x - top_left.x + 1;
	rect.height = right_bottom.y - top_left.y + 1;

	SetCtrlAttribute(canvas->panel_id, canvas->canvas, ATTR_PEN_COLOR,  colour);
    SetCtrlAttribute(canvas->panel_id, canvas->canvas, ATTR_PEN_FILL_COLOR,  colour);

	return CanvasDrawOval (canvas->panel_id, canvas->canvas, rect, drawMode);
}

Well* find_canvas_region (ScaledCanvas* canvas, int x, int y)
{
    int i;
    Well *well = NULL;
	Rect rect;

    for (i=0; i < canvas->stage_plate_module->_current_number_of_wells; i++)
    {
		well = &canvas->stage_plate_module->wells[i];

		rect.left = (int) (well->region.cx - well->region.radius);
		rect.top = (int) (well->region.cy - well->region.radius);
		rect.width = (int) (well->region.radius * 2);
		rect.height = (int) (well->region.radius * 2);

		rect = translate_rect_to_canvas_rect(canvas, rect.left, rect.top, rect.width, rect.height);

        if (RectContainsPoint (rect, MakePoint(x, y)))
        {
            return well;
        }
    }
        
    return NULL;
}

static void canvas_select_all_wells (ScaledCanvas* canvas)
{
    int i;

    for (i=0; i < canvas->stage_plate_module->_current_number_of_wells; i++)
    {
	    canvas->stage_plate_module->wells[i].selected = 1;
    }
        
    return;
}

static void canvas_unselect_all_wells (ScaledCanvas* canvas)
{
    int i;

    for (i=0; i < canvas->stage_plate_module->_current_number_of_wells; i++)
    {
	    canvas->stage_plate_module->wells[i].selected = 0;
    }
        
    return;
}

static void canvas_select_wells_in_region (ScaledCanvas* canvas,
										   float left, float top, float right, float bottom)
{
    int i;
    Well *well = NULL;
	Rect rect;

	//rect = translate_rect_to_canvas_rect(canvas, left, top, right - left + 1, bottom - top + 1);
	rect.left = (int) (left - 1);
	rect.top = (int) (top - 1);
	right = (right + 1);
	bottom = (bottom + 1);	// Round up
	rect.width = (int) (right - left + 1);
	rect.height = (int) (bottom - top + 1);

    for (i=0; i < canvas->stage_plate_module->_current_number_of_wells; i++)
    {
		well = &canvas->stage_plate_module->wells[i];

        if (RectContainsPoint (rect, MakePoint((int) well->region.cx, (int) well->region.cy)))
            well->selected = 1;
//		else
//			well->selected = 0;
    }
        
    return;
}

int CVICALLBACK OnCanvasEvent (int panelHandle, int controlID, int event, void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_LEFT_CLICK:
		{
			ScaledCanvas* canvas = (ScaledCanvas*) callbackData;
			StagePlateModule* stage_plate_module = (StagePlateModule*) canvas->stage_plate_module;

			StagePlate plate;

			int iPanel, x, y, left, right, keys, ctrlTop, ctrlLeft, panel_width, panel_height;
			static Well* previous_well = NULL;
			Well *well = NULL;

			stage_plate_get_current_plate(stage_plate_module, &plate);

			GetGlobalMouseState (&iPanel, &x, &y, &left, &right, &keys);   

			GetPanelAttribute(panelHandle, ATTR_WIDTH, &panel_width);
            GetPanelAttribute(panelHandle, ATTR_HEIGHT, &panel_height);
            GetCtrlAttribute(panelHandle, controlID, ATTR_LEFT, &ctrlLeft);
            GetCtrlAttribute(panelHandle, controlID, ATTR_TOP, &ctrlTop);
            x = eventData2 - ctrlLeft;
            y = eventData1 - ctrlTop;

			well = find_canvas_region (canvas, x, y);

			if(well == NULL)
				return -1;

			if(keys & VAL_SHIFT_MODIFIER)
			{
				// If we have a previous well
				// select the bounding region
				if(previous_well != NULL) {

					float left = MIN(previous_well->region.cx, well->region.cx) - well->region.radius;
					float right = MAX(previous_well->region.cx, well->region.cx) + well->region.radius;
					float top = MIN(previous_well->region.cy, well->region.cy) - well->region.radius;
					float bottom = MAX(previous_well->region.cy, well->region.cy) + well->region.radius;
			
					canvas_select_wells_in_region (canvas, left, top, right, bottom);

					previous_well = NULL; // stop the area selection
				}	
			}
			else if(keys & VAL_MENUKEY_MODIFIER)  // with CTRL
			{
				if(well != NULL) {	
					well->selected = !well->selected;	
					previous_well = well;
				}
			}
			else {

				canvas_unselect_all_wells(canvas);
				
				canvas_select_wells_in_region (canvas, well->region.cx - well->region.radius, well->region.cy - well->region.radius, well->region.cx + well->region.radius, well->region.cy + well->region.radius);

					// Uncomment these lines, instead of those above, to allow individual selection of wells, this leads to disjointed sections of wells
					// only allowing PLATE_SEL_DIALOG_RECT as a result
	//			if(well != NULL) {	
	//				well->selected = !well->selected;	
	//			}

				previous_well = well;
			}


			stage_plate_module->_region_selection_result = PLATE_SEL_DIALOG_RECT;  // only one allowed now

//			PostMessage(stage_plate_module->_region_selection_hwnd, WM_SIZE, 0, MAKELPARAM(panel_width, panel_height)); 
			
			draw_current_region_selection_dialog(stage_plate_module, &plate);

            break;
		}
	}

    return 0;
}


void draw_wells_on_canvas (StagePlateModule* stage_plate_module, ScaledCanvas* canvas, StagePlate *plate)
{
	int i;
	Well *well = NULL;

	if(plate->type != PLATE_WELLPLATE)
		return;

    for (i=0; i < canvas->stage_plate_module->_current_number_of_wells; i++)
    {
		well = &canvas->stage_plate_module->wells[i];

		if(well->selected) {

			scaled_canvas_draw_circle_around_center(canvas,
						 well->region.cx, well->region.cy, plate->x_size / 2 * 1000.0,
						 VAL_DRAW_FRAME_AND_INTERIOR, VAL_GREEN);
		}
		else {

			scaled_canvas_draw_circle_around_center(canvas,
						 well->region.cx, well->region.cy, plate->x_size / 2 * 1000.0,
						 VAL_DRAW_FRAME, VAL_GREEN);

		}
    }
}

static Rect calculate_canvas_scale(StagePlateModule* stage_plate_module, int panel, int control)
{
	int width, height;
	Rect rect;

	GetCtrlAttribute(panel, control, ATTR_WIDTH, &width);
	GetCtrlAttribute(panel, control, ATTR_HEIGHT, &height);

	if(stage_plate_module->stage_aspect_ratio < 1.0) {

		// Height of stage limits is larger
		width = (int) (height * stage_plate_module->stage_aspect_ratio);
		rect.width = width;
		rect.height = (int) (width * stage_plate_module->stage_aspect_ratio);
	}
	else {
		
		// Width is greater		
		height = (int) (width / stage_plate_module->stage_aspect_ratio);
		rect.height = height;
		rect.width = (int) (height * stage_plate_module->stage_aspect_ratio);	
	}

	rect.left = (width - rect.width) / 2;
	rect.top = (height - rect.height) / 2;

	return rect;
}


void draw_plate_on_canvas (StagePlateModule* stage_plate_module, ScaledCanvas* canvas, StagePlate *plate)
{
    RECT roi;
    int draw_mode=VAL_DRAW_FRAME;   // default draw mode
    int xrange, yrange;

	CanvasStartBatchDraw (canvas->panel_id, canvas->canvas);

	CanvasClear (canvas->panel_id, canvas->canvas, VAL_ENTIRE_OBJECT);

	// draw stage limits
	scaled_canvas_draw_rect(canvas,
									stage_plate_module->stage->_limits.min_x,
									stage_plate_module->stage->_limits.min_y,
									stage_plate_module->stage->_limits.max_x - stage_plate_module->stage->_limits.min_x,
									stage_plate_module->stage->_limits.max_y - stage_plate_module->stage->_limits.min_y,
									VAL_DRAW_FRAME, VAL_BLACK);
						 
	// Draw plate safe region
	xrange = (int) (plate->safe_right_bottom.x - plate->safe_left_top.x);
	yrange = (int) (plate->safe_right_bottom.y - plate->safe_left_top.y);
	
	if (plate->safe_region_shape == STAGE_SHAPE_RECTANGLE){

		scaled_canvas_draw_rect (canvas,
									plate->safe_left_top.x, plate->safe_left_top.y,
									xrange,
									yrange,
									VAL_DRAW_FRAME, VAL_RED);
	}
	else {
	//	CanvasDrawScaledOval (stage_plate_module, panel, control,
	//								plate->safe_left_top.x, plate->safe_left_top.y,
	//								xrange,
	//								yrange,
	//								VAL_DRAW_FRAME, VAL_RED, scale_rect);
	}

	 
	// Draw plate outline and wells
	
	stage_plate_get_entire_region_of_interest_for_plate(stage_plate_module, plate, &roi);

	scaled_canvas_draw_rect (canvas,
									roi.left, roi.top,
									roi.right - roi.left,
									roi.bottom - roi.top,
									VAL_DRAW_FRAME, VAL_BLUE);

	if(plate->type == PLATE_WELLPLATE) {

		draw_wells_on_canvas (stage_plate_module, canvas, plate);
	}	

	CanvasEndBatchDraw (canvas->panel_id, canvas->canvas);
}