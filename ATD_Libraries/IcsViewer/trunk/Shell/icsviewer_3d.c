#include "icsviewer_window.h" 
#include "icsviewer_private.h"

#include "icsviewer_uir.h"
#include "gci_utils.h" 

#include "icsviewer_3d.h"   
#include "icsviewer_ics3d.h" 

#include "string_utils.h"

#include "profile.h"

#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Arithmetic.h"

#include <ansi_c.h>
#include <userint.h>

static int CVICALLBACK OnDimensionSliderMove (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	MultiDimensionalSliderData *sd = (MultiDimensionalSliderData *) callbackData;
	Manipulater3d* iface = (Manipulater3d *) sd->iface;
	IcsViewerWindow *window = iface->window;                   
	
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			int slice, dimension;
			FIBITMAP *dib = NULL;
	
			GetCtrlVal(panel, MD_SLIDER_DIMENSION, &dimension);
			GetCtrlVal(panel, control, &slice);
			
			window->last_dimension_moved = dimension;
			window->last_3d_slice_shown = slice;
	
			dib = manipulater3d_get_slice_image (window->importer3d, dimension, slice);      
			
			assert(dib != NULL);
			
			GCI_ImagingWindow_LoadImageAdvanced(window, dib, 0); 
			
			SetCtrlVal(window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, 0);

			FreeImage_Unload(dib);

			break;
		}
	}
	
	return 0;
}

static void TurnOffAllSliderPlayButtons(Manipulater3d* iface)
{
	int i, panel_id;
		
	for(i=2; i < iface->number_of_dimensions; i++) {                
		
		panel_id = iface->sliders_data[i].panel_id;        
		
		SetCtrlAttribute (panel_id, MD_SLIDER_TIMER, ATTR_ENABLED, 0);   
		SetCtrlVal (panel_id, MD_SLIDER_PLAY_BUTTON, 0);   
	}
		
}

static void BuildDimensionalSliders(Manipulater3d* iface)
{
	int i, size;
	char label[50];
	
	iface->current_top = iface->dimensional_slider_top;
		
	for(i=2; i < iface->number_of_dimensions; i++) {
		
		manipulater3d_get_dimension_details (iface, iface->dims[i], label, &size) ;               
		
		CreateMultiDimensionalSlider(iface, i, label, size); 
	}	
}


void CreateMultiDimensionalSlider(Manipulater3d* iface, int dimension, const char* dimension_name, int max)
{
	IcsViewerWindow *window = iface->window; 
	int subwindow_height, panel_id, slider_id;
	char description[500];

	MultiDimensionalSliderData *sd = &(iface->sliders_data[dimension]);

	sd->iface = iface;

	panel_id = sd->panel_id =
		LoadPanel(window->multidimension_panel_id, uir_file_path, MD_SLIDER);
	
	slider_id = sd->slider_id =
		NewCtrl(panel_id, CTRL_NUMERIC_HSLIDE, "", 13, 65);

	SetCtrlAttribute(panel_id, slider_id, ATTR_DATA_TYPE , VAL_INTEGER);
	SetCtrlAttribute(panel_id, slider_id, ATTR_LABEL_LEFT, 1);
	SetCtrlAttribute(panel_id, slider_id, ATTR_LABEL_TOP, 11);
	SetCtrlAttribute(panel_id, slider_id, ATTR_LABEL_WIDTH, 50);	
	SetCtrlAttribute(panel_id, slider_id, ATTR_LABEL_JUSTIFY, VAL_CENTER_JUSTIFIED);	
	SetCtrlAttribute(panel_id, slider_id, ATTR_WIDTH, 294);
	SetCtrlAttribute(panel_id, slider_id, ATTR_HEIGHT, 11);
	SetCtrlAttribute(panel_id, slider_id, ATTR_MARKER_STYLE, VAL_FULL_MARKERS);
	SetCtrlAttribute(panel_id, slider_id, ATTR_TICK_STYLE, VAL_NO_MINOR_TICKS);
	SetCtrlAttribute(panel_id, slider_id, ATTR_FILL_COLOR, VAL_TRANSPARENT);
	SetCtrlAttribute(panel_id, slider_id, ATTR_SHOW_DIG_DISP, 0);
	SetCtrlAttribute(panel_id, slider_id, ATTR_NUM_DIVISIONS, VAL_AUTO);
	SetCtrlAttribute(panel_id, slider_id, ATTR_INCR_VALUE, 1); 
	SetCtrlAttribute(panel_id, slider_id, ATTR_MIN_VALUE, 0); 
	
	if(max <= 1) {
		SetCtrlAttribute(panel_id, slider_id, ATTR_DIMMED, 1); 
	}
	else {
		SetCtrlAttribute(panel_id, slider_id, ATTR_DIMMED, 0); 
		SetCtrlAttribute(panel_id, slider_id, ATTR_MAX_VALUE, max - 1); 
	}

	sprintf(description, "%s", dimension_name);
	
	SetCtrlAttribute(panel_id, slider_id, ATTR_LABEL_TEXT, description);
	
	GetPanelAttribute(panel_id, ATTR_HEIGHT, &subwindow_height);      
	
	SetPanelAttribute(panel_id, ATTR_TITLEBAR_VISIBLE, 0);  
	SetPanelAttribute(panel_id, ATTR_LEFT, 10);      
	SetPanelAttribute(panel_id, ATTR_TOP, iface->current_top);
	
	iface->current_top += subwindow_height;
	
	SetCtrlVal(panel_id, MD_SLIDER_DIMENSION, dimension); 
	
	if ( InstallCtrlCallback (panel_id, slider_id, OnDimensionSliderMove, sd) < 0)
		return;
	
	if ( InstallCtrlCallback (panel_id, MD_SLIDER_PLAY_BUTTON, OnMultiDimensionPlay, sd) < 0)
		return;

	if ( InstallCtrlCallback (panel_id, MD_SLIDER_TIMER, OnPlayTimerTicked, sd) < 0)
		return;
	
	if ( InstallCtrlCallback (panel_id, MD_SLIDER_SUM_BUTTON, OnMultiDimensionSumProjection, sd) < 0)
		return;
	
	if ( InstallCtrlCallback (panel_id, MD_SLIDER_MAX_BUTTON, OnMultiDimensionMaxProjection, sd) < 0)
		return;
	
	if ( InstallCtrlCallback (panel_id, MD_SLIDER_AVERAGE_BUTTON, OnMultiDimensionAverageProjection, sd) < 0)
		return;

	DisplayPanel(panel_id);
}


static void UpdateMultiDimensionalSlider(Manipulater3d* iface, int dimension, const char* dimension_name, int max)
{
	char description[500];

	int panel_id = iface->sliders_data[dimension].panel_id;
	int slider_id = iface->sliders_data[dimension].slider_id;

	sprintf(description, "%s", dimension_name);
	
	SetCtrlAttribute(panel_id, slider_id, ATTR_LABEL_TEXT, description);
	
	SetCtrlAttribute(panel_id, slider_id, ATTR_MAX_VALUE, max - 1); 
	SetCtrlVal(panel_id, MD_SLIDER_DIMENSION, dimension); 
	
	// Set slider to 0
	SetCtrlVal(panel_id, slider_id, 0); 

	if(max <= 1) {
		SetCtrlAttribute(panel_id, slider_id, ATTR_DIMMED, 1); 
	}
	else {
		SetCtrlAttribute(panel_id, slider_id, ATTR_DIMMED, 0); 
		SetCtrlAttribute(panel_id, slider_id, ATTR_MAX_VALUE, max - 1); 
	}
}


static void UpdateDimensionalSliders(Manipulater3d* iface)
{
	int i, size, err;
	char label[50];
	
	for(i=2; i < iface->number_of_dimensions; i++) {                
		
		err = manipulater3d_get_dimension_details (iface, iface->dims[i], label, &size) ;               
		
		UpdateMultiDimensionalSlider(iface, i, label, size); 
	}	
}

static void Load3DFile(IcsViewerWindow *window, int number_of_dimensions)
{
	FIBITMAP *dib = NULL;
	ICS *ics = NULL;
	int number_of_slices = 0, interpret_as_colour_image = 0;
	char name[200] = "";

	if(window->interpret_3_dims_as_colour && number_of_dimensions == 3) {

		manipulater3d_get_dimension_details (window->importer3d, 2, name, &number_of_slices);  

		// Has the third dimension got only one slice ?
		if(number_of_slices == 3) {
			interpret_as_colour_image = 1;
		}	
	}

	if(interpret_as_colour_image) {
		manipulater3d_load_multidimensional_data (window->importer3d, window->filename);
		dib =  manipulater3d_get_colour_image (window->importer3d);
		SetCtrlVal(window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, 1);
		//SetCtrlAttribute(window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, ATTR_DIMMED, 0);
	}
	else {
		dib = manipulater3d_get_slice_image (window->importer3d, window->last_dimension_moved, 0); 
		SetCtrlVal(window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, 0);
		//SetCtrlAttribute(window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, ATTR_DIMMED, 1);
	}

	if(dib == NULL) {
		GCI_MessagePopup("Error", "Failed to load image");
	}
	else {
		GCI_ImagingWindow_LoadImageAdvanced(window, dib, 0); 
		FreeImage_Unload(dib);
	}
}

void manipulater3d_show_multidimensional_dialog (Manipulater3d* iface)
{
	int panel_height, space_for_close_button = 40, dim_interpret_as_colour = 1;
	
	if (iface == NULL)
		return;

	iface->old_dim0_value = 0;
	iface->old_dim1_value = 1;
		
	iface->number_of_dimensions = manipulater3d_get_number_of_dimensions (iface->window->importer3d);
		
	SetCtrlAttribute(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, ATTR_DIMMED, 1);

	if(iface->number_of_dimensions < 2) {
		free(iface);
		return;
	}
	else if(iface->number_of_dimensions == 3) {

		char name[200] = "";
		int number_of_slices;

		manipulater3d_get_dimension_details (iface->window->importer3d, 2, name, &number_of_slices);  

		if(number_of_slices == 3)
			dim_interpret_as_colour = 0;		
	}

	//manipulater3d_arrange_dimensions_with_first_two_dims_as (
	//				window->importer3d, window->last_3d_dimension1_shown, window->last_3d_dimension2_shown) ;  
		
	if(iface->window->multidimension_panel_id == 0) {
		iface->window->multidimension_panel_id = LoadPanel(0, uir_file_path, MD_PNL);        
		    
	}	
		
	CreateViewableAxisWindow(iface);    
	BuildDimensionalSliders(iface);     
			
	panel_height = iface->current_top + 10 + space_for_close_button;
	SetPanelAttribute(iface->window->multidimension_panel_id, ATTR_HEIGHT, panel_height);  
	SetCtrlAttribute(iface->window->multidimension_panel_id, MD_PNL_CLOSE_BUTTON, ATTR_TOP, panel_height - 40);
	SetCtrlAttribute(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, ATTR_TOP, panel_height - 40);
	SetCtrlAttribute(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, ATTR_LEFT, 10);

	if ( InstallCtrlCallback (iface->window->multidimension_panel_id, MD_PNL_CLOSE_BUTTON, OnMultiDimensionPanelClose, iface) < 0) {
		free(iface);
		return;
	}
	
	if ( InstallCtrlCallback (iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, OnInterpretAsMultiDimensional, iface) < 0) {
		free(iface);
		return;
	}
	
	Load3DFile(iface->window, iface->number_of_dimensions);

	SetCtrlAttribute(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, ATTR_DIMMED, dim_interpret_as_colour);

	DisplayPanel(iface->window->multidimension_panel_id); 
}


int CVICALLBACK OnInterpretAsMultiDimensional (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Manipulater3d* iface = (Manipulater3d *) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;

			GetCtrlVal(panel, control, &val);
			
			if(val) {
		
				iface->window->interpret_3_dims_as_colour = 1;
			}
			else {

				iface->window->interpret_3_dims_as_colour = 0;
			}

			Load3DFile(iface->window, iface->number_of_dimensions);

			break;
		}
	}

	return 0;
}

int CVICALLBACK OnMultiDimensionPanelClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Manipulater3d* iface = (Manipulater3d *) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			// Turn off play timers
			TurnOffAllSliderPlayButtons(iface);        
			
			if(iface->window->multidimension_panel_id < 1)
				return 0;
			
			DiscardPanel(iface->window->multidimension_panel_id);
			iface->window->multidimension_panel_id = 0;
			
			break;
		}
	}
	
	return 0;
}


static int FillDimensionComboBox(Manipulater3d* iface, int panel, int ctrl, int index)
{
	IcsViewerWindow *window = iface->window;            
	int i, size, number_of_dimesions = manipulater3d_get_number_of_dimensions (window->importer3d);
	char label[50];    	
	
	for(i=0; i < number_of_dimesions; i++) {
		
		manipulater3d_get_dimension_details (window->importer3d, i, label, &size) ;
			
		InsertListItem (panel, ctrl, i, label, i);
	}
	
	SetCtrlAttribute(panel, ctrl, ATTR_DFLT_INDEX, index);  
	SetCtrlAttribute(panel, ctrl, ATTR_CTRL_INDEX, index); 
	
	return size;
}



int CVICALLBACK OnPlayTimerTicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index, max;
	MultiDimensionalSliderData *sd = (MultiDimensionalSliderData *) callbackData;
	Manipulater3d* iface = (Manipulater3d *) sd->iface;                    
	
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			GetCtrlVal(panel, sd->slider_id, &index);
			GetCtrlAttribute (panel, sd->slider_id, ATTR_MAX_VALUE, &max);
			
			if (index >= max)
				SetCtrlVal(panel, sd->slider_id, 0);
			else
				SetCtrlAttribute (panel, sd->slider_id , ATTR_CTRL_VAL, index+1);

			OnDimensionSliderMove(panel, sd->slider_id, EVENT_VAL_CHANGED, sd, 0, 0);
			
			break;
		}
	}
		
	return 0;
}


int CVICALLBACK OnMultiDimensionPlay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	MultiDimensionalSliderData *sd = (MultiDimensionalSliderData *) callbackData;
	Manipulater3d* iface = (Manipulater3d *) sd->iface;                   
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int status;
			
			GetCtrlVal(panel, control, &status);
			
			// Turn off play timers
			TurnOffAllSliderPlayButtons(iface);     
				
			if(status) { // Play button is on, 
				
				SetCtrlAttribute (panel, MD_SLIDER_TIMER, ATTR_ENABLED, 1);   
				SetCtrlVal (panel, control, 1);       
			}
				
			SetCtrlVal(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, 0);	

			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnMultiDimensionSumProjection (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	MultiDimensionalSliderData *sd = (MultiDimensionalSliderData *) callbackData;
	Manipulater3d* iface = (Manipulater3d *) sd->iface;   
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			FIBITMAP *dib = NULL;      
			int dimension;
			
			// Turn off play timers
			TurnOffAllSliderPlayButtons(iface);     
			
			//window->last_3d_slice_shown = slice;
			GetCtrlVal(panel, MD_SLIDER_DIMENSION, &dimension);   
			
			SetWaitCursor (1);  
			
			dib = manipulater3d_get_sum_intensity_image (iface, dimension);     

			SetWaitCursor (0);  
			
			GCI_ImagingWindow_LoadImageAdvanced(iface->window, dib, 0); 
			
			SetCtrlVal(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, 0);

			FreeImage_Unload(dib);

			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnMultiDimensionMaxProjection (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	MultiDimensionalSliderData *sd = (MultiDimensionalSliderData *) callbackData;
	Manipulater3d* iface = (Manipulater3d *) sd->iface;    
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			FIBITMAP *dib = NULL;      
			int dimension;
			
			// Turn off play timers
			TurnOffAllSliderPlayButtons(iface);     
			
			//window->last_3d_slice_shown = slice;
			GetCtrlVal(panel, MD_SLIDER_DIMENSION, &dimension);   
			
			SetWaitCursor (1);  
			
			dib = manipulater3d_get_max_intensity_image (iface, dimension);     

			SetWaitCursor (0);
			
			GCI_ImagingWindow_LoadImageAdvanced(iface->window, dib, 0); 
			
			SetCtrlVal(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, 0);

			FreeImage_Unload(dib);

			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnMultiDimensionAverageProjection (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	MultiDimensionalSliderData *sd = (MultiDimensionalSliderData *) callbackData;
	Manipulater3d* iface = (Manipulater3d *) sd->iface;   
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			FIBITMAP *dib = NULL;      
			int dimension, number_of_slices;
			char tmp[200];
			
			// Turn off play timers
			TurnOffAllSliderPlayButtons(iface);     
			
			//window->last_3d_slice_shown = slice;
			GetCtrlVal(panel, MD_SLIDER_DIMENSION, &dimension);   
			
			SetWaitCursor (1);
			
			dib = manipulater3d_get_sum_intensity_image (iface, dimension);     

			SetWaitCursor (0);
			
			// Divide this image by the mumber of slices in the dimension
			manipulater3d_get_dimension_details (iface, dimension, tmp, &number_of_slices);     
			
			if(number_of_slices > 1) {

				if(FIA_DivideGreyLevelImageConstant(dib, number_of_slices) == FIA_ERROR)
					return -1;
			}
			
			GCI_ImagingWindow_LoadImageAdvanced(iface->window, dib, 0); 
			
			SetCtrlVal(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, 0);

			FreeImage_Unload(dib);

			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnViewAxisComboChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Manipulater3d* iface = (Manipulater3d *) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int first_control, second_control, other_control, old_index;
			int first_control_index, second_control_index;
			
			SetWaitCursor (1);  
			
			SetCtrlVal(iface->window->multidimension_panel_id, MD_PNL_RGB_INTERPRET, 0);

			first_control = MD_V_AXIS_VIEW_AXIS1;      
			second_control = MD_V_AXIS_VIEW_AXIS2;  
			
			if(control == MD_V_AXIS_VIEW_AXIS1) {
				
				other_control = MD_V_AXIS_VIEW_AXIS2;
				old_index = iface->old_dim0_value; // cvi is crap doesn't tell you old val of combobox
				
			}
			else {
			
				other_control = MD_V_AXIS_VIEW_AXIS1;
				old_index = iface->old_dim1_value;  
				
			}
			
			
			GetCtrlIndex(panel, first_control, &first_control_index); 
			GetCtrlIndex(panel, second_control, &second_control_index); 
			
			
			// If the indexes are the same change the othe combo to the old
			// value of the combobox that is changing.
			if(first_control_index == second_control_index) {
				
				SetCtrlAttribute(panel, other_control, ATTR_CTRL_INDEX, old_index); 
				
				if(control == MD_V_AXIS_VIEW_AXIS1) {
					iface->old_dim1_value = old_index;
					second_control_index = old_index;
					
				}
				else {
					iface->old_dim0_value = old_index;
					first_control_index = old_index;
				}
			}
			
			iface->old_dim0_value = first_control_index; 
			iface->old_dim1_value = second_control_index;
			
			iface->window->last_3d_dimension1_shown = first_control_index;
			iface->window->last_3d_dimension2_shown = second_control_index;
			
			// Two dimensions must be different.
			assert(first_control_index != second_control_index);
			
			manipulater3d_arrange_dimensions_with_first_two_dims_as (iface, first_control_index, second_control_index) ; 
			
			UpdateDimensionalSliders(iface); 	
			
			SetWaitCursor (0);  
			
			break;
		}
	}
	
	return 0;
}

void CreateViewableAxisWindow(Manipulater3d* iface)
{
	int view_axis_panel_id = LoadPanel(iface->window->multidimension_panel_id, uir_file_path, MD_V_AXIS);   
	
	int  subwindow_height; 
	
	SetPanelAttribute(view_axis_panel_id, ATTR_TITLEBAR_VISIBLE, 0);  
	SetPanelAttribute(view_axis_panel_id, ATTR_LEFT, 10); 
	GetPanelAttribute(view_axis_panel_id, ATTR_HEIGHT, &subwindow_height);   
	SetPanelAttribute(view_axis_panel_id, ATTR_TOP, 10);   
	
	FillDimensionComboBox(iface, view_axis_panel_id, MD_V_AXIS_VIEW_AXIS1, 0);         
	FillDimensionComboBox(iface, view_axis_panel_id, MD_V_AXIS_VIEW_AXIS2, 1);            
	
	if ( InstallCtrlCallback (view_axis_panel_id, MD_V_AXIS_VIEW_AXIS1, OnViewAxisComboChanged, iface) < 0)
		return;
	
	if ( InstallCtrlCallback (view_axis_panel_id, MD_V_AXIS_VIEW_AXIS2, OnViewAxisComboChanged, iface) < 0)
		return;
		
	iface->dimensional_slider_top = 10 + subwindow_height;  
	
	DisplayPanel(view_axis_panel_id);
	
}


void CVICALLBACK
GCI_ImagingWindow_MultipleDimensionsShow(int menuBar, int menuItem, void *callbackData, int panel) 
{
	IcsViewerWindow *window = (IcsViewerWindow *) callbackData;   

	manipulater3d_show_multidimensional_dialog (window->importer3d);
}

void manipulater3d_constructor(IcsViewerWindow *window, Manipulater3d* iface)
{
	int i;
	
	memset(iface, 0, sizeof(Manipulater3d));
	
	iface->vtable = (vtable_3d*) malloc(sizeof(vtable_3d));
	
	memset(iface->vtable, 0, sizeof(vtable_3d));
	
	iface->window = window;
	
	for(i=0; i < MAX_NUMBER_OF_DIMENSIONS; i++)
		iface->dims[i] = i;
}

void manipulater3d_deconstructor(Manipulater3d* iface)
{
	iface->current_top = 0;
	iface->number_of_dimensions = 0;
	iface->dimensional_slider_top = 0;
	memset(iface->dims, 0, MAX_NUMBER_OF_DIMENSIONS);
	memset(iface->sliders_data, 0, MAX_NUMBER_OF_DIMENSIONS);
	
	//DiscardPanel(iface->window->multidimension_panel_id);
	//iface->window->multidimension_panel_id = 0;
	
	IMPORTER_3D_VTABLE_CALL(iface, on_destroy)(iface);   
}

Manipulater3d* get_3d_importer_for_file(IcsViewerWindow *window, const char *filepath)
{
	char ext[10];

	get_file_extension(filepath, ext); 
	
	if(strcmp(ext, ".ics") == 0)
		return ics_3d_manipulater_new(window);

	return NULL;
}

int manipulater3d_get_number_of_dimensions (Manipulater3d* iface)
{
	if(IMPORTER_3D_VTABLE(iface, get_number_of_dimensions) == NULL) {
	
    	return -1;
  	}

  	return IMPORTER_3D_VTABLE_CALL(iface, get_number_of_dimensions)(iface);
}


int manipulater3d_get_dimension_details (Manipulater3d* iface, int dimension, char *name, int *number_of_slices)
{
	if(IMPORTER_3D_VTABLE(iface, get_dimension_details) == NULL) {
	
    	return -1;
  	}

  	return IMPORTER_3D_VTABLE_CALL(iface, get_dimension_details)(iface, dimension, name, number_of_slices);	
}

FIBITMAP* manipulater3d_get_slice_image (Manipulater3d* iface, int dimension, int slice)
{
	if(IMPORTER_3D_VTABLE(iface, get_slice_image) == NULL) {
	
    	return NULL;
  	}

  	return IMPORTER_3D_VTABLE_CALL(iface, get_slice_image)(iface, dimension, slice);	
}

FIBITMAP* manipulater3d_get_roi_slice_image (Manipulater3d* iface, int dimension, int slice, FIARECT roi)
{
	if(IMPORTER_3D_VTABLE(iface, get_roi_slice_image) == NULL) {
	
    	return NULL;
  	}

  	return IMPORTER_3D_VTABLE_CALL(iface, get_roi_slice_image)(iface, dimension, slice, roi);	
}


FIBITMAP* manipulater3d_get_sum_intensity_image (Manipulater3d* iface, int dimension)
{
	if(IMPORTER_3D_VTABLE(iface, get_sum_intensity_image) == NULL) {
	
    	return NULL;
  	}

  	return IMPORTER_3D_VTABLE_CALL(iface, get_sum_intensity_image)(iface, dimension);	
}


FIBITMAP* manipulater3d_get_max_intensity_image (Manipulater3d* iface, int dimension)
{
	if(IMPORTER_3D_VTABLE(iface, get_max_intensity_image) == NULL) {
	
    	return NULL;
  	}

  	return IMPORTER_3D_VTABLE_CALL(iface, get_max_intensity_image)(iface, dimension);	
}

int manipulater3d_arrange_dimensions_with_first_two_dims_as (Manipulater3d *iface, int dim1, int dim2)
{
	if(IMPORTER_3D_VTABLE(iface, arrange_dimensions_with_first_two_dims_as) == NULL) {
	
    	return -1;
  	}

  	return IMPORTER_3D_VTABLE_CALL(iface, arrange_dimensions_with_first_two_dims_as)(iface, dim1, dim2);		
}


int manipulater3d_load_multidimensional_data (Manipulater3d* iface, const char* filepath)
{
	if(IMPORTER_3D_VTABLE(iface, load_multidimensional_data) == NULL) {

    	return -1;
  	}

  	if(IMPORTER_3D_VTABLE_CALL(iface, load_multidimensional_data)(iface, filepath) < 0)	
		return -1;

	return 0;
}

