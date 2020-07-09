#include<iostream>
using namespace std;

int main(){
        int a1,a2,a3,a4,a5;

        int *b1=new int;
        int *b2=new int;
        int *b3=new int;    
        int *b4=new int;    

        cout << "address of a1 is " << &a1 << endl;
        cout << "address of a2 is " << &a2 << endl;
        cout << "address of a3 is " << &a3 << endl;
        cout << "address of a4 is " << &a4 << endl;
        cout << "address of a5 is " << &a5 << endl;

        cout << "address of b1 is " << &b1 << ", value is " << b1 << endl;
        cout << "address of b2 is " << &b2 << ", value is " << b2 << endl;
        cout << "address of b3 is " << &b3 << ", value is " << b3 << endl;
        cout << "address of b4 is " << &b4 << ", value is " << b4 << endl;
        delete b1,b2,b3,b4;
}
