#include "list.h"
#include "input.h"
#include "memory_management.h"

struct node {
  input_t *stored_input;
  node_t *next;
  node_t *prev;
};

struct list {
  node_t *head;
  node_t *tail;
};

static node_t *new_node(input_t *input) {
  node_t *node = (node_t *)malloc(sizeof(node_t));
  MEMORY_ALLOCATION_CHECK(node);
  node->stored_input = input;
  node->next = NULL;
  node->prev = NULL;
  return node;
};

/* cleanup to use when the error occurred in the constructor*/
static void list_init_cleanup(list_t *list) {
  errno = ENOMEM;
  free(list->head);
  free(list->tail);
  free(list);
}

list_t *list_new_list() {
  list_t *list = (list_t *)malloc(sizeof(list_t));
  MEMORY_ALLOCATION_CHECK(list);
  list->head = new_node(NULL);
  list->tail = new_node(NULL);

  if (!list->head || !list->tail) {
    list_init_cleanup(list);
    return NULL;
  }

  list->head->next = list->tail;
  list->tail->prev = list->head;

  return list;
};

static void list_link(node_t *first_node, node_t *second_node) {
  first_node->next = second_node;
  second_node->prev = first_node;
}

static void delete_node(node_t *node) {
  if (node->stored_input) {
    input_set_to_empty(node->stored_input);
    list_link(node->prev, node->next);
  }
  free(node);
}

void list_delete_list(list_t *list) {
  if (!list)
    return; // in case it is null
  node_t *current_node = list->head->next;
  while (current_node != list->tail) {
    node_t *next_node = current_node->next;
    delete_node(current_node);
    current_node = next_node; // moving to next element
  }
  free(list->head);
  free(list->tail);
  free(list);
}

void list_remove_connection(list_t *list, input_t *input_to_remove) {
  node_t *current_node = list->head->next;
  while (current_node) {
    input_t *current_connection = current_node->stored_input;

    if (current_connection == input_to_remove) {
      delete_node(current_node);
      return;
    }
    current_node = current_node->next; // moving to next node
  }
}

ssize_t list_size(list_t *list) {
  ssize_t size = 0;
  node_t *current_node = list->head->next;
  if (current_node == list->tail)
    return 0;

  while (current_node) { // while node is not null
    size++;
    current_node = current_node->next;
    ; // moving to next element
  }
  return size - 1;
}

nand_t *list_ith_connection(list_t *list, ssize_t k) {
  node_t *current_node = list->head->next;
  for (ssize_t i = 0; i < k; i++) {
    current_node = current_node->next;
    ; // moving to next element
  }

  return input_mother_nand(current_node->stored_input);
}

/*returns pointer to added element if succeeded, NULL otherwise*/
node_t *list_push_new_connection(list_t *list, input_t *input_to_add) {
  node_t *node = new_node(input_to_add);
  MEMORY_ALLOCATION_CHECK(node);
  list_link(list->tail->prev, node);
  list_link(node, list->tail);
  return node;
};
