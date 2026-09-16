#include "render.hh"

namespace graphics_engine
{

Renderer::Renderer(int width, int height, float pixelsPerMeter)
    : screenWidth_(width), screenHeight_(height),
      pixelsPerMeter_(pixelsPerMeter)
{
    InitWindow(screenWidth_, screenHeight_, "WORLD");
    SetTargetFPS(60);
}

Renderer::~Renderer()
{
    CloseWindow();
}

bool
Renderer::windowShouldClose()
{
    return WindowShouldClose();
}

void
Renderer::beginFrame()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

void
Renderer::endFrame()
{
    EndDrawing();
}

Vector2
Renderer::toScreen(const generalFunctions::Vector2d& coord)
{
    Vector2 screenPos;
    screenPos.x = screenWidth_  / 2.0f + (float)coord[0] * pixelsPerMeter_;
    screenPos.y = screenHeight_ / 2.0f - (float)coord[1] * pixelsPerMeter_;
    return screenPos;
}

void
Renderer::drawWorld(physics_engine::World& world)
{
    for (const auto& obj : world.getObjects())
    {
        Vector2 pos = toScreen(obj->getCoord());
        DrawCircleV(pos, PARTICLE_RADIUS, RED);
    }
}

}