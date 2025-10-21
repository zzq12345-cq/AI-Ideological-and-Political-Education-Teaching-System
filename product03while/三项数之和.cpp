#include "iostream"
using namespace std;

int main()
{
    int n;
    cin >> n;

    int a = 1, b = 1, c = 1;
    int i = 4;

    while (i <= n){
        int d = a + b + c; // 不能防在外面否则无法动态更新
        a = b;
        b = c;
        c = d;
        i++;
    }
    cout << c;
    return 0;
}
