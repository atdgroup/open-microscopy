#ifndef __FLIM__
#define __FLIM__

#include "gci_ui_module.h"

#define FLIM_SUCCESS 0
#define FLIM_ERROR -1

typedef struct _Flim Flim;

struct _Flim
{
  UIModule parent;

  int _main_ui_panel;

  double *x_data;
  double *y_data;
  int number_of_points;
  int plot_colour;
  int width;
  int start;
  double end_time;
  double time_per_point;
};

Flim* flim_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, char *data_dir);
int flim_display(Flim* flim);
int flim_hide(Flim* flim);

void flim_set_data(Flim *flim, double endTime, int nPoints, double *Xdata, double *Ydata, int colour);

void flim_CalculateTau(double width, double A1, double A2, double A3, double *T, double *Io, double *B12, double *B23);
//int GCI_FLIM_CreateTauImage(unsigned short *data, int x, int y, int t, double tpp, double *tau);
//void GCI_FLIM_SetRequiredImType(int imtype);

int  CVICALLBACK OnShowFits(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCloseFlim(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCursors(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLoadCursors(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSaveCursors(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnShowFits(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif