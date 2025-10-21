#include "iostream"
using namespace std;

/**
 * 某程序员开始工作，年薪a万，他希望在厦门买房子，现在价格是200万，假设房子价格以每年百分之b的速度增长，并且该程序员未来年薪不变，且不吃不喝，不用交税，每年所得a万全都积攒起来，问是否能在20年之内买下这套房子？如果可以，那么第几年能够买下这套房子？（第一年年薪a万，房价200万）

输入描述
一个正整数n，表示案例的数量。
每组案例由两个正整数a和b组成（10<=a<=50, 1<=b<=20)
输出描述
如果在第20年或者之前就能买下这套房子，则输出一个整数M，表示最早需要在第M年能买下，否则输出No。
每组案例输出完都要换行。
样例输入
1
50 10
样例输出
8
 * @return
 */
int main(){
    int n;
    cin >> n;

    while (n--)
    {
        float a,b; // 年薪 a, 房价 b
        cin >> a >> b;

        float housePrice = 200; // 初始化房价
        int year = 0; // 初始化年份
        int totalsavings = a; /// 初始化累计saving

        // 循环
        while (year <= 20)
        {
            year++;
            // 计算房价
            housePrice = housePrice * (1 + b / 100);
            // 计算年薪累计
            totalsavings += a;
            // 判断是否可以购买
            if (totalsavings >= housePrice){
                cout << year + 1 << endl;
                break;
            }
        }

        // 判断是否超过20年
        if (year > 20) {
            cout << "No" << endl;
        }
    }
    return 0;
}