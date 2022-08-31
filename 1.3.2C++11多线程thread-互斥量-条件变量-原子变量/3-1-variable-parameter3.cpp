//3-1-variable-parameter3 逗号表达式展开参数包
#include <iostream>

using namespace std;

template <class T>
void printarg(T t)
{
    cout << t << endl;
}

template <class ...Args>
void expand(Args... args)
{
    int arr[] = {(printarg(args), 0)...};
}

int main()
{
    expand(1,2,3,4);
    return 0;
}
