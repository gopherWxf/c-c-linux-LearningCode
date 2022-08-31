#include <functional>
#include <queue>
#include <vector>
#include <iostream>

template<typename T>
void print_queue(T &q) {
    while (!q.empty()) {
        std::cout << q.top() << " ";
        q.pop();
    }
    std::cout << '\n';
}

int main() {
    std::priority_queue<int> q;

    for (int n: {1, 8, 5, 6, 3, 4, 0, 9, 7, 2})
        q.push(n);

    print_queue(q);

    std::priority_queue<int, std::vector<int>, std::greater<int> > q2;

    for (int n: {1, 8, 5, 6, 3, 4, 0, 9, 7, 2})
        q2.push(n);

    print_queue(q2);

    // 用 lambda 比较元素。如果是自定的对象插入priority_queue，要自己写比较函数
//    auto cmp = [](int left, int right) { return (left) < (right); };  // 从大到小排序
    auto cmp = [](int left, int right) { return (left) > (right); };  // 从小到大排序
    std::priority_queue<int, std::vector<int>, decltype(cmp)> q3(cmp);

    for (int n: {1, 8, 5, 6, 3, 4, 0, 9, 7, 2})
        q3.push(n);

    print_queue(q3);

}
