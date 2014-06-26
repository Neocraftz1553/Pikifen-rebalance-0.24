/*
 * Copyright (c) Andr� 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally accessible functions.
 */

#define _USE_MATH_DEFINES

#pragma warning(disable : 4996) //Disables warning about localtime being deprecated.

#include <algorithm>
#include <math.h>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "data_file.h"
#include "drawing.h"
#include "functions.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Returns the vector coordinates of an angle.
 * angle:    The angle.
 * magnitue: Its magnitude.
 * *_coord:  Variables to return the coordinates to.
 */
void angle_to_coordinates(const float angle, const float magnitude, float* x_coord, float* y_coord) {
    *x_coord = cos(angle) * magnitude;
    *y_coord = sin(angle) * magnitude;
}

/* ----------------------------------------------------------------------------
 * Returns the color that was provided, but with the alpha changed.
 * color: The color to change the alpha on.
 * a:     The new alpha, [0-255].
 */
ALLEGRO_COLOR change_alpha(const ALLEGRO_COLOR c, const unsigned char a) {
    ALLEGRO_COLOR c2;
    c2.r = c.r; c2.g = c.g; c2.b = c.b;
    c2.a = a / 255.0;
    return c2;
}

/* ----------------------------------------------------------------------------
 * Returns whether the distance between two points is smaller
 * or equal to the comparison distance. This function does not
 * use sqrt, so it's faster.
 */
bool check_dist(float x1, float y1, float x2, float y2, float distance_to_check) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return (dx * dx + dy * dy) <= distance_to_check * distance_to_check;
}

/* ----------------------------------------------------------------------------
 * Returns whether a circle is touching a line or not.
 * cx, cy: Coordinates of the circle.
 * cr:     Radius of the circle.
 * x*, y*: Coordinates of the line.
 * li*:    If not NULL, the line intersection coordinates are returned here.
 */
bool circle_intersects_line(float cx, float cy, float cr, float x1, float y1, float x2, float y2, float* lix, float* liy) {

    //Code by http://www.melloland.com/scripts-and-tutos/collision-detection-between-circles-and-lines
    
    float vx = x2 - x1;
    float vy = y2 - y1;
    float xdiff = x1 - cx;
    float ydiff = y1 - cy;
    float a = vx * vx + vy * vy;
    float b = 2 * ((vx * xdiff) + (vy * ydiff));
    float c = xdiff * xdiff + ydiff * ydiff - cr * cr;
    float quad = b * b - (4 * a * c);
    if (quad >= 0) {
        // An infinite collision is happening, but let's not stop here
        float quadsqrt = sqrt(quad);
        for (int i = -1; i <= 1; i += 2) {
            // Returns the two coordinates of the intersection points
            float t = (i * -b + quadsqrt) / (2 * a);
            float x = x1 + (i * vx * t);
            float y = y1 + (i * vy * t);
            // If one of them is in the boundaries of the segment, it collides
            if (x >= min(x1, x2) && x <= max(x1, x2) && y >= min(y1, y2) && y <= max(y1, y2)) {
                if(lix) *lix = x;
                if(liy) *liy = y;
                return true;
            }
        }
    }
    return false;
}

/* ----------------------------------------------------------------------------
 * Clears the textures of the area's sectors from memory.
 */
void clear_area_textures() {
    for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        sector* s_ptr = cur_area_map.sectors[s];
        if(s_ptr->bitmap && s_ptr->bitmap != bmp_error) {
            bitmaps.detach("Textures/" + s_ptr->file_name);
        }
    }
}

/* ----------------------------------------------------------------------------
 * Returns the angle and magnitude of vector coordinates.
 * *_coord:   The coordinates.
 * angle:     Variable to return the angle to.
 * magnitude: Variable to return the magnitude to.
 */
void coordinates_to_angle(const float x_coord, const float y_coord, float* angle, float* magnitude) {
    *angle = atan2(y_coord, x_coord);
    *magnitude = dist(0, 0, x_coord, y_coord);
}

/* ----------------------------------------------------------------------------
 * Prints something onto the error log.
 * s: String that represents the error.
 * d: If not null, this will be used to obtain the file_name and line that caused the error.
 */
void error_log(string s, data_node* d) {
    if(d) {
        s += " (" + d->file_name;
        if(d->line_nr != 0) s += " line " + itos(d->line_nr);
        s += ")";
    }
    s += "\n";
    
    if(no_error_logs_today) {
        no_error_logs_today = false;
        time_t tt;
        time(&tt);
        struct tm t = *localtime(&tt);
        s =
            "\n" +
            itos(t.tm_year + 1900) + "/" +
            leading_zero(t.tm_mon + 1) + "/" +
            leading_zero(t.tm_mday) + " " +
            leading_zero(t.tm_hour) + ":" +
            leading_zero(t.tm_min) + ":" +
            leading_zero(t.tm_sec) + "; Pikmin fangame engine version " +
            itos(VERSION_MAJOR) + "." + itos(VERSION_MINOR) +
            "." + itos(VERSION_REV) + "\n" + s;
    }
    
    string prev_error_log;
    string line;
    ALLEGRO_FILE* file_i = al_fopen("Error_log.txt", "r");
    if(file_i) {
        while(!al_feof(file_i)) {
            getline(file_i, line);
            prev_error_log += line + "\n";
        }
        prev_error_log.erase(prev_error_log.size() - 1);
        al_fclose(file_i);
    }
    
    ALLEGRO_FILE* file_o = al_fopen("Error_log.txt", "w");
    if(file_o) {
        al_fwrite(file_o, prev_error_log + s);
        al_fclose(file_o);
    }
}

/* ----------------------------------------------------------------------------
 * Returns whether or not the string s is inside the vector of strings v.
 */
bool find_in_vector(const vector<string> v, const string s) {
    for(auto i = v.begin(); i != v.end(); i++) if(*i == s) return true;
    return false;
}

/* ----------------------------------------------------------------------------
 * Stores the names of all files in a folder into a vector.
 * folder_name: Name of the folder.
 * folders:     If true, only read folders. If false, only read files.
 */
vector<string> folder_to_vector(string folder_name, const bool folders) {
    vector<string> v;
    
    //Normalize the folder's path.
    replace(folder_name.begin(), folder_name.end(), '\\', '/');
    if(folder_name[folder_name.size() - 1] == '/') folder_name.erase(folder_name.size() - 1, 1);
    
    ALLEGRO_FS_ENTRY* folder = NULL;
    folder = al_create_fs_entry(folder_name.c_str());
    if(!folder) return v;
    
    ALLEGRO_FS_ENTRY* entry = NULL;
    
    if(al_open_directory(folder)) {
        while((entry = al_read_directory(folder)) != NULL) {
            if(
                (folders && (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR)) ||
                (!folders && !(al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR))) {
                
                string entry_name = al_get_fs_entry_name(entry);
                if(folders) {   //If we're using folders, remove the trailing slash, lest the string be fully deleted.
                    if(entry_name[entry_name.size() - 1] == '/' || entry_name[entry_name.size() - 1] == '\\') {
                        entry_name = entry_name.substr(0, entry_name.size() - 1);
                    }
                }
                
                //Only save what's after the final slash.
                size_t pos_bs = entry_name.find_last_of("\\");
                size_t pos_fs = entry_name.find_last_of("/");
                size_t pos = pos_bs;
                if(pos_fs != string::npos)
                    if(pos_fs > pos_bs || pos_bs == string::npos) pos = pos_fs;
                    
                if(pos != string::npos) entry_name = entry_name.substr(pos + 1, entry_name.size() - pos - 1);
                v.push_back(entry_name);
            }
            al_destroy_fs_entry(entry);
        }
        al_close_directory(folder);
        al_destroy_fs_entry(folder);
    }
    
    return v;
}

/* ----------------------------------------------------------------------------
 * Generates the images that make up the area.
 */
void generate_area_images() {
    //First, clear all existing area images.
    for(size_t x = 0; x < area_images.size(); x++) {
        for(size_t y = 0; y < area_images[x].size(); y++) {
            al_destroy_bitmap(area_images[x][y]);
        }
        area_images[x].clear();
    }
    area_images.clear();
    
    //Now, figure out how big our area is.
    size_t n_sectors = cur_area_map.sectors.size();
    if(n_sectors == 0) return;
    
    float min_x, max_x, min_y, max_y;
    size_t n_vertices = cur_area_map.vertices.size();
    min_x = max_x = cur_area_map.vertices[0]->x;
    min_y = max_y = cur_area_map.vertices[0]->y;
    
    for(size_t v = 0; v < n_vertices; v++) {
        vertex* v_ptr = cur_area_map.vertices[v];
        min_x = min(v_ptr->x, min_x);
        max_x = max(v_ptr->x, max_x);
        min_y = min(v_ptr->y, min_y);
        max_y = max(v_ptr->y, max_y);
    }
    
    area_x1 = min_x; area_y1 = min_y;
    
    //Create the new bitmaps on the vectors.
    float area_width = max_x - min_x;
    float area_height = max_y - min_y;
    unsigned area_image_cols = ceil(area_width / AREA_IMAGE_SIZE);
    unsigned area_image_rows = ceil(area_height / AREA_IMAGE_SIZE);
    
    for(size_t x = 0; x < area_image_cols; x++) {
        area_images.push_back(vector<ALLEGRO_BITMAP*>());
        
        for(size_t y = 0; y < area_image_rows; y++) {
            area_images[x].push_back(al_create_bitmap(AREA_IMAGE_SIZE, AREA_IMAGE_SIZE));
        }
    }
    
    //For every sector, draw it on the area images it belongs on.
    for(size_t s = 0; s < n_sectors; s++) {
        size_t n_linedefs = cur_area_map.sectors[s]->linedefs.size();
        if(n_linedefs == 0) continue;
        
        float s_min_x, s_max_x, s_min_y, s_max_y;
        unsigned sector_start_col, sector_end_col, sector_start_row, sector_end_row;
        s_min_x = s_max_x = cur_area_map.sectors[s]->linedefs[0]->vertices[0]->x;
        s_min_y = s_max_y = cur_area_map.sectors[s]->linedefs[0]->vertices[0]->y;
        
        for(size_t l = 1; l < n_linedefs; l++) { //Start at 1, because we already have the first linedef's values.
            float x = cur_area_map.sectors[s]->linedefs[l]->vertices[0]->x;
            float y = cur_area_map.sectors[s]->linedefs[l]->vertices[0]->y;
            
            s_min_x = min(x, s_min_x);
            s_max_x = max(x, s_max_x);
            s_min_y = min(y, s_min_y);
            s_max_y = max(y, s_max_y);
        }
        
        sector_start_col = (s_min_x - area_x1) / AREA_IMAGE_SIZE;
        sector_end_col =   ceil((s_max_x - area_x1) / AREA_IMAGE_SIZE) - 1;
        sector_start_row = (s_min_y - area_y1) / AREA_IMAGE_SIZE;
        sector_end_row =   ceil((s_max_y - area_y1) / AREA_IMAGE_SIZE) - 1;
        
        al_set_separate_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
        
        for(size_t x = sector_start_col; x <= sector_end_col; x++) {
            for(size_t y = sector_start_row; y <= sector_end_row; y++) {
                ALLEGRO_BITMAP* prev_target_bmp = al_get_target_bitmap();
                al_set_target_bitmap(area_images[x][y]); {
                
                    draw_sector(cur_area_map.sectors[s], x * AREA_IMAGE_SIZE + area_x1, y * AREA_IMAGE_SIZE + area_y1);
                    
                } al_set_target_bitmap(prev_target_bmp);
            }
        }
        
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
        
    }
    
}

/* ----------------------------------------------------------------------------
 * Returns the daylight effect color for the current time, for the current weather.
 */
ALLEGRO_COLOR get_daylight_color() {
    //ToDo optimize: don't fetch the points from the weather's map every time.
    //ToDo find out how to get the iterator to give me the value of the next point, instead of putting all points in a vector.
    vector<unsigned> point_nrs;
    for(auto p_nr = cur_weather.lighting.begin(); p_nr != cur_weather.lighting.end(); p_nr++) {
        point_nrs.push_back(p_nr->first);
    }
    
    size_t n_points = point_nrs.size();
    if(n_points > 1) {
        for(size_t p = 0; p < n_points - 1; p++) {
            if(day_minutes >= point_nrs[p] && day_minutes < point_nrs[p + 1]) {
                return interpolate_color(
                           day_minutes,
                           point_nrs[p],
                           point_nrs[p + 1],
                           cur_weather.lighting[point_nrs[p]],
                           cur_weather.lighting[point_nrs[p + 1]]
                       );
            }
        }
    }
    
    //If anything goes wrong, don't apply lighting at all.
    return al_map_rgba(0, 0, 0, 0);
}

/* ----------------------------------------------------------------------------
 * Returns an ALLEGRO_TRANSFORM that transforms world coordinates into screen coordinates.
 */
ALLEGRO_TRANSFORM get_world_to_screen_transform() {
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_translate_transform(
        &t,
        -cam_x + scr_w / 2 * 1 / (cam_zoom),
        -cam_y + scr_h / 2 * 1 / (cam_zoom)
    );
    al_scale_transform(&t, cam_zoom, cam_zoom);
    return t;
}

/* ----------------------------------------------------------------------------
 * Returns the interpolation between two colors, given a number in an interval.
 * n: The number.
 * n1, n2: The interval the number falls on.
 ** The closer to n1, the closer the final color is to c1.
 * c1, c2: Colors.
 */
ALLEGRO_COLOR interpolate_color(const float n, const float n1, const float n2, const ALLEGRO_COLOR c1, const ALLEGRO_COLOR c2) {
    float progress = (float) (n - n1) / (float) (n2 - n1);
    return al_map_rgba_f(
               c1.r + progress * (c2.r - c1.r),
               c1.g + progress * (c2.g - c1.g),
               c1.b + progress * (c2.b - c1.b),
               c1.a + progress * (c2.a - c1.a)
           );
}

/* ----------------------------------------------------------------------------
 * Loads an area into memory.
 */
void load_area(const string name, const bool load_for_editor) {

    data_node file = load_data_file(AREA_FOLDER "/" + name + ".txt");
    
    string weather_condition_name = file.get_child_by_name("weather")->value;
    if(load_for_editor) {
        ed_weather_name = weather_condition_name;
    } else {
        if(weather_conditions.find(weather_condition_name) == weather_conditions.end()) {
            error_log("Area " + name + " refers to a non-existing weather condition, \"" + weather_condition_name + "\"!", &file);
            cur_weather = weather();
        } else {
            cur_weather = weather_conditions[weather_condition_name];
        }
    }
    
    
    cur_area_map.clear();
    
    //Vertices.
    size_t n_vertices = file.get_child_by_name("vertices")->get_nr_of_children_by_name("vertex");
    for(size_t v = 0; v < n_vertices; v++) {
        data_node* vertex_data = file.get_child_by_name("vertices")->get_child_by_name("vertex", v);
        vector<string> words = split(vertex_data->value);
        if(words.size() == 2) cur_area_map.vertices.push_back(new vertex(tof(words[0]), tof(words[1])));
    }
    
    //Linedefs.
    size_t n_linedefs = file.get_child_by_name("linedefs")->get_nr_of_children_by_name("linedef");
    for(size_t l = 0; l < n_linedefs; l++) {
        data_node* linedef_data = file.get_child_by_name("linedefs")->get_child_by_name("linedef", l);
        linedef* new_linedef = new linedef();
        
        vector<string> s_nrs = split(linedef_data->get_child_by_name("s")->value);
        if(s_nrs.size() < 2) s_nrs.insert(s_nrs.end(), 2, "-1");
        for(size_t s = 0; s < 2; s++) {
            if(s_nrs[s] == "-1") new_linedef->sector_nrs[s] = string::npos;
            else new_linedef->sector_nrs[s] = toi(s_nrs[s]);
        }
        
        vector<string> v_nrs = split(linedef_data->get_child_by_name("v")->value);
        if(v_nrs.size() < 2) v_nrs.insert(v_nrs.end(), 2, "0");
        
        new_linedef->vertex_nrs[0] = toi(v_nrs[0]);
        new_linedef->vertex_nrs[1] = toi(v_nrs[1]);
        
        cur_area_map.linedefs.push_back(new_linedef);
    }
    
    //Sectors.
    size_t n_sectors = file.get_child_by_name("sectors")->get_nr_of_children_by_name("sector");
    for(size_t s = 0; s < n_sectors; s++) {
        data_node* sector_data = file.get_child_by_name("sectors")->get_child_by_name("sector", s);
        sector* new_sector = new sector();
        
        new_sector->type = sector_types.get_nr(sector_data->get_child_by_name("type")->value);
        if(new_sector->type == 255) new_sector->type = SECTOR_TYPE_NORMAL;
        new_sector->brightness = tof(sector_data->get_child_by_name("brightness")->get_value_or_default(itos(DEF_SECTOR_BRIGHTNESS)));
        new_sector->z = tof(sector_data->get_child_by_name("z")->value);
        new_sector->fade = tob(sector_data->get_child_by_name("fade")->value);
        
        new_sector->file_name = sector_data->get_child_by_name("texture")->value;
        new_sector->rot = tof(sector_data->get_child_by_name("texture_rotate")->value);
        
        vector<string> scales = split(sector_data->get_child_by_name("texture_scale")->value);
        if(scales.size() >= 2) {
            new_sector->scale_x = tof(scales[0]);
            new_sector->scale_y = tof(scales[0]);
        }
        vector<string> translations = split(sector_data->get_child_by_name("texture_trans")->value);
        if(translations.size() >= 2) {
            new_sector->trans_x = tof(translations[0]);
            new_sector->trans_y = tof(translations[0]);
        }
        
        
        //ToDo elements (and tags, if I really am gonna use them...).
        
        cur_area_map.sectors.push_back(new_sector);
    }
    
    //Mobs.
    size_t n_mobs = file.get_child_by_name("mobs")->get_nr_of_children();
    for(size_t m = 0; m < n_mobs; m++) {
    
        data_node* mob_node = file.get_child_by_name("mobs")->get_child(m);
        
        mob_gen* mob_ptr = new mob_gen();
        
        vector<string> coords = split(mob_node->get_child_by_name("pos")->value);
        mob_ptr->x = (coords.size() >= 1 ? tof(coords[0]) : 0);
        mob_ptr->y = (coords.size() >= 2 ? tof(coords[1]) : 0);
        mob_ptr->angle = tof(mob_node->get_child_by_name("angle")->get_value_or_default("0"));
        mob_ptr->vars = mob_node->get_child_by_name("vars")->value;
        
        mob_ptr->folder = mob_folders.get_nr_from_sname(mob_node->name);
        string mt = mob_node->get_child_by_name("type")->value;
        mob_folders.set_mob_type_ptr(mob_ptr, mt);
        
        if(!mob_ptr->type && !load_for_editor) {
            //Error.
            error_log(
                "Unknown \"" + mob_folders.get_sname(mob_ptr->folder) + "\" mob type \"" +
                mt + "\"!", mob_node
            );
        }
        
        if((mob_ptr->folder == MOB_FOLDER_NONE || mob_ptr->folder == string::npos) && !load_for_editor) {
        
            error_log("Unknown mob folder \"" + mob_node->name + "\"!", mob_node);
            mob_ptr->folder = MOB_FOLDER_NONE;
            
        }
        
        cur_area_map.mob_generators.push_back(mob_ptr);
    }
    
    //Tree shadows.
    size_t n_shadows = file.get_child_by_name("tree_shadows")->get_nr_of_children();
    for(size_t s = 0; s < n_shadows; s++) {
    
        data_node* shadow_node = file.get_child_by_name("tree_shadows")->get_child(s);
        
        tree_shadow* s_ptr = new tree_shadow();
        
        vector<string> words = split(shadow_node->get_child_by_name("pos")->value);
        s_ptr->x = (words.size() >= 1 ? tof(words[0]) : 0);
        s_ptr->y = (words.size() >= 2 ? tof(words[1]) : 0);
        words = split(shadow_node->get_child_by_name("size")->value);
        s_ptr->w = (words.size() >= 1 ? tof(words[0]) : 0);
        s_ptr->h = (words.size() >= 2 ? tof(words[1]) : 0);
        s_ptr->angle = tof(shadow_node->get_child_by_name("angle")->value);
        s_ptr->alpha = toi(shadow_node->get_child_by_name("alpha")->value);
        s_ptr->file_name = shadow_node->get_child_by_name("file")->value;
        s_ptr->bitmap = bitmaps.get("Textures/" + s_ptr->file_name, NULL);
        
        if(s_ptr->bitmap == bmp_error && !load_for_editor) {
            error_log("Unknown tree shadow texture \"" + s_ptr->file_name + "\"!", shadow_node);
        }
        
        cur_area_map.tree_shadows.push_back(s_ptr);
        
    }
    
    
    //Background.
    ed_bg_file_name = file.get_child_by_name("bg_file_name")->value;
    ed_bg_x = tof(file.get_child_by_name("bg_x")->value);
    ed_bg_y = tof(file.get_child_by_name("bg_y")->value);
    ed_bg_w = tof(file.get_child_by_name("bg_w")->value);
    ed_bg_h = tof(file.get_child_by_name("bg_h")->value);
    ed_bg_a = toi(file.get_child_by_name("bg_alpha")->get_value_or_default("255"));
    
    
    
    //Set up stuff.
    //ToDo error checking.
    for(size_t l = 0; l < cur_area_map.linedefs.size(); l++) {
        cur_area_map.linedefs[l]->fix_pointers(cur_area_map);
    }
    for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        cur_area_map.sectors[s]->connect_linedefs(cur_area_map, s);
    }
    for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
        cur_area_map.vertices[v]->connect_linedefs(cur_area_map, v);
    }
    
    //Triangulate everything.
    for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        sector* s_ptr = cur_area_map.sectors[s];
        s_ptr->triangles.clear();
        triangulate(s_ptr);
    }
}

/* ----------------------------------------------------------------------------
 * Loads the area's sector textures.
 */
void load_area_textures() {
    for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        sector* s_ptr = cur_area_map.sectors[s];
        
        for(unsigned char t = 0; t < ((s_ptr->fade) ? 2 : 1); t++) {
            if(s_ptr->file_name.size() == 0) {
                s_ptr->bitmap = NULL;
            } else {
                s_ptr->bitmap = bitmaps.get("Textures/" + s_ptr->file_name, NULL);
            }
        }
    }
}

/* ----------------------------------------------------------------------------
 * Loads a bitmap from the game's content.
 * If the node is present, it'll be used to report errors.
 */
ALLEGRO_BITMAP* load_bmp(const string file_name, data_node* node, bool report_error) {
    ALLEGRO_BITMAP* b = NULL;
    b = al_load_bitmap((GRAPHICS_FOLDER "/" + file_name).c_str());
    if(!b && report_error) {
        error_log("Could not open image " + file_name + "!", node);
        b = bmp_error;
    }
    
    return b;
}

/* ----------------------------------------------------------------------------
 * Loads a game control.
 */
void load_control(const unsigned char action, const unsigned char player, const string name, data_node &file, const string def) {
    string s = file.get_child_by_name("p" + itos((player + 1)) + "_" + name)->get_value_or_default((player == 0) ? def : "");
    vector<string> possible_controls = split(s, ",");
    size_t n_possible_controls = possible_controls.size();
    
    for(size_t c = 0; c < n_possible_controls; c++) {
        controls.push_back(control_info(action, player, possible_controls[c]));
    }
}

/* ----------------------------------------------------------------------------
 * Loads a data file from the game's content.
 */
data_node load_data_file(const string file_name) {
    data_node n = data_node(file_name);
    if(!n.file_was_opened) {
        error_log("Could not open data file " + file_name + "!");
    }
    
    return n;
}

/* ----------------------------------------------------------------------------
 * Loads all of the game's content.
 */
void load_game_content() {
    //ToDo.
    statuses.push_back(status(0, 0, 1, true, al_map_rgb(128, 0, 255), STATUS_AFFECTS_ENEMIES));
    statuses.push_back(status(1.5, 1.5, 1, false, al_map_rgb(255, 64, 64), STATUS_AFFECTS_PIKMIN));
    
    spray_types.push_back(spray_type(&statuses[0], false, 10, al_map_rgb(160, 0, 255), NULL, NULL));
    spray_types.push_back(spray_type(&statuses[1], true, 40, al_map_rgb(255, 160, 192), NULL, NULL));
    //spray_types.push_back(spray_type(&statuses[1], true, 40, al_map_rgb(255, 255, 0), NULL, NULL));
    
    //Mob types.
    load_mob_types(true);
    
    //Weather.
    weather_conditions.clear();
    data_node weather_file = load_data_file(WEATHER_FILE);
    size_t n_weather_conditions = weather_file.get_nr_of_children_by_name("weather");
    
    for(size_t wc = 0; wc < n_weather_conditions; wc++) {
        data_node* cur_weather = weather_file.get_child_by_name("weather", wc);
        
        string name = cur_weather->get_child_by_name("name")->value;
        if(name.size() == 0) name = "default";
        
        map<unsigned, ALLEGRO_COLOR> lighting;
        size_t n_lighting_points = cur_weather->get_child_by_name("lighting")->get_nr_of_children();
        
        for(size_t lp = 0; lp < n_lighting_points; lp++) {
            data_node* lighting_node = cur_weather->get_child_by_name("lighting")->get_child(lp);
            
            unsigned point_time = toi(lighting_node->name);
            ALLEGRO_COLOR point_color = toc(lighting_node->value);
            
            lighting[point_time] = point_color;
        }
        
        if(lighting.size() == 0) {
            error_log("Weather condition " + name + " has no lighting!");
        } else {
            if(lighting.find(24 * 60) == lighting.end()) {
                //If there is no data for the last hour, use the data from the first point (this is because the day loops after 24:00; needed for interpolation)
                lighting[24 * 60] = lighting.begin()->second;
            }
        }
        
        unsigned char percipitation_type = toi(cur_weather->get_child_by_name("percipitation_type")->get_value_or_default(itos(PERCIPITATION_TYPE_NONE)));
        interval percipitation_frequency = interval(cur_weather->get_child_by_name("percipitation_frequency")->value);
        interval percipitation_speed = interval(cur_weather->get_child_by_name("percipitation_speed")->value);
        interval percipitation_angle = interval(cur_weather->get_child_by_name("percipitation_angle")->get_value_or_default(ftos((M_PI + M_PI_2))));
        
        weather_conditions[name] = weather(name, lighting, percipitation_type, percipitation_frequency, percipitation_speed, percipitation_angle);
    }
}

/* ----------------------------------------------------------------------------
 * Loads the player's options.
 */
void load_options() {
    data_node file = data_node("Options.txt");
    if(!file.file_was_opened) return;
    
    //Load joysticks.
    joystick_numbers.clear();
    int n_joysticks = al_get_num_joysticks();
    for(int j = 0; j < n_joysticks; j++) {
        joystick_numbers[al_get_joystick(j)] = j;
    }
    
    //Load controls.
    //Format of a control: "p<player number>_<action>=<possible control 1>,<possible control 2>,<...>"
    //Format of a possible control: "<input method>_<parameters, underscore separated>"
    //Input methods: "k" (keyboard key), "mb" (mouse button), "mwu" (mouse wheel up), "mwd" (down),
    //"mwl" (left), "mwr" (right), "jb" (joystick button), "jap" (joystick axis, positive), "jan" (joystick axis, negative).
    //The parameters are the key/button number, joystick number, joystick stick and axis, etc.
    //Check the constructor of control_info for more information.
    controls.clear();
    
    for(unsigned char p = 0; p < 4; p++) {
        load_control(BUTTON_THROW,                p, "punch", file, "mb_1");
        load_control(BUTTON_WHISTLE,              p, "whistle", file, "mb_2");
        load_control(BUTTON_MOVE_RIGHT,           p, "move_right", file, "k_4");
        load_control(BUTTON_MOVE_UP,              p, "move_up", file, "k_23");
        load_control(BUTTON_MOVE_LEFT,            p, "move_left", file, "k_1");
        load_control(BUTTON_MOVE_DOWN,            p, "move_down", file, "k_19");
        load_control(BUTTON_MOVE_CURSOR_RIGHT,    p, "move_cursor_right", file, "");
        load_control(BUTTON_MOVE_CURSOR_UP,       p, "move_cursor_up", file, "");
        load_control(BUTTON_MOVE_CURSOR_LEFT,     p, "move_cursor_left", file, "");
        load_control(BUTTON_MOVE_CURSOR_DOWN,     p, "move_cursor_down", file, "");
        load_control(BUTTON_MOVE_GROUP_TO_CURSOR, p, "move_group_to_cursor", file, "k_75");
        load_control(BUTTON_MOVE_GROUP_RIGHT,     p, "move_group_right", file, "");
        load_control(BUTTON_MOVE_GROUP_UP,        p, "move_group_up", file, "");
        load_control(BUTTON_MOVE_GROUP_LEFT,      p, "move_group_left", file, "");
        load_control(BUTTON_MOVE_GROUP_DOWN,      p, "move_group_down", file, "");
        load_control(BUTTON_SWITCH_CAPTAIN_RIGHT, p, "switch_captain_right", file, "k_64");
        load_control(BUTTON_SWITCH_CAPTAIN_LEFT,  p, "switch_captain_left", file, "");
        load_control(BUTTON_DISMISS,              p, "dismiss", file, "k_217");
        load_control(BUTTON_USE_SPRAY_1,          p, "use_spray_1", file, "k_18");
        load_control(BUTTON_USE_SPRAY_2,          p, "use_spray_2", file, "k_6");
        load_control(BUTTON_USE_SPRAY,            p, "use_spray", file, "k_18");
        load_control(BUTTON_SWITCH_SPRAY_RIGHT,   p, "switch_spray_right", file, "k_5");
        load_control(BUTTON_SWITCH_SPRAY_LEFT,    p, "switch_spray_left", file, "k_17");
        load_control(BUTTON_SWITCH_ZOOM,          p, "switch_zoom", file, "k_3");
        load_control(BUTTON_ZOOM_IN,              p, "zoom_in", file, "mwu");
        load_control(BUTTON_ZOOM_OUT,             p, "zoom_out", file, "mwd");
        load_control(BUTTON_SWITCH_TYPE_RIGHT,    p, "switch_type_right", file, "");
        load_control(BUTTON_SWITCH_TYPE_LEFT,     p, "switch_type_left", file, "");
        load_control(BUTTON_SWITCH_MATURITY_UP,   p, "switch_maturity_up", file, "");
        load_control(BUTTON_SWITCH_MATURITY_DOWN, p, "switch_maturity_down", file, "");
        load_control(BUTTON_LIE_DOWN,             p, "lie_down", file, "k_26");
        load_control(BUTTON_PAUSE,                p, "pause", file, "k_59");
    }
    
    //Weed out controls that didn't parse correctly.
    size_t n_controls = controls.size();
    for(size_t c = 0; c < n_controls; ) {
        if(controls[c].action == BUTTON_NONE) {
            controls.erase(controls.begin() + c);
        } else {
            c++;
        }
    }
    
    for(unsigned char p = 0; p < 4; p++) {
        mouse_moves_cursor[p] = tob(file.get_child_by_name("p" + itos((p + 1)) + "_mouse_moves_cursor")->get_value_or_default((p == 0) ? "true" : "false"));
    }
    
    //Other options.
    daylight_effect = tob(file.get_child_by_name("daylight_effect")->get_value_or_default("true"));
    draw_cursor_trail = tob(file.get_child_by_name("draw_cursor_trail")->get_value_or_default("true"));
    game_fps = toi(file.get_child_by_name("fps")->get_value_or_default("30"));
    scr_h = toi(file.get_child_by_name("height")->get_value_or_default(itos(DEF_SCR_H)));
    particle_quality = toi(file.get_child_by_name("particle_quality")->get_value_or_default("2"));
    pretty_whistle = tob(file.get_child_by_name("pretty_whistle")->get_value_or_default("true"));
    scr_w = toi(file.get_child_by_name("width")->get_value_or_default(itos(DEF_SCR_W)));
    smooth_scaling = tob(file.get_child_by_name("smooth_scaling")->get_value_or_default("true"));
    window_x = toi(file.get_child_by_name("window_x")->get_value_or_default(itos(INT_MAX)));
    window_y = toi(file.get_child_by_name("window_y")->get_value_or_default(itos(INT_MAX)));
}

/* ----------------------------------------------------------------------------
 * Loads an audio sample from the game's content.
 */
sample_struct load_sample(const string file_name, ALLEGRO_MIXER* const mixer) {
    ALLEGRO_SAMPLE* sample = al_load_sample((AUDIO_FOLDER "/" + file_name).c_str());
    if(!sample) {
        error_log("Could not open audio sample " + file_name + "!");
    }
    
    return sample_struct(sample, mixer);
}

/* ----------------------------------------------------------------------------
 * Returns the movement necessary to move a point.
 * x/y:          Coordinates of the initial point.
 * tx/ty:        Coordinates of the target point.
 * speed:        Speed at which the point can move.
 * reach_radius: If the point is within this range of the target, consider it as already being there.
 * mx/my:        Variables to return the amount of movement to.
 * angle:        Variable to return the angle the point faces to.
 * reached:      Variable to return whether the point reached the target or not to.
 */
void move_point(const float x, const float y, const float tx, const float ty, const float speed, const float reach_radius, float* mx, float* my, float* angle, bool* reached) {
    float dx = tx - x, dy = ty - y;
    float dist = sqrt(dx * dx + dy * dy);
    
    if(dist > reach_radius) {
        float move_amount = min(dist / delta_t / 2, speed);
        
        dx *= move_amount / dist;
        dy *= move_amount / dist;
        
        if(mx) *mx = dx;
        if(my) *my = dy;
        if(angle) *angle = atan2(dy, dx);
        if(reached) *reached = false;
    } else {
        if(mx) *mx = 0;
        if(my) *my = 0;
        if(reached) *reached = true;
    }
}

/* ----------------------------------------------------------------------------
 * Normalizes an angle so that it's between 0 and M_PI * 2.
 */
float normalize_angle(float a) {
    a = fmod((double) a, M_PI * 2);
    if(a < 0) a += M_PI * 2;
    return a;
}

/* ----------------------------------------------------------------------------
 * Returns a random float between the provided range, inclusive.
 */
float randomf(float min, float max) {
    if(min > max) swap(min, max);
    if(min == max) return min;
    return (float) rand() / ((float) RAND_MAX / (max - min)) + min;
}

/* ----------------------------------------------------------------------------
 * Returns a random integer between the provided range, inclusive.
 */
int randomi(int min, int max) {
    if(min > max) swap(min, max);
    if(min == max) return min;
    return ((rand()) % (max - min + 1)) + min;
}

/* ----------------------------------------------------------------------------
 * Rotates a point by an angle. The x and y are meant to represent the difference between the point and the center of the rotation.
 */
void rotate_point(const float x, const float y, const float angle, float* final_x, float* final_y) {
    float c = cos(angle);
    float s = sin(angle);
    if(final_x) *final_x = c * x - s * y;
    if(final_y) *final_y = s * x + c * y;
}

/* ----------------------------------------------------------------------------
 * Saves the player's options.
 */
void save_options() {
    //ToDo make this prettier. Like a list of constants somewhere where it associates an action with the name on the text file.
    ALLEGRO_FILE* file = al_fopen("Options.txt", "w");
    
    if(!file) return;
    
    //First, group the controls by action and player.
    map<string, string> grouped_controls;
    
    //Tell the map what they are.
    for(unsigned char p = 0; p < 4; p++) {
        string prefix = "p" + itos((p + 1)) + "_";
        grouped_controls[prefix + "punch"].clear();
        grouped_controls[prefix + "whistle"].clear();
        grouped_controls[prefix + "move_right"].clear();
        grouped_controls[prefix + "move_up"].clear();
        grouped_controls[prefix + "move_left"].clear();
        grouped_controls[prefix + "move_down"].clear();
        grouped_controls[prefix + "move_cursor_right"].clear();
        grouped_controls[prefix + "move_cursor_up"].clear();
        grouped_controls[prefix + "move_cursor_left"].clear();
        grouped_controls[prefix + "move_cursor_down"].clear();
        grouped_controls[prefix + "move_group_right"].clear();
        grouped_controls[prefix + "move_group_up"].clear();
        grouped_controls[prefix + "move_group_left"].clear();
        grouped_controls[prefix + "move_group_down"].clear();
        grouped_controls[prefix + "move_group_to_cursor"].clear();
        grouped_controls[prefix + "switch_captain_right"].clear();
        grouped_controls[prefix + "switch_captain_left"].clear();
        grouped_controls[prefix + "dismiss"].clear();
        grouped_controls[prefix + "use_spray_1"].clear();
        grouped_controls[prefix + "use_spray_2"].clear();
        grouped_controls[prefix + "use_spray"].clear();
        grouped_controls[prefix + "switch_spray_right"].clear();
        grouped_controls[prefix + "switch_spray_left"].clear();
        grouped_controls[prefix + "switch_zoom"].clear();
        grouped_controls[prefix + "zoom_in"].clear();
        grouped_controls[prefix + "zoom_out"].clear();
        grouped_controls[prefix + "switch_type_right"].clear();
        grouped_controls[prefix + "switch_type_left"].clear();
        grouped_controls[prefix + "switch_maturity_up"].clear();
        grouped_controls[prefix + "switch_maturity_down"].clear();
        grouped_controls[prefix + "lie_down"].clear();
        grouped_controls[prefix + "pause"].clear();
    }
    
    size_t n_controls = controls.size();
    for(size_t c = 0; c < n_controls; c++) {
        string name = "p" + itos((controls[c].player + 1)) + "_";
        if(controls[c].action == BUTTON_THROW)                     name += "punch";
        else if(controls[c].action == BUTTON_WHISTLE)              name += "whistle";
        else if(controls[c].action == BUTTON_MOVE_RIGHT)           name += "move_right";
        else if(controls[c].action == BUTTON_MOVE_UP)              name += "move_up";
        else if(controls[c].action == BUTTON_MOVE_LEFT)            name += "move_left";
        else if(controls[c].action == BUTTON_MOVE_DOWN)            name += "move_down";
        else if(controls[c].action == BUTTON_MOVE_CURSOR_RIGHT)    name += "move_cursor_right";
        else if(controls[c].action == BUTTON_MOVE_CURSOR_UP)       name += "move_cursor_up";
        else if(controls[c].action == BUTTON_MOVE_CURSOR_LEFT)     name += "move_cursor_left";
        else if(controls[c].action == BUTTON_MOVE_CURSOR_DOWN)     name += "move_cursor_down";
        else if(controls[c].action == BUTTON_MOVE_GROUP_RIGHT)     name += "move_group_right";
        else if(controls[c].action == BUTTON_MOVE_GROUP_UP)        name += "move_group_up";
        else if(controls[c].action == BUTTON_MOVE_GROUP_LEFT)      name += "move_group_left";
        else if(controls[c].action == BUTTON_MOVE_GROUP_DOWN)      name += "move_group_down";
        else if(controls[c].action == BUTTON_MOVE_GROUP_TO_CURSOR) name += "move_group_to_cursor";
        else if(controls[c].action == BUTTON_SWITCH_CAPTAIN_RIGHT) name += "switch_captain_right";
        else if(controls[c].action == BUTTON_SWITCH_CAPTAIN_LEFT)  name += "switch_captain_left";
        else if(controls[c].action == BUTTON_DISMISS)              name += "dismiss";
        else if(controls[c].action == BUTTON_USE_SPRAY_1)          name += "use_spray_1";
        else if(controls[c].action == BUTTON_USE_SPRAY_2)          name += "use_spray_2";
        else if(controls[c].action == BUTTON_USE_SPRAY)            name += "use_spray";
        else if(controls[c].action == BUTTON_SWITCH_SPRAY_RIGHT)   name += "switch_spray_right";
        else if(controls[c].action == BUTTON_SWITCH_SPRAY_LEFT)    name += "switch_spray_left";
        else if(controls[c].action == BUTTON_SWITCH_ZOOM)          name += "switch_zoom";
        else if(controls[c].action == BUTTON_ZOOM_IN)              name += "zoom_in";
        else if(controls[c].action == BUTTON_ZOOM_OUT)             name += "zoom_out";
        else if(controls[c].action == BUTTON_SWITCH_TYPE_RIGHT)    name += "switch_type_right";
        else if(controls[c].action == BUTTON_SWITCH_TYPE_LEFT)     name += "switch_type_left";
        else if(controls[c].action == BUTTON_SWITCH_MATURITY_UP)   name += "switch_maturity_up";
        else if(controls[c].action == BUTTON_SWITCH_MATURITY_DOWN) name += "switch_maturity_down";
        else if(controls[c].action == BUTTON_LIE_DOWN)             name += "lie_down";
        else if(controls[c].action == BUTTON_PAUSE)                name += "pause";
        
        grouped_controls[name] += controls[c].stringify() + ",";
    }
    
    //Save controls.
    for(auto c = grouped_controls.begin(); c != grouped_controls.end(); c++) {
        if(c->second.size()) c->second.erase(c->second.size() - 1); //Remove the final character, which is always an extra comma.
        
        al_fwrite(file, c->first + "=" + c->second + "\n");
    }
    
    for(unsigned char p = 0; p < 4; p++) {
        al_fwrite(file, "p" + itos((p + 1)) + "_mouse_moves_cursor=" + btos(mouse_moves_cursor[p]) + "\n");
    }
    
    //Other options.
    al_fwrite(file, "daylight_effect=" + btos(daylight_effect) + "\n");
    al_fwrite(file, "draw_cursor_trail=" + btos(draw_cursor_trail) + "\n");
    al_fwrite(file, "fps=" + itos(game_fps) + "\n");
    al_fwrite(file, "height=" + itos(scr_h) + "\n");
    al_fwrite(file, "particle_quality=" + itos(particle_quality) + "\n");
    al_fwrite(file, "pretty_whistle=" + btos(pretty_whistle) + "\n");
    al_fwrite(file, "width=" + itos(scr_w) + "\n");
    al_fwrite(file, "smooth_scaling=" + btos(smooth_scaling) + "\n");
    al_fwrite(file, "window_x=" + itos(window_x) + "\n");
    al_fwrite(file, "window_y=" + itos(window_y) + "\n");
    
    al_fclose(file);
}

/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:        The string to split.
 * del:         The delimiter. Default is space.
 * inc_empty:   If true, include empty substrings on the vector.
 ** i.e. if two delimiters come together in a row, keep an empty substring between.
 * inc_del:     If true, include the delimiters on the vector as a substring.
 */
vector<string> split(string text, const string del, const bool inc_empty, const bool inc_del) {
    vector<string> v;
    size_t pos;
    size_t del_size = del.size();
    
    do {
        pos = text.find(del);
        if (pos != string::npos) {  //If it DID find the delimiter.
            //Get the text between the start and the delimiter.
            string sub = text.substr(0, pos);
            
            //Add the text before the delimiter to the vector.
            if(sub != "" || inc_empty)
                v.push_back(sub);
                
            //Add the delimiter to the vector, but only if requested.
            if(inc_del)
                v.push_back(del);
                
            text.erase(text.begin(), text.begin() + pos + del_size);    //Delete everything before the delimiter, including the delimiter itself, and search again.
        }
    } while (pos != string::npos);
    
    //Text after the final delimiter. (If there is one. If not, it's just the whole string.)
    
    if (text != "" || inc_empty) //If it's a blank string, only add it if we want empty strings.
        v.push_back(text);
        
    return v;
}

/* ----------------------------------------------------------------------------
 * Starts panning the camera towards another point.
 */
void start_camera_pan(const int final_x, const int final_y) {
    cam_trans_pan_initial_x = cam_x;
    cam_trans_pan_initial_y = cam_y;
    cam_trans_pan_final_x = final_x;
    cam_trans_pan_final_y = final_y;
    cam_trans_pan_time_left = CAM_TRANSITION_DURATION;
}

/* ----------------------------------------------------------------------------
 * Starts moving the camera towards another zoom level.
 */
void start_camera_zoom(const float final_zoom_level) {
    cam_trans_zoom_initial_level = cam_zoom;
    cam_trans_zoom_final_level = final_zoom_level;
    cam_trans_zoom_time_left = CAM_TRANSITION_DURATION;
    
    sfx_camera.play(0, false);
}

/* ----------------------------------------------------------------------------
 * Starts the display of a text message. If the text is empty, it closes the message box.
 * text:        Text to display.
 * speaker_bmp: Bitmap representing the speaker.
 */
void start_message(string text, ALLEGRO_BITMAP* speaker_bmp) {
    if(text.size()) if(text.back() == '\n') text.pop_back();
    cur_message = text;
    cur_message_char = 0;
    cur_message_char_time = MESSAGE_CHAR_INTERVAL;
    cur_message_speaker = speaker_bmp;
    cur_message_stopping_chars.clear();
    cur_message_stopping_chars.push_back(0); //First character. Makes it easier.
    cur_message_section = 0;
    
    vector<string> lines = split(text, "\n");
    for(size_t line_trio = 0; line_trio < lines.size(); line_trio += 3) {
        cur_message_stopping_chars.push_back(0);
        for(size_t l = 0; l < (line_trio + 1) * 3 && l < lines.size(); l++) {
            cur_message_stopping_chars.back() += lines[l].size() + 1; //+1 because of the new line character.
        }
    }
    cur_message_stopping_chars.back()--; //Remove one because the last line doesn't have a new line character. Even if it does, it's invisible.
}

/* ----------------------------------------------------------------------------
 * Converts an entire string into lowercase.
 */
string str_to_lower(string s) {
    unsigned short n_characters = s.size();
    for(unsigned short c = 0; c < n_characters; c++) {
        s[c] = tolower(s[c]);
    }
    return s;
}

/* ----------------------------------------------------------------------------
 * Uses up a spray.
 */
void use_spray(const size_t spray_nr) {
    if(spray_amounts[spray_nr] == 0) return;
    
    leader* cur_leader_ptr = leaders[cur_leader_nr];
    float shoot_angle = cursor_angle + ((spray_types[spray_nr].burpable) ? M_PI : 0);
    
    random_particle_spray(
        PARTICLE_TYPE_BITMAP,
        bmp_smoke,
        cur_leader_ptr->x + cos(shoot_angle) * cur_leader_ptr->type->size / 2,
        cur_leader_ptr->y + sin(shoot_angle) * cur_leader_ptr->type->size / 2,
        shoot_angle,
        spray_types[spray_nr].main_color
    );
    
    spray_amounts[spray_nr]--;
    
    cur_leader_ptr->anim.change(LEADER_ANIM_DISMISS, true, false, false);
}

//Calls al_fwrite, but with an std::string instead of a c-string.
void al_fwrite(ALLEGRO_FILE* f, string s) { al_fwrite(f, s.c_str(), s.size()); }
//Converts a boolean to a string, returning either "true" or "false".
string btos(bool b) { return b ? "true" : "false"; }
//Converts a string to a boolean, judging by the English language words that represent true and false.
bool tob(string s) {
    s = str_to_lower(s);
    s = trim_spaces(s);
    if(s == "yes" || s == "true" || s == "y" || s == "t") return true;
    else return (toi(s) != 0);
}
//Converts a string to an Allegro color. Components are separated by spaces, and the final one (alpha) is optional.
ALLEGRO_COLOR toc(string s) {
    s = trim_spaces(s);
    
    unsigned char alpha = 255;
    vector<string> components = split(s);
    if(components.size() >= 2) alpha = toi(components[1]);
    
    if(s == "nothing" || s == "none") return al_map_rgba(0,   0,   0,   0);
    if(s == "black")                  return al_map_rgba(0,   0,   0,   alpha);
    if(s == "gray" || s == "grey")    return al_map_rgba(128, 128, 128, alpha);
    if(s == "white")                  return al_map_rgba(255, 255, 255, alpha);
    if(s == "yellow")                 return al_map_rgba(255, 255, 0,   alpha);
    if(s == "orange")                 return al_map_rgba(255, 128, 0,   alpha);
    if(s == "brown")                  return al_map_rgba(128, 64,  0,   alpha);
    if(s == "red")                    return al_map_rgba(255, 0,   0,   alpha);
    if(s == "violet")                 return al_map_rgba(255, 0,   255, alpha);
    if(s == "purple")                 return al_map_rgba(128, 0,   255, alpha);
    if(s == "blue")                   return al_map_rgba(0,   0,   255, alpha);
    if(s == "cyan")                   return al_map_rgba(0,   255, 255, alpha);
    if(s == "green")                  return al_map_rgba(0,   255, 0,   alpha);
    
    ALLEGRO_COLOR c =
        al_map_rgba(
            ((components.size() > 0) ? toi(components[0]) : 0),
            ((components.size() > 1) ? toi(components[1]) : 0),
            ((components.size() > 2) ? toi(components[2]) : 0),
            ((components.size() > 3) ? toi(components[3]) : 255)
        );
    return c;
}
//Converts a string to a float, trimming the spaces and accepting commas or points.
double tof(string s) { s = trim_spaces(s); replace(s.begin(), s.end(), ',', '.'); return atof(s.c_str()); }
//Converts a string to an integer.
int toi(string s) { return tof(s); }
