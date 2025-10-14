#include "iostream"
using namespace std;

int main()
{
    // 输入一个正整数n，输出1到n中所有能被4整除的数字的和
    int n;
    cin >> n;
    int sum = 0;
    for (int i = 1; i <= n; ++i) {
        if (i % 4 == 0){
            sum += i;
        }
    }
    cout << sum ;
    return 0;
}