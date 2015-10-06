#include "writer.hh"
#include <locale>
#include <streambuf>

using namespace sio;


nl_t sio::nl;
end_t sio::end;
flush_t sio::flush;
ret_t sio::ret;

discarding_writer sio::nirvana;


writeable::ios_cache::ref_set::~ref_set() {}


class dummy_ostreambuf final: public std::streambuf {
public:
    void use(char *buffer, std::size_t size) {
        setp(buffer, buffer+size);
    }

    std::size_t pos() const {
        return static_cast<std::size_t>(pptr() - pbase());
    }
};


class dummy_ios final: public std::ios_base {};


void
writer::ios_cache::create_refs() {
    m_refs = std::make_unique<ref_set>();
}

void
writer::ios_cache::create_ios_base() {
    refs().ios_base = std::make_unique<dummy_ios>();
}

void
writer::ios_cache::create_streambuf() {
    refs().streambuf = std::make_unique<dummy_ostreambuf>();
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
sio::write(writeable &w, const Number &v, bitfield<fmt> flags, unsigned precision) {
    std::ios_base::fmtflags iosflags {};
    if (flags & fmt::oct) {
        iosflags |= std::ios_base::oct;
    } else if (flags & fmt::hex) {
        iosflags |= std::ios_base::hex;
    } else {
        iosflags |= std::ios_base::dec;
    }
    if (flags & fmt::sci) {
        iosflags |= std::ios_base::scientific;
    } else if (flags & fmt::fixed) {
        iosflags |= std::ios_base::fixed;
    }
    if (flags & fmt::show_base) {
        iosflags |= std::ios_base::showbase;
    }
    if (flags & fmt::show_point) {
        iosflags |= std::ios_base::showpoint;
    }
    if (flags & fmt::show_sign) {
        iosflags |= std::ios_base::showpos;
    }
    if (flags & fmt::uppercase) {
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

template void sio::write(writeable &, const char &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const unsigned char &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const signed char &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const unsigned short &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const short &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const unsigned int &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const int &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const unsigned long &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const long &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const unsigned long long &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const long long &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const float &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const double &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const long double &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const void *const &, bitfield<fmt>, unsigned);
template void sio::write(writeable &, const bool &, bitfield<fmt>, unsigned);
