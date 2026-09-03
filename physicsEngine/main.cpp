#include <math.h>
#include <chrono>

#include "objects.h"
#include "integrator.h"
#include "physicsConst.h"

using namespace physics_engine; 

int main(){
    class World defaultWorld;

    auto gravity = std::make_shared<Field>(DEFAULT_GRAVITY_ACCEL);
    defaultWorld.getFields().push_back(gravity);

    auto firstParticle = std::make_shared<Particle>(0, 0, sqrt(3) / 2, 0.5, 0, 0, 1);
    defaultWorld.getObjects().push_back(firstParticle);

    auto lastTime = std::chrono::high_resolution_clock::now();
    while( true ){
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        defaultWorld.increaseTime( elapsedTime.count());

        while( elapsedTime.count() >= BASIC_SIMULATION_STEP )
        {
            for ( auto* obj : defaultWorld.getObjects()){
                obj->update(BASIC_SIMULATION_STEP);
            }
        }
    }
}