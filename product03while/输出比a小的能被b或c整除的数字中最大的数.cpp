#include "iostream"
using namespace std;

int main(){
    int a,b,c;
    cin >> a >> b >> c;

    int num = a - 1;
    while(num >= 1){
        if (num % b == 0 || num % c == 0){
            cout << num;
            return 0;
        }
        num--;
    }
}