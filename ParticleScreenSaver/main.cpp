#include "Application.h"

int main(void) {
    Application application("Particle Simulation", 1000000, 1.01f, 1.15f, IntVector2(800, 800));
    application.run();

    return 0;
}
