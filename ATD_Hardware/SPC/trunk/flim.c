#include "Flim.h"
#include "Flim_ui.h"

#include "FreeImageAlgorithms_Utilities.h"

#include <userint.h>
#include <utility.h>
#include <analysis.h>

#include "gci_utils.h"

//#include <ansi_c.h>
//>
//h>
//

//#include "toolbox.h"


////////////////////////////////////////////////////////////////////////
//Lifetime image creation
//Do all displays and calculations in picoseconds and double precision
//Tried with table of logs instead of log() function. Time was exactly the same.

#define Round  RoundRealToNearestInteger

//static int gImtype=0;		 //0=intensity, 1=TauS, 2-TauL

static void flim_replotdata(Flim *flim)
{
	DeleteGraphPlot (flim->_main_ui_panel, FLIM_GRAPH, -1, VAL_IMMEDIATE_DRAW);

	PlotXY (flim->_main_ui_panel, FLIM_GRAPH, flim->x_data, flim->y_data,
			flim->number_of_points, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
			VAL_NO_POINT, VAL_SOLID, 1, flim->plot_colour);
}

static void	FLIM_ShowFitL(Flim *flim)
{
	double *fitData, *errData, Io, T, B12;
	int i, start, end, inc, minidx, maxidx;
	double e = 2.718281828, dT;
	double x_cursor[2], y_cursor[2], minval, maxval;
	int show_fit, scale=0;
	
	flim_replotdata(flim);
	//FLIM_drawCursors(flim);
	
	//RJL 210206 - two lines below added
	GetCtrlVal(flim->_main_ui_panel, FLIM_SHOW_LONG_FIT, &show_fit);
	if (!show_fit) return;
	
	fitData = (double *)malloc(flim->number_of_points * sizeof(double));
	errData = (double *)malloc(flim->number_of_points * sizeof(double));
	
  	GetCtrlVal(flim->_main_ui_panel, FLIM_TAU, &T);
  	GetCtrlVal(flim->_main_ui_panel, FLIM_I_ZERO, &Io);
  	GetCtrlVal(flim->_main_ui_panel, FLIM_B_12, &B12);
  	
	GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 1, &x_cursor[0], &y_cursor[0]);
	GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, &x_cursor[1], &y_cursor[1]);
  	start = Round(x_cursor[0]/flim->time_per_point);
  	dT = x_cursor[1] - x_cursor[0];
  	inc = Round(dT/flim->time_per_point);
	end = start + 3*inc;
	
  	for (i=start; i<end; i++) {
		fitData[i] = Io * pow(e, -flim->x_data[i-start]/T) + B12;
		errData[i] = flim->y_data[i] - fitData[i];
	}

	//RJL 110106 - LT_GRAY changed to RED and THIN to FAT
	PlotXY (flim->_main_ui_panel, FLIM_GRAPH, &flim->x_data[start], &fitData[start],
			end-start, VAL_DOUBLE, VAL_DOUBLE, VAL_FAT_LINE,
			VAL_NO_POINT, VAL_SOLID, 1, VAL_RED);
	
	DeleteGraphPlot (flim->_main_ui_panel, FLIM_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
	MaxMin1D (errData, flim->number_of_points, &maxval, &maxidx, &minval, &minidx);
	while (maxval > 10) {
		maxval /= 10;
		scale ++;
	}
	minval = -ceil(maxval)*pow(10.0, scale);
	maxval = -minval;
	SetAxisScalingMode (flim->_main_ui_panel, FLIM_GRAPH_2, VAL_LEFT_YAXIS, VAL_MANUAL, minval, maxval);
	PlotXY (flim->_main_ui_panel, FLIM_GRAPH_2, &flim->x_data[start], &errData[start],
			end-start, VAL_DOUBLE, VAL_DOUBLE, VAL_FAT_LINE,
			VAL_NO_POINT, VAL_SOLID, 1, VAL_RED);

	free(fitData);
	free(errData);
}

static void	FLIM_ShowFitS(Flim *flim)
{
	double *fitData, *errData, Io, T, B12;
	int show_fit, i, start, end, inc;
	double e = 2.718281828, dT;
	double x_cursor[2], y_cursor[2];
	
	GetCtrlVal(flim->_main_ui_panel, FLIM_SHOW_SHORT_FIT, &show_fit);
	
	if (!show_fit) 
		return;
	
	fitData = (double *)malloc(flim->number_of_points * sizeof(double));
	errData = (double *)malloc(flim->number_of_points * sizeof(double));
	
  	GetCtrlVal(flim->_main_ui_panel, FLIM_tau, &T);
  	GetCtrlVal(flim->_main_ui_panel, FLIM_i_zero, &Io);
  	GetCtrlVal(flim->_main_ui_panel, FLIM_b_12, &B12);
  	
	GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 1, &x_cursor[0], &y_cursor[0]);
	GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, &x_cursor[1], &y_cursor[1]);
  	start = Round(x_cursor[0]/flim->time_per_point);
  	dT = x_cursor[1] - x_cursor[0];
  	inc = Round(dT/flim->time_per_point);
	end = start + inc;
	
  	for (i=start; i<end; i++) {
		fitData[i] = Io * pow(e, -flim->x_data[i-start]/T) + B12;
		errData[i] = flim->y_data[i] - fitData[i];
	}

	//RJL 110106 - BLACK changed to BLUE and THIN to FAT
	PlotXY (flim->_main_ui_panel, FLIM_GRAPH, &flim->x_data[start], &fitData[start],
			end-start, VAL_DOUBLE, VAL_DOUBLE, VAL_FAT_LINE,
			VAL_NO_POINT, VAL_SOLID, 1, VAL_BLUE);
	
	PlotXY (flim->_main_ui_panel, FLIM_GRAPH_2, &flim->x_data[start], &errData[start],
			end-start, VAL_DOUBLE, VAL_DOUBLE, VAL_FAT_LINE,
			VAL_NO_POINT, VAL_SOLID, 1, VAL_BLUE);

	free(fitData);
	free(errData);
}

static void	FLIM_CalculateTau(Flim *flim)
{
	double x_cursor[2], y_cursor[2];
	int start, inc, i;
	double dT, a1, a2, a3;
	double T, Io, B12, B23, Bi=0;
	
	GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 1, &x_cursor[0], &y_cursor[0]);
	GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, &x_cursor[1], &y_cursor[1]);

	//Calculate Tau for the shorter interval
  	start = Round(x_cursor[0]/flim->time_per_point);
  	dT = (x_cursor[1] - x_cursor[0])/3.0;
	//RJL 130206 - line below changed
  	inc = Round(dT/flim->time_per_point)-1;
  	
  	a1 = 0;
  	a2 = 0;
  	a3 = 0;

  	for (i = start; i < start + inc; i++) {
  		a1 += flim->y_data[i];
  		a2 += flim->y_data[i+inc];
  		a3 += flim->y_data[i+2*inc];
  	}
  	
  	a1 *= flim->time_per_point;
  	a2 *= flim->time_per_point;
  	a3 *= flim->time_per_point;
  	
  	SetCtrlVal(flim->_main_ui_panel, FLIM_a1, a1);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_a2, a2);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_a3, a3);
  	
  	flim_CalculateTau(dT, a1, a2, a3, &T, &Io, &B12, &B23);
  	
  	SetCtrlVal(flim->_main_ui_panel, FLIM_tau, T);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_i_zero, Io);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_b_12, B12);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_b_23, B23);
  	
	//Calculate Tau for the longer interval
  	inc *= 3;
  	
  	a1 = 0;
  	a2 = 0;
  	a3 = 0;

  	for (i = start; i < start + inc; i++) {
  		a1 += flim->y_data[i];
  		a2 += flim->y_data[i+inc];
  		a3 += flim->y_data[i+2*inc];
  	}
  	
  	a1 *= flim->time_per_point;
  	a2 *= flim->time_per_point;
  	a3 *= flim->time_per_point;
  	
  	SetCtrlVal(flim->_main_ui_panel, FLIM_A1, a1);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_A2, a2);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_A3, a3);
  	
  	flim_CalculateTau(dT*3, a1, a2, a3, &T, &Io, &B12, &B23);
  	
  	SetCtrlVal(flim->_main_ui_panel, FLIM_TAU, T);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_I_ZERO, Io);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_B_12, B12);
  	SetCtrlVal(flim->_main_ui_panel, FLIM_B_23, B23);
  	
  	FLIM_ShowFitL(flim);
  	FLIM_ShowFitS(flim);
  	
  	//Average points before Tzero to get another indication of B
  	for (i=0; i<flim->number_of_points/10; i++)
  		Bi += flim->y_data[i];
  	SetCtrlVal(flim->_main_ui_panel, FLIM_Bi, Bi/i);
}

static void	FLIM_CalculateTauS(Flim *flim, double *tranData, double *tauS)
{
	int inc, i;
	double dT, a1, a2, a3;
	double T, Io, B12, B23;
	
	//Calculate Tau for the shorter interval
  	dT = flim->width * flim->time_per_point;
  	inc = flim->width;
  	
  	a1 = 0;
  	a2 = 0;
  	a3 = 0;

  	for (i = flim->start; i < flim->start + inc; i++) {
  		a1 += tranData[i];
  		a2 += tranData[i+inc];
  		a3 += tranData[i+2*inc];
  	}
  	
  	a1 *= flim->time_per_point;
  	a2 *= flim->time_per_point;
  	a3 *= flim->time_per_point;
  	
  	flim_CalculateTau(dT, a1, a2, a3, &T, &Io, &B12, &B23);
  	*tauS = T;
}

static void	FLIM_CalculateTauL(Flim *flim, double *tranData, double *tauL)
{
	int inc, i;
	double dT, a1, a2, a3;
	double T, Io, B12, B23;
	
	//Calculate Tau for the shorter interval
  	dT = flim->width*3*flim->time_per_point;
	//RJL 130206 - line below changed
  	inc = flim->width*3-1;
  	
  	a1 = 0;
  	a2 = 0;
  	a3 = 0;

  	for (i = flim->start; i < flim->start + inc; i++) {
  		a1 += tranData[i];
  		a2 += tranData[i+inc];
  		a3 += tranData[i+2*inc];
  	}
  	
  	a1 *= flim->time_per_point;
  	a2 *= flim->time_per_point;
  	a3 *= flim->time_per_point;
  	
  	flim_CalculateTau(dT, a1, a2, a3, &T, &Io, &B12, &B23);
  	*tauL = T;
}

static void FLIM_setCursors(Flim *flim)
{
	flim_replotdata(flim);
	//FLIM_drawCursors();
	FLIM_CalculateTau(flim);
}

static void flim_cursors(Flim *flim)
{
	double x_cursor[2], y_cursor[2];
	int i, rounder;
	
	//Called when the cursors on the graph are moved
	
	//Ensure cursors do not lie between points
	for (i=0; i<2; i++) {
  		GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, i+1, &x_cursor[i], &y_cursor[i]);
  		rounder = Round(x_cursor[i]/flim->time_per_point);
  		x_cursor[i] = rounder * flim->time_per_point;
  		SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, i+1, x_cursor[i], y_cursor[i]);
	}

	FLIM_setCursors(flim);
}




Flim* flim_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, char *data_dir)
{
	Flim* flim = (Flim*) malloc(sizeof(Flim));
	
	memset(flim, 0, sizeof(Flim));
	
	ui_module_constructor(UIMODULE_CAST(flim), name);
	ui_module_set_description(UIMODULE_CAST(flim), description);
	ui_module_set_error_handler(UIMODULE_CAST(flim), handler, flim);
	ui_module_set_data_dir(UIMODULE_CAST(flim), data_dir);

	return flim;
}

int flim_display(Flim* flim)
{
	char path[GCI_MAX_PATHNAME_LEN]="";
	
	if ((flim->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(flim), "flim_ui.uir", FLIM, 1)) < 0)
		return FLIM_ERROR; 

	ui_module_display_panel(UIMODULE_CAST(flim), flim->_main_ui_panel);

	return FLIM_SUCCESS;
}

int flim_hide(Flim *flim)
{
	ui_module_hide_panel(UIMODULE_CAST(flim), flim->_main_ui_panel);

	return FLIM_SUCCESS;
}


static void flim_draw_cursors(Flim *flim)
{
	double x_cursor[2], y_cursor[2], tMax, dT;
	double axisMin, axisMax, x;
	int i, rounder, mode;
	
	for (i=0; i<2; i++) 
  		GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, i+1, &x_cursor[i], &y_cursor[i]);
  		
	if (x_cursor[1] < x_cursor[0]) {
  		SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 1, x_cursor[1], y_cursor[0]);
  		SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, x_cursor[0], y_cursor[1]);
  		GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 1, &x_cursor[0], &y_cursor[0]);
  		GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, &x_cursor[1], &y_cursor[1]);
	}
	
	FIA_FindDoubleMax (flim->x_data, flim->number_of_points, &tMax);
  	dT = x_cursor[1] - x_cursor[0];

  	if (((dT * 3) + x_cursor[0]) > tMax) {
  		dT = (tMax - x_cursor[0])/3;
  		rounder = Round(dT/flim->time_per_point) - 1;
  		dT = rounder * flim->time_per_point;
  		x_cursor[1] = x_cursor[0] + dT;
  		SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, x_cursor[1], y_cursor[1]);
  	}
  	
  	dT /= 3.0;
	GetAxisScalingMode (flim->_main_ui_panel, FLIM_GRAPH, VAL_LEFT_YAXIS, &mode, &axisMin, &axisMax);
	x = x_cursor[0] + dT;
	PlotLine (flim->_main_ui_panel, FLIM_GRAPH, x, axisMax*0.8, x, axisMax, VAL_BLUE);
	x = x_cursor[0] + 2*dT;
	PlotLine (flim->_main_ui_panel, FLIM_GRAPH, x, axisMax*0.8, x, axisMax, VAL_BLUE);
	x = x_cursor[0] + 6*dT;
	PlotLine (flim->_main_ui_panel, FLIM_GRAPH, x, axisMax*0.8, x, axisMax, VAL_RED);
	x = x_cursor[0] + 9*dT;
	PlotLine (flim->_main_ui_panel, FLIM_GRAPH, x, axisMax*0.8, x, axisMax, VAL_RED);
	
	//Set globals for use in Tau image creation
  	flim->start = Round(x_cursor[0]/flim->time_per_point);
  	flim->width = Round(dT/flim->time_per_point);
}

//RJL 130106 - make Ydata double, (was unsigned short)
void flim_set_data(Flim *flim, double end_time, int number_of_points, double *x_data, double *y_data, int colour)
{
	double fyMax;
	int scale=0, maxIdx, rounder;
	double yMax;
	static int gFirstCall = 1;
	
	flim->x_data = x_data;
	flim->y_data = y_data;
	flim->plot_colour = colour;
	flim->number_of_points = number_of_points;
	flim->end_time = end_time;

	DeleteGraphPlot (flim->_main_ui_panel, FLIM_GRAPH, -1, VAL_IMMEDIATE_DRAW);

	// Set suitable axis scaling
	flim->time_per_point = flim->end_time / flim->number_of_points;
	
	// Autoscale x axis
	while (flim->end_time > 10) {
		flim->end_time /= 10;
		scale ++;
	}

	SetAxisScalingMode (flim->_main_ui_panel, FLIM_GRAPH_2, VAL_BOTTOM_XAXIS, VAL_MANUAL, 0, ceil(flim->end_time)*pow(10.0, scale));
	SetAxisScalingMode (flim->_main_ui_panel, FLIM_GRAPH, VAL_BOTTOM_XAXIS, VAL_MANUAL, 0, ceil(flim->end_time)*pow(10.0, scale));
	SetCtrlAttribute (flim->_main_ui_panel, FLIM_GRAPH, ATTR_XLOOSE_FIT_AUTOSCALING_UNIT, scale);
	SetCtrlAttribute (flim->_main_ui_panel, FLIM_GRAPH, ATTR_XLOOSE_FIT_AUTOSCALING, 1);

	// Autoscale Y axis
	scale = 0;
	maxIdx = FIA_FindDoubleMax (flim->y_data, flim->number_of_points, &yMax);
	fyMax = (double) yMax;
	
	while (fyMax > 10) {
		fyMax /= 10;
		scale ++;
	}

	SetAxisScalingMode (flim->_main_ui_panel, FLIM_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, 0, ceil(fyMax)*pow(10.0, scale));
	SetCtrlAttribute (flim->_main_ui_panel, FLIM_GRAPH, ATTR_YLOOSE_FIT_AUTOSCALING_UNIT, scale);
	SetCtrlAttribute (flim->_main_ui_panel, FLIM_GRAPH, ATTR_YLOOSE_FIT_AUTOSCALING, 1);
	
	PlotXY (flim->_main_ui_panel, FLIM_GRAPH, flim->x_data, flim->y_data,
			flim->number_of_points, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
			VAL_NO_POINT, VAL_SOLID, 1, flim->plot_colour);
	
	// Set some initial cursor positions
  	rounder = maxIdx;
	SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 1, rounder * flim->time_per_point, 0);
  	rounder = maxIdx + flim->number_of_points/16;
	SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, rounder * flim->time_per_point, 0);

	flim_draw_cursors(flim);

	FLIM_CalculateTau(flim);
}

/*
void main()
{
	double xData[256], y[256], tpp;
	unsigned short yData[256];
	double e = 2.718281828;
	int i, Tz=27;
	
	flim_DisplayPanel();
	
	tpp = 10.0/256;	//ns per point
	for (i=0; i<Tz; i++) {
		xData[i] = i*tpp;
		y[i] = 3.0;
		yData[i] = 3;
	}
	for (i=Tz; i<256; i++) {
		xData[i] = i*tpp;
		y[i] = 59.0 * pow(e, -xData[i-Tz]*.5) + 3;
		yData[i] = Round(59.0 * pow(e, -xData[i-Tz]*.5) + 3);
	}
	flim_setData(10.0, 256, xData, yData, VAL_GREEN);
	
	RunUserInterface();
	
}
*/

void flim_CalculateTau(double width, double A1, double A2, double A3, double *T, double *Io, double *B12, double *B23)
{
	double Q=0, fact1, fact2;
	double Ao, Ainf, Tau;
	double e = 2.718281828;
	double epsilon = 1.0e-10;
	
	*T = 0;
	*Io = 0;
	*B12 = 0;
	*B23 = 0;

	if (fabs(A1-A2) <= epsilon) {
		//GCI_MessagePopup("Can't calculate Tau", "Interval dT is too small");
		return ;
	}
  
	Q = (A2-A3)/(A1-A2);
	if (Q <= 0) {
		//GCI_MessagePopup("Can't calculate Tau", "Q <= 0");
		return ;
	}
	fact1 = log(Q);
	//fact1 = gLnTable[Round(Q*1000)];
	fact2 = -width*pow((1-Q), 2);
  
	Tau = width/-fact1;
	*T = Tau;
	
	Ao = ((A1-A2)*fact1)/fact2;
	*Io = Ao;
	
	Ainf = (A1 - ((A1-A2)/(1-pow(e,-width/Tau))))/width;
	*B12 = Ainf;
	Ainf = (A2 - ((A2-A3)/(1-pow(e,-width/Tau))))/width;
	*B23 = Ainf;
}











/*
void flim_setParams(double flim->end_time, int nPoints)
{
	double x_cursor[2], y_cursor[2], dT;
	int i, rounder;
	
	//Called when ADC resolution is changed
	
	if (flim->_main_ui_panel < 0) return;	//Not loaded yet
	
	//RJL 130206 - 5 lines below added
	if (gXdata) free(gXdata);
	if (gYdata) free(gYdata);
	
	gXdata = (double *)malloc(nPoints * sizeof(double));
	gYdata = (double *)malloc(nPoints * sizeof(double));
	flim->number_of_points = nPoints;

	//Reset globals for use in Tau image creation
	flim->time_per_point = flim->end_time/nPoints;

	//Ensure cursors do not lie between points
	for (i=0; i<2; i++) {
  		GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, i+1, &x_cursor[i], &y_cursor[i]);
  		rounder = Round(x_cursor[i]/flim->time_per_point);
  		x_cursor[i] = rounder * flim->time_per_point;
  		SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, i+1, x_cursor[i], y_cursor[i]);
	}
  	dT = x_cursor[1] - x_cursor[0];
  	flim->start = Round(x_cursor[0]/flim->time_per_point);
  	flim->width = Round(dT/flim->time_per_point);
}
*/

/*
static void FLIM_MakeLogLUT()
{
	int i;
	
	//Make ln LUT to save time when making Tau images.
	for (i=1; i<1001; i++)
		gLnTable[i] = log((double)i/1000.0);
}
*/


int CVICALLBACK OnQuitFlim (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Flim *flim = (Flim*) callbackData;
			flim_hide(flim);
		}
	}

	return 0;
}

int CVICALLBACK OnSaveCursors (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FILE *fp;
	char path[GCI_MAX_PATHNAME_LEN];
	double x_cursor[2], y_cursor[2];

	switch (event)
	{
		case EVENT_COMMIT:
		{
			Flim *flim = (Flim*) callbackData;
		  	GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 1, &x_cursor[0], &y_cursor[0]);
		  	GetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, &x_cursor[1], &y_cursor[1]);
		  	
			GetProjectDir (path);
			strcat(path, "\\data\\cursors.flim");
			SetFileAttrs (path, 0, -1, -1, -1);	   //clear read-only
			fp = fopen (path, "w");
			fprintf (fp, "%f\n", x_cursor[0]);
			fprintf (fp, "%f\n", x_cursor[1]);
			fclose(fp);
			SetFileAttrs (path, 1, -1, -1, -1);	   //set read-only
		
			break;
		}	
	}

	return 0;
}

int CVICALLBACK OnLoadCursors (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int fSize;
	FILE *fp;
	char path[GCI_MAX_PATHNAME_LEN];
	double x_cursor[2];
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Flim *flim = (Flim*) callbackData;
			GetProjectDir (path);
			strcat(path, "\\data\\cursors.flim");
			
			if (!FileExists (path, &fSize))
				break;
			
			fp = fopen (path, "r");
			fscanf (fp, "%lf\n", &x_cursor[0]);
			fscanf (fp, "%lf\n", &x_cursor[1]);
			fclose(fp);

	  		SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 1, x_cursor[0], 0);
	  		SetGraphCursor (flim->_main_ui_panel, FLIM_GRAPH, 2, x_cursor[1], 0);
			FLIM_setCursors(flim);
			break;
		}
	}
	
	return 0;
}




int CVICALLBACK OnFLIMcursors (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT: 
		{
			Flim *flim = (Flim*) callbackData;
			flim_cursors(flim);
			break;
		}
	}

	return 0;
}

/*
int flim_CreateTauImage(unsigned short *data, int x, int y, int t, double tpp, double *tau)
{
	double *tranData, *tp, T, maxVal;
	int r, c, i;
	
	if (gImtype == 0) return -1;
	
	//Create an array of floating point pixels. We display Tau images in pico-seconds.
	tranData = (double *) calloc (t, sizeof(double));
	
	for (r=0; r<y; r++) {
		for (c=0; c<x; c++) {
			tp = tranData;
			for (i=0; i<t; i++)
				*tp++ = *data++;
			FIA_FindDoubleMax (tranData, t, &maxVal);
			if (maxVal < 5) {
				*tau++ = 0.0;
				continue;
			}
			if (gImtype == 1) FLIM_CalculateTauS(tranData, &T);
			else if (gImtype == 2) FLIM_CalculateTauL(tranData, &T);
			*tau++ = max(T, 0.0);
		}
	}
	
	free(tranData);
	
	return 0;
}

void flim_SetRequiredImType(int imtype)
{
	gImtype = imtype;
	
}
*/

int CVICALLBACK OnShowFits (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Flim *flim = (Flim*) callbackData;
			DeleteGraphPlot (flim->_main_ui_panel, FLIM_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
		  	FLIM_ShowFitL(flim);
		  	FLIM_ShowFitS(flim);

			break;
		}
	}

	return 0;
}
