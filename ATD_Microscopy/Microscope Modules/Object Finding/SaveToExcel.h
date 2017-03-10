#ifndef __CELL_EXCELL__
#define __CELL_EXCELL__

#include "cell.h" 

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

int  CellFinding_CreateNewExcelFile(char *fname, int *number_of_cells);

int  CellFinding_OpenExcelFile(char *fname, int *number_of_cells);

void CellFinding_IntFromExcel(int sheet, int r, int c, int *val);

void CellFinding_DoubleFromExcel(int sheet, int r, int c, double *val);

void CellFinding_DoubleColumnFromExcel(int sheet, int c, double *data, int length);

void CellFinding_IntColumnFromExcel(int sheet, int c, int *data, int length);

void CellFinding_DoubleColumnToExcel(int sheet, int r, int c, double *data, int length);

void CellFinding_DoubleToExcel(int sheet, int r, int c, double val); 

void CellFinding_IntToExcel(int sheet, int r, int c, int val) ;

void CellFinding_GetClassInfo(int *singles, int *doublets, int *grot);

void CellFinding_WriteCellData(Cell cell);

void CellFinding_SaveExperimentData(char *title, char *dish, char *mag, int objective,
									int cube, char *cube_name, double exposure, double gain,
									double region_x, double region_y, 
									double first_frame_x, double first_frame_y,
									int number_of_cells, int number_of_singles,
									int number_of_doublets, int number_of_grot);

int CellFinding_ReadExperimentData(char *title, char *dish, char *mag, int *objective,
									int *cube, char *cube_name, double *exposure, double *gain,
									double *region_x, double *region_y, 
									double *first_frame_x, double *first_frame_y,
									int *number_of_cells, int *number_of_singles,
									int *number_of_doublets, int *number_of_grot);

#endif
