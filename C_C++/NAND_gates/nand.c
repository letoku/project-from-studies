#include "nand.h"
#include "input.h"
#include "list.h"
#include "memory_management.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

const unsigned long long int ULLONG_MAX = 18446744073709551615U;
unsigned long long dfs_index = 0; // number of last dfs call - used to
// determine whether state of the gate was calculated in current dfs call or
// whether it is already outdated

enum dfs_state {
  CALCULATED_1 = 1,
  CALCULATED_0 = 0,
  CIRCULAR_DEPENDENCY = -1,
  NULL_DEPENDENCY = -2,
  VISITED_NOT_YET_CALCULATED = 2,
  NOT_VISITED = 3
};

typedef struct signal_critical_path_length_pair {
  enum dfs_state signal;
  int critical_path_length;
} pair_t;

struct nand {
  list_t *out_connections;
  input_t **inputs;
  unsigned int n_of_inputs;

  enum dfs_state dfs_state;
  unsigned long long int dfs_index;
  int max_critical_path_length_at_input;
};

static pair_t evaluate_gate(nand_t *g);

static void init_cleanup_of_inputs_and_out_connections(nand_t *nand) {
  errno = ENOMEM;
  list_delete_list(nand->out_connections);
  free(nand->inputs);
  free(nand);
}

static void
init_cleanup_of_partially_allocated_inputs(nand_t *nand,
                                           int index_of_last_allocated) {
  errno = ENOMEM;
  for (int i = 0; i <= index_of_last_allocated; i++) {
    free(nand->inputs[i]);
  }
}

nand_t *nand_new(unsigned n) {
  nand_t *nand = (nand_t *)malloc(sizeof(nand_t));
  MEMORY_ALLOCATION_CHECK(nand);
  nand->n_of_inputs = n;
  nand->out_connections = list_new_list();
  nand->inputs = (input_t **)malloc(sizeof(input_t *) * n);
  if (!nand->out_connections || !nand->inputs) {
    init_cleanup_of_inputs_and_out_connections(nand);
    return NULL;
  }

  for (unsigned int i = 0; i < n; i++) {
    nand->inputs[i] = input_new(nand);
    if (!nand->inputs[i]) {
      init_cleanup_of_partially_allocated_inputs(nand, i - i);
      init_cleanup_of_inputs_and_out_connections(nand);
      return NULL;
    }
  }

  nand->dfs_state = NOT_VISITED;
  nand->dfs_index = 0;
  nand->max_critical_path_length_at_input = 0;

  return nand;
}

static void nand_delete_inputs(nand_t *g) {
  for (unsigned int i = 0; i < g->n_of_inputs; i++) {
    input_delete(g->inputs[i]);
  }
  free(g->inputs);
}

static void nand_delete_outputs(nand_t *g) {
  list_delete_list(g->out_connections);
}

void nand_delete(nand_t *g) {
  if (!g)
    return; // if receives null pointer then does nothing

  nand_delete_inputs(g);
  nand_delete_outputs(g);
  free(g);
}

static int add_out_connection(nand_t *nand, input_t *connection) {
  if (list_push_new_connection(nand->out_connections, connection)) {
    return 0;
  }
  return -1;
}

/* returns true if gate is not null and k is valid index of input*/
static bool is_nand_and_k_valid(const nand_t *nand, unsigned k) {
  if (!nand || k >= nand->n_of_inputs) {
    errno = EINVAL;
    return false;
  }
  return true;
}

/* returns true if input can be connected to nand */
/* input is void* because it can be one of either two types */
static bool is_connection_valid(const void *input, const nand_t *nand,
                                unsigned k) {
  if (!input) {
    errno = EINVAL;
    return false;
  }
  return is_nand_and_k_valid(nand, k);
}

int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {
  if (!is_connection_valid(g_out, g_in, k))
    return -1;

  input_remove_in_connection(g_in->inputs[k]);
  input_connect_nand(g_in->inputs[k], g_out, g_out->out_connections);
  return add_out_connection(g_out, g_in->inputs[k]);
}

int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
  if (!is_connection_valid(s, g, k))
    return -1;

  input_remove_in_connection(g->inputs[k]);
  input_connect_signal(g->inputs[k], s);
  return 0;
}

static pair_t pass_result_of_already_evaluated(nand_t *g) {
  pair_t val = {g->dfs_state, g->max_critical_path_length_at_input + 1};
  return val;
}

/*propagates info about empty input to the state of gate*/
static void propagate_empty_input(nand_t *g) { g->dfs_state = NULL_DEPENDENCY; }

/* returns negation of input, propagating further info about input*/
static bool propagate_signal_input(nand_t *g, unsigned int i) {
  bool signal = *input_signal(g->inputs[i]);
  return !signal;
}

/* returns negation of input and does all necessary calculations*/
static bool propagate_nand_input(nand_t *g, unsigned int i) {
  pair_t input_val = evaluate_gate(input_in_nand(g->inputs[i]));
  if (input_val.signal == NULL_DEPENDENCY ||
      input_val.signal == CIRCULAR_DEPENDENCY) {
    g->dfs_state = input_val.signal;
  }
  g->max_critical_path_length_at_input =
      MAX(g->max_critical_path_length_at_input, input_val.critical_path_length);

  if (input_val.signal == CALCULATED_0)
    return true;
  return false;
}

/* evaluates inputs of the gate to calculate out-signal of the gate and length
 * of critical path*/
static pair_t evaluate_inputs_of_gate(nand_t *g) {
  bool exists_false_signal = false;

  for (unsigned int i = 0; i < g->n_of_inputs; i++) {
    if (input_is_empty(g->inputs[i])) {
      propagate_empty_input(g);
    } else {
      if (input_is_signal(g->inputs[i])) {
        exists_false_signal |= propagate_signal_input(g, i);
      } else {
        exists_false_signal |= propagate_nand_input(g, i);
      }
    }

    // in case there is one of these errors, function can stop immediately
    if (g->dfs_state == NULL_DEPENDENCY ||
        g->dfs_state == CIRCULAR_DEPENDENCY) {
      return pass_result_of_already_evaluated(g);
    }
  }

  g->dfs_state = (exists_false_signal) ? CALCULATED_1 : CALCULATED_0;
  return pass_result_of_already_evaluated(g);
}

static void set_init_conditions_before_input_evaluation(nand_t *g) {
  g->dfs_index = dfs_index;
  g->max_critical_path_length_at_input = 0;
  g->dfs_state = VISITED_NOT_YET_CALCULATED;
}

/* returns a pair of the form:
 * {s - evaluated out signal of gate(possible some error), l - length of
 * critical path of this gate} */
static pair_t evaluate_gate(nand_t *g) {
  if (!g) { // NULL gate
    pair_t val = {NULL_DEPENDENCY, -1};
    return val;
  }
  if (g->n_of_inputs == 0) { // special case of zero-input gate
    g->dfs_state = CALCULATED_0;
    g->max_critical_path_length_at_input = -1;
    return pass_result_of_already_evaluated(g);
  }

  if (g->dfs_index == dfs_index) { // case it was already evaluated in this dfs
    if (g->dfs_state == VISITED_NOT_YET_CALCULATED) {
      g->dfs_state = CIRCULAR_DEPENDENCY;
    }
    return pass_result_of_already_evaluated(g);
  }
  // otherwise there is need to calculate value of this gate
  set_init_conditions_before_input_evaluation(g);
  return evaluate_inputs_of_gate(g);
}

ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
  // checking whether input is valid
  if (!g || !s || m == 0 || dfs_index == ULLONG_MAX) {
    errno = EINVAL;
    return -1;
  }
  dfs_index++; // mark new dfs-call

  ssize_t critical_path = 0;
  for (unsigned int i = 0; i < m; i++) {
    if (!g[i]) { // empty gate in the given array means errno should be set to
                 // EINVAL
      errno = EINVAL;
      return -1;
    }
    pair_t result_i = evaluate_gate(g[i]);
    if (result_i.signal == NULL_DEPENDENCY ||
        result_i.signal == CIRCULAR_DEPENDENCY) {
      errno = ECANCELED;
      return -1;
    }
    s[i] = (bool)result_i.signal;
    critical_path = MAX(critical_path, result_i.critical_path_length);
  }

  return critical_path;
}

ssize_t nand_fan_out(nand_t const *g) {
  if (!g) {
    errno = EINVAL;
    return -1;
  }
  return list_size(g->out_connections);
}

void *nand_input(nand_t const *g, unsigned k) {
  if (!is_nand_and_k_valid(g, k))
    return NULL;
  if (input_is_empty(g->inputs[k])) {
    errno = 0;
    return NULL;
  }
  if (input_is_signal(g->inputs[k]))
    return (void *)input_signal(g->inputs[k]);
  return input_in_nand(g->inputs[k]);
}

nand_t *nand_output(nand_t const *g, ssize_t k) {
  // it wasn't in the question that any errno value should be set
  if (!g)
    return NULL;
  if (k < 0 || k >= nand_fan_out(g))
    return NULL;

  return list_ith_connection(g->out_connections, k);
}
