#pragma once
#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H

#include <iostream>
#include <GLFW/glfw3.h>

class FPSCounter {
private:
    double startTime;
    int frameCount;

public:
    FPSCounter();
    void update();
};

#endif // FPSCOUNTER_H
