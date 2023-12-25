#ifndef SIMULATIONFILEWRITER_H
#define SIMULATIONFILEWRITER_H

#include "FrameData.hpp"
#include "SimulationInformation.hpp"
#include <filesystem>

namespace simulation_io
{
  class SimulationWriter// NOLINT(*-special-member-functions)
  {

  public:
    virtual ~SimulationWriter() = default;
    virtual void AppendSimulationFrame(FrameData *frame_data) = 0;

  protected:
    std::filesystem::path folder_path;
    std::filesystem::path file_path;
  };
}// namespace simulation_io

#endif//SIMULATIONFILEWRITER_H
