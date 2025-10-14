#include "iostream"
using namespace std;

int main()
{
    int  m,n,p;
    cin >> m >> n >> p;

    if ((p < m && p > n) || (p < n && p > m)){
        cout << 1;
    } else {
        cout << 0;
    }
    return 0;
}