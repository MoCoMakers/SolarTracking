#ifndef _LCD_TERMINAL__H_
#define _LCD_TERMINAL__H_

#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

#include <Streaming.h>

#include <StateMachine.h>
#include <RingBuffer.h>
#include "PrintBuffer.h"

#define _CLS '\x0C'
#define _ESC '\x1B'
#define _SPC ' '
#define _DEG '\xDF'

char* _CURPOS(const byte row, const byte col)
{
  static char buf[10];
  byte r = row + 1;
  byte c = col + 1;
  char* p = buf;
  *p++ = _ESC;
  *p++ = '[';
  if (r > 9) {
    *p++ = (char) ('0' + (r / 10));
    r %= 10;
  }
  *p++ = (char) ('0' + r);
  *p++ = ';';
  if (c > 9) {
    *p++ = (char) ('0' + (c / 10));
    c %= 10;
  }
  *p++ = (char) ('0' + c);
  *p++ = 'H';
  *p++ = '\0';
  return buf;
}

class LcdTerminal : public StateMachine, public Print
{
private:
  enum E_STATE {
    eChar, eCsi, eEscSeq, eParm1, eParm2
  } m_state;
  
  Adafruit_LiquidCrystal& m_printer;
  RingBuffer m_rb;
  byte m_parm1;
  byte m_parm2;
  byte m_savedRow;
  byte m_savedCol;
  
public:
  LcdTerminal(Adafruit_LiquidCrystal& printer, const size_t rbSize) :
    StateMachine(10L, false),
    Print(),
    m_state(eChar),
    m_printer(printer),
    m_rb(rbSize),
    m_parm1(0),
    m_parm2(0),
    m_savedRow(0),
    m_savedCol(0)
  {
  }

  virtual bool update()
  {
    if (StateMachine::update()) {
      if (!m_rb.isEmpty()) {
        byte c = m_rb.pop();
        if (m_state == eChar) {
          if (c == _CLS) {
            m_printer.clear();
          } else if (c == _ESC) {
            m_state = eCsi;
          } else {
            m_printer.write((uint8_t) c);
          }
        } else if (m_state == eCsi) {
          if (c == '[') {
            m_state = eEscSeq;
          } else {
            m_state = eChar;
          }
        } else if (m_state == eEscSeq) {
          if (isDigit(c)) {
            m_state = eParm1;
            m_parm1 = (byte) (c - '0');
          } else if (isAlpha(c)) {
            // Act upon completed sequence.
            m_state = eChar;
          }
        } else if (m_state == eParm1) {
          if (isDigit(c)) {
            m_parm1 = 10 * m_parm1 + (byte) (c - '0');
          } else if (c == ';') {
            m_state = eParm2;
            m_parm2 = 0;
          } else if (isAlpha(c)) {
            // Act upon completed sequence.
            m_state = eChar;
          } else {
            m_state = eChar;
          }
        } else if (m_state == eParm2) {
          if (isDigit(c)) {
            m_parm2 = 10 * m_parm2 + (byte) (c - '0');
          } else if (isAlpha(c)) {
            if (c == 'H' || c == 'f') {
              m_printer.setCursor(m_parm2 - 1, m_parm1 - 1);
            }
            m_state = eChar;
          } else {
            m_state = eChar;
          }
        }
      }
      return true;
    }
    return false;
  }

  virtual size_t write(uint8_t b)
  {
    if (m_rb.isFull()) {
      return (size_t) 0;
    } else {
      m_rb.push((byte) b);
      return (size_t) 1;
    }
  }
};

class Field
{
private:
  PrintBuffer m_buffer;
  char* m_hold;
  const byte m_row;
  const byte m_col;
  const size_t m_width;
  
public:
  Field(const byte row, const byte col, const size_t width) :
    m_buffer(width), m_hold(new char[width + 10]),
    m_row(row), m_col(col), m_width(width)
  {
  }

  template <class T>
  char* rjust(T val)
  {
    char* p = m_hold;
    char* s = _CURPOS(m_row, m_col);
    while (*s != _NUL) {
      *p++ = *s++;
    }
    m_buffer << val;
    byte len = m_buffer.size();
    for (byte i = len; i < m_width; ++i) {
      *p++ = _SPC;
    }
    for (byte i = 0; i < len; ++i) {
      *p++ = m_buffer.read();
    }
    *p = _NUL;
    return m_hold;
  }
};
  
#endif

