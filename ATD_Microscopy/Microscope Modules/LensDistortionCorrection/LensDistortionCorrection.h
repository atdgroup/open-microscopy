#ifndef __MOD_DISTORTION__
#define __MOD_DISTORTION__

#include "HardwareTypes.h"
#include "gci_ui_module.h"

#include "FreeImageAlgorithms.h"

typedef struct _LensDistortion LensDistortion;

struct _LensDistortion
{
	UIModule parent;
	
	FIBITMAP *input;

	int panel_id;  
	int poly_panel_id;   
	int options_panel_id;  
	int offset_panel_id;
	int profile_panel_id;
	int	number_of_points;
	int profile_plot;

	double a;
	double b;
	double c;
	double d;
	double last_point_angle;
	double discal_normalisation;

	int			coeffs_set;
	FIAPOINT	offset;
	FIAPOINT	center;
	FIAPOINT	end;
	HWND		window;

	BYTE	*profile;
	double	*points;
	double	*regular_points;
};

LensDistortion* lens_distortion_new(Microscope *ms, char* data_dir);

void lens_distortion_set_coeffs(LensDistortion* ld, double a, double b, double c, double d);

void lens_distortion_set_offset(LensDistortion* ld, FIAPOINT point);

FIBITMAP* lens_distortion_correct_distortion(LensDistortion* ld, FIBITMAP *input);

void lens_distortion_fit_polynomial(LensDistortion* ld, FIBITMAP *input);

#endif
