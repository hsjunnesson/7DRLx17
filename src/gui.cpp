#include "gui.h"

namespace gui {
Gui::Gui(Allocator &allocator)
: allocator(allocator) {}

void update(Gui &gui, uint32_t t, double dt) {
    (void)gui;
    (void)t;
    (void)dt;
}

void render(Gui &gui, SDL_Renderer *renderer) {
    (void)gui;
    (void)renderer;
}

} // namespace gui