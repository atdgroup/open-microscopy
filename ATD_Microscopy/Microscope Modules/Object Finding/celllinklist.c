/************************************************************
*	linklist.c												*
*	Provides generic routines for building linked lists     *
*	This particular instance is singly linked				*
************************************************************/

#include <ansi_c.h> 
#include "celllinklist.h"

struct listnode
{
	CellNode next_node;
	Cell cell;
};

struct linkedlist
{
	int		 number_of_cells; 

	CellNode first_node;
	CellNode last_node;
	CellNode current_node;
};


static CellNode cell_list_get_next_node(CellLinkedList list)
{
	if(list->current_node == NULL)
		list->current_node = list->first_node;
	else
		list->current_node = list->current_node->next_node;
	
	if(list->current_node != NULL)
		return list->current_node;
	else
		return NULL;
}


CellLinkedList cell_list_new(void)
{
	CellLinkedList list;
	
	if( (list = (struct linkedlist *) malloc(sizeof(struct linkedlist))) == NULL) {
	
		fprintf(stderr, "Allocation of memory for new node failed!\n");
		return NULL;
	}
	
	list->number_of_cells = 0;
	list->first_node = NULL;
	list->last_node = NULL;
	list->current_node = NULL;

	return list;
}


void cell_list_destroy(CellLinkedList list)
{
	CellNode next_node, next_next_node;	

	if(list == NULL)
		return;

	next_node = list->first_node;
	
	while (next_node != NULL) { 
		next_next_node = next_node->next_node; 
		free(next_node);
		next_node = next_next_node;
	}
	if(list!= NULL)
		free(list);
}


/* Make a new tail for the list */
CellNode cell_list_add(CellLinkedList list, Cell cell)
{
	CellNode new_node;
	
	if ( ( new_node = (struct listnode *)malloc(sizeof(struct listnode)) ) == NULL) {
	
		fprintf(stderr, "Allocation of memory for new node failed!\n");
		return NULL;
	}
	
	if(list->first_node == NULL)
		list->first_node = new_node;	
		
	new_node->next_node = NULL;
	new_node->cell = cell;
		
	if(list->last_node != NULL)
		list->last_node->next_node = new_node;

	list->last_node = new_node;
	
	list->number_of_cells++; 
	
	return new_node;
}


int cell_list_count(CellLinkedList list)
{
	return list->number_of_cells;
}


void cell_list_reset(CellLinkedList list)
{
	list->current_node = NULL;
}


Cell* cell_list_get_next(CellLinkedList list)
{
	CellNode node = cell_list_get_next_node(list);

	if(node != NULL)
		return &(node->cell);
		
	return NULL;
}


Cell* cell_list_to_array(CellLinkedList list)
{
	Cell *cell_array, *next_cell;
	int size = cell_list_count(list), i=0;

	cell_array = (Cell *) malloc( sizeof(Cell) * size );
	
	while( (next_cell = cell_list_get_next(list)) != NULL) {
	
		cell_array[i].id = next_cell->id;
		cell_array[i].type = next_cell->type;
		cell_array[i].x = next_cell->x;
		cell_array[i].y = next_cell->y; 
		cell_array[i].area = next_cell->area; 
		cell_array[i].perimeter = next_cell->perimeter; 
		cell_array[i].shape = next_cell->shape; 
		cell_array[i].intensity = next_cell->intensity; 
		cell_array[i].intensity_stddev = next_cell->intensity_stddev; 
		cell_array[i].dose = next_cell->dose; 
		 
		i++;
	}
	
	return cell_array;
}


static CellNode cell_list_get_node_from_id(CellLinkedList list, int id)
{
	CellNode node = list->first_node;	
	
	if(node->cell.id == id)		 //i.e. id = 1
		return node;

	while ( (node = node->next_node) != NULL) { 
		
		if(node->cell.id == id)
			return node;
	}
	
	return NULL;
}


Cell cell_list_get_cell_with_id(CellLinkedList list, int id)
{
	CellNode node = cell_list_get_node_from_id(list, id);	
	
	return node->cell;
}


void cell_list_update_cell_with_id(CellLinkedList list, int id, Cell cell)
{
	CellNode node = cell_list_get_node_from_id(list, id);	
	
	node->cell = cell;
}
