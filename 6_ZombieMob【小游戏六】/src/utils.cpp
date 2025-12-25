#include "utils.h"
#include "mysprite.h"

Point calcVelocity(int direction, float vel) {
    Point v{0.f, 0.f};
    if (direction == 0) {
        v.y = -vel; // north
    } else if (direction == 2) {
        v.x = vel; // east
    } else if (direction == 4) {
        v.y = vel; // south
    } else if (direction == 6) {
        v.x = -vel; // west
    }
    return v;
}

void reverseDirection(MySprite* sprite) {
    if (!sprite) return;
    if (sprite->direction == 0) sprite->direction = 4;
    else if (sprite->direction == 2) sprite->direction = 6;
    else if (sprite->direction == 4) sprite->direction = 0;
    else if (sprite->direction == 6) sprite->direction = 2;
}
