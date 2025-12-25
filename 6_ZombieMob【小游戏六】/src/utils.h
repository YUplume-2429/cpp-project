#pragma once

struct Point {
    float x {0.0f};
    float y {0.0f};
    Point() = default;
    Point(float xx, float yy) : x(xx), y(yy) {}
};

class MySprite; // fwd

Point calcVelocity(int direction, float vel = 1.0f);
void reverseDirection(MySprite* sprite);
