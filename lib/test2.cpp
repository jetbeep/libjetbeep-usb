#include <iostream>
#include <boost/asio.hpp>

using namespace std;
using namespace boost;

void test2() {

}


void serial() {
  boost::asio::io_service io;
  boost::asio::serial_port serial(io);
}
