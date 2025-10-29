#include "iostream"
using namespace std;

/*
 * 问题描述
判断一个整数x是不是在a~b之间（包含a和b）
输入描述
一个正整数n，表示有n组案例。
在每组案例中有三个整数x、a、b，其中x表示要判断的数字，a和b表示范围。
输出描述
针对每组案例，如果x在a~b之间，则输出Yes，否则输出No。每组案例输出完都要换行。
样例输入
2
6 5 100
6 10 20
样例输出
Yes
No

 */
int main()
{
    int n;
    cin >> n;
    while (n--)
    {
        int x,a,b;
        cin >> x >> a >> b;
        if (x >= a && x <= b){
            cout << "Yes" << endl;
        } else {
            cout << "No" << endl;
        }
    }
    return 0;
}