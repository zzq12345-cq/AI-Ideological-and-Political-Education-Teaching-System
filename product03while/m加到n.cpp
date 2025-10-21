#include "iostream"
using namespace std;

//输入两个正整数m和n，输出从m加到n的结果（m保证比n小，统计总和时需要包括m和n）
int main(){
    int m,n;
    cin >> m >> n;

    int sum = 0;
    while(m <= n){
        sum += m;
        m++;
    }

    cout << sum << endl;
}