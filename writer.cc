#include "writer.hh"
#include <locale>
#include <streambuf>

using namespace fio;


nl_t fio::nl;
end_t fio::end;
flush_t fio::flush;


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


namespace fio {

template<>
inline const std::num_put<char> &
writer::ios_cache::locale_facet<std::num_put<char>>(const std::locale &locale) {
    auto &r = refs(locale);
    return cached_facet(r.num_put, *r.locale);
}


template const std::num_put<char> &
writer::ios_cache::locale_facet<std::num_put<char>>(const std::locale &);

} // namespace fio


const std::locale &
writer::locale() const {
    return std::locale::classic();
}


template<typename Writer>
void fio::write(Writer &w, int v) {
    auto &ios = w.ios().ios_base();
    ios.flags(std::ios_base::fmtflags{});
    auto &fac = w.ios().template locale_facet<std::num_put<char>>(w.locale());
    auto &buf = static_cast<dummy_ostreambuf&>(w.ios().streambuf());
    char raw[20];
    buf.use(raw, 20);
    std::ostreambuf_iterator<char> start(&buf);
    fac.put(std::ostreambuf_iterator<char>(&buf), ios, ' ', static_cast<long>(v));
    w.write(raw, buf.pos());
}


template void fio::write(string_writer &, int);
