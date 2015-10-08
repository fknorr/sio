#pragma once

#include "writer.hh"
#include "../stream/file.hh"


namespace sio {


extern stream_writer<file_out_stream> out;
extern stream_writer<file_out_stream> log;
extern stream_writer<file_out_stream> err;


// for "debug" reference
#ifdef __GNUC__
[[gnu::unused]]
#endif

#ifndef NDEBUG
static auto &debug = err;
#else
static auto &debug = nirvana;
#endif


template<typename CharSequence, typename ...Params>
void
printf(const CharSequence &fmt, Params &&...args) {
    write_formatted(out, fmt, std::make_tuple(args...));
}

template<typename CharSequence, typename ...Params>
void
log_printf(const CharSequence &fmt, Params &&...args) {
    write_formatted(log, fmt, std::make_tuple(args...));
}

template<typename CharSequence, typename ...Params>
void
err_printf(const CharSequence &fmt, Params &&...args) {
    write_formatted(err, fmt,  std::make_tuple(args...));
}



#ifndef NDEBUG

template<typename CharSequence, typename ...Params>
void
debug_printf(const CharSequence &fmt, Params &&...args) {
    write_formatted(debug, fmt, std::make_tuple(args...));
}

#else

template<typename ...Params>
void
debug_printf(Params &&...args) {}

#endif


} // namespace sio
