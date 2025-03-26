#include "input.h"

#include "list.h"
#include "memory_management.h"

enum in_type { NAND_INPUT, BOOL_SIGNAL_INPUT, EMPTY_INPUT };

struct input {
  enum in_type type;
  nand_t *in_nand;      // nand which gives its output to this input
  list_t *in_nand_list; // some nand's list of out connections which stores
                        // this input
  const bool *in_signal;
  nand_t *mother_nand; // nand which this input is part of
};

/* removes external objects connecting to this input*/
static void input_remove_external_connections(input_t *input) {
  if (input->type == NAND_INPUT) {
    list_remove_connection(input->in_nand_list, input);
  }
}

void input_set_to_empty(input_t *input) {
  input->in_nand = NULL;
  input->in_signal = NULL;
  input->type = EMPTY_INPUT;
}

/* clears input, so that nothing is connected to it*/
void input_remove_in_connection(input_t *input) {
  input_remove_external_connections(input);
  input_set_to_empty(input);
}

input_t *input_new(nand_t *mother_nand) {
  input_t *input = (input_t *)malloc(sizeof(input_t));
  MEMORY_ALLOCATION_CHECK(input);
  input_set_to_empty(input);

  input->mother_nand = mother_nand;

  return input;
}

input_t *input_delete(input_t *input) {
  input_remove_external_connections(input);
  free(input);
  return NULL;
}

void input_connect_nand(input_t *input, nand_t *in_nand, list_t *in_nand_list) {
  input->type = NAND_INPUT;
  input->in_nand = in_nand;
  input->in_nand_list = in_nand_list;
}

void input_connect_signal(input_t *input, const bool *s) {
  input->type = BOOL_SIGNAL_INPUT;
  input->in_signal = s;
}

bool input_is_empty(input_t *input) {
  if (input->type == EMPTY_INPUT)
    return true;
  return false;
}

bool input_is_signal(input_t *input) {
  if (input->type == BOOL_SIGNAL_INPUT)
    return true;
  return false;
}

/*returns connected signal value - can be used only when such signal
 * is connected*/
const bool *input_signal(input_t *input) {
  if (input_is_signal(input))
    return input->in_signal;
  errno = EINVAL;
  return NULL;
}

/*returns connected in nand - can be used only when such nand
 * is connected*/
nand_t *input_in_nand(input_t *input) {
  if (input->type == NAND_INPUT)
    return input->in_nand;
  errno = EINVAL;
  return NULL;
}

/*returns mother nand of this input i.e. gate that this input is part of*/
nand_t *input_mother_nand(input_t *input) { return input->mother_nand; }