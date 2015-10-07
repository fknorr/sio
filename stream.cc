#include "stream.hh"
#include <fstream>
#include <iostream>
#include <memory>

using namespace sio;


file_stream::file_stream(const std::string &name, int perm, open_mode mode)
    : m_owns_streambuf(true) {
    std::ios::openmode ios_mode = std::ios::binary;
    if (perm & allow_get) ios_mode |= std::ios::in;
    if ((perm & allow_put) || mode == open_mode::overwrite) ios_mode |= std::ios::out;
    if (mode == open_mode::append) ios_mode |= std::ios::app;
    if (mode == open_mode::truncate) ios_mode |= std::ios::trunc;

    std::unique_ptr<std::filebuf> buf(new std::filebuf);
    if (buf->open(name, ios_mode)) {
        m_streambuf = buf.release();
    } else {
        throw 0;
    }
}


file_stream::~file_stream() {
    if (m_owns_streambuf) {
        delete m_streambuf;
    }
}


std::size_t
file_in_stream::v_get(void *out, std::size_t bytes) {
    return static_cast<std::size_t>(streambuf().sgetn(
            static_cast<char*>(out), static_cast<std::streamsize>(bytes)));
}


std::size_t
file_out_stream::v_put(const void *in, std::size_t bytes) {
    return static_cast<std::size_t>(streambuf().sputn(
            static_cast<const char*>(in), static_cast<std::streamsize>(bytes)));
}


void
file_out_stream::v_flush() {
    streambuf().pubsync();
}


static stream_pos
seek_streambuf(std::streambuf &buf, stream_off off, sio::seek rel, std::ios::openmode which) {
    std::ios::seekdir dir;
    switch (rel) {
        case sio::seek::set: dir = std::ios::beg; break;
        case sio::seek::cur: dir = std::ios::cur; break;
        default: dir = std::ios::end;
    }
    return static_cast<stream_pos>(buf.pubseekoff(static_cast<std::ios::off_type>(off),
            dir, which));
}

static stream_pos
tell_streambuf(const std::streambuf &buf, std::ios::openmode which) {
    return static_cast<stream_pos>(const_cast<std::streambuf&>(buf).pubseekoff(0,
            std::ios::cur, which));
}

stream_pos
file_read_stream::v_seek_get(stream_off offset, sio::seek rel) {
    return seek_streambuf(streambuf(), offset, rel, std::ios::in);
}


stream_pos
file_read_stream::v_tell_get() const {
    return tell_streambuf(streambuf(), std::ios::in);
}

stream_pos
file_write_stream::v_seek_put(stream_off offset, sio::seek rel) {
    return seek_streambuf(streambuf(), offset, rel, std::ios::out);
}


stream_pos
file_write_stream::v_tell_put() const {
    return tell_streambuf(streambuf(), std::ios::out);
}


stream_pos
file_rw_stream::seek(stream_off offset, sio::seek rel) {
    return seek_streambuf(streambuf(), offset, rel, std::ios::in | std::ios::out);
}


file_out_stream sio::stdout_stream(*std::cout.rdbuf());
file_out_stream sio::stderr_stream(*std::cerr.rdbuf());
file_out_stream sio::buffered_stderr_stream(*std::clog.rdbuf());
file_in_stream sio::stdin_stream(*std::cin.rdbuf());
