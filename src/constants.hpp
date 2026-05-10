#pragma once

// Simulation-to-display coordinate scale factor.
// Applied by the vertex shader (sphereVertex.vs: uniform float transScale)
// and by getCOM() when converting raw simulation coordinates to display-space coordinates.
constexpr float kSimToDisplayScale = 0.25f;
