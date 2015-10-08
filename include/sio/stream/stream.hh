#pragma once

#include <cstdint>
#include "../enum.hh"


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


class stream {
public:
    virtual inline ~stream() = 0;
};

stream::~stream() {}


class in_stream: public virtual stream {
protected:
    virtual std::size_t v_get(void *out, std::size_t bytes) = 0;

public:
    std::size_t get(void *out, std::size_t bytes) {
        return v_get(out, bytes);
    }
};


class out_stream: public virtual stream {
protected:
    virtual std::size_t v_put(const void *in, std::size_t bytes) = 0;

    virtual void v_flush() {}

public:
    std::size_t put(const void *in, std::size_t bytes) {
        return v_put(in, bytes);
    }

    void flush() {
        v_flush();
    }
};


class read_stream: public virtual in_stream {
protected:
    virtual stream_pos v_seek_get(stream_off offset, sio::seek rel) = 0;

    virtual stream_pos v_tell_get() const = 0;

public:
    stream_pos seek_get(stream_off offset, sio::seek rel) {
        return v_seek_get(offset, rel);
    }

    stream_pos seek_get(stream_pos new_pos) {
        return v_seek_get(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    stream_pos seek(stream_off offset, sio::seek rel) {
        return v_seek_get(offset, rel);
    }

    stream_pos seek(stream_pos new_pos) {
        return v_seek_get(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    stream_pos tell_get() const {
        return v_tell_get();
    }

    stream_pos tell() const {
        return v_tell_get();
    }
};


class write_stream: public virtual out_stream {
protected:
    virtual stream_pos v_seek_put(stream_off offset, sio::seek rel) = 0;

    virtual stream_pos v_tell_put() const = 0;

public:
    stream_pos seek_put(stream_off offset, sio::seek rel) {
        return v_seek_put(offset, rel);
    }

    stream_pos seek_put(stream_pos new_pos) {
        return v_seek_put(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    stream_pos seek(stream_off offset, sio::seek rel) {
        return v_seek_put(offset, rel);
    }

    stream_pos seek(stream_pos new_pos) {
        return v_seek_put(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    stream_pos tell_put() const {
        return v_tell_put();
    }

    stream_pos tell() const {
        return v_tell_put();
    }
};


class duplex_stream: public virtual in_stream, public virtual out_stream {};

class rw_stream: public virtual read_stream, public virtual write_stream,
    public duplex_stream {};


template<typename Stream>
using is_in_stream = std::is_base_of<in_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_out_stream = std::is_base_of<out_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_read_stream = std::is_base_of<read_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_write_stream = std::is_base_of<write_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_duplex_stream = std::is_base_of<duplex_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_rw_stream = std::is_base_of<rw_stream, std::decay_t<Stream>>;



} // namespace sio
