#include <math.h>
#include <iostream>

#include "graphicsEngine/render.hh"

#include "physicsEngine/objects.h"
#include "physicsEngine/physicsConst.h"

#include "measurmentSystem.h"
#include "simulation.h"
#include "config.h"

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

    defaultWorld.getObjects().push_back( firstParticle);
    defaultWorld.getObjects().push_back( secondParticle);

    // Renderer renderer( WINDOW_WIDTH, WINDOW_HEIGHT, 50.0f);

    class simulation::Simulator simulation;
    simulation.run( &defaultWorld, SIMULATION_TIME);

    return 0;
}