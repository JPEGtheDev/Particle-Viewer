/*
 * gamepad_input.hpp
 *
 * SDL3 gamepad input handler for Particle-Viewer.
 *
 * Wraps SDL3's SDL_Gamepad API to provide normalized axis values,
 * dead zone filtering, and per-frame button edge detection (just-pressed).
 *
 * Usage:
 *   GamepadInput gamepad;
 *   gamepad.openFirstGamepad();          // call once on startup
 *
 *   // In event loop:
 *   gamepad.handleEvent(event);          // handles GAMEPAD_ADDED/REMOVED
 *
 *   // Once per frame after event loop:
 *   gamepad.poll();                      // snapshot current SDL3 state
 *
 *   // Query state:
 *   float fwd = -gamepad.getLeftStickY();  // negative Y = forward on stick
 *   bool a_pressed = gamepad.isButtonJustPressed(SDL_GAMEPAD_BUTTON_SOUTH);
 */

#ifndef PARTICLE_VIEWER_INPUT_GAMEPAD_INPUT_H
#define PARTICLE_VIEWER_INPUT_GAMEPAD_INPUT_H

#include <array>
#include <cmath>
#include <cstdint>

// clang-format off
#include <SDL3/SDL.h>  // NOLINT(llvm-include-order)
// clang-format on

/*
 * GamepadInput wraps SDL3 gamepad access with dead zone filtering and
 * per-frame edge detection.  Only one gamepad is supported at a time.
 */
class GamepadInput
{
  public:
    /*
     * Dead zone threshold applied to all axes.  Inputs with magnitude below
     * this fraction of the axis range are treated as zero.
     */
    static constexpr float DEFAULT_DEAD_ZONE = 0.15f;

    GamepadInput();
    ~GamepadInput();

    // Non-copyable: owns an SDL_Gamepad* handle
    GamepadInput(const GamepadInput&) = delete;
    GamepadInput& operator=(const GamepadInput&) = delete;

    /*
     * Open the first detected gamepad.  Safe to call even if no gamepad is
     * connected — isConnected() will return false until one is available.
     */
    void openFirstGamepad();

    /*
     * Forward an SDL event so GAMEPAD_ADDED / GAMEPAD_REMOVED are handled.
     * Call from the application's SDL_PollEvent loop.
     */
    void handleEvent(const SDL_Event& event);

    /*
     * Snapshot the current SDL3 gamepad state and advance button edge detection.
     * Must be called once per frame, after the event loop, before querying state.
     */
    void poll();

    /*
     * True if a gamepad is currently open and connected.
     */
    bool isConnected() const;

    // ============================================
    // Axis values — normalized to [-1.0, 1.0] with dead zone applied.
    // Left stick Y: negative = up (forward), positive = down (backward).
    // ============================================
    float getLeftStickX() const;
    float getLeftStickY() const;
    float getRightStickX() const;
    float getRightStickY() const;

    /*
     * Trigger values normalized to [0.0, 1.0] (0 = released, 1 = fully pressed).
     */
    float getLeftTrigger() const;
    float getRightTrigger() const;

    /*
     * True while the button is held down this frame.
     */
    bool isButtonHeld(SDL_GamepadButton button) const;

    /*
     * True only on the frame the button transitions from released to pressed.
     */
    bool isButtonJustPressed(SDL_GamepadButton button) const;

    // ============================================
    // Pure utility functions — public and static for testability.
    // ============================================

    /*
     * Normalize a raw Sint16 axis value to [-1.0, 1.0] with dead zone.
     * Values with magnitude below dead_zone are returned as 0.0.
     */
    static float normalizeAxis(Sint16 raw_value, float dead_zone = DEFAULT_DEAD_ZONE);

    /*
     * Normalize a raw Sint16 trigger value to [0.0, 1.0].
     * SDL3 triggers range from 0 to SDL_JOYSTICK_AXIS_MAX (32767).
     */
    static float normalizeTrigger(Sint16 raw_value);

  private:
    SDL_Gamepad* gamepad_;
    std::array<bool, SDL_GAMEPAD_BUTTON_COUNT> button_state_{};
    std::array<bool, SDL_GAMEPAD_BUTTON_COUNT> button_prev_state_{};

    void close();
};

// ============================================================================
// Inline implementations
// ============================================================================

inline GamepadInput::GamepadInput() : gamepad_(nullptr)
{
    button_state_.fill(false);
    button_prev_state_.fill(false);
}

inline GamepadInput::~GamepadInput()
{
    close();
}

inline void GamepadInput::close()
{
    if (gamepad_ != nullptr) {
        SDL_CloseGamepad(gamepad_);
        gamepad_ = nullptr;
    }
}

inline void GamepadInput::openFirstGamepad()
{
    if (gamepad_ != nullptr) {
        return; // already open
    }
    int count = 0;
    SDL_JoystickID* ids = SDL_GetGamepads(&count);
    if (ids != nullptr && count > 0) {
        gamepad_ = SDL_OpenGamepad(ids[0]);
    }
    SDL_free(ids);
}

inline void GamepadInput::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        if (gamepad_ == nullptr) {
            gamepad_ = SDL_OpenGamepad(event.gdevice.which);
        }
    } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        if (gamepad_ != nullptr && SDL_GetGamepadID(gamepad_) == event.gdevice.which) {
            close();
        }
    }
}

inline void GamepadInput::poll()
{
    button_prev_state_ = button_state_;
    if (gamepad_ == nullptr) {
        button_state_.fill(false);
        return;
    }
    for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i) {
        button_state_[static_cast<std::size_t>(i)] =
            SDL_GetGamepadButton(gamepad_, static_cast<SDL_GamepadButton>(i)) != 0;
    }
}

inline bool GamepadInput::isConnected() const
{
    return gamepad_ != nullptr;
}

inline float GamepadInput::getLeftStickX() const
{
    if (gamepad_ == nullptr) {
        return 0.0f;
    }
    return normalizeAxis(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFTX));
}

inline float GamepadInput::getLeftStickY() const
{
    if (gamepad_ == nullptr) {
        return 0.0f;
    }
    return normalizeAxis(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFTY));
}

inline float GamepadInput::getRightStickX() const
{
    if (gamepad_ == nullptr) {
        return 0.0f;
    }
    return normalizeAxis(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHTX));
}

inline float GamepadInput::getRightStickY() const
{
    if (gamepad_ == nullptr) {
        return 0.0f;
    }
    return normalizeAxis(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHTY));
}

inline float GamepadInput::getLeftTrigger() const
{
    if (gamepad_ == nullptr) {
        return 0.0f;
    }
    return normalizeTrigger(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
}

inline float GamepadInput::getRightTrigger() const
{
    if (gamepad_ == nullptr) {
        return 0.0f;
    }
    return normalizeTrigger(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
}

inline bool GamepadInput::isButtonHeld(SDL_GamepadButton button) const
{
    const int idx = static_cast<int>(button);
    if (idx < 0 || idx >= SDL_GAMEPAD_BUTTON_COUNT) {
        return false;
    }
    return button_state_[static_cast<std::size_t>(idx)];
}

inline bool GamepadInput::isButtonJustPressed(SDL_GamepadButton button) const
{
    const int idx = static_cast<int>(button);
    if (idx < 0 || idx >= SDL_GAMEPAD_BUTTON_COUNT) {
        return false;
    }
    return button_state_[static_cast<std::size_t>(idx)] && !button_prev_state_[static_cast<std::size_t>(idx)];
}

inline float GamepadInput::normalizeAxis(Sint16 raw_value, float dead_zone)
{
    // SDL3 axis range: -32768 to 32767.  Divide by 32767 for positive values
    // and by 32768 for negative values to get a symmetric [-1, 1] range.
    constexpr float AXIS_MAX = 32767.0f;
    constexpr float AXIS_MIN = 32768.0f;

    float normalized =
        (raw_value >= 0) ? (static_cast<float>(raw_value) / AXIS_MAX) : (static_cast<float>(raw_value) / AXIS_MIN);

    if (std::fabs(normalized) < dead_zone) {
        return 0.0f;
    }

    // Rescale so that dead_zone maps to 0 and 1.0 maps to 1.0
    float sign = (normalized > 0.0f) ? 1.0f : -1.0f;
    return sign * (std::fabs(normalized) - dead_zone) / (1.0f - dead_zone);
}

inline float GamepadInput::normalizeTrigger(Sint16 raw_value)
{
    // SDL3 triggers: 0 (released) to SDL_JOYSTICK_AXIS_MAX (32767 = fully pressed).
    constexpr float TRIGGER_MAX = 32767.0f;
    if (raw_value <= 0) {
        return 0.0f;
    }
    float result = static_cast<float>(raw_value) / TRIGGER_MAX;
    return result > 1.0f ? 1.0f : result;
}

#endif // PARTICLE_VIEWER_INPUT_GAMEPAD_INPUT_H
