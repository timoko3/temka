#include <math.h>
#include <chrono>
#include <iostream>

#include "objects.h"
#include "integrator.h"
#include "physicsConst.h"

using namespace physics_engine; 

int main(){
    class World defaultWorld;

    auto gravity = std::make_shared<Field>( MathVector<m_s2_t, CUR_SIM_DIM>{0, -DEFAULT_GRAVITY_ACCEL});
    defaultWorld.getFields().push_back(gravity);

    auto firstParticle = std::make_shared<Particle>( MathVector<m_t, CUR_SIM_DIM>{0, 0},
                                                     MathVector<m_s_t, CUR_SIM_DIM>{sqrt(3) / 2, 0.5},
                                                     MathVector<m_s2_t, CUR_SIM_DIM>{0, 0},
                                                     1, 
                                                     &defaultWorld);

    // std::cout << firstParticle->getCoord() << "\n";

    defaultWorld.getObjects().push_back(firstParticle);

    auto lastTime = std::chrono::high_resolution_clock::now();
    while( defaultWorld.getTime() < 10 ){
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        while( elapsedTime.count() >= BASIC_SIMULATION_STEP )
        {
            defaultWorld.update( BASIC_SIMULATION_STEP);

            // std::cout << "MEOW";
            for (const auto& obj : defaultWorld.getObjects()){
                obj->update( BASIC_SIMULATION_STEP);
                obj->print();
            }

            defaultWorld.increaseTime( BASIC_SIMULATION_STEP);
        }
    }
}