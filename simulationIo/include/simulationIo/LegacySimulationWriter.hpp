#ifndef CUBEBUILDER_LEGACYSIMULATIONFILEWRITER_H
#define CUBEBUILDER_LEGACYSIMULATIONFILEWRITER_H

#include "FrameData.hpp"
#include "SimulationWriter.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace simulation_io
{
  class LegacySimulationWriter : public SimulationWriter
  {

  public:
    void AppendSimulationFrame(FrameData *frame_data) override;

    explicit LegacySimulationWriter(const std::filesystem::path &folder_path,
                                    bool overwrite_existing_simulation =
                                            false);
  };
}// namespace simulation_io

#endif//CUBEBUILDER_LEGACYSIMULATIONFILEWRITER_H
