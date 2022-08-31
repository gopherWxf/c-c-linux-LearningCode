//3-1-variable-parameter2 递归函数方式展开参数包
#include <iostream>

using namespace std;

//递归终止函数
void print()
{
   cout << "empty" << endl;
}
//template <class T>
//void print(T t)
//{
//   cout << t << endl;
//}
//展开函数
template <class T, class ...Args>
void print(T head, Args... rest)
{
   cout << "parameter " << head << endl;
   print(rest...);
}


int main(void)
{
   print(1,2,3,"darren", "youzi");
   return 0;
}

//template<typename T>
//T sum(T t)
//{
//    return t;
//}
//template<typename T, typename ... Types>
//T sum (T first, Types ... rest)
//{
//    return first + sum<T>(rest...);
//}
//sum(1,2,3,4); //10
