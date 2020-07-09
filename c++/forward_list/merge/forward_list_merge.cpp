#include <iostream>
#include <forward_list>

using namespace std;

int main() {
    forward_list<int> list1 = {100, 3, 11, 4, 5};
    forward_list<int> list2 = {7, 8, 9, 10};

    list1.merge(list2);
    cout << "After merge: " << endl;
    for(auto& i: list1)
        cout << i << " ";
    cout << endl;
    return 0;
}
