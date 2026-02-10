/*
 * main.cpp
 *
 * Application entry point.
 * Creates and runs the ViewerApp which manages the entire rendering pipeline.
 *
 * Command-line flags:
 *   --resolution, --res <resolution>  Set display resolution (4k, 1080, 720)
 *   --debug-camera, -d                Enable debug camera overlay showing position,
 *                                     target, up vector, projection info, and viewport
 */

#include "viewer_app.hpp"

int main(int argc, char* argv[])
{
    ViewerApp app;
    app.parseArgs(argc, argv);
    if (app.initialize()) {
        app.run();
    }
    return 0;
}
