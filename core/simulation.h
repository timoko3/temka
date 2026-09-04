#ifndef SIMULATION_H
#define SIMULATION_H

#include "physicsEngine/objects.h"

#include "measurmentSystem.h"

namespace simulation
{

class Simulator
{
public:
    void run( physics_engine::World* world, s_t simTime);
};

} 


#endif /* SIMULATION_H */