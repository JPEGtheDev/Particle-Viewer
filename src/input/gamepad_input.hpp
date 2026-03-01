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
 *   gamepad.handleEvent(event);          // handles GAMEPAD_ADDED/REMOVED and
 *                                        // JOYSTICK_ADDED fallback for unmapped devices
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
#include <string>

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
     * Also handles JOYSTICK_ADDED as a fallback for gamepad-type devices
     * not yet in SDL3's mapping database (e.g. 8BitDo 2.4GHz on non-SteamOS).
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

    /*
     * Scan joystick devices for gamepad-type hardware that SDL3 hasn't mapped
     * yet (e.g. 8BitDo 2.4GHz dongle).  For each, add a generic XInput layout
     * and open the first one as a gamepad.
     */
    void tryOpenFirstJoystickAsGamepad();

    /*
     * Add a standard XInput-compatible gamepad mapping for a joystick device
     * that SDL3 has not yet mapped.  Uses the device's GUID so the mapping
     * applies only to this specific hardware.
     */
    void addXInputMapping(SDL_JoystickID instance_id);
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

    // Primary: devices SDL3 already has in its gamepad mapping database.
    int count = 0;
    SDL_JoystickID* ids = SDL_GetGamepads(&count);
    if (ids != nullptr && count > 0) {
        gamepad_ = SDL_OpenGamepad(ids[0]);
    }
    SDL_free(ids);

    if (gamepad_ != nullptr) {
        return;
    }

    // Fallback: scan joystick devices for gamepad-type hardware that SDL3
    // hasn't mapped yet (e.g. 8BitDo 2.4GHz dongle on non-SteamOS Linux).
    tryOpenFirstJoystickAsGamepad();
}

inline void GamepadInput::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        if (gamepad_ == nullptr) {
            gamepad_ = SDL_OpenGamepad(event.gdevice.which);
        }
    } else if (event.type == SDL_EVENT_JOYSTICK_ADDED) {
        // Fallback: a joystick arrived that SDL3 may not have a mapping for.
        // If it is HID-type gamepad (e.g. 8BitDo 2.4GHz), add a generic XInput
        // mapping so the SDL3 gamepad API can drive it.
        if (gamepad_ == nullptr) {
            if (SDL_IsGamepad(event.jdevice.which)) {
                // SDL3 recognises it as a gamepad — open directly.
                gamepad_ = SDL_OpenGamepad(event.jdevice.which);
            } else if (SDL_GetJoystickTypeForID(event.jdevice.which) == SDL_JOYSTICK_TYPE_GAMEPAD) {
                addXInputMapping(event.jdevice.which);
                if (SDL_IsGamepad(event.jdevice.which)) {
                    gamepad_ = SDL_OpenGamepad(event.jdevice.which);
                }
            }
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

inline void GamepadInput::tryOpenFirstJoystickAsGamepad()
{
    int joy_count = 0;
    SDL_JoystickID* joy_ids = SDL_GetJoysticks(&joy_count);
    if (joy_ids == nullptr) {
        return;
    }
    for (int i = 0; i < joy_count && gamepad_ == nullptr; ++i) {
        if (SDL_IsGamepad(joy_ids[i])) {
            // SDL3 has a mapping for this device but SDL_GetGamepads() missed it —
            // open it directly.
            gamepad_ = SDL_OpenGamepad(joy_ids[i]);
        } else if (SDL_GetJoystickTypeForID(joy_ids[i]) == SDL_JOYSTICK_TYPE_GAMEPAD) {
            // HID class identifies this as a gamepad but SDL3 has no mapping
            // (e.g. 8BitDo 2.4GHz dongle on non-SteamOS Linux).  Inject a
            // generic XInput-compatible layout and open it.
            addXInputMapping(joy_ids[i]);
            if (SDL_IsGamepad(joy_ids[i])) {
                gamepad_ = SDL_OpenGamepad(joy_ids[i]);
            }
        }
    }
    SDL_free(joy_ids);
}

inline void GamepadInput::addXInputMapping(SDL_JoystickID instance_id)
{
    char guid_str[33];
    SDL_GUIDToString(SDL_GetJoystickGUIDForID(instance_id), guid_str, sizeof(guid_str));

    // Standard XInput-compatible gamepad layout.  Covers 8BitDo controllers
    // in X-Input mode and most other modern USB/2.4GHz gamepads:
    //   Face:     A=b0  B=b1  X=b2  Y=b3
    //   Bumpers:  LB=b4 RB=b5
    //   Menu:     Back=b6  Start=b7  Guide=b8
    //   Sticks:   LS=b9  RS=b10
    //   Axes:     LX=a0 LY=a1  LT=a2  RX=a3 RY=a4  RT=a5
    //   D-pad:    hat0
    std::string mapping = guid_str;
    mapping += ",Gamepad,";
    mapping += "a:b0,b:b1,x:b2,y:b3,";
    mapping += "back:b6,guide:b8,start:b7,";
    mapping += "leftshoulder:b4,rightshoulder:b5,";
    mapping += "leftstick:b9,rightstick:b10,";
    mapping += "leftx:a0,lefty:a1,rightx:a3,righty:a4,";
    mapping += "lefttrigger:a2,righttrigger:a5,";
    mapping += "dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,";
    mapping += "platform:";
    mapping += SDL_GetPlatform();

    SDL_AddGamepadMapping(mapping.c_str());
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
