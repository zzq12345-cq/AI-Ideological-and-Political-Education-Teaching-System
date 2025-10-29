#include "iostream"
using namespace std;
/*
 * 问题描述
判断3个数字是否只存在某两个数字是相等的（不包含三个数字都相等的情况）
输入描述
一个正整数n，表示有n组案例。
在每组案例中有三个整数a、b、c，表示要判断的三个数。
输出描述
针对每组案例，如果a、b、c中有某两个数字是相等的（不包含三个数都相等的情况），则输出Yes，否则输出No。每组案例输出完都要换行。
样例输入
3
10 10 5
5 6 7
5 5 5
样例输出
Yes
No
No
 */

int main()
{
    int n;
    cin >> n;
    for (int i = 0; i < n; ++i) {
        int a,b,c;
        cin >> a >> b >> c;
        if (a == b && b == c){
            cout << "No" << endl;
        } else if (a == b || a == c || b == c){
            cout << "Yes" << endl;
        } else {
            cout << "No" << endl;
        }
    }
    return 0;
}