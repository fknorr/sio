#include <boost/test/unit_test.hpp>
#include <sio/writer/writer.hh>
#include <string>

using namespace sio::ops;


BOOST_AUTO_TEST_CASE(string_writer) {
    BOOST_CHECK_EQUAL(std::string {} << "Hello " << "World!" << sio::ret, "Hello World!");
}
