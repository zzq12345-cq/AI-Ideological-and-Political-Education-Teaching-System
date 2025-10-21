#include "iostream"
using namespace std;

int main(){
    int n;
    cin >> n;

    int result = 1;
    int day = n - 1;

    while (day >= 1){
        result = result * 2 + 1;
        day--;
    }

    cout << result;
    return 0;
}