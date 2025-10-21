#include "iostream"
using namespace std;

//有一个数列是这样的规律：1、2、4、8、16、…。这组数字的前一项是1，后面每一项都是前一项的两倍。求该数列中首个比1000大的数字。
int main(){
    int i = 1;
    while (true){
        i = i * 2;
        if (i > 1000)
            break;
    }
    cout << i << endl;
}