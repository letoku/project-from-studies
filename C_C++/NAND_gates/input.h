#ifndef PROJEKT1__INPUT_H_
#define PROJEKT1__INPUT_H_

#include <stdbool.h>

typedef struct input input_t;
typedef struct nand nand_t;
typedef struct list list_t;

input_t*    input_new(nand_t *mother_nand);
void        input_set_to_empty(input_t *input);
void        input_remove_in_connection(input_t *input);
void        input_connect_nand(input_t *input, nand_t *in_nand, list_t *in_nand_list);
void        input_connect_signal(input_t *input, const bool *s);
bool        input_is_empty(input_t *input);
bool        input_is_signal(input_t *input);
const bool* input_signal(input_t *input);
nand_t*     input_in_nand(input_t *input);
nand_t*     input_mother_nand(input_t *input);
input_t*    input_delete(input_t *input);

#endif // PROJEKT1__INPUT_H_