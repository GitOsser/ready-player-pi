#pragma once
#include <algorithm>
#include "GyroPi.h"

class Ball {
public:
    explicit Ball(Vec2 startPosition)
        : position_(startPosition), startPosition_(startPosition) {}

    void applyTilt(float tiltX, float tiltY) {
        position_.x -= tiltX * BALL_SPEED;
        position_.y -= tiltY * BALL_SPEED;
        clampToBounds();
    }

    void resetToStart() {
        position_ = startPosition_;
    }

    Vec2 getPosition()      const { return position_;      }
    Vec2 getStartPosition() const { return startPosition_; }

private:
    Vec2 position_;
    Vec2 startPosition_;

    void clampToBounds() {
        position_.x = std::max(0.f, std::min(static_cast<float>(COURSE_WIDTH),  position_.x));
        position_.y = std::max(0.f, std::min(static_cast<float>(COURSE_HEIGHT), position_.y));
    }
};

