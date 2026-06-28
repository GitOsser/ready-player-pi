#pragma once
#include <cmath>
#include "GyroPi.h"

class BlackHole {
public:
    BlackHole(Vec2 position, float radius)
        : position_(position), radius_(radius) {}

    bool contains(Vec2 point) const {
        float deltaX   = point.x - position_.x;
        float deltaY   = point.y - position_.y;
        float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
        return distance < radius_;
    }

    Vec2  getPosition() const { return position_; }
    float getRadius()   const { return radius_;   }

private:
    Vec2  position_;
    float radius_;
};

