#include <iostream>
#include <forward_list>

using namespace std;

int main() {
    forward_list<int> list1 = {100, 3, 11, 4, 5};
    list1.reverse();

    cout << "After reverse: " << endl;
    for(auto& i: list1)
        cout << i << " ";
    cout << endl;
    return 0;
}
