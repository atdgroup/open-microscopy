#include "timelapse.h"
#include "StagePlate.h"

static int compare_int(const int lhs, const int rhs, int direction)
{
	int ret;

	if(direction != -1 && direction != 1)
	{
		GCI_MessagePopup("Error", "Assert");
		return 0;
	}
	
	if(lhs < rhs)
		ret = -1;
	else if (lhs > rhs)
		ret = 1;
	else 
		ret = 0;

	ret *= direction;

	return ret;
}

static int HORIZONTAL = 1;
static StagePlateHorizontalStartPosition horzontal_start_position = TL_WG_START_LEFT;
static StagePlateVerticalStartPosition vertical_start_position = TL_WG_START_TOP;
static int shortest_path = 1;

static int compare_row_elemets(const Well *lhs, const Well *rhs,
							   StagePlateHorizontalStartPosition horz_start,
							   StagePlateVerticalStartPosition vert_start,
							   int shortest_path)
{
	if(shortest_path)
	{
		int even_or_odd = lhs->row % 2;		// 0 if even 1 if odd
		even_or_odd = ((even_or_odd * 2) - 1) * -1;		// Produces 1 for even and -1 for odd rows

		// Account for horizontal start position
		return compare_int(lhs->col, rhs->col, even_or_odd * horz_start * vert_start);
	}
	else 
		return compare_int(lhs->col, rhs->col, horz_start);
}

static int compare_col_elemets(const Well *lhs, const Well *rhs,
							   StagePlateHorizontalStartPosition horz_start,
							   StagePlateVerticalStartPosition vert_start,
							   int shortest_path)
{
	if(shortest_path)
	{
		int even_or_odd = lhs->col % 2;		// 0 if even 1 if odd
		even_or_odd = ((even_or_odd * 2) - 1) * -1;		// Produces 1 for even and -1 for odd cols

		// Account for horizontal start position
		return compare_int(lhs->row, rhs->row, even_or_odd * horz_start * vert_start);
	}
	else
		return compare_int(lhs->row, rhs->row, vert_start);
}

void plate_points_set_sorting_params(int is_horizontal, StagePlateHorizontalStartPosition horz_start,
														StagePlateVerticalStartPosition vert_start,
														int is_shortest_path)
{
	HORIZONTAL = is_horizontal;
	horzontal_start_position = horz_start;
	vertical_start_position = vert_start;
	shortest_path = is_shortest_path;
}

int __cdecl plate_point_sort(const void *lhs_ptr, const void *rhs_ptr)
{
	int cmp;

	Well *lhs = (Well *) lhs_ptr;
	Well *rhs = (Well *) rhs_ptr;

	if(HORIZONTAL == 1)
	{
		cmp = compare_int((int)lhs->region.cy, (int)rhs->region.cy, (int)vertical_start_position);

		if(cmp == 0)
			return compare_row_elemets(lhs, rhs, horzontal_start_position, vertical_start_position, shortest_path);
	}
	else
	{
		cmp = compare_int((int)lhs->region.cx, (int)rhs->region.cx, (int)horzontal_start_position);

		if(cmp == 0)
			return compare_col_elemets(lhs, rhs, horzontal_start_position, vertical_start_position, shortest_path);
	}

	return cmp;
}