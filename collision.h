#pragma once
#include<SDL.h>
class Collision {
public:
    static bool checkCollision(const SDL_Rect& rec1, const SDL_Rect& rec2) {
        int left1 = rec1.x+6;
        int right1 = rec1.x + rec1.w-6;
        int top1 = rec1.y+3;
        int bottom1 = rec1.y + rec1.h-6;

        int left2 = rec2.x;
        int right2 = rec2.x + rec2.w;
        int top2 = rec2.y;
        int bottom2 = rec2.y + rec2.h;

        if (bottom1 <= top2 || top1 >= bottom2 || right1 <= left2 || left1 >= right2) {
            return false;
        }
        else {
            return true;
        }
    }
};
