#include <iostream>
#include <forward_list>

using namespace std;

int main() {
    forward_list<int> list1 = {100, 3, 11, 4, 5};
    forward_list<int> list2 = {100, 111, 112, 115, 231};
    cout << "[Before] Length of list1 is " << distance(list1.begin(), list1.end()) \
		<< ", Length of list2 is " << distance(list2.begin(), list2.end()) << endl;

    cout << "list1: ";
    for(auto& i: list1)
        cout << i << " ";
    cout << endl;

    cout << "list2: ";
    for(auto& i: list2)
        cout << i << " ";
    cout << endl;

    list1.splice_after(list1.begin(), list2);
    cout << "After splice_after: " << endl;

    for(auto& i: list1)
        cout << i << " ";
    cout << endl;

    cout << "[After] Length of list1 is " << distance(list1.begin(), list1.end()) \
		<< ", Length of list2 is " << distance(list2.begin(), list2.end()) << endl;
    return 0;
}
