#include <stdexcept>
#include <limits>
#include <iostream>

using namespace std;

void MyFunc(int c)
{
    if (c > numeric_limits< char> ::max())
        throw invalid_argument("throw MyFunc argument too large.");
    //...
}

int main()
{
//    try
    {
        MyFunc(256); //cause an exception to throw
    }
//    catch (invalid_argument& e)
//    {
//        cerr << "catch " << e.what() << endl;
//        return -1;
//    }
    //...
    cout << "end\n";
    return 0;
}
