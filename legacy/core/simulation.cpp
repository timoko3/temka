#include <chrono>

#include "simulation.h"
#include "config.h"

namespace simulation
{
 
void 
Simulator::run( physics_engine::World* world, double simTime)
{
    // Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT, 50.0f);

    auto lastTime = std::chrono::high_resolution_clock::now();

    while( /* !renderer.windowShouldClose() && */ world->getTime() < simTime ){
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        while( elapsedTime.count() >= BASIC_SIMULATION_STEP )
        {
            world->update( BASIC_SIMULATION_STEP);

            for (const auto& obj : world->getObjects()){
                obj->update( BASIC_SIMULATION_STEP);
            }

            world->increaseTime( BASIC_SIMULATION_STEP);
            elapsedTime -= std::chrono::duration<double>(BASIC_SIMULATION_STEP);
        }

        // renderer.beginFrame();
        // renderer.drawWorld(world);
        // renderer.endFrame();
    }   
}

}