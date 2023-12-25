#ifndef FRAMEDATA_H
#define FRAMEDATA_H

#include "Float4.hpp"

#include <vector>


namespace simulation_io
{
  class FrameData
  {
  public:
    FrameData(int number_of_particles, const std::vector<Float4> &positions,
              const std::vector<Float4> &velocities);

    auto GetPositions() -> std::vector<Float4>;

    auto GetVelocities() -> std::vector<Float4>;

    [[nodiscard]] auto GetNumberOfParticles() const -> int;

  private:
    int number_of_particles_;
    std::vector<Float4> positions_;
    std::vector<Float4> velocities_;
  };
}// namespace simulation_io


#endif//FRAMEDATA_H
