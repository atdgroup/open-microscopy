#ifndef _IMPORTER_3D_
#define _IMPORTER_3D_

#include "FreeImageAlgorithms.h"
#include "FreeImageIcs_IO.h" 
#include "icsviewer_window.h"

#define MAX_NUMBER_OF_DIMENSIONS 10       

typedef struct _Manipulater3d Manipulater3d; 

typedef struct _vtable_3d vtable_3d;   

struct _vtable_3d
{
	int (*on_destroy) (Manipulater3d*); 
	int (*get_number_of_dimensions) (Manipulater3d*);
	int (*get_dimension_details) (Manipulater3d*, int dimension, char *name, int *number_of_slices);   
	
	FIBITMAP* (*get_slice_image) (Manipulater3d*, int dimension, int slice);
	FIBITMAP* (*get_colour_image) (Manipulater3d*);
	FIBITMAP* (*get_roi_slice_image) (Manipulater3d*, int dimension, int slice, FIARECT roi);  
	FIBITMAP* (*get_sum_intensity_image) (Manipulater3d*, int dimension);
	FIBITMAP* (*get_max_intensity_image) (Manipulater3d*, int dimension);                      
	
	int (*arrange_dimensions_with_first_two_dims_as) (Manipulater3d *iface, int dim1, int dim2);
	 
	// This function takes some data in byte for and returns a file descriptor
	// ro an ics file 
	int (*load_multidimensional_data) (Manipulater3d*, const char* filepath);     
	
};


typedef struct _MultiDimensionalData MultiDimensionalData;


typedef struct
{
	Manipulater3d *iface; 
	int panel_id;
	int slider_id;
	int dimension;       
	
} MultiDimensionalSliderData;


struct _Manipulater3d       
{
	vtable_3d* vtable;
	
	IcsViewerWindow *window;
	
	int dims[MAX_NUMBER_OF_DIMENSIONS];
	
	int number_of_dimensions;
	int current_top;
	int dimensional_slider_top;
	int old_dim0_value;
	int old_dim1_value;
	
	MultiDimensionalSliderData sliders_data[MAX_NUMBER_OF_DIMENSIONS];
};


#define IMPORTER_3D_CAST(obj) ((Manipulater3d *)obj)

#define IMPORTER_3D_VTABLE(obj, member) (IMPORTER_3D_CAST(obj)->vtable->member)

#define IMPORTER_3D_VTABLE_CALL(obj, member) (*IMPORTER_3D_VTABLE(obj, member))

void manipulater3d_constructor(IcsViewerWindow *window, Manipulater3d*);

void manipulater3d_deconstructor(Manipulater3d* importer);

Manipulater3d* get_3d_importer_for_file(IcsViewerWindow *window, const char *filepath);

int manipulater3d_get_number_of_dimensions (Manipulater3d*);

int manipulater3d_get_dimension_details (Manipulater3d*, int dimension, char *name, int *number_of_slices);    

FIBITMAP* manipulater3d_get_slice_image (Manipulater3d*, int dimension, int slice);

FIBITMAP* manipulater3d_get_colour_image (Manipulater3d* iface);

FIBITMAP* manipulater3d_get_roi_slice_image (Manipulater3d*, int dimension, int slice, FIARECT roi);

FIBITMAP* manipulater3d_get_sum_intensity_image (Manipulater3d* iface, int dimension);

FIBITMAP* manipulater3d_get_max_intensity_image (Manipulater3d* iface, int dimension);  

int manipulater3d_arrange_dimensions_with_first_two_dims_as (Manipulater3d *iface, int dim1, int dim2); 

int manipulater3d_load_multidimensional_data (Manipulater3d*, const char* filepath);     

void manipulater3d_show_multidimensional_dialog (Manipulater3d*);

void __cdecl
GCI_ImagingWindow_MultipleDimensionsShow(int menuBar, int menuItem, void *callbackData, int panel);


void CreateMultiDimensionalSlider(Manipulater3d* iface, int dimension, const char* dimension_name, int max);
void CreateViewableAxisWindow(Manipulater3d* iface);

int CVICALLBACK OnMultiDimensionPlay (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnMultiDimensionSumProjection (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnMultiDimensionMaxProjection (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnMultiDimensionAverageProjection (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnViewAxisComboChanged (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnInterpretAsMultiDimensional (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnMultiDimensionPanelClose (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnPlayTimerTicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
