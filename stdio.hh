#pragma once

#include "writer.hh"
#include "stream.hh"


namespace sio {

extern stream_writer<file_out_stream> out;
extern stream_writer<file_out_stream> err;

} // namespace sio
