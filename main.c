//
//  main.c
//  scheduling
//
//  Created by Linisac Wu on 3/5/19.
//  Copyright © 2019 LINIS WORKSHOP. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//The algorithm is based on the tree representation of set given in Section 21.3 Disjoint-set forests
//in textbook, where the 2 heuristics 'union by rank' and 'path compression' are used. Whenever a
//deadline of a task is given, we look for the representative of the set in which that deadline is a
//member, and reference its available_slot (detailed in next paragraph) to assign it to the task; the
//entire set is then merged (linked) to the set that precedes it in time. (If the above available_slot
//is the first time slot, then we merge the entire set with the LAST set in time.)

//The type slot_set is used for sets of slots. We have an additional attribute 'available_slot'
//to point to the (real) available time slot of a set of slots, since the representative of the set
//(i.e. the root of the tree) may not itself be the available slot. In this implementation, time slots
//are put in an array and hence are referenced by index instead of pointer.
typedef struct slot_set slot_set;
struct slot_set {
    unsigned int available_slot;
    unsigned int parent;
    unsigned int rank;
};

//The procedures based on those in textbook. Note that we use an array and indices to refer to a time
//slot, so an additional parameter, i.e. the slot_set[] array, is given as part of the input.
//Also, instead of pointers, we use array indices in the procedures.
//Since the function link is part of C standard library, we rename the procedure link in textbook to
//merge here.
void make_set(slot_set[], unsigned int);
void unite(slot_set[], unsigned int, unsigned int);
void merge(slot_set[], unsigned int, unsigned int); //the link procedure in textbook
unsigned int find_set(slot_set[], unsigned int);

//This procedure is optional. It is used to display in a table the membership of each
//time slot in a set; time slots in the same set have the same time slot (index) as the
//representative of the set.
void display_all_sets(slot_set [], unsigned int);

//To be compatible with textbook, we let time slots and tasks start from 1. In other words, time 1
//is the time interval [0, 1]. However, in our internal representation we let both start from 0
//due to C's array indexing. That said, the messages printed out on screen is not affected, though.
//On the other hand, this implementation allows the user to enter the number of tasks and randomly
//generates the deadlines; in case the user does not enter a valid number, the default tasks are
//used. The tasks are assumed to have been sorted into monotonically decreasing order by penalty.
//Hence, only the deadlines of tasks are relevant, and penalties are not implemented in this program.
int main(int argc, const char * argv[]) {
    unsigned int size;
    unsigned int *deadline_of_task;
    
    
    printf("Enter the number of task(s): ");
    if (scanf("%u", &size) == 0) {//invalid argument for size, default task deadlines are used
        size = 10;
        deadline_of_task = (unsigned int *) calloc(size, sizeof(unsigned int));
        deadline_of_task[0] = 0;
        deadline_of_task[1] = 6;
        deadline_of_task[2] = 1;
        deadline_of_task[3] = 9;
        deadline_of_task[4] = 2;
        deadline_of_task[5] = 5;
        deadline_of_task[6] = 3;
        deadline_of_task[7] = 3;
        deadline_of_task[8] = 6;
        deadline_of_task[9] = 0;
    }
    else {//valid argument for size, randomly generated task deadlines are used
        deadline_of_task = (unsigned int *) calloc(size, sizeof(unsigned int));
        srand(time(NULL));
        for (unsigned int i = 0; i < size; i++)
            deadline_of_task[i] = rand() % size;
    }
    
    
    unsigned int last = size - 1;
    unsigned int format_length = 1 + (int)log10(size); //for message formatting
    
    
    //print out the description of task deadlines
    printf("Description of task(s)\n----------------------\n");
    char task_description[29 + 2 * format_length];
    sprintf(task_description,
            "task %%%uu has deadline at time %%%uu\n",
            format_length, format_length);
    for (unsigned int i = 0; i < size; i++)
        printf(task_description, i + 1, deadline_of_task[i] + 1);
    
    
    //initialize the time slots, invoking make_set
    slot_set slot[size];
    for (unsigned int i = 0; i < size; i++)
        make_set(slot, i);
    
    
    //schedule the tasks according to the given algorithm, and then print out the schedule
    printf("\nScheduling of task(s)\n---------------------\n");
    char schedule_description[34 + 2 * format_length];
    sprintf(schedule_description,
            "task %%%uu is scheduled in time slot %%%uu\n",
            format_length, format_length);
    for (unsigned int i = 0; i < size; i++) {
        unsigned int index_of_slot_to_schedule = slot[find_set(slot, deadline_of_task[i])].available_slot;
        
        printf(schedule_description, i + 1, index_of_slot_to_schedule + 1);
        
        if (i != size - 1) //no need to unite for the last iteration as there is only one set
            unite(slot,
                  index_of_slot_to_schedule,
                  (index_of_slot_to_schedule == 0) ? last : index_of_slot_to_schedule - 1);
        
        display_all_sets(slot, size);
    }
    
    
    free(deadline_of_task);
    return 0;
}

//The following functions/procedures are based on those given in textbook. As explained above,
//the available_slot attribute refers to the array index of the actual available slot of a set
//of slots, because the representative of the set may not be the available slot itself.
void make_set(slot_set slot[], unsigned int i) {
    slot[i].available_slot = i;
    slot[i].parent = i;
    slot[i].rank = 0;
}

void unite(slot_set slot[], unsigned int i, unsigned int j) {
    merge(slot, find_set(slot, i), find_set(slot, j));
}

void merge(slot_set slot[], unsigned int i, unsigned int j) {
    //slot i has just been assigned to a task. Now merge the whole set in which slot i is
    //a member to the set in which slot j is a member.
    if (slot[i].rank > slot[j].rank) {
        slot[j].parent = i;
        slot[i].available_slot = slot[j].available_slot;
    }
    else {
        slot[i].parent = j;
        
        if (slot[i].rank == slot[j].rank)
            slot[j].rank = slot[i].rank + 1;
    }
}

unsigned int find_set(slot_set slot[], unsigned int i) {
    if (i != slot[i].parent) {
        slot[i].parent = find_set(slot, slot[i].parent);
        slot[i].available_slot = slot[slot[i].parent].available_slot;
    }
    
    return slot[i].parent;
}

void display_all_sets(slot_set slot[], unsigned int size) {
    unsigned int format_length = 1 + (int)log10(size);
    char table_field[format_length + 1];
    sprintf(table_field, "%%%uu", format_length);
    
    printf("time slot         |");
    for (unsigned int i = 0; i < size; i++) {
        printf(table_field, i + 1);
        
        if (i == size - 1)
            printf("\n");
        else
            printf(" ");
    }
    
    printf("------------------|");
    for (unsigned int i = 0; i < size; i++) {
        if (i == size - 1) {
            for (unsigned int j = 0; j < format_length; j++)
                printf("-");
            
            printf("\n");
        }
        else {
            for (unsigned int j = 0; j <= format_length; j++)
                printf("-");
        }
    }
    
    printf("repre. of its set |"); //representative of its set
    for (unsigned int i = 0; i < size; i++) {
        printf(table_field, slot[find_set(slot, i)].available_slot + 1);
        
        if (i == size - 1)
            printf("\n");
        else
            printf(" ");
    }
}
