/*
 * main.cpp
 *
 * Application entry point.
 * Creates a GLFWContext and injects it into ViewerApp (dependency injection).
 *
 * Command-line flags:
 *   --resolution, --res <resolution>  Set display resolution (4k, 1080, 720)
 *   --debug-camera, -d                Enable debug camera overlay showing position,
 *                                     target, up vector, projection info, and viewport
 */

#include "graphics/GLFWContext.hpp"
#include "viewer_app.hpp"

namespace
{

/*
 * Parse resolution from command-line arguments.
 * Returns the resolution string (e.g., "4k", "1080") or empty for default 720p.
 */
std::string parseResolution(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if ((arg == "--resolution" || arg == "--res") && i + 1 < argc) {
            return argv[i + 1];
        }
    }
    return "";
}

/*
 * Get window dimensions for a resolution string.
 */
void getWindowSize(const std::string& resolution, int& width, int& height)
{
    if (resolution == "4k") {
        width = 3840;
        height = 2160;
    } else if (resolution == "1080" || resolution == "1080p" || resolution == "HD") {
        width = 1920;
        height = 1080;
    } else {
        width = 1280;
        height = 720;
    }
}

} // namespace

int main(int argc, char* argv[])
{
    // Parse resolution early so we can create the context with the right size
    std::string resolution = parseResolution(argc, argv);
    int width = 1280;
    int height = 720;
    getWindowSize(resolution, width, height);

    // Create the OpenGL context (GLFW window + GLAD initialization)
    GLFWContext context(width, height, "Particle-Viewer");
    if (!context.isValid()) {
        return 1;
    }

    // Inject context into ViewerApp
    ViewerApp app(&context);
    app.parseArgs(argc, argv);
    if (app.initialize()) {
        app.run();
    }
    return 0;
}
