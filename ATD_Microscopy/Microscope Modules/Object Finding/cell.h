#ifndef __CELL__
#define __CELL__

////////////////////////////////////////////////////////////////////
// Module to define a cell object for the microfocus system
// Glenn Pierce - July 2005
////////////////////////////////////////////////////////////////////

typedef enum {SINGLE = 1, CLUSTER, GROT} CellType;

typedef struct {

	int 	id;
	int 	type;
	int		image_x;
	int		image_y;
	int 	plotted;
	
	double 	x;
	double 	y;
	double 	z;
	double 	area;
	double  diameter;
	double  perimeter;
	double 	shape;
	double 	intensity;
	double 	intensity_stddev;
	double  dose;

} Cell;


Cell 	gci_cell_new(int id, int type, double x, double y);

long	gci_cell_get_colour_for_type(Cell *cell);

double	gci_cell_get_distance_between_two_cells(Cell *first_cell, Cell *second_cell);

double 	gci_cell_get_x_coordinate(Cell *cell);

double 	gci_cell_get_y_coordinate(Cell *cell);

int 	gci_cell_get_type(Cell *cell);

int		__cdecl gci_cell_compare_by_x(const void *lhs, const void *rhs);

int		__cdecl gci_cell_compare_by_y(const void *lhs, const void *rhs);

#endif
