#include "iostream"
using namespace std;

int main(){
    //斐波那契数列是这样一组有规律的数字：1、1、2、3、5、8、13、21、34、…。这组数字的前两项都是1，从第三项开始，每个数字都是前两个数字的和。输出这个数列第n项的值。
    int n;
    cin>>n;

    if (n <= 2){
        cout<<1;
        return 0;
    }

    int a = 1, b = 1;
    int i = 3;

    while (i <= n){
        int c = a + b;
        a = b;
        b = c;
        i++;
    }

    cout << b;
    return 0;

}