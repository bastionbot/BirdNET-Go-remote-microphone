#pragma once
#include <cstddef>

class Transport
{
public:
  virtual void begin() = 0;
  virtual void send(void *data, size_t bytes) = 0;
};
