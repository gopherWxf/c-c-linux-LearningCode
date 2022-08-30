#include <iostream>
#include <string>
#include <unordered_map>
#include <map>

void unordered_map_test() {
    std::cout << "unordered_map_test\n";
    // 创建三个 string 的 unordered_map （映射到 string ）
    std::unordered_map<std::string, std::string> u = {
            {"2RED",   "#FF0000"},
            {"1GREEN", "#00FF00"},
            {"3BLUE",  "#0000FF"},
            {"a",      "#0000FF"},
            {"b",      "#0000FF"}
    };

    // 迭代并打印 unordered_map 的关键和值
    for (const auto &n: u) {
        std::cout << "Key:[" << n.first << "] Value:[" << n.second << "]\n";
    }

    // 添加新入口到 unordered_map
    u["BLACK"] = "#000000";
    u["WHITE"] = "#FFFFFF";

    // 用关键输出值
    std::cout << "The HEX of color RED is:[" << u["RED"] << "]\n";
    std::cout << "The HEX of color BLACK is:[" << u["BLACK"] << "]\n";
}

void ordered_map_test() {
    std::cout << "\nordered_map_test\n";
    // 创建三个 string 的 unordered_map （映射到 string ）
    std::map<std::string, std::string> u = {
            {"2RED",   "#FF0000"},
            {"1GREEN", "#00FF00"},
            {"3BLUE",  "#0000FF"},
            {"a",      "#0000FF"},
            {"b",      "#0000FF"}
    };

    // 迭代并打印 unordered_map 的关键和值
    for (const auto &n: u) {
        std::cout << "Key:[" << n.first << "] Value:[" << n.second << "]\n";
    }

    // 添加新入口到 unordered_map
    u["BLACK"] = "#000000";
    u["WHITE"] = "#FFFFFF";

    // 用关键输出值
    std::cout << "The HEX of color RED is:[" << u["RED"] << "]\n";
    std::cout << "The HEX of color BLACK is:[" << u["BLACK"] << "]\n";
}

int main() {
    unordered_map_test();
    ordered_map_test();
    return 0;
}
