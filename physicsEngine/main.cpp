#include <math.h>
#include <chrono>
#include <iostream>

#include "objects.h"
#include "integrator.h"
#include "physicsConst.h"
#include "graphicsEngine/render.hh"

using namespace physics_engine;
using namespace graphics_engine;

int main(){
    class World defaultWorld;

    auto gravity = std::make_shared<Field>( MathVector<m_s2_t, CUR_SIM_DIM>{0, -DEFAULT_GRAVITY_ACCEL});
    defaultWorld.getFields().push_back(gravity);

    auto firstParticle = std::make_shared<Particle>( MathVector<m_t,    CUR_SIM_DIM>{-10, 10},
                                                     MathVector<m_s_t,  CUR_SIM_DIM>{sqrt(3) / 2, 0.5},
                                                     MathVector<m_s2_t, CUR_SIM_DIM>{0, 0},
                                                     1, 
                                                     &defaultWorld);

    auto secondParticle = std::make_shared<Particle> ( MathVector<m_t,    CUR_SIM_DIM>{-15, 5},
                                                       MathVector<m_s_t,  CUR_SIM_DIM>{10, 10},
                                                       MathVector<m_s2_t, CUR_SIM_DIM>{0, 0},
                                                       3, 
                                                       &defaultWorld);

    defaultWorld.getObjects().push_back(firstParticle);
    defaultWorld.getObjects().push_back(secondParticle);

    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT, 50.0f);

    auto lastTime = std::chrono::high_resolution_clock::now();

    while( !renderer.windowShouldClose() && defaultWorld.getTime() < 10 ){
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        while( elapsedTime.count() >= BASIC_SIMULATION_STEP )
        {
            defaultWorld.update( BASIC_SIMULATION_STEP);

            for (const auto& obj : defaultWorld.getObjects()){
                obj->update( BASIC_SIMULATION_STEP);
            }

            defaultWorld.increaseTime( BASIC_SIMULATION_STEP);
            elapsedTime -= std::chrono::duration<double>(BASIC_SIMULATION_STEP);
        }

        renderer.beginFrame();
        renderer.drawWorld(defaultWorld);
        renderer.endFrame();
    }

    return 0;
}