//5-5-nested_exception  nested_exception example
#include <iostream>       // std::cerr
#include <exception>      // std::exception, std::throw_with_nested, std::rethrow_if_nested
#include <stdexcept>      // std::logic_error

// recursively print exception whats:
void print_what (const std::exception& e)
{
    std::cout << __FUNCTION__ << ", L"<< __LINE__ << ", what:" << e.what() << '\n';
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& nested) {
        std::cerr << "nested: ";
        print_what(nested);
    }
}

// throws an exception nested in another:
void throw_nested()
{
    try {
        throw std::logic_error ("first");
    } catch (const std::exception& e) {
        std::throw_with_nested(std::logic_error("second"));
    }
}

int main ()
{
    try {
        std::cout << __FUNCTION__ << ", L"<< __LINE__ << std::endl;
        throw_nested();
    } catch (std::exception& e) {
        std::cout << __FUNCTION__ << ", L"<< __LINE__ << std::endl;
        print_what(e);
    }

    return 0;
}
