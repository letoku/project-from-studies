#include <limits.h>

#include <memory>
#include <vector>

using namespace std;

unsigned int L = 0;
unsigned int R = UINT_MAX;

unsigned int convertToUINT(int x) {
  return static_cast<unsigned int>((long long int)x - INT_MIN);
}

struct Node {
  unsigned int val;
  unsigned int x, y;
  shared_ptr<Node> left;
  shared_ptr<Node> right;

  Node(unsigned int v, unsigned int x1, unsigned int y1)
      : val(v), x(x1), y(y1){};
};

vector<shared_ptr<Node>> roots;
bool found;

void createBranch(unsigned int val, shared_ptr<Node> current,
                  shared_ptr<Node> prev) {
  if (current->x == current->y) return;
  unsigned int mid =
      (unsigned int)(((long long int)current->x + (long long int)current->y) / 2);

  if (val <= mid) {  // we go with new branch creating to the left
    shared_ptr<Node> left = make_shared<Node>(current->val, current->x, mid);
    current->left = left;
    if (prev != nullptr) {
      createBranch(val, left, prev->left);
      current->right = prev->right;
    } else {
      createBranch(val, left, nullptr);
      current->right = nullptr;
    }
  } else {  // we go with new branch creating to the right
    shared_ptr<Node> right =
        make_shared<Node>(current->val, mid + 1, current->y);
    current->right = right;
    if (prev != nullptr) {
      createBranch(val, right, prev->right);
      current->left = prev->left;
    } else {
      createBranch(val, right, nullptr);
      current->left = nullptr;
    }
  }
}

void pushBack(int value) {
  unsigned int x = convertToUINT(value);
  shared_ptr<Node> root = make_shared<Node>(roots.size(), L, R);
  shared_ptr<Node> prev = (roots.size() > 0) ? roots[roots.size() - 1] : nullptr;
  createBranch(x, root, prev);
  roots.push_back(root);
}

void init(const vector<int> &seq) {
  int n = (int) seq.size();
  for (int i = 0; i < n; i++) {
    pushBack(seq[i]);
  }
}

unsigned int search(unsigned int lo, unsigned int hi, shared_ptr<Node> node) {
  if (node == nullptr) return 0;
  if (node->x > hi || node->y < lo) return 0;

  if (node->x >= lo && node->y <= hi) {
    found = true;
    return node->val;
  }

  return max(search(lo, hi, node->left), search(lo, hi, node->right));
}

int prevInRange(int i, int lo, int hi) {
  unsigned int lo2 = convertToUINT(lo);
  unsigned int hi2 = convertToUINT(hi);
  found = false;
  unsigned int ans = search(lo2, hi2, roots[i]);
  if (found) return (int)ans;
  return -1;
}

void done() { roots.clear(); }
