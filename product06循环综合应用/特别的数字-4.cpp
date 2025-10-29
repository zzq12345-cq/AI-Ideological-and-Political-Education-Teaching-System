#include "iostream"
#include "cmath"
using namespace std;

/*
 * 问题描述
判断3个数字是否都相等
输入描述
一个正整数n，表示有n组案例。
在每组案例中有三个整数a、b、c，表示要判断的三个数。
输出描述
针对每组案例，如果a、b、c都相等，则输出Yes，否则输出No。每组案例输出完都要换行。
样例输入
2
5 5 5
5 6 7
样例输出
Yes
No
 */

int main()
{
    int n;
    cin >> n;
    while(n--)
    {
        int a,b,c;
        cin >> a >> b >> c;
        if (a == b && b == c)
        {
            cout << "Yes" << endl;
        } else
        {
            cout << "No" << endl;
        }
    }
    return 0;
}