#include "iostream"
using namespace std;

//输入一个正整数n，然后输入n个整数，输出这n个整数中大于10的数字的个数和总和
int main(){
   int n;
   cin>>n;

   int count = 0; // 计数器
   int sum = 0;
   int i = 0;
   while(i < n){
       int num;
       cin>>num;

       if (num > 10){
           count++;
           sum += num;
       }
       i++;
   }
   cout << count << " " << sum;
   return 0;
}