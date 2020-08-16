#ifndef FASTREAD_TOKENIZER_H_
#define FASTREAD_TOKENIZER_H_

#include "cpp11/R.hpp"
#include "cpp11/list.hpp"
#include "cpp11/protect.hpp"

#include "Warnings.h"
#include "boost.h"

class Token;

typedef const char* SourceIterator;
typedef std::pair<SourceIterator, SourceIterator> SourceIterators;
typedef void (*UnescapeFun)(
    SourceIterator, SourceIterator, boost::container::string*);

class Tokenizer;
typedef boost::shared_ptr<Tokenizer> TokenizerPtr;

class Tokenizer {
  Warnings* pWarnings_;

public:
  Tokenizer() : pWarnings_(NULL) {}
  virtual ~Tokenizer() {}

  virtual void tokenize(SourceIterator begin, SourceIterator end) = 0;
  virtual Token nextToken() = 0;
  // Percentage & bytes
  virtual std::pair<double, size_t> progress() = 0;

  virtual void unescape(
      SourceIterator begin,
      SourceIterator end,
      boost::container::string* pOut) {
    pOut->reserve(end - begin);
    for (SourceIterator cur = begin; cur != end; ++cur)
      pOut->push_back(*cur);
  }

  void setWarnings(Warnings* pWarnings) { pWarnings_ = pWarnings; }

  inline void warn(
      int row,
      int col,
      const std::string& expected,
      const std::string& actual = "") {
    if (pWarnings_ == NULL) {
      cpp11::warning(
          "[%i, %i]: expected %s", row + 1, col + 1, expected.c_str());
      return;
    }
    pWarnings_->addWarning(row, col, expected, actual);
  }

  static TokenizerPtr create(cpp11::list spec);
};

// -----------------------------------------------------------------------------
// Helper class for parsers - ensures iterator always advanced no matter
// how loop is exited

class Advance : boost::noncopyable {
  SourceIterator* pIter_;

public:
  Advance(SourceIterator* pIter) : pIter_(pIter) {}
  ~Advance() { (*pIter_)++; }
};

#endif
