#include "writer.hh"
#include <sstream>


namespace sio {


template<typename Value>
struct ostream_operator_defined {
private:
    template<typename V>
    static std::true_type test(
            std::decay_t<decltype(std::declval<std::ostream&>() << std::declval<V>())> *);

    template<typename V>
    static std::false_type test(...);

public:
    enum : bool { value = decltype(test<Value>(nullptr))::value };

    constexpr ostream_operator_defined() noexcept {}
    constexpr operator bool() noexcept { return value; }
    constexpr bool operator()() noexcept { return value; }
};


template<typename Writer, typename Value,
        std::enable_if_t<!write_function_defined<Writer, Value>{}
            && ostream_operator_defined<Value>{}, int> = 0>
void dispatch_write(Writer &w, Value &&v) {
    std::ostringstream oss;
    oss << std::forward<Value>(v);
    write(w, oss.str());
}


}
