/*
 * debugOverlay.hpp
 *
 * Debug overlay rendering for camera information display.
 * Uses ImGui for text rendering in the top-right corner of the viewport.
 */

#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include <cmath>
#include <sstream>
#include <string>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "imgui.h"

// FPS smoothing constants for exponential moving average
constexpr float FPS_SMOOTHING_FACTOR = 0.95f;
constexpr float FPS_NEW_WEIGHT = 0.05f;
constexpr float FPS_INIT_THRESHOLD = 0.001f;

// Debug overlay window padding from viewport edges
constexpr float DEBUG_OVERLAY_PADDING = 10.0f;

/*
 * Renders debug camera information as an ImGui overlay window.
 * Displays FPS, build version, camera position, target, up vector,
 * projection parameters, and viewport size.
 * Positioned in the top-right corner of the viewport, below the menu bar.
 *
 * Parameters:
 *   cam - Pointer to the Camera object
 *   screenWidth - Viewport width
 *   screenHeight - Viewport height
 *   fps - Current frames per second (0.0 if unavailable)
 *   build_version - Build version string (e.g. "0.6.0")
 */
inline void renderCameraDebugOverlay(Camera* cam, int screenWidth, int screenHeight, float fps = 0.0f,
                                     const char* build_version = "")
{
    if (cam == nullptr) {
        return;
    }

    // Get camera information
    glm::vec3 pos = cam->getPosition();
    glm::vec3 target = cam->getTarget();
    glm::vec3 up = cam->getUpVector();
    glm::vec3 front = cam->getFrontVector();
    glm::vec3 com = cam->getCenterOfMass();
    float fov = cam->getFOV();
    float nearPlane = cam->getNearPlane();
    float farPlane = cam->getFarPlane();
    float yaw = cam->getYaw();
    float pitch = cam->getPitch();

    // Check if COM is being used (non-zero)
    bool comActive = (glm::length(com) > 0.001f);

    // Calculate useful distances and metrics
    float distToTarget = glm::length(target - pos);

    // COM-related metrics (only if COM is active)
    float distToCOM = 0.0f;
    glm::vec3 directionToCOM = glm::vec3(0.0f);
    if (comActive) {
        distToCOM = glm::length(com - pos);
        directionToCOM = (distToCOM > 0.001f) ? glm::normalize(com - pos) : glm::vec3(0.0f);
    }

    // Calculate composition metrics for visual regression tests
    float estimatedSubjectSize = 12.0f;
    float fovRadians = glm::radians(fov);
    float tanHalfFov = std::tan(fovRadians / 2.0f);

    float referenceDistance = comActive ? distToCOM : 50.0f;
    float estimatedCoverage = 0.0f;
    if (referenceDistance > 0.001f) {
        estimatedCoverage = (estimatedSubjectSize / referenceDistance) / tanHalfFov;
        estimatedCoverage = std::min(estimatedCoverage * 100.0f, 100.0f);
    }

    float distFor50Pct = estimatedSubjectSize / (0.5f * tanHalfFov);
    float distFor40Pct = estimatedSubjectSize / (0.4f * tanHalfFov);

    // Smooth FPS using exponential moving average
    static float smoothed_fps = 0.0f;
    if (smoothed_fps < FPS_INIT_THRESHOLD) {
        smoothed_fps = fps;
    } else {
        smoothed_fps = smoothed_fps * FPS_SMOOTHING_FACTOR + fps * FPS_NEW_WEIGHT;
    }

    // Position the window in the top-right corner, below the menu bar
    float menu_bar_height = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(
        ImVec2(static_cast<float>(screenWidth) - DEBUG_OVERLAY_PADDING, menu_bar_height + DEBUG_OVERLAY_PADDING),
        ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.7f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("##DebugOverlay", nullptr, flags)) {
        ImGui::Text("[DEBUG CAMERA]  FPS: %d", static_cast<int>(smoothed_fps));
        if (build_version[0] != '\0') {
            ImGui::SameLine();
            ImGui::TextDisabled("  v%s", build_version);
        }
        ImGui::Separator();
        ImGui::Text("Pos: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Target: (%.2f, %.2f, %.2f)", target.x, target.y, target.z);
        ImGui::TextDisabled("  (lookat point: Pos + Front)");
        ImGui::Text("Up: (%.2f, %.2f, %.2f)", up.x, up.y, up.z);
        ImGui::Text("Front: (%.2f, %.2f, %.2f)", front.x, front.y, front.z);
        ImGui::Text("Yaw: %.2f deg  Pitch: %.2f deg", yaw, pitch);
        ImGui::Text("Dist to Target: %.2f units", distToTarget);

        if (comActive) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "--- Simulation Tracking ---");
            ImGui::Text("COM: (%.2f, %.2f, %.2f)", com.x, com.y, com.z);
            ImGui::Text("Dist to COM: %.2f units", distToCOM);
            ImGui::Text("Dir to COM: (%.2f, %.2f, %.2f)", directionToCOM.x, directionToCOM.y, directionToCOM.z);
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "--- Composition (using COM) ---");
            ImGui::Text("Est. Coverage: ~%d%% of viewport", static_cast<int>(estimatedCoverage));
        } else {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "--- Composition (est.) ---");
            ImGui::Text("Est. Coverage: ~%d%% (ref=%.2fu)", static_cast<int>(estimatedCoverage), referenceDistance);
        }
        ImGui::Text("For 50%% coverage: dist=%.2f units", distFor50Pct);
        ImGui::Text("For 40%% coverage: dist=%.2f units", distFor40Pct);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "--- Projection ---");
        ImGui::Text("Proj: Perspective FOV=%.2f deg", fov);
        ImGui::Text("      Near=%.2f Far=%.2f", nearPlane, farPlane);
        ImGui::Text("View: %dx%d", screenWidth, screenHeight);
    }
    ImGui::End();
}

#endif
