#ifndef FASTREAD_SOURCE_H_
#define FASTREAD_SOURCE_H_

#include "cpp11/list.hpp"
#include "utils.h"

#include "boost.h"

class Source;
typedef boost::shared_ptr<Source> SourcePtr;

class Source {
public:
  Source() : skippedRows_(0) {}
  virtual ~Source() {}

  virtual const char* begin() = 0;
  virtual const char* end() = 0;

  const char* skipLines(
      const char* begin,
      const char* end,
      int n,
      bool skipEmptyRows = true,
      const std::string& comment = "");

  const char* skipLine(const char* begin, const char* end, bool isComment);

  const char* skipDoubleQuoted(const char* begin, const char* end);

  size_t skippedRows() { return skippedRows_; }

  static const char* skipBom(const char* begin, const char* end);

  static SourcePtr create(cpp11::list spec);

private:
  static bool
  inComment(const char* cur, const char* end, const std::string& comment) {
    boost::iterator_range<const char*> haystack(cur, end);
    return boost::starts_with(haystack, comment);
  }

  size_t skippedRows_;
};

#endif
