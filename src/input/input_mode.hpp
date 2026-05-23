/*
 * input_mode.hpp
 *
 * Enumerates the two high-level input modes for Particle-Viewer.
 * ViewMode: gamepad/keyboard controls the camera and particles.
 * MenuMode: input is directed to the controller panel overlay.
 */

#ifndef PARTICLE_VIEWER_INPUT_INPUT_MODE_H
#define PARTICLE_VIEWER_INPUT_INPUT_MODE_H

enum class InputMode
{
    ViewMode,
    MenuMode
};

#endif
