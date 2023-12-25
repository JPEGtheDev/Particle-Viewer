#include "../include/simulationIo/LegacySimulationWriter"

namespace simulation_io
{
  using namespace std::filesystem;
  using std::cout;
  using std::ofstream;

  void LegacySimulationWriter::AppendSimulationFrame(FrameData *frame_data)
  {
    auto positions = frame_data->GetPositions();
    auto velocities = frame_data->GetVelocities();
    ofstream file_stream(file_path,
                         std::ios::out | std::ios::binary | std::ios::app);
    if (!file_stream)
    {
      throw std::ios::failure("Cannot open file!");
    }

    file_stream.write(reinterpret_cast<const char *>(positions.data()), positions.size() * sizeof(Float4));  // NOLINT(*-pro-type-cstyle-cast)
    file_stream.write(reinterpret_cast<const char *>(velocities.data()), velocities.size() * sizeof(Float4));// NOLINT(*-pro-type-cstyle-cast)

    file_stream.close();
  }

  LegacySimulationWriter::LegacySimulationWriter(const path &folder_path, bool overwrite_existing_simulation)
  {
    if (!exists(folder_path))
    {
      create_directory(folder_path);
      cout << "Created Directory: " << folder_path << "\n";
    }

    this->file_path = folder_path / "PosAndVel";

    if (overwrite_existing_simulation || !exists(file_path))
    {
      cout << "Simulation File Created at: " << file_path;
      ofstream file_stream(file_path, std::ios::out | std::ios::binary);
      file_stream.close();
    }
  }

}// namespace simulation_io
