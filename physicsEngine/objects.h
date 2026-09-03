#ifndef OBJECTS_H
#define OBJECTS_H

#include <vector>
#include <memory>
#include <iostream>

#include "physicsConst.h"

#include "generalFunctions/mathVector.h"

using namespace generalFunctions;

namespace physics_engine
{

class Field
{
    Vector2d accel_;
public:
    Field( Vector2d accel) : accel_(accel) {}

    Vector2d
    getAccel()
    {
        return accel_;
    }
};

class World;

class Object
{
    MathVector<m_t, CUR_SIM_DIM> coord_;
    MathVector<m_s_t, CUR_SIM_DIM> velocity_;
    MathVector<m_s2_t, CUR_SIM_DIM> accel_;

    kg_t mass_;

    World* world_; 

public:
    Object(
        const MathVector<m_t, CUR_SIM_DIM>& coord,
        const MathVector<m_s_t, CUR_SIM_DIM>& velocity,
        const MathVector<m_s2_t, CUR_SIM_DIM>& accel,
        kg_t mass,
        World* world
    )
        : coord_(coord),
          velocity_(velocity),
          accel_(accel),
          mass_(mass),
          world_(world)
    {}

    virtual ~Object() = default;

    virtual void update(s_t dt);

    virtual void print()
    {
        std::cout << "Object State:\n"
                  << "  Position:  " << coord_ << '\n'
                  << "  Velocity:  " << velocity_ << '\n'
                  << "  Accel:     " << accel_ << '\n'
                  << "  Mass:      " << mass_ << '\n'
                  << "---------------------------\n";
    }

    World*
    getWorld(){
        return world_;
    }

    const MathVector<m_t, CUR_SIM_DIM>&
    getCoord() const
    {
        return coord_;
    }
};

class Particle : public Object
{
public:
    Particle(
        const MathVector<m_t, CUR_SIM_DIM>& coord,
        const MathVector<m_s_t, CUR_SIM_DIM>& velocity,
        const MathVector<m_s2_t, CUR_SIM_DIM>& accel,
        kg_t mass,
        World* world
    )
        : Object(coord, velocity, accel, mass, world)
    {}
};

class World
{
    std::vector<std::shared_ptr<Field>>  fields;
    std::vector<std::shared_ptr<Object>> objects;
    s_t                                  time_ = 0;
    Vector2d                             worldAccel_;

public:
    std::vector<std::shared_ptr<Field>>&
    getFields(){
        return fields;
    };

    std::vector<std::shared_ptr<Object>>&
    getObjects(){
        return objects;
    };

    s_t 
    getTime(){
        return time_;
    };

    Vector2d
    getWorldAccel()
    {
        return worldAccel_;
    }

    void 
    increaseTime( s_t timeShift){
        time_ += timeShift;
    }

    void 
    update( s_t dt){
        worldAccel_ = {0, 0};
        for( auto& field: fields)
        {
            worldAccel_ += field->getAccel();
        }
    }
};

}

#endif /* OBJECTS_H */