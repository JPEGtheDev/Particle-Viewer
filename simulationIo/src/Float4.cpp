#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-easily-swappable-parameters"

#include "simulationIo/Float4.hpp"

namespace simulation_io
{
  Float4::Float4(const float x, const float y, const float z, const float w)// NOLINT(*-identifier-length)
      : x(x), y(y), z(z), w(w)
  {}

}// namespace simulation_io

#pragma clang diagnostic pop