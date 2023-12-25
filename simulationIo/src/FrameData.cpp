#include "simulationIo/FrameData.hpp"

#include <stdexcept>

namespace simulation_io
{
  FrameData::FrameData(const int number_of_particles, const std::vector<Float4> &positions,
                       const std::vector<Float4> &velocities) : number_of_particles_(number_of_particles)
  {
    if (positions.size() != number_of_particles)
    {
      throw std::length_error(
              "Size of Positions Does not Equal the number of Particles");
    }
    if (velocities.size() != number_of_particles)
    {
      throw std::length_error(
              "Size of Velocities Does not Equal the number of Particles");
    }

    this->positions_ = positions;
    this->velocities_ = velocities;
  }

  auto FrameData::GetPositions() -> std::vector<Float4>
  {
    return positions_;
  }

  auto FrameData::GetVelocities() -> std::vector<Float4>
  {
    return velocities_;
  }

  auto FrameData::GetNumberOfParticles() const -> int
  {
    return number_of_particles_;
  }
}// namespace simulation_io
