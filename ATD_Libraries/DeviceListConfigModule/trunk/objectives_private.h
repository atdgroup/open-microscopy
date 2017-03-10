#ifndef __OBJECTIVE_MANAGER_PRIVATE__
#define __OBJECTIVE_MANAGER_PRIVATE__

int  		objective_manager_add_objective(ObjectiveManager* objective_manager);
int  		objective_manager_remove_objective(ObjectiveManager* objective_manager, int id);
int 		objective_manager_edit_objective(ObjectiveManager* objective_manager, int index);
int 		objective_manager_edit_objective_ui(ObjectiveManager* objective_manager, int index);
int 		objective_manager_add_objective_ui(ObjectiveManager* objective_manager);
int  		objective_manager_load_all_possible_objectives_into_ui(ObjectiveManager* objective_manager);
int  		objective_manager_remove_obj_at_turret_position(ObjectiveManager* objective_manager, int pos);
int  		objective_manager_edit_objective_at_turret_pos(ObjectiveManager* objective_manager, int pos, Objective *obj);
int  		objective_manager_add_objective_to_turret(ObjectiveManager* objective_manager, int id);
int 		objective_manager_switch_turret_position(ObjectiveManager* objective_manager, int id1, int id2);

#endif

 
