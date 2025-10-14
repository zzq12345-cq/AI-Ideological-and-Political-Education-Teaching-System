#include "iostream"
using namespace std;

int main(){
    // 输入年月日，输出从该月开始连续三个月每个月的天数。
    int y,m,d,days;
    cin>>y>>m>>d;

    for(int i=0;i<3;i++){
        if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12)
        {
            days = 31;
        }
        else if (m == 4 || m == 6 || m == 9 || m == 11)
        {
            cout<<"30" << " ";
        }
         else if (m == 2)
        {
             // 闰年判断
             if (y % 4 == 0 && y % 100 != 0 || y % 400 == 0 &&  y % 3200 != 0)
            {
                cout<<"29" << " ";
            }
            else
             {
                cout<<"28" << " ";
             }
        }
         m++;
         if (m > 12)
         {
             m = 1;
             y ++;
         }
    }
    return 0;
    }
