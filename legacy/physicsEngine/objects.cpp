#include "objects.h"

namespace physics_engine
{

void
Object::update(s_t dt)
{
    velocity_ += (world_->getWorldAccel() + accel_)  * dt;
    coord_ += velocity_ * dt;
}

}
