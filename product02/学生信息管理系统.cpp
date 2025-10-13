#include "iostream"

using namespace std;




struct Student{

    string id;

    string name;

    string gender;

    double score;

};



class StudentList {

private:

    Student data[10];

    int length;

public:

    StudentList() {

        length = 0;

    }



// 创建顺序表

    void create() {

        cout << "=============================================" << endl;

        cout << "              学生信息线性表的建立              " << endl;

        cout << "=============================================" << endl;

        cout << "有几位学生？请输入：" << endl;

        cin >> length;

        if (length > 10) throw "输入的数组长度超出范围";

        for (int i = 0; i < length; ++i) {
            cout << "请输入第" << i + 1 << "位学生的信息：" << endl;
            cout << "学号：";
            cin >> data[i].id;
            cout << "姓名：";
            cin >> data[i].name;
            cout << "性别：";
            cin >> data[i].gender;
            cout << "成绩：";
            cin >> data[i].score;
        }
    }

// 删除指定位置的学生

    Student Delete(int i) {

        Student x;

        if (length == 0) throw "下溢";

        if (i < 1 || i > length) throw "删除位置错误";

        x = data[i - 1];

        for (int j = i; j < length; ++j) {

            data[j - 1] = data[j];

        }

        length--;

        return x;

    }



// 在指定位置插入学生

    void Insert(int i , Student x) {

        if (length >= 10) throw "上溢";

        if (i < 1 || i > length + 1) throw "插入位置错误";

        for (int j = length; j > i - 1 ; j--)

        {

            data[j] = data[j - 1];

        }

        data[i - 1] = x; // 插入

        length++;

        cout << "插入成功" << endl;

    }

    // 增加逆置功能
    void Invert()
    {
        // 如果列表只有一个元素无需逆置
        if (length < 2)
        {
            return;
        }

        // 从头尾向中间运行
        for (int i = 0; i < length / 2; ++i) {
            int j = length - 1 - i;

            Student temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }

    // 修改指定位置的学生信息
    void  Modify(int i, Student s)
    {
        if (i < 1 || i > length) throw "修改位置错误";
        data[i - 1] = s;
    }


// 获取指定位置的学生对象

   Student   Get(int i) {

        if (i < 1 || i > length) throw "获取位置错误";

        else return data[i - 1];

    }



// 打印所有学生信息

    void Printlist() {

        cout << "\n=================所有学生信息===================" << endl;

        for (int i = 0; i < length; ++i) {

            cout << " 学号：" << data[i].id << endl;
            cout << " 姓名：" << data[i].name << endl;
            cout << " 性别：" << data[i].gender << endl;
            cout << " 成绩：" << data[i].score << endl;
            cout << "===================================================" << endl;
        }
    }



// 按学号查找

    int LocateById(string id) {

        for (int i = 0; i < length; ++i) {

            if (data[i].id == id)

                return i + 1;

        }

        return 0;

    }



// 按姓名查找

    int LocateByName(string name) {

        for (int i = 0; i < length; ++i) {

            if (data[i].name == name)

                return i + 1;

        }

        return 0;

    }



// 获取长度

    int Length() {

        return length;

    }

};

//菜单

void MenuList()

{

    cout << "\n================学生信息管理系统==================" << endl;

    cout << "1.学生信息线性表的建立" << endl;

    cout << "2.插入学生信息" << endl;

    cout << "3.查询学生信息" << endl;

    cout << "4.删除学生信息" << endl;

    cout << "5.输出所有学生信息" << endl;

    cout << "6.逆置学生信息线性表" << endl;

    cout << "7.修改学生信息" << endl;

    cout << "0.退出系统" << endl;

    cout << "===================================================" << endl;

    cout << "请选择0~7：" ;

}



int main() {

    StudentList list;

    int choice;


    while (true) {

        MenuList();

        cin >> choice;

        try {

            if (choice == 1) {

                list.create();

                cout << "学生信息线性表创建成功" << endl;

            } else if (choice == 2) {

                int position;

                cout << "请输入插入位置：";

                cin >> position;

                cout << "请输入要插入的学生信息：" << endl;
                Student x;
                cout << "学号：" ;
                cin >> x.id;
                cout << "姓名：" ;
                cin >> x.name;
                cout << "性别：" ;
                cin >> x.gender;
                cout << "成绩：" ;
                cin >> x.score;
                list.Insert(position, x);

            } else if (choice == 3) {


                cout << "请输入要查询的方式：" << endl;

                cout << "1.按学号查询" << endl;

                cout << "2.按姓名查询" << endl;

                int a;

                cin >> a;

                if (a == 1) {

                    string id;

                    cout << "请输入要查询的学号：";

                    cin >> id;

                    int position = list.LocateById(id);

                    if (position == 0) {

                        cout << "未找到该学生" << endl;

                    } else {

                        Student s = list.Get(position);

                        cout << "学号：" << s.id << " 姓名：" << s.name << " 性别：" << s.gender << " 成绩：" << s.score

                             << endl;

                    }

                } else if (a == 2) {

                    string name;

                    cout << "请输入要查询的姓名：";

                    cin >> name;

                    int position = list.LocateByName(name);

                    if (position == 0) {

                        cout << "未找到该学生" << endl;

                    } else {

                        Student s = list.Get(position);

                        cout << "学号：" << s.id << " 姓名：" << s.name << " 性别：" << s.gender << " 成绩：" << s.score

                             << endl;

                    }

                }

            } else if (choice == 4) {
                cout <<"请选择删除记录的方式：" << endl;
                cout << "1.按学号删除" << endl;
                cout << "2.按姓名删除" << endl;
                int delete_choice;
                cin >> delete_choice;
                if (delete_choice == 1)
                {
                    string id_todelete;
                    cout << "请输入要删除的学生学号：";
                    cin >> id_todelete;

                    // 查找
                    int position = list.LocateById(id_todelete);
                    // 根据查找结果进行操作
                    if (position == 0)
                    {
                        cout << "未找到该学生" << endl;
                    } else {
                       Student s = list.Delete(position);
                        cout << "删除成功!被删除的学生是："  << s.name << "学号：" << s.id  << ")" << endl;
                    }
                }
                else if (delete_choice == 2)
                {
                    string name_todelete;
                    cout << "请输入要删除的学生姓名：";
                    cin >> name_todelete;

                    // 查找
                    int position = list.LocateByName(name_todelete);
                    // 根据查找结果进行操作
                    if (position == 0)
                    {
                        cout << "未找到该学生" << endl;
                    } else {
                        Student s = list.Delete(position);
                        cout << "删除成功!被删除的学生是："  << s.name << "学号：" << s.id  << ")" << endl;
                    }
                }
            } else if (choice == 5) {
                list.Printlist();

            } else if (choice == 6)
            {
                list.Invert();
                cout << "逆置成功" << endl;
                list.Printlist();
            } else if (choice == 7) {
                string id_tomodify;
                cout << "请输入要修改的学生学号：";
                cin >> id_tomodify;

                // 先查找
                int position = list.LocateById(id_tomodify);
                if (position == 0) {
                    cout << "未找到该学生" << endl;
                } else {
                    // 获取当前学生的信息并输出
                    Student s = list.Get(position);
                    cout << "学号：" << s.id << " 姓名：" << s.name << " 性别：" << s.gender << " 成绩：" << s.score
                         << endl;

                    // 提供需要修改的内容
                    cout << "请选择要修改的内容" << endl;
                    cout << "1.修改姓名" << endl;
                    cout << "2.修改成绩" << endl;
                    cout << "3.以上都需要修改" << endl;
                    int modify_choice;
                    cin >> modify_choice;

                    // 根据选择进行修改
                    if (modify_choice == 1) {
                        cout << "请输入新的姓名：";
                        cin >> s.name;
                    } else if (modify_choice == 2) {
                        cout << "请输入新的成绩：";
                        cin >> s.score;
                    } else if (modify_choice == 3) {
                        cout << "请输入新的姓名：";
                        cin >> s.name;
                        cout << "请输入新的成绩：";
                        cin >> s.score;
                    } else {
                        cout << "输入错误" << endl;
                        continue;
                    }
                    // 调用修改函数返回修改后的学生信息
                    list.Modify(position, s);
                    cout << "修改成功" << endl;
                }
            }
            else if (choice == 0) {

                cout << "退出系统" << endl;

                break;

            }

        } catch (const char *e) {

            cout << e << endl;

        }

    }
    return 0;
}