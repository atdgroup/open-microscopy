#ifndef _ICS_3D_IMPORTER_
#define _ICS_3D_IMPORTER_

#include "icsviewer_ics3d.h"
#include "gci_utils.h"
#include "icsviewer_private.h" 

struct _Ics3DManipulater
{
	Manipulater3d parent;

	ICS *ics;
	ICS *output_ics;
	
};

static int ics_get_number_of_dimensions(Manipulater3d *iface)
{
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface;            
	
	return FreeImageIcs_NumberOfDimensions (ics_iface->ics); 	
}


static int ics_load_multidimensional_data (Manipulater3d* iface, const char* filepath)
{
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface; 
	int number_of_dimensions;
	
	if(ics_iface->ics != NULL) {
		FreeImageIcs_IcsClose(ics_iface->ics);  
		ics_iface->ics = NULL;
	}

	if(ics_iface->output_ics != NULL) {
		FreeImageIcs_IcsClose(ics_iface->output_ics);  
		ics_iface->output_ics = NULL;
	}

	if(FreeImageIcs_IcsOpen (&(ics_iface->ics), filepath, "r") != IcsErr_Ok)
		return -1;
	
	number_of_dimensions = FreeImageIcs_NumberOfDimensions (ics_iface->ics);
	
	if(number_of_dimensions < 2)
		return -1;
	
	if(FreeImageIcs_IcsOpen (&(ics_iface->output_ics), filepath, "r") != IcsErr_Ok)
		return -1;
	
	return 1;
}


static int ics_get_dimension_details (Manipulater3d* iface, int dimension,
	char *name, int *number_of_slices)
{
	char order[50];
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface;        
	
	return FreeImageIcs_GetDimensionDetails (ics_iface->output_ics, dimension, order, name, number_of_slices);	
}


static int ics_on_destroy (Manipulater3d *iface)   
{
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface;
	
	if(ics_iface->ics != NULL)
		FreeImageIcs_IcsClose(ics_iface->ics);
	
	if(ics_iface->output_ics != NULL)
		FreeImageIcs_IcsClose(ics_iface->output_ics);
	
	return 0;
}


static FIBITMAP* ics_get_slice_image(Manipulater3d *iface, int dimension, int slice)
{
	char name[100];
	int number_of_slices;
	
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface;
	
	FIBITMAP* dib = FreeImageIcs_GetIcsImageDimensionalDataSlice(ics_iface->output_ics, dimension, slice);
			
	ics_get_dimension_details (iface, dimension, name, &number_of_slices);
	
	assert(slice < number_of_slices);
	
	assert(dib != NULL);
			
	return dib; 	
}

static FIBITMAP* ics_get_colour_image(Manipulater3d *iface)
{
	Ics3DManipulater *ics_iface = NULL;

	if(iface->number_of_dimensions != 3)
		return NULL;

	ics_iface = (Ics3DManipulater*) iface;
	
	return FreeImageIcs_LoadFIBFromColourIcsFile(ics_iface->output_ics);	
}

FIBITMAP* manipulater3d_get_colour_image (Manipulater3d* iface)
{
	//manipulater3d_load_multidimensional_data (iface, window->filename);
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface;

	return FreeImageIcs_LoadFIBFromColourIcsFile (ics_iface->output_ics); 	
}
	

static FIBITMAP* ics_get_sum_intensity_image(Manipulater3d *iface, int dimension)
{
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface;
	
	FIBITMAP* dib = FreeImageIcs_SumIntensityProjection(ics_iface->output_ics, dimension);

	assert(dib != NULL);
			
	return dib; 	
}


static FIBITMAP* ics_get_max_intensity_image(Manipulater3d *iface, int dimension)
{
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface;
	
	FIBITMAP* dib = FreeImageIcs_MaximumIntensityProjection(ics_iface->output_ics, dimension);

	assert(dib != NULL);
			
	return dib; 	
}

static int ics_arrange_dimensions_with_first_two_dims_as(Manipulater3d *iface, int dim1, int dim2)
{
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) iface;
	FIBITMAP *temp_dib;
	char tmp_ics_file_path[GCI_MAX_PATHNAME_LEN];
		
	strcpy(tmp_ics_file_path, iface->window->temp_dir_path);
	strcat(tmp_ics_file_path, "\\swap_dimensions.ics");
	
	if(FreeImageIcs_SaveIcsFileWithFirstTwoDimensionsAs(ics_iface->ics, tmp_ics_file_path, dim1, dim2)
		== FIA_ERROR)
	{
		GCI_MessagePopup("Error", "Can not save temporary file.");	
		return -1;
	}
			
	if(FreeImageIcs_IcsClose (ics_iface->output_ics) != IcsErr_Ok)
		return -1;
		
	if(FreeImageIcs_IcsOpen (&(ics_iface->output_ics), tmp_ics_file_path, "r") != IcsErr_Ok)
		return -1;
	
	temp_dib = ics_get_slice_image(iface, 2, 0); 
	
	GCI_ImagingWindow_LoadImageAdvanced(iface->window, temp_dib, 0);    
	
	FreeImage_Unload(temp_dib);

	return 1;    
}

Manipulater3d* ics_3d_manipulater_new(IcsViewerWindow *window)
{
	Ics3DManipulater *ics_iface = (Ics3DManipulater*) malloc(sizeof(Ics3DManipulater));
	Manipulater3d *iface = (Manipulater3d*) ics_iface;
	
	manipulater3d_constructor(window, iface);     
	
	ics_iface->ics = NULL;
	ics_iface->output_ics = NULL;
		
	IMPORTER_3D_VTABLE(iface, get_number_of_dimensions) = ics_get_number_of_dimensions;
	IMPORTER_3D_VTABLE(iface, get_slice_image) = ics_get_slice_image; 
	IMPORTER_3D_VTABLE(iface, get_colour_image) = ics_get_colour_image;
	IMPORTER_3D_VTABLE(iface, get_sum_intensity_image) = ics_get_sum_intensity_image; 
	IMPORTER_3D_VTABLE(iface, get_max_intensity_image) = ics_get_max_intensity_image;  
	IMPORTER_3D_VTABLE(iface, arrange_dimensions_with_first_two_dims_as) = ics_arrange_dimensions_with_first_two_dims_as;        
	IMPORTER_3D_VTABLE(iface, get_dimension_details) = ics_get_dimension_details;
	IMPORTER_3D_VTABLE(iface, get_roi_slice_image) = NULL;
	IMPORTER_3D_VTABLE(iface, load_multidimensional_data) = ics_load_multidimensional_data;
	IMPORTER_3D_VTABLE(iface, on_destroy) = ics_on_destroy;  
	
	return iface;
}


#endif
