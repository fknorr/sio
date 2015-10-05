#include "writer.hh"
#include <chrono>
#include <sstream>
#include <iostream>
#include "bitfield.hh"
#include "compat.hh"


using namespace sio::ops;

using stclk = std::chrono::steady_clock;


[[gnu::noinline]] void first() {
    std::string str;
    sio::string_writer sw(str);

    for (std::size_t i = 0; i < 1000000; ++i) {
        sw << sio::format(i, sio::fmt::hex | sio::fmt::show_base) << " ";
    }
    sw << sio::nl << sio::flush;
    //std::cout << str;
}

[[gnu::noinline]] void second() {
    std::ostringstream oss;
    oss.flags(std::ios_base::hex | std::ios_base::showbase);
    for (std::size_t i = 0; i < 1000000; ++i) {
        oss << i << " ";
    }
    oss << "\n";
    //std::cout << oss.str();
}


void foo(const std::string &s) {
    std::cout << s << "\n";
}

enum class flgs { foo, bar };

struct s {};
struct t {};

std::ostream &operator<<(std::ostream &os, s) {
    return os << "struct s";
}


int main(void) {
    auto now = stclk::now();
    first();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stclk::now() - now).count() << "\n";


    now = stclk::now();
    second();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stclk::now() - now).count() << "\n";


    foo(std::string{} << 123 << " + " << sio::format(3.14159265, sio::fmt::show_sign)
            << sio::ret);

    foo(std::string{} << (sio::fmt::oct | sio::fmt::fixed | static_cast<sio::fmt>(2048)) << sio::ret);

    foo(std::string{} << sio::line_ending::cr << sio::ret);

    foo(std::string{} << s{} << sio::ret);
}
