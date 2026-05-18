/*
 * recording_state.hpp
 *
 * State for recording frames to disk.
 * Extracted from viewer_app.hpp to allow independent inclusion in tests
 * and to serve as the out-parameter type for file_dialog_helpers.hpp.
 */
#ifndef PARTICLE_VIEWER_RECORDING_STATE_H
#define PARTICLE_VIEWER_RECORDING_STATE_H

#include <string>

struct RecordingState
{
    bool is_active = false;
    std::string folder;
    int error_count = 0;
    // Fixed policy value: maximum consecutive errors before silencing the log.
    static constexpr int error_max = 5;
};

#endif // PARTICLE_VIEWER_RECORDING_STATE_H
