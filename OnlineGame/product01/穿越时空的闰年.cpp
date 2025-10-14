#include "iostream"
#include "cmath"
using namespace std;

int main()
{
    // 一个历史学家在研究两个特定年份之间的历史事件。请你编写一个程序，计算从a年到b年（包含a年和b年本身）之间，一共经过了多少个闰年。
    int a,b,y1,y2,y;
    cin>>a>>b;

    if (a > 0 && b > 0)
    {
        y1 = (a - 1) / 4 - (a - 1) / 100 + (a - 1) / 400;
        y2 = b / 4 - b / 100 + b / 400;
        y = y2 - y1;
    }

    else if (a < 0 && b < 0)
    {
        a = abs(a);
        b = abs(b);
        y1 = a / 4 - a / 100 + a / 400;
        y2 = (b - 1) / 4 - (b - 1) / 100 + (b - 1) / 400;
        y = y1 - y2;
    }

    else
    {
       a = abs(a);
       y1 = a / 4 - a / 100 + a / 400;
        y2 = b / 4 - b / 100 + b / 400;
        y = y1 + y2;
    }
    cout <<y <<endl;
}