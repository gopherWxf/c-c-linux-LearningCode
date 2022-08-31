#include <iostream>
#include <memory>
#include <thread>

using namespace std;

std::weak_ptr<int> gw;

void f() {
    auto spt = gw.lock(); //
    if (gw.expired()) {
        cout << "gw Invalid, resource released";
    }
    else {
        cout << "gw Valid, *spt = " << *spt << endl;
    }
}


int main1() {
    {
        auto sp = std::make_shared<int>(42);
        gw = sp;
        f();
    }
    f();
    return 0;
}

void f2() {
    cout << "lock\n";
    auto spt = gw.lock();  // 锁好资源再去判断是否有效
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (gw.expired()) {
        cout << "gw Invalid, resource released\n";
    }
    else {
        cout << "gw Valid, *spt = " << *spt << endl;
    }
}

int main() {



    {
        auto sp = std::make_shared<int>(42);
        gw = sp;
        std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            cout << "sp reset\n";
            sp.reset();
        }).detach();

        f2();
    }
    f2();
    return 0;
}
