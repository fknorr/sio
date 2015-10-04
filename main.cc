#include "string.hh"
#include <chrono>
#include <sstream>
#include <iostream>


using stclk = std::chrono::steady_clock;


[[gnu::noinline]] void first() {
    std::string str;
    fio::string_writer sw(str);

    for (std::size_t i = 0; i < 1000000; ++i) {
        sw << int(i) << " ";
    }
    sw << fio::nl;
    sw.flush();
    //std::cout << str;
}

[[gnu::noinline]] void second() {
    std::ostringstream oss;
    for (std::size_t i = 0; i < 1000000; ++i) {
        oss << int(i) << " ";
    }
    oss << "\n";
    //std::cout << oss.str();
}


int main(void) {
    auto now = stclk::now();
    first();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stclk::now() - now).count() << "\n";


    now = stclk::now();
    second();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stclk::now() - now).count() << "\n";
}
