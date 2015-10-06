#pragma once

#include "writer.hh"
#include "stream.hh"


namespace sio {


extern stream_writer<file_out_stream> out;
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


} // namespace sio
