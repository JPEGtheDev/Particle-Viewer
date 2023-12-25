#ifndef FLOAT4_HPP
#define FLOAT4_HPP

namespace simulation_io
{
  /// \brief Simple bundled group of 4 floats in a row. Can be cast to Float4 or
  /// glm::vec4 easily
  struct Float4 {
    [[maybe_unused]] float x;
    [[maybe_unused]] float y;
    [[maybe_unused]] float z;
    [[maybe_unused]] float w;

    Float4(float x, float y, float z, float w);// NOLINT(*-identifier-length)
  };
}// namespace simulation_io

#endif// FLOAT4_HPP
