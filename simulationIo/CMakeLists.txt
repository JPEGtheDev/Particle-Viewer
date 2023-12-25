project(SimulationIo)

add_library(simulationIo STATIC
"${SimulationIo_SOURCE_DIR}/include/simulationIo/Float4.hpp"
"${SimulationIo_SOURCE_DIR}/include/simulationIo/FrameData.hpp"
"${SimulationIo_SOURCE_DIR}/include/simulationIo/LegacySimulationWriter"
"${SimulationIo_SOURCE_DIR}/include/simulationIo/LegacySimulationWriter.hpp"
"${SimulationIo_SOURCE_DIR}/include/simulationIo/SimulationInformation.hpp"
"${SimulationIo_SOURCE_DIR}/include/simulationIo/SimulationWriter.hpp"
"${SimulationIo_SOURCE_DIR}/src/Float4.cpp"
"${SimulationIo_SOURCE_DIR}/src/FrameData.cpp"
"${SimulationIo_SOURCE_DIR}/src/LegacySimulationWriter.cpp"
)

set_target_properties(simulationIo PROPERTIES
                      OUTPUT_NAME ${SimulationIo_LIB_NAME}
                      VERSION ${SimulationIo_VERSION_MAJOR}.${SimulationIo_VERSION_MINOR}
                      SOVERSION ${SimulationIo_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      DEFINE_SYMBOL _SimulationIo_BUILD_DLL
                      FOLDER "SimulationIo")

target_include_directories(simulationIo PUBLIC
                           "$<BUILD_INTERFACE:${SimulationIo_SOURCE_DIR}/include>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_include_directories(simulationIo PRIVATE
                           "${SimulationIo_SOURCE_DIR}/src"
                           "${SimulationIo_BINARY_DIR}/src")

