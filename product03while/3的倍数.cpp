#include "iostream"
using namespace std;

int main(){
    int n;
    cin >> n;

    int count0 = 0, count1 = 0, count2=0, count3=0, count4=0;

    for (int i = 0; i < n; i++) {
        int num;
        cin >> num;
        int result = num % 3;
        if (result == 0){
            count0++;
        } else if (result == 1){
            count1++;
        } else if (result == 2){
            count2++;
        } else if (result == -1) {
            count3++;
        } else if (result == -2) {
            count4++;
        }
    }
    if (count0 >= 2 || (count1 >= 1 && count2 >= 1 ) || (count3 >= 1 && count4 >= 1)) {
        cout << "Yes";
    } else {
        cout << "No";
    }
    return 0;
}