#include <iostream>
#include "strqueue.h"

using namespace std;


int main() {
    auto id = cxx::strqueue_new();
    size_t position = 0;
    ::cxx::strqueue_insert_at(id, position, "napis");
    auto s = ::cxx::strqueue_size(id);

    const char* str = ::cxx::strqueue_get_at(id, position);
    cout << id << endl;
    cout << *str << endl;

}