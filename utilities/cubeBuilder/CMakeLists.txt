link_libraries(simulationIo)

add_executable(cubeBuilder
        src/breathingCube.cpp
)

target_include_directories(cubeBuilder PUBLIC)
set_target_properties(cubeBuilder PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(cubeBuilder simulationIo ${CMAKE_DL_LIBS})
