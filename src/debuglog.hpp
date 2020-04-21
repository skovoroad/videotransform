#pragma once
#include <iostream>

namespace vt {
  #define DEBUG_CERR
  #ifdef DEBUG_CERR
  #define stdcerr std::cerr
  #else
  class NullStream : public std::ostream {
      class NullBuffer : public std::streambuf {
      public:
	  int overflow( int c ) { return c; }
      } m_nb;
  public:
      NullStream() : std::ostream( &m_nb ) {}
  };
  static NullStream nullstream;
  #define stdcerr nullstream
  #endif
}

