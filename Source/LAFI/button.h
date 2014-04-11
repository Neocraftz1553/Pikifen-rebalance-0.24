#ifndef LAFI_BUTTON_INCLUDED
#define LAFI_BUTTON_INCLUDED

#include <string>

using namespace std;

#include "widget.h"

class lafi_button : public lafi_widget {
public:
    string text;
    ALLEGRO_BITMAP* icon;
    
    lafi_button(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, string text = "", string description = "", ALLEGRO_BITMAP* icon = NULL, lafi_style* style = NULL, unsigned char flags = 0);
    ~lafi_button();
    
    void draw_self();
};

#endif //ifndef LAFI_BUTTON_INCLUDED