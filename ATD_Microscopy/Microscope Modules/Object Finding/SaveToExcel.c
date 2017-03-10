#include <formatio.h>
#include <ddesupp.h>
#include <utility.h>
#include <toolbox.h>
#include <analysis.h>
#include <ansi_c.h>
#include <userint.h>
#include "ExcelAutomation.h"

#include "string_utils.h"
#include "SaveToExcel.h"

////////////////////////////////////////////////////////////////////////////
// Module to perform Excel file I/O for the microfocus system
// Glenn Pierce - July 2005
////////////////////////////////////////////////////////////////////
// Ros Locke - October 2005
// Added CellFinding_ReadExperimentData()
////////////////////////////////////////////////////////////////////
// Ros Locke - June 2006
// Added CellFinding_DoubleColumnFromExcel(), CellFinding_IntColumnFromExcel()
// and CellFinding_DoubleColumnToExcel()
////////////////////////////////////////////////////////////////////

#define FIRST_CELL_ROW 3

void CellFinding_DoubleColumnFromExcel(int sheet, int c, double *data, int length) 
{
	GCI_ReadDoubleColumnFromExcel (sheet, FIRST_CELL_ROW, c, data, length);
}

void CellFinding_IntColumnFromExcel(int sheet, int c, int *data, int length) 
{
	double *buffer;
	
	buffer = (double *)calloc(length, sizeof(double));
	
	GCI_ReadDoubleColumnFromExcel (sheet, FIRST_CELL_ROW, c, buffer, length);
	ConvertArrayType (buffer, VAL_DOUBLE, data, VAL_INTEGER, length);
	
	free(buffer);
}

void CellFinding_DoubleColumnToExcel(int sheet, int r, int c, double *data, int length) 
{
    GCI_WriteColumnToExcel (1, r, c, data, length, 4L);
}

void CellFinding_DoubleFromExcel(int sheet, int r, int c, double *val) 
{
	double cellVal;
	
	GCI_ReadDoubleFromExcelCell(sheet, r, c, &cellVal);
	*val = cellVal;
}


void CellFinding_IntFromExcel(int sheet, int r, int c, int *val) 
{
	double cellVal;
	
	GCI_ReadDoubleFromExcelCell(sheet, r, c, &cellVal);
	*val = (int)cellVal;
}


void CellFinding_DoubleToExcel(int sheet, int r, int c, double val) 
{
	double cellVal;
	
	GCI_WriteDoubleToExcelCell(sheet, r, c, val);
}


void CellFinding_IntToExcel(int sheet, int r, int c, int val) 
{
	GCI_WriteDoubleToExcelCell(sheet, r, c, (int) val);
}


int CellFinding_CreateNewExcelFile(char *fname, int *number_of_cells)
{
	int ret, number_of_rows=0, status;   
	
	
	if (GCI_ShutdownExcelApp ())
		return -1;
	
	if (GCI_LaunchExcelApp())
		return -1;
	
	status = GCI_CreateNewWorkbookAndSheets();
  		
  	if (status < 0) {
    
      	MessagePopup ("", "I cannot create a new spreadsheet.");
      	return -1;
  	}
  
  	status = GCI_SaveAsExcelFile(fname);
  	
  	if (status < 0) {
    	
   		MessagePopup ("", "I cannot save this spreadsheet.");
      	return -1;
  	}
  
  	ret = GCI_WriteStringToExcelCell(1, 1, 1, "Cell");
	ret = GCI_WriteStringToExcelCell(1, 1, 2, "x (um)");
	ret = GCI_WriteStringToExcelCell(1, 1, 3, "y (um)");
	ret = GCI_WriteStringToExcelCell(1, 1, 4, "Area (um)");
	ret = GCI_WriteStringToExcelCell(1, 1, 5, "Perimeter (um)");
	ret = GCI_WriteStringToExcelCell(1, 1, 6, "Shape");
	ret = GCI_WriteStringToExcelCell(1, 1, 7, "Mean Intensity");
	ret = GCI_WriteStringToExcelCell(1, 1, 8, "Std. Deviation");
	ret = GCI_WriteStringToExcelCell(1, 1, 9, "Cell Type");
	ret = GCI_WriteStringToExcelCell(1, 1, 10, "Thumbnail");
	ret = GCI_WriteStringToExcelCell(1, 1, 11, "Dose");

	CellFinding_IntFromExcel(2, 12, 2, &number_of_rows); 
    	  	
    *number_of_cells = number_of_rows - FIRST_CELL_ROW;
    
  	return 0;
}


int CellFinding_OpenExcelFile(char *fname, int *number_of_cells)
{

	//Close it first in case an instance of Excel is already open
	if (GCI_ShutdownExcelApp ())
		return -1;
	
	if (GCI_LaunchExcelApp())
		return -1;

    GCI_SetExcelVisibility(0);

  	if ( GCI_OpenExcelFile(fname) < 0) {
    	MessagePopup ("", "I cannot open this spreadsheet.");
    	return -1;
  	}
	
	CellFinding_IntFromExcel(2, 12, 2, number_of_cells);

	return 0;
}


void CellFinding_WriteCellData(Cell cell)
{
	int row_number;

	row_number = cell.id + 2;

	CellFinding_IntToExcel(1, row_number, 1, cell.id);
	CellFinding_IntToExcel(1, row_number, 9, cell.type);
	
	CellFinding_DoubleToExcel(1, row_number, 2, cell.x);
	CellFinding_DoubleToExcel(1, row_number, 3, cell.y);
	CellFinding_DoubleToExcel(1, row_number, 4, cell.area);
	CellFinding_DoubleToExcel(1, row_number, 5, cell.perimeter);
	CellFinding_DoubleToExcel(1, row_number, 6, cell.shape);
	CellFinding_DoubleToExcel(1, row_number, 7, cell.intensity);
	CellFinding_DoubleToExcel(1, row_number, 8, cell.intensity_stddev);
	CellFinding_DoubleToExcel(1, row_number, 11, cell.dose);  
}


void CellFinding_SaveExperimentData(char *title, char *dish, char *mag, int objective,
									int cube, char *cube_name, double exposure, double gain,
									double region_x, double region_y, 
									double first_frame_x, double first_frame_y,
									int number_of_cells, int number_of_singles,
									int number_of_doublets, int number_of_grot) 
{
	//Save experimental details on sheet 2 in same workbook
	//Unfortunately can't use ClientDDEWrite because it always puts it on sheet 1 (bummer)
	
	char temp[500], *time;
	
	if (number_of_cells < 1)
		return;
		
	if (GCI_SetActiveWorkbookSheet(2))
		return;
	
	GCI_WriteStringToExcelCell(2, 1, 1, "Experiment");
	GCI_WriteStringToExcelCell(2, 1, 2, title);
	
	GCI_WriteStringToExcelCell(2, 2, 1, "Dish");
	GCI_WriteStringToExcelCell(2, 2, 2, dish);
	
	GCI_WriteStringToExcelCell(2, 3, 1, "Date");
	english_date(temp);
	GCI_WriteStringToExcelCell(2, 3, 2, temp);
	
	GCI_WriteStringToExcelCell(2, 4, 1, "Time");
	time = TimeStr ();
	GCI_WriteStringToExcelCell(2, 4, 2, time);
	
	GCI_WriteStringToExcelCell(2, 5, 1, "Mode");
	GCI_WriteStringToExcelCell(2, 5, 2, "Fluorescence");
	
	GCI_WriteStringToExcelCell(2, 6, 1, "Objective");
	sprintf(temp, "No. %d (Magnification %s)", objective, mag);
	GCI_WriteStringToExcelCell(2, 6, 2, temp);
	
	GCI_WriteStringToExcelCell(2, 7, 1, "Cube");
	sprintf(temp, "No. %d (%s)", cube, cube_name);
	GCI_WriteStringToExcelCell(2, 7, 2, temp);
	//GCI_WriteIntToExcelCell(2, 7, 2, cube);
	
	GCI_WriteStringToExcelCell(2, 8, 1, "Exposure (ms)");
	sprintf(temp, "%.2f", exposure);
	GCI_WriteStringToExcelCell(2, 8, 2, temp);
	
	GCI_WriteStringToExcelCell(2, 9, 1, "Gain");
	sprintf(temp, "%.2f", gain);
	GCI_WriteStringToExcelCell(2, 9, 2, temp);
	
	GCI_WriteStringToExcelCell(2, 10, 1, "Region size (um)");
	sprintf(temp, "%.1f x %.1f", region_x, region_y);
	GCI_WriteStringToExcelCell(2, 10, 2, temp);
	
	GCI_WriteStringToExcelCell(2, 11, 1, "First Frame (um)");
	sprintf(temp, "%.2f,%.2f", first_frame_x, first_frame_y);
	GCI_WriteStringToExcelCell(2, 11, 2, temp);
	
	GCI_WriteStringToExcelCell(2, 12, 1, "No. Objects");
	GCI_WriteIntToExcelCell(2, 12, 2, number_of_cells);
	
	GCI_WriteStringToExcelCell(2, 13, 1, "Single Cells");
	GCI_WriteIntToExcelCell(2, 13, 2, number_of_singles);
	
	GCI_WriteStringToExcelCell(2, 14, 1, "Multiple Cells");
	GCI_WriteIntToExcelCell(2, 14, 2, number_of_doublets);
	
	GCI_WriteStringToExcelCell(2, 15, 1, "Grot");
	GCI_WriteIntToExcelCell(2, 15, 2, number_of_grot);
	
	GCI_SetActiveWorkbookSheet(1);

	GCI_SaveExcelWorkbookNoPrompt();
}

int CellFinding_ReadExperimentData(char *title, char *dish, char *mag, int *objective,
									int *cube, char *cube_name, double *exposure, double *gain,
									double *region_x, double *region_y, 
									double *first_frame_x, double *first_frame_y,
									int *number_of_cells, int *number_of_singles,
									int *number_of_doublets, int *number_of_grot) 
{
	char temp[500];
	double ftemp;
	
	//Read experimental details from sheet 2 in same workbook

	if (GCI_SetActiveWorkbookSheet(2))
		return -1;
	
	if (GCI_ReadStringFromExcelCell (2, 1, 2, title)) strcpy(title, "");
	if (GCI_ReadStringFromExcelCell (2, 2, 2, dish)) strcpy(dish, "");
	
	if (GCI_ReadStringFromExcelCell (2, 6, 2, temp)) return -1;
	sscanf(temp, "No. %d (Magnification %s)", objective, mag);
	
	if (GCI_ReadStringFromExcelCell (2, 7, 2, temp)) return -1;
	sscanf(temp, "No. %d (%s)", cube, cube_name);
	
	if (GCI_ReadDoubleFromExcelCell (2, 8, 2, exposure)) return -1;
	if (GCI_ReadDoubleFromExcelCell (2, 9, 2, gain)) return -1;

	if (GCI_ReadStringFromExcelCell (2, 10, 2, temp)) return -1;
	sscanf(temp, "%lf x %lf", region_x, region_y);
	
	if (GCI_ReadStringFromExcelCell (2, 11, 2, temp)) return -1;
	sscanf(temp, "%lf,%lf", first_frame_x, first_frame_y);

	if (GCI_ReadDoubleFromExcelCell (2, 12, 2, &ftemp)) return -1;
	*number_of_cells = (int)ftemp;

	if (GCI_ReadDoubleFromExcelCell (2, 13, 2, &ftemp)) return -1;
	*number_of_singles = (int)ftemp;

	if (GCI_ReadDoubleFromExcelCell (2, 14, 2, &ftemp)) return -1;
	*number_of_doublets = (int)ftemp;

	if (GCI_ReadDoubleFromExcelCell (2, 15, 2, &ftemp)) return -1;
	*number_of_grot = (int)ftemp;

	GCI_SetActiveWorkbookSheet(1);
	
	return 0;
}


void CellFinding_GetClassInfo(int *singles, int *doublets, int *grot)
{
	CellFinding_IntFromExcel(2, 13, 2, singles);
	CellFinding_IntFromExcel(2, 14, 2, doublets);
	CellFinding_IntFromExcel(2, 15, 2, grot);
}

