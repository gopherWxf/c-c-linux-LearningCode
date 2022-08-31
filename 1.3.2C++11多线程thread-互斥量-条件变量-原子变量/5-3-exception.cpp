#include<exception>
#include<iostream>
using namespace std;

class MyException:public exception{
   public:
      const char* what()const throw(){ //throw () 表示不允许任何异常产生
           return "ERROR! Don't divide a number by integer zero.\n";
      }
};
void check(int y) throw(MyException){ //throw (MyException)表示只允许myException的异常发生
    if(y==0) throw MyException();
}

int main()
{
   int x=100,y=0;
   try{
       check(y);
       cout<<x/y;
   }catch(MyException& me){
       cout<<me.what();
       cout << "finish exception\n";
       return -1;
   }
   cout << "finish ok\n";
   return 0;
}
