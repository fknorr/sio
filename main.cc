#include "writer.hh"
#include <chrono>
#include <sstream>
#include <iostream>



using stclk = std::chrono::steady_clock;


[[gnu::noinline]] void first() {
    std::string str;
    string_writer sw(str);

    for (std::size_t i = 0; i < 1000000; ++i) {
        sw << 123 << nl;
    }
}

[[gnu::noinline]] void second() {
    std::ostringstream oss;
    for (std::size_t i = 0; i < 1000000; ++i) {
        oss << 123 << "\n";
    }
    oss.str();
}


int main(void) {
    auto now = stclk::now();
    first();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stclk::now() - now).count() << "\n";


    now = stclk::now();
    second();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stclk::now() - now).count() << "\n";
}
