#include <iostream>
#include <forward_list>

using namespace std;

int main() {
    forward_list<int> list_test = {2, 3, 4, 5};
    list_test.insert_after(list_test.begin(), 9);
    for(auto& i: list_test)
        cout << i << endl;
    return 0;
}
