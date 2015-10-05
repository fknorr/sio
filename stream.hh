#pragma once

#include "enum.hh"
#include <cstdint>
#include <vector>
#include <limits>


namespace sio {


enum class seek {
    set,
    cur,
    end
};

template<>
struct enum_names<seek> {
    enum_name_list<seek> operator()() const {
        return { "sio::seek::", {
            { seek::set, "set" }, { seek::cur, "cur" }, { seek::end, "end" }
        } };
    }
};


using stream_pos = std::uint_least64_t;
using stream_off = std::make_signed_t<stream_pos>;


class ra_stream {
public:
    virtual ~ra_stream() {}

    virtual stream_pos seek(stream_off offset, sio::seek rel) = 0;

    stream_pos seek(stream_pos new_pos) {
        return seek(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    virtual stream_pos pos() const = 0;
};


class in_stream {
public:
    virtual ~in_stream();

    virtual std::size_t read(void *out, std::size_t bytes) = 0;
};


class out_stream {
public:
    virtual ~out_stream();

    virtual std::size_t write(const void *in, std::size_t bytes) = 0;
};


class ra_in_stream: public virtual ra_stream, public virtual in_stream {};

class ra_out_stream: public virtual ra_stream, public virtual out_stream {};

class duplex_stream: public virtual in_stream, public virtual out_stream {};

class ra_duplex_stream: public virtual ra_in_stream, public virtual ra_out_stream,
    public duplex_stream {};



class memory_stream final: public ra_duplex_stream {
public:
    virtual stream_pos seek(stream_off offset, sio::seek rel) override {
        stream_off new_pos;
        switch (rel) {
            case sio::seek::set: new_pos = offset; break;
            case sio::seek::cur: new_pos = static_cast<stream_off>(m_pos) + offset; break;
            case sio::seek::end: new_pos = static_cast<stream_off>(m_data.size()) - offset; break;
        }
        if (new_pos < 0) new_pos = 0;
        if (new_pos > static_cast<stream_off>(max_pos)) new_pos = static_cast<stream_off>(max_pos);
        return m_pos;
    }

    virtual stream_pos pos() const override {
        return m_pos;
    }

    virtual std::size_t read(void *out, std::size_t bytes) override {
        bytes = std::max(bytes, m_data.size() - static_cast<std::size_t>(m_pos));
        auto start = &m_data[0] + m_pos;
        std::copy(start, start + bytes,  static_cast<unsigned char*>(out));
        return bytes;
    }

    virtual std::size_t write(const void *in, std::size_t bytes) override {
        if (static_cast<stream_pos>(m_pos) + bytes > max_pos) {
            bytes = static_cast<std::size_t>(max_pos) - m_pos;
        }
        if (m_pos + bytes > m_data.size()) {
            m_data.resize(m_pos + bytes);
        }
        auto byte_in = static_cast<const unsigned char*>(in);
        std::copy(byte_in, byte_in + bytes, &m_data[0] + m_pos);
        return bytes;
    }

private:
    std::size_t m_pos;
    std::vector<unsigned char> m_data;

    enum : stream_pos { max_pos = std::numeric_limits<std::size_t>::max() };
};


} // namespace sio
