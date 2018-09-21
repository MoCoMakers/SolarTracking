#ifndef _PRINT_BUFFER__H_
#define _PRINT_BUFFER__H_

#include <RingBuffer.h>

#define _NUL '\0'

class PrintBuffer : public Print
{
private:
  RingBuffer m_rb;
  size_t m_size;

public:
  PrintBuffer(const size_t len) : m_rb(len), m_size(0)
  {
  }

  virtual size_t write(uint8_t c)
  {
    if (m_rb.isFull()) {
      return 0;
    } else {
      m_rb.push(c);
      ++m_size;
      return 1;
    }
  }

  size_t read(char* buffer, size_t len)
  {
    size_t n = 1; // allow for null terminator
    char* p = buffer;
    while (n < len && !m_rb.isEmpty()) {
      *p++ = m_rb.pop();
      --m_size;
      ++n;
    }
    *p = _NUL;
    return n;
  }

  char read()
  {
    if (m_rb.isEmpty()) {
      return _NUL;
    } else {
      --m_size;
      return m_rb.pop();
    }
  }

  size_t size() const
  {
    return m_size;
  }

  void clear()
  {
    m_rb.clear();
  }
};

#endif

