#include "FPSCounter.h"

FPSCounter::FPSCounter() : startTime(glfwGetTime()), frameCount(0) {}

void FPSCounter::update() {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - startTime;

    // Update FPS every second
    if (deltaTime >= 1.0) {
        int fps = static_cast<int>(frameCount / deltaTime);
        std::cout << "FPS: " << fps << std::endl;

        // Reset for the next second
        startTime = currentTime;
        frameCount = 0;
    }

    frameCount++;
}