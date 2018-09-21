#include <RingBuffer.h>

// Define a RingBuffer that can contain up to 4 bytes.
RingBuffer rb(4);

// Display full/empty status of ring buffer.
void show()
{
  Serial.println(rb.isEmpty() ? "rb is EMPTY" : "rb is NOT EMPTY");
  Serial.println(rb.isFull()  ? "rb is FULL"  : "rb is NOT FULL");
}

// One-time test.
void setup() 
{
  // Set up consolee output.
  Serial.begin(115200);
  // Show status of (empty) ring buffer.
  show();
  // Try pushing 5 bytes (0, 1, 2, 3, 4) into ring buffer without popping
  // any bytes out.  5th one will be skipped because buffer is full.
  for (byte b = 0; b < 5; ++b) {
    if (!rb.isFull()) {
      Serial.print("pushing ");
      Serial.println(b, DEC);
      rb.push(b);
    } else {
      Serial.print("skipping ");
      Serial.println(b, DEC);
    }
    // Show status after each (attempt to) push.
    show();
  }
  // Start popping bytes until buffer is empty again.
  while (!rb.isEmpty()) {
    Serial.print("popped ");
    Serial.println(rb.pop(), DEC);
    show();
  }
  // Pop one more time, should get zero.
  Serial.print("popped ");
  Serial.println(rb.pop(), DEC);
  show();
}

void loop()
{
  // Nothing to see here.
}
