#include "iostream"
using namespace std;

int main()
{
    int a,b,c,d;
    cin>>a>>b>>c>>d;

    if ((c <= a && d <= b) || (c <= b && d <= a)){
        cout << 1;
    } else {
        cout << 0;
    }
    return 0;
}