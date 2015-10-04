#include "writer.hh"
#include <locale>
#include <streambuf>

using namespace sio;


nl_t sio::nl;
end_t sio::end;
flush_t sio::flush;


class dummy_ostreambuf final: public std::streambuf {
public:
    void use(char *buffer, std::size_t size) {
        setp(buffer, buffer+size);
    }

    std::size_t pos() const {
        return pptr() - pbase();
    }
};


class dummy_ios final: public std::ios_base {};


std::unique_ptr<std::ios_base>
writer::ios_cache::create_ios_base() {
    return std::make_unique<dummy_ios>();
}

std::unique_ptr<std::streambuf>
writer::ios_cache::create_streambuf() {
    return std::make_unique<dummy_ostreambuf>();
}



template<typename Facet>
const Facet &
writer::ios_cache::cached_facet(Facet *&ptr, const std::locale &locale) {
    if (!ptr) {
        ptr = &std::use_facet<Facet>(locale);
    }
    return *ptr;
}


namespace sio {

template<>
inline const std::num_put<char> &
writer::ios_cache::locale_facet<std::num_put<char>>(const std::locale &locale) {
    auto &r = refs(locale);
    return cached_facet(r.num_put, *r.locale);
}

} // namespace sio


template const std::num_put<char> &
writer::ios_cache::locale_facet<std::num_put<char>>(const std::locale &);


const std::locale &
writeable::locale() const {
    return std::locale::classic();
}


template<typename Number, std::enable_if_t<std::is_integral<Number>{}
        && std::is_signed<Number>{} && sizeof(Number) <= sizeof(long), int> = 0>
static auto widen_integer(const Number &num) {
    return static_cast<long>(num);
}

template<typename Number, std::enable_if_t<std::is_integral<Number>{}
        && std::is_unsigned<Number>{} && sizeof(Number) <= sizeof(long), int> = 0>
static auto widen_integer(const Number &num) {
    return static_cast<unsigned long>(num);
}

template<typename Number, std::enable_if_t<!std::is_integral<Number>{}
        || (sizeof(Number) > sizeof(long)), int> = 0>
static auto widen_integer(const Number &num) {
    return num;
}


template<typename Number, std::enable_if_t<std::is_arithmetic<Number>{}
        || std::is_same<Number, bool>{} || std::is_same<Number, const void*>{}, int>>
void
sio::write(writeable &w, const Number &v, format_flags flags, unsigned precision) {
    std::ios_base::fmtflags iosflags {};
    if (flags & format_flags::oct) {
        iosflags |= std::ios_base::oct;
    } else if (flags & format_flags::hex) {
        iosflags |= std::ios_base::hex;
    } else {
        iosflags |= std::ios_base::dec;
    }
    if (flags & format_flags::sci) {
        iosflags |= std::ios_base::scientific;
    } else if (flags & format_flags::fixed) {
        iosflags |= std::ios_base::fixed;
    }
    if (flags & format_flags::show_base) {
        iosflags |= std::ios_base::showbase;
    }
    if (flags & format_flags::show_point) {
        iosflags |= std::ios_base::showpoint;
    }
    if (flags & format_flags::show_sign) {
        iosflags |= std::ios_base::showpos;
    }
    if (flags & format_flags::uppercase) {
        iosflags |= std::ios_base::uppercase;
    }

    auto &ios = w.ios().ios_base();
    ios.flags(iosflags);
    ios.precision(precision);
    //ios.width(0);

    auto &fac = w.ios().template locale_facet<std::num_put<char>>(w.locale());
    auto &buf = static_cast<dummy_ostreambuf&>(w.ios().streambuf());
    char raw[20];
    buf.use(raw, 20);
    std::ostreambuf_iterator<char> start(&buf);
    fac.put(std::ostreambuf_iterator<char>(&buf), ios, 0, widen_integer(v));
    w.write(raw, buf.pos());
}

template void sio::write(writeable &, const char &, format_flags, unsigned);
template void sio::write(writeable &, const unsigned char &, format_flags, unsigned);
template void sio::write(writeable &, const signed char &, format_flags, unsigned);
template void sio::write(writeable &, const unsigned short &, format_flags, unsigned);
template void sio::write(writeable &, const short &, format_flags, unsigned);
template void sio::write(writeable &, const unsigned int &, format_flags, unsigned);
template void sio::write(writeable &, const int &, format_flags, unsigned);
template void sio::write(writeable &, const unsigned long &, format_flags, unsigned);
template void sio::write(writeable &, const long &, format_flags, unsigned);
template void sio::write(writeable &, const unsigned long long &, format_flags, unsigned);
template void sio::write(writeable &, const long long &, format_flags, unsigned);
template void sio::write(writeable &, const float &, format_flags, unsigned);
template void sio::write(writeable &, const double &, format_flags, unsigned);
template void sio::write(writeable &, const long double &, format_flags, unsigned);
template void sio::write(writeable &, const void *const &, format_flags, unsigned);
template void sio::write(writeable &, const bool &, format_flags, unsigned);
