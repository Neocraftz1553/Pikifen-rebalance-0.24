/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the in-game HUD class and
 * in-game HUD-related functions.
 */

#ifndef HUD_INCLUDED
#define HUD_INCLUDED


#include "../../gui.h"
#include "hud_bubble_manager.h"


namespace HUD {
extern const float SUN_METER_SUN_SPIN_SPEED;
}


enum STANDBY_TYPE_RELATIONS {
    STANDBY_TYPE_PREVIOUS = 0,
    STANDBY_TYPE_CURRENT = 1,
    STANDBY_TYPE_NEXT = 2,
};


/* ----------------------------------------------------------------------------
 * Holds information about the in-game HUD.
 */
struct hud_struct {
    static const string HUD_FILE_NAME;
    static const float LEADER_SWAP_JUICE_DURATION;
    static const float STANDBY_SWAP_JUICE_DURATION;
    
    struct leader_icon_bubble {
        ALLEGRO_BITMAP* bmp;
        ALLEGRO_COLOR color;
    };
    struct leader_health_bubble {
        float ratio;
        float caution_timer;
    };
    
    //GUI manager.
    gui_manager gui;
    //Bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_bubble;
    //Group counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_group;
    //Field counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_field;
    //Standby counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_standby;
    //Total counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_total;
    //Day counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_day_bubble;
    //Distant Pikmin marker graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_distant_pikmin_marker;
    //Hard bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_hard_bubble;
    //No Pikmin marker graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_no_pikmin_bubble;
    //Sun icon graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_sun;
    //Bubble manager for leader icon items.
    hud_bubble_manager<leader_icon_bubble> leader_icon_mgr;
    //Bubble manager for leader health items.
    hud_bubble_manager<leader_health_bubble> leader_health_mgr;
    //Bubble manager for the standby type.
    hud_bubble_manager<ALLEGRO_BITMAP*> standby_icon_mgr;
    //Spray 1 amount text. Cache for convenience.
    gui_item* spray_1_amount;
    //Spray 2 amount text. Cache for convenience.
    gui_item* spray_2_amount;
    //Current standby count.
    size_t standby_count_nr;
    //Standby count text. Cache for convenience.
    gui_item* standby_amount;
    //Current group count.
    size_t group_count_nr;
    //Group count text. Cache for convenience.
    gui_item* group_amount;
    //Current field count.
    size_t field_count_nr;
    //Field count text. Cache for convenience.
    gui_item* field_amount;
    //Current total count.
    size_t total_count_nr;
    //Total count text. Cache for convenience.
    gui_item* total_amount;
    
    hud_struct();
    ~hud_struct();
    void tick(const float delta_t);
    
private:
    void draw_standby_icon(STANDBY_TYPE_RELATIONS which);
};



#endif //ifndef HUD_INCLUDED
