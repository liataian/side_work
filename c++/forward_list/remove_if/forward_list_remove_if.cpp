#include <iostream>
#include <forward_list>

using namespace std;

int main() {
    forward_list<int> list1 = {100, 3, 11, 4, 5};
    cout << "Before remove_if: " << endl;
    for(auto& i: list1)
        cout << i << " ";
    cout << endl;

    list1.remove_if([] (int n) {
        return n >= 11;
    });

    cout << "After remove_if: " << endl;
    for(auto& i: list1)
        cout << i << " ";
    cout << endl;
    return 0;
}
