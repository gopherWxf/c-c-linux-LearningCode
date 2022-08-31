//3-1-variable-parameter4
#include <iostream>

using namespace std;
template<class F, class... Args>void expand(const F& f, Args&&...args)
{
  //这里用到了完美转发
  initializer_list<int>{(f(std::forward< Args>(args)),0)...};
}
int main()
{
    expand([](int i){cout<<i<<endl;}, 1,2,3);
    return 0;
}
