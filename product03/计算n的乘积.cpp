#include "iostream"
using namespace std;

int main()
{
    int  n,num;
    int product = 1;
    cin>>n;
    for (int i = 0; i < n; ++i) {
        cin>>num;
        product = product * num;
    }
    cout<< product <<endl;
    return 0;
}