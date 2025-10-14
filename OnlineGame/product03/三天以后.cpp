#include "iostream"
using namespace std;

int main()
{
    // 求三天以后是哪天
    int y,m,d;
    cin >> y >> m >> d;

     d = d + 3; //  计算三天后的日期
     // 使用for循环处理日期超出
    for (int i = 0; i < 3; ++i) {
        int day;
        if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12)
        {
            day = 31;
        } else if (m == 4 || m == 6 || m == 9 || m == 11)
        {
            day = 30;
        } else if (m == 2)
        {
            if (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0) && y % 3200 != 0)
            {
                day = 29;
            } else
            {
                day = 28;
            }
        }
        if (d <= day){
            break;
        } else {
            d -= day;
            m += 1;
        }
        if (m > 12){
            m = 1;
            y += 1;
        }
    }
    cout << y << " " << m << " " << d << endl;
}