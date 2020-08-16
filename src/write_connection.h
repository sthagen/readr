#ifndef READR_WRITE_CONNECTION_H_
#define READR_WRITE_CONNECTION_H_

#include <boost/iostreams/categories.hpp> // sink_tag
#include <ios>                            // streamsize

#include "connection.h"

// http://www.boost.org/doc/libs/1_63_0/libs/iostreams/doc/tutorial/container_sink.html
namespace io = boost::iostreams;

class connection_sink {
private:
  SEXP con_;

public:
  typedef char char_type;
  typedef io::sink_tag category;

  connection_sink(SEXP con);
  std::streamsize write(const char* s, std::streamsize n);
};

#endif
