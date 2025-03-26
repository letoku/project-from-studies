#include <vector>
#include "wys.h"
#include <unordered_map>
#include <memory>


using namespace std;

int INF = 10000;
int n, k, g;

struct gameTreeNode{
  int x;  // if 0 still don't know. If positive then this is true x
  int d;  // distance - how many questions needed to determine x
  int q;  // number to ask about
  shared_ptr<gameTreeNode> false_case_node;
  shared_ptr<gameTreeNode>  true_case_node;
  shared_ptr<gameTreeNode>  min_step_node;

  gameTreeNode(int x1, int d1) : x(x1), d(d1) {
    false_case_node = nullptr;
    true_case_node = nullptr;
    min_step_node = nullptr;
    q = 0;
  };
};


int nSentencesFalse(int x, const vector<pair<int, bool>> &hist){
  int falseSentences = 0;
  for(unsigned int i = 0; i < hist.size(); i++){
    int y = hist[i].first;
    bool ans = hist[i].second;
    if(x >= y && ans) falseSentences++;
    if(x < y && !ans) falseSentences++;
  }
  return falseSentences;
}

bool isPossible(int x, const vector<pair<int, bool>> &hist){
  int falseSentences = nSentencesFalse(x, hist);
  if(falseSentences <= k) return true;
  return false;
}

struct VectorHash {
  size_t operator()(const vector<int>& s) const {
    size_t hash = 0;
    for (int elem : s) {
      hash ^= std::hash<int>()(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    return hash;
  }
};


struct VectorEqual {
  bool operator()(const vector<int>& lhs, const vector<int>& rhs) const {
    return lhs == rhs;
  }
};


vector<int> histToState(const vector<pair<int, bool>> &hist){
  vector<int> n_false(13, 0);

  for (int i = 1; i <= 12; ++i) {
    n_false[i] = min(nSentencesFalse(i, hist), k + 1);
  }

  return n_false;
}

shared_ptr<gameTreeNode> minStep(vector<pair<int, bool>> &hist);
shared_ptr<gameTreeNode> maxStep(int q, int min_bob, vector<pair<int, bool>> hist);


unordered_map<vector<int>, shared_ptr<gameTreeNode>, VectorHash, VectorEqual> mem;
shared_ptr<gameTreeNode> root;


/*
 * return 0 if can't yet determine what was x
 * return x (> 0) value of true x if it can be determined
 * return -1 if this situation is not possible(contradictory set of questions and answers)
 */
vector<int> determineX(vector<pair<int, bool>> &hist){
  vector<int> possibles;
  for (int i = 1; i <= n; i++) {
    if(isPossible(i, hist)){
      possibles.push_back(i);
    }
  }

  return possibles;
}

/*
 * Used to update index so that search will be done from the middle
 */
void jump_i(int* i, int* jump){
  (*i) += (*jump);
  (*jump) *= -1;
  if((*jump) < 0){
    (*jump)--;
  } else{
    (*jump)++;
  }
}

/*
 * Bob step
 */
shared_ptr<gameTreeNode> minStep(vector<pair<int, bool>> &hist){
  vector<int> x = determineX(hist);
  if(x.size() == 0 || x.size() == 1) {  // in both cases search ends here. If it is -1 then it will never happen in the real game
    if(x.size() == 0) x.push_back(-1);
    shared_ptr<gameTreeNode> node = make_shared<gameTreeNode> (x[0], 0);
    return node;
  }

  auto state = histToState(hist);
  if(mem.find(state) != mem.end()){
    return mem[state];
  }
  // otherwise we need to ask more questions
  vector<shared_ptr<gameTreeNode>> options;
  int min_d = INF;
  int q = 0;
  int lo = x[0];
  int hi = x[x.size() - 1];
  shared_ptr<gameTreeNode> correct_node = nullptr;

  int i = (lo + 1 + hi) / 2;  // first more promising moves will be considered
  int jump = 1;
  while (i > lo && i <= hi){
    options.push_back(maxStep(i, min_d, hist));
    int d = options[options.size() - 1]->d;
    if(d < min_d){
      min_d = d;
      q = i;
      correct_node = options[options.size() - 1];
    }
    jump_i(&i, &jump);
  }

  // so we create intermediate node, containing info about what to play and what where to move next
  shared_ptr<gameTreeNode> node = make_shared<gameTreeNode>(0, min_d + 1);
  if(q == 0){  // there is also a case when all steps down were cut due to optimizations
    node->d = INF;
  } else {
    node->min_step_node = correct_node;
    node->q = q;
  }

  mem[state] = node;
  return node;
}

/*
 * Alice step
 * q - last question (is x lower than q)
 */
shared_ptr<gameTreeNode> maxStep(int q, int min_bob, vector<pair<int, bool>> hist){
  shared_ptr<gameTreeNode> node = make_shared<gameTreeNode>(0, 0);

  // first we consider option when Alice answers negatively
  hist.push_back(pair<int, bool> {q, false});
  shared_ptr<gameTreeNode> false_case_node = minStep(hist);
  node->d = false_case_node->d;

  shared_ptr<gameTreeNode> true_case_node = nullptr;
  if(false_case_node->d < min_bob) {  // beta cut otherwise
    hist[hist.size() - 1].second = true;  // then case of positive answer
    true_case_node = minStep(hist);
    node->d = max(false_case_node->d, true_case_node->d);
  }

  node->false_case_node = false_case_node;
  node->true_case_node = true_case_node;

  return node;
}

void determineOptimalStrategy(){
  vector<pair<int, bool>> hist;
  root = minStep(hist);
}

void play(){
  shared_ptr<gameTreeNode> current = root;
  while (true) {
    if (current->x > 0) {
      odpowiedz(current->x);
      return;
    }
    bool ans = mniejszaNiz(current->q);
    current = current->min_step_node;
    if(ans){
      current = current->true_case_node;
    } else{
      current = current->false_case_node;
    }
  }

}

int main() {
  dajParametry(n, k, g);
  determineOptimalStrategy();

  while (g--) {
    play();
  }
}
