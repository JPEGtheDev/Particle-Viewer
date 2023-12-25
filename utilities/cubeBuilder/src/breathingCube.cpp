#include <cmath>
#include <iostream>
#include <memory>

#include <simulationIo/LegacySimulationWriter>

using namespace simulation_io;

auto CreateFrameData(int number_of_particles, const std::vector<Float4> &positions,
                     const std::vector<Float4> &velocities) -> std::unique_ptr<FrameData>;


auto CreateCubeFrame(int cube_size, float distance) -> std::shared_ptr<FrameData>;

void RunSimulation(int cube_size, double time_step,
                   double total_simulation_time,
                   double times_to_breathe,
                   SimulationWriter &writer);

auto main() -> int
{
  constexpr int kCubeSize = 40;
  constexpr double kTimeStep = .2;
  constexpr double kTotalSimulationTime = 100;
  constexpr double kTimesToBreathe = 3;

  auto const writer = LegacySimulationWriter("./BreathingCube", true);
  RunSimulation(kCubeSize, kTimeStep, kTotalSimulationTime, kTimesToBreathe,
                (SimulationWriter &) writer);

  return 0;
}

auto CreateFrameData(int number_of_particles, const std::vector<Float4> &positions, const std::vector<Float4> &velocities) -> std::unique_ptr<FrameData>
{

  try
  {
    std::unique_ptr<FrameData> frame_data = std::make_unique<FrameData>(number_of_particles, positions, velocities);
    return frame_data;
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
    throw std::runtime_error("Error creating frame data");
  }
}
auto CreateCubeFrame(int cube_size, float distance) -> std::shared_ptr<FrameData>
{
  int cube_size_squared = cube_size * cube_size;
  int cube_size_cubed = cube_size_squared * cube_size;
  int number_of_particles = cube_size_cubed;
  std::vector<Float4> positions;
  std::vector<Float4> velocities;

  for (int i = 0; i < number_of_particles; i++)
  {
    positions.emplace_back(i % cube_size * distance,// NOLINT(*-narrowing-conversions)
                           static_cast<float>(i % cube_size_squared) / static_cast<float>(cube_size) * distance,
                           static_cast<float>(i % cube_size_cubed) / static_cast<float>(cube_size_squared) * distance,
                           500);// NOLINT(*-magic-numbers)
  }
  for (int i = 0; i < number_of_particles; i++)
  {
    velocities.emplace_back(i * 1.0, i * 2.0, i * 3.0, 0);// NOLINT(*-magic-numbers)
  }
  return CreateFrameData(number_of_particles, positions, velocities);
}


#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-narrowing-conversions"
void RunSimulation(const int cube_size, const double time_step, const double total_simulation_time,// NOLINT(*-easily-swappable-parameters)
                   const double times_to_breathe,
                   SimulationWriter &writer)
{
  const auto pi_value = 2 * acos(0.0);
  for (double i = 0; i < total_simulation_time;)
  {
    std::cout << "Frame:" << i << "\n";
    const double value = times_to_breathe * i * pi_value / total_simulation_time;
    auto frame_data_ptr = CreateCubeFrame(cube_size, std::abs(1.25 * cos(value)));// NOLINT(*-magic-numbers)
    writer.AppendSimulationFrame(frame_data_ptr.get());
    i += time_step;
  }
}
#pragma clang diagnostic pop