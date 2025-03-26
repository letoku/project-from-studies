#include <cassert>
#include <vector>
#include <iostream>

#include "prev.h"

using namespace std;

int main() {
//  std::vector<int> seq = {6, 2, 9, 1, 4, 6};
//  init(seq);
//  assert(prevInRange(5, 7, 10) == 2);
//  assert(prevInRange(5, 12, 14) == -1);
//  assert(prevInRange(5, 6, 6) == 5);
//  assert(prevInRange(0, 3, 7) == 0);
//  pushBack(3);
//  assert(prevInRange(5, 1, 3) == 3);
//  assert(prevInRange(6, 1, 3) == 6);
//  pushBack(6);
//  done();

  int n, m;
  cin >> n >> m;
  vector<int> seq(n);
  for(int i = 0; i < n; i++){
    cin >> seq[i];
  }
  init(seq);
  int c;
  for(int i = 0; i < m; i++){
    cin >> c;
    if(c == 0){ // new number in seq
      int b;
      cin >> b;
      pushBack(b);
    } else{
      int j, lo, hi;
      cin >> j >> lo >> hi;
      int ans = prevInRange(j, lo, hi);
      cout << ans << "\n";
    }
  }
  done();
}