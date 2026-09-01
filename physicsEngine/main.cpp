#include <stdio.h>
#include <math.h>

#include "objects.h"
#include "integrator.h"
#include "physicsConst.h"

using namespace physics_engine; 

int main( ){
    class World defaultWorld;

    auto gravity = std::make_shared<Field>(DEFAULT_GRAVITY_ACCEL);
    defaultWorld.getFields().push_back(gravity);

    auto firstParticle = std::make_shared<Particle>(0, 0, sqrt(3) / 2, 0.5, 0, 0, 1);
    defaultWorld.getObjects().push_back(firstParticle);
}