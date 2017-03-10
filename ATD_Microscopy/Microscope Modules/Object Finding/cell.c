#include "cell.h"
#include <ansi_c.h> 
#include <userint.h> 

////////////////////////////////////////////////////////////////////
// Module to define a cell object for the microfocus system
// Glenn Pierce - July 2005
////////////////////////////////////////////////////////////////////


Cell gci_cell_new(int id, int type, double x, double y)
{
	Cell cell;
	
	cell.id = id;
	cell.type = type;
	cell.x = x;
	cell.y = y;
	cell.plotted = 0;
	cell.area = 0.0;
	cell.diameter = 0.0;
	cell.perimeter = 0.0;
	cell.shape = 0.0;
	cell.intensity = 0.0;
	cell.intensity_stddev = 0.0;
	cell.dose = 0.0;
	
	cell.image_x = 0;
	cell.image_y = 0;
	
	return cell;
}


long gci_cell_get_colour_for_type(Cell *cell)
{
	switch(cell->type)
	{
		case SINGLE:
		
			return VAL_RED;
			
		case CLUSTER:
		
			return VAL_BLUE;
			
		case GROT:
		
			return VAL_GREEN;
	}

	return VAL_YELLOW;
}


int __cdecl gci_cell_compare_by_x(const void *lhs, const void *rhs)
{
	Cell *cell_lhs, *cell_rhs;
	
	cell_lhs = (Cell *) lhs;
	cell_rhs = (Cell *) rhs;

	if(cell_lhs->x < cell_rhs->x) {
	
		return -1;
	}
	else if (cell_lhs->x > cell_rhs->x) {
	
		return 1;
	}
	
	return 0;   // objs x's are equal
}


int __cdecl gci_cell_compare_by_y(const void *lhs, const void *rhs)
{
	Cell *cell_lhs, *cell_rhs;
	
	cell_lhs = (Cell *) lhs;
	cell_rhs = (Cell *) rhs;

	if(cell_lhs->y < cell_rhs->y) {
	
		return -1;
	}
	else if (cell_lhs->y > cell_rhs->y) {
	
		return 1;
	}
	
	return 0;   // objs x's are equal
}


double gci_cell_get_distance_between_two_cells(Cell *first_cell, Cell *second_cell)
{
	double x_diff, y_diff;

	//double abs_p1x = abs(first_cell->x);
	//double abs_p1y = abs(first_cell->y);
	//double abs_p2x = abs(second_cell->x);
	//double abs_p2y = abs(second_cell->y);
	
	//x_diff = (abs_p1x > abs_p2x) ? (abs_p1x - abs_p2x) : (abs_p2x - abs_p1x);
	//y_diff = (abs_p1y > abs_p2y) ? (abs_p1y - abs_p2y) : (abs_p2y - abs_p1y); 
	
	x_diff = first_cell->x - second_cell->x;
	y_diff = first_cell->y - second_cell->y;

	return sqrt(pow(x_diff, 2.0) + pow(y_diff, 2.0));
}


double gci_cell_get_x_coordinate(Cell *cell)
{
	return cell->x;
}

double gci_cell_get_y_coordinate(Cell *cell)
{
	return cell->y;
}


int gci_cell_get_type(Cell *cell)
{
	return cell->type;
}
