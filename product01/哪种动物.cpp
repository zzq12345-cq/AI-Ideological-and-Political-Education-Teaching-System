#include "iostream"
using namespace std;

// 涂涂养了5种动物：狮子、鹰、蚂蚁、蜘蛛、蜈蚣。
//蜈蚣的腿的数量是奇数对，通常在15到191对之间‌。其他动物的腿的数量，养过的同学应该都知道。（都不建议在学校里养）
//现在已知某一只动物的腿的数量，问这只动物是上述5种中的哪一种？
int main()
{
    // 一个正整数，表示某一只动物的腿的数量。
    int legs;
    cin >> legs;

    if (legs == 4)
    {
        cout << "Lion";
    } else if (legs == 2)
    {
        cout << "Eagle";
    } else if (legs == 8)
    {
        cout << "Spider";
    } else if (legs == 6)
    {
        cout << "Ant";
        // 奇数判断：n / 2 % 2 不等于 0
    } else if (legs >= 30 && legs <= 382 && legs % 2 == 0 && legs / 2 % 2 != 0)
    {
        cout << "Centipede";
    } else
    {
        cout << "None";
    }
    return 0;
}