#ifndef _CELL_LINKLIST_H_
#define _CELL_LINKLIST_H_

#include "cell.h"

struct linkedlist;
typedef struct linkedlist *CellLinkedList;

struct listnode; 
typedef struct listnode *CellNode;


CellLinkedList cell_list_new(void);

void cell_list_destroy(CellLinkedList list);

int cell_list_count(CellLinkedList list);

CellNode cell_list_add (CellLinkedList list, Cell cell);

void cell_list_reset(CellLinkedList list);

Cell* cell_list_get_next(CellLinkedList list);

Cell* cell_list_to_array(CellLinkedList list);

Cell cell_list_get_cell_with_id(CellLinkedList list, int id);

void cell_list_update_cell_with_id(CellLinkedList list, int id, Cell cell);

#endif /* _CELL_LINKLIST_H_ */
