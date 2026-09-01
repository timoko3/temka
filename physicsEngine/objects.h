#ifndef OBJECTS_H
#define OBJECTS_H

#include <vector>
#include <memory>

#include "physicsConst.h"

namespace physics_engine
{
class Field
{
    double fieldStrength;

public:
    Field( double strength){ fieldStrength = strength; }
};

class Object
{
    m_t x_, y_;
    m_s_t velocityX_, velocityY_;
    m_s2_t accelX_, accelY_;  
    kg_t mass_;
public:
    Object(m_t x,  m_t y,
           m_s_t vX, m_s_t vY,
           m_s2_t aX, m_s2_t aY,
           kg_t mass) 
        : x_(x), y_(y),
          velocityX_(vX), velocityY_(vY),
          accelX_(aX), accelY_(aY),
          mass_(mass) {}

    virtual ~Object() = default;
};

class Particle : public Object
{
public:
    Particle(m_t x,  m_t y,
             m_s_t vX, m_s_t vY,
             m_s2_t aX, m_s2_t aY,
             kg_t mass) 
        : Object(x, y, 
                 vX, vY, 
                 aX, aY, 
                 mass) {}

};

class World
{
    std::vector<std::shared_ptr<Field>>  fields;
    std::vector<std::shared_ptr<Object>> objects;
    double              time_;

public:
    std::vector<std::shared_ptr<Field>>&
    getFields(){
        return fields;
    };

    std::vector<std::shared_ptr<Object>>&
    getObjects(){
        return objects;
    };
};

}

#endif /* OBJECTS_H */