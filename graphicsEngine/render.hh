#ifndef RENDER_H
#define RENDER_H

#include "raylib.h"
#include "physicsEngine/objects.h"

#define WINDOW_WIDTH  2000
#define WINDOW_HEIGHT 1500

namespace graphics_engine
{

class Renderer
{
    int   screenWidth_;
    int   screenHeight_;
    float pixelsPerMeter_;
    float particleRadius_;

public:
    Renderer(int width, int height, float pixelsPerMeter);
    ~Renderer();

    bool windowShouldClose();
    void beginFrame();
    void endFrame();
    void drawWorld(physics_engine::World& world);

private:
    Vector2 toScreen(const generalFunctions::Vector2d& coord);
};

}

#endif /* RENDER_H */