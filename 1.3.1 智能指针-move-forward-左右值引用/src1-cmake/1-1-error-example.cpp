#include <iostream>
#include <memory>

using namespace std;

int main() {
    cout << "Hello World!" << endl;
    {
        int *ptr = new int;
        shared_ptr<int> p1(ptr);
        shared_ptr<int> p2(ptr); // 逻辑错误
    }
    function(shared_ptr<int>(new int), g()); //有缺陷

    cout << "finish" << endl;
    return 0;
}
