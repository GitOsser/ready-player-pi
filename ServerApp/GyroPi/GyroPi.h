#pragma once
#include <mutex>

struct Vec2 { float x, y; };

static constexpr int   COURSE_WIDTH  = 800;
static constexpr int   COURSE_HEIGHT = 480;
static constexpr float BALL_SPEED    = 3.0f;
static constexpr int   TICK_MS       = 33;
static constexpr float GOAL_RADIUS   = 30.0f;

struct GameState {
    std::mutex mutex;
    float      tiltX  = 0;
    float      tiltY  = 0;
    int        score  = 0;
    bool       active = false;
};

