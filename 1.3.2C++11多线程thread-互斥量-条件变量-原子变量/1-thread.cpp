#include<thread>
#include <iostream>
using namespace std;

void threadFun(int &a) // 引用传递
{
  cout << "this is thread fun !" <<endl;
  cout <<" a = "<<(a+=10)<<endl;
}
int main()
{
  int x = 10;
  thread t1(threadFun, std::ref(x));
  thread t2(std::move(t1)); // t1 线程失去所有权
  thread t3;
  t3 = std::move(t2); // t2 线程失去所有权
  //t1.join(); // ？
  t3.join();
  cout<<"Main End "<<"x = "<<x<<endl;
  return 0;
}
