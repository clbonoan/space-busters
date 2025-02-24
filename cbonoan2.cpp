#include "fonts.h"

void show_christine(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Christine - Unknown");
}

void show_instructions(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "WASD to move");
    ggprint8b(r, 16, 0x00ff00ff, "Shift to boost");
    ggprint8b(r, 16, 0x00ff00ff, "Q to turn left");
    ggprint8b(r, 16, 0x00ff00ff, "E to turn right");
    ggprint8b(r, 16, 0x00ff00ff, "Space to shoot");
}
