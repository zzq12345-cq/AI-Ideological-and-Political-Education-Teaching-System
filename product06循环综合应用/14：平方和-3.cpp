#include "iostream"
#include "cmath"
using namespace std;

/*
 * 问题描述
已知一个正整数x，问x能否凑成三个互不相同的正整数的平方和。
输入描述
一个正整数n，表示测试案例的数量。
每组案例由一个正整数x组成（x不大于1e+8）。
输出描述
针对每组案例，如果x可以表示成三个互不相同的正整数的平方和，那么输出Yes，否则输出No。
每组案例输出完都要换行。
样例输入
2
30
10
样例输出
Yes
No
提示说明
30=1*1+2*2+5*5
 */
int main()
{
    int n;
    cin >> n;

    while (n--)
    {
        long long int x;
        cin >> x;

        // 如果x< 14 最小的3个正整数相加 = 14
        if (x < 14)
        {
            cout << "No" << endl;
        } else
        {
            bool found = false;
            for (long long int i = 1;  i * i < x && !found ; ++i) {
                for (long long int j= i + 1; j * j + i * i < x; ++j) {
                    long long int k= sqrt(x - (i * i + j * j));
                    // 检查k是否是整数且不等于i和j
                    if (k * k == x - (i * i + j * j) && k!=i && k!=j && i!=j) {
                        found = true;
                    }
                }
            }
            if ( found){
                cout << "Yes" << endl;
            } else {
                cout << "No" << endl;
            }
        }
    }
    return 0;
}