#ifndef PROJEKT1_UTILS_list_H_
#define PROJEKT1_UTILS_list_H_

#include <sys/types.h>
#include "input.h"

typedef struct list list_t;
typedef struct node node_t;
typedef struct nand nand_t;

list_t* list_new_list();
void    list_delete_list(list_t *list);
ssize_t list_size(list_t *list);
nand_t* list_ith_connection(list_t *list, ssize_t k);
void    list_remove_connection(list_t *list, input_t *input_to_remove);
node_t* list_push_new_connection(list_t *list, input_t *input_to_add);

#endif //PROJEKT1_UTILS_list_H_
