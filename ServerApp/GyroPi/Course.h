#pragma once
#include <vector>
#include <cmath>
#include "GyroPi.h"
#include "BlackHole.h"

class Course {
public:

    Course()
        : start_({50, 50}),
          goal_({700, 400}),
          holes_({
              BlackHole({300, 200}, 40),
              BlackHole({500, 100}, 35),
              BlackHole({400, 350}, 45)
          })
    {}

    bool isGoalReached(Vec2 ballPosition) const {
        float deltaX            = ballPosition.x - goal_.x;
        float deltaY            = ballPosition.y - goal_.y;
        float distanceToGoal    = std::sqrt(deltaX * deltaX + deltaY * deltaY);
        return distanceToGoal < GOAL_RADIUS;
    }

    Vec2                           getStart() const { return start_; }
    Vec2                           getGoal()  const { return goal_;  }
    const std::vector<BlackHole> & getHoles() const { return holes_; }

private:
    Vec2                   start_;
    Vec2                   goal_;
    std::vector<BlackHole> holes_;
};

