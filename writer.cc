#include "writer.hh"
#include <locale>
#include <cstdio>
#include <streambuf>


nl_t nl;
end_t end;


template<std::size_t N>
class fixed_ostreambuf final: public std::streambuf {
public:
    fixed_ostreambuf() {
        setp(m_buffer, m_buffer+N);
    }

    const char *buffer() const {
        return m_buffer;
    }

    std::size_t size() const {
        return N;
    }

    std::size_t pos() const {
        return pptr() - pbase();
    }

private:
    char m_buffer[N];
};


class dummy_ios final: public std::ios_base {};


std::unique_ptr<std::ios_base>
writer::ios_cache::create_ios_base() {
    return std::make_unique<dummy_ios>();
}


template<typename Writer>
void write(Writer &w, int v) {
    fixed_ostreambuf<20> putbuf;
    auto &ios = w.ios().ios_base();
    ios.flags(std::ios_base::fmtflags{});
    auto &fac = w.ios().template locale_facet<std::num_put<char>>(w.locale());
    fac.put(std::ostreambuf_iterator<char>(&putbuf), ios, ' ', static_cast<long>(v));
    w.write(putbuf.buffer(), putbuf.pos());
}


template void write(string_writer &, int);
