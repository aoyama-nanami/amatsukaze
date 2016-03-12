#ifndef HTTPCHUNKEDFILTER_H
#define HTTPCHUNKEDFILTER_H

#include <string>

#include <qDebug>

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>

class HttpChunkedFilter {
public:
  typedef char char_type;
  typedef boost::iostreams::input_filter_tag category;

  template <typename Source>
  int get(Source& source) {
    if (end_of_chunk_) return EOF;


    if (chunk_remaining_ == 0) {
      int ret = ReadChunkHeader_(source);
      if (ret <= 0) {
        end_of_chunk_ = true;
        return EOF;
      }
      chunk_remaining_ = ret;
    }

    int c = boost::iostreams::get(source);
    if (c != EOF) {
      if (--chunk_remaining_ == 0) {
        boost::iostreams::get(source);
        boost::iostreams::get(source);
      }
    }
    return c;
  }

private:
  int chunk_remaining_ = 0;
  bool end_of_chunk_ = false;

  template <typename Source>
  int ReadChunkHeader_(Source& source) {
    std::string line;
    while (true) {
      int c = boost::iostreams::get(source);
      if (c == EOF) return EOF;
      line += c;
      if (c == '\n') break;
    }

    return std::stoi(line, nullptr, 16);
  }
};

#endif // HTTPCHUNKEDFILTER_H
