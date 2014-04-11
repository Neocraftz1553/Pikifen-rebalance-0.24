#ifndef LAFI_STYLE_INCLUDED
#define LAFI_STYLE_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"

class lafi_style {
public:
    ALLEGRO_COLOR bg_color;
    ALLEGRO_COLOR lighter_bg_color;
    ALLEGRO_COLOR darker_bg_color;
    
    ALLEGRO_COLOR fg_color;
    ALLEGRO_COLOR alt_color; //Used, for instance, as a background in selected textbox text.
    
    ALLEGRO_COLOR disabled_bg_color;
    ALLEGRO_COLOR lighter_disabled_bg_color;
    ALLEGRO_COLOR darker_disabled_bg_color;
    
    ALLEGRO_COLOR disabled_fg_color;
    ALLEGRO_COLOR disabled_alt_color;
    
    ALLEGRO_FONT* text_font;
    
    lafi_style(
        ALLEGRO_COLOR bg_color = al_map_rgb(LAFI_DEF_STYLE_BG_R, LAFI_DEF_STYLE_BG_G, LAFI_DEF_STYLE_BG_B),
        ALLEGRO_COLOR fg_color = al_map_rgb(LAFI_DEF_STYLE_FG_R, LAFI_DEF_STYLE_FG_G, LAFI_DEF_STYLE_FG_B),
        ALLEGRO_COLOR alt_color = al_map_rgb(LAFI_DEF_STYLE_ALT_R, LAFI_DEF_STYLE_ALT_G, LAFI_DEF_STYLE_ALT_B),
        ALLEGRO_FONT* text_font = al_create_builtin_font()
    );
    lafi_style(lafi_style &s2);
    ~lafi_style();
    
    static ALLEGRO_COLOR lighten_color(ALLEGRO_COLOR color);
    static ALLEGRO_COLOR darken_color(ALLEGRO_COLOR color);
};

#endif //ifndef LAFI_STYLE_INCLUDED
