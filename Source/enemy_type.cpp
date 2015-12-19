/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy type class and enemy type-related functions.
 */

#include "enemy_type.h"
#include "functions.h"

void enemy_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    drops_corpse = s2b(file->get_child_by_name("drops_corpse")->get_value_or_default("yes"));
    is_boss = s2b(file->get_child_by_name("is_boss")->value);
    pikmin_seeds = s2i(file->get_child_by_name("pikmin_seeds")->value);
    regenerate_speed = s2b(file->get_child_by_name("regenerate_speed")->value);
    revive_speed = s2f(file->get_child_by_name("revive_speed")->value);
    value = s2f(file->get_child_by_name("value")->value);
}