#include "stream.hh"
#include "writer.hh"


namespace sio {

stream_writer<file_out_stream> out(stdout_stream);
stream_writer<file_out_stream> err(stderr_stream);

} // namespace sio
