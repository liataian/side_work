#include <iostream>
#include <vector>
using namespace std;

int main(){    
        vector<int> myvec(5); //A vector including 5 integers
        int *p = myvec.data();
        *p = 10; 
        ++p;
        *p = 20; 
        p[2] = 100;

        std::cout << "myvec contains:";
        for (unsigned i=0; i<myvec.size(); ++i)
                std::cout << ' ' << myvec[i];
        std::cout << '\n';
        return 0;
}
