#include "armored_zombie.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp" // roof_slope_offset
#include "reanim/reanim_zombie.hpp"

extern Window win;

/*Legacy sheet armor overlays: walk/credit-state slot -> first helm frame in
the CONE/BUCKET sheets (frame offset the body state sits on).*/
static const map<int, int> OVERLAY_FRAME_BIAS = {
    {ZOMBIE_CREDIT1_DIRECTORY, 0},
    {ZOMBIE_CREDIT2_DIRECTORY, 22},
    {ZOMBIE_WALK1_DIRECTORY, 44},
    {ZOMBIE_WALK2_DIRECTORY, 91},
    {ZOMBIE_EATING_DIRECTORY, 138}};

static const ArmorSpec SPEC_CONE{
    CONE_TYPE,
    ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK2_DIRECTORY,
    166, 144,
    ZOMBIE_CREDIT1_DIRECTORY, ZOMBIE_CREDIT2_DIRECTORY,
    144, // rand(0, WINDOW_HEIGHT - dir_height)
    CONE1_DIRECTORY,
    true,
    88, 68, 70, -12,
    &OVERLAY_FRAME_BIAS,
    reanimz::DROP_CONE_IMG,
    CONE_DROP_DIRECTORY,
    0, 0};

static const ArmorSpec SPEC_BUCKET{
    BUCKET_TYPE,
    ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK2_DIRECTORY,
    166, 144,
    ZOMBIE_CREDIT1_DIRECTORY, ZOMBIE_CREDIT2_DIRECTORY,
    78, // legacy BUCKET_HEIGHT roll bound (NOT the frame height)
    BUCKET1_DIRECTORY,
    true,
    86, 78, 65, -5,
    &OVERLAY_FRAME_BIAS,
    reanimz::DROP_BUCKET_IMG,
    BUCKET_DROP_DIRECTORY,
    0, 0};

static const ArmorSpec SPEC_DOOR{
    DOOR_TYPE,
    DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK2_1_DIRECTORY,
    196, 197,
    DOOR_ZOMBIE_CREDIT1_DIRECTORY, DOOR_ZOMBIE_CREDIT2_DIRECTORY,
    197, // rand(0, WINDOW_HEIGHT - dir_height)
    NULL_DIRECTORY,
    false,
    0, 0, 0, 0,
    nullptr,
    reanimz::DROP_DOOR_IMG,
    NULL_DIRECTORY, // the degrade chain swaps the door sheets to the plain body
    166, 144};

const ArmorSpec &armor_spec(int type)
{
    switch (type)
    {
    case CONE_TYPE:
        return SPEC_CONE;
    case BUCKET_TYPE:
        return SPEC_BUCKET;
    default: // DOOR_TYPE (only caller of the door shell)
        return SPEC_DOOR;
    }
}

void ArmoredZombie::init()
{
    dir_width = spec.dir_w;
    dir_height = spec.dir_h;
    health = *ZOMBIE_HEALTH_LIMIT[spec.zombie_type].rbegin();
    directory_num = rand(spec.walk_dir_lo, spec.walk_dir_hi);
    armor_dir = spec.start_armor_dir;
}

void ArmoredZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - spec.credit_y_margin);
    directory_num = rand(spec.credit_dir_lo, spec.credit_dir_hi);
    armor_dir = spec.start_armor_dir;
}

void ArmoredZombie::armor_drop(vector<ZombiePart> &zombie_parts)
{
    // Threshold, not exact hit: a melon (4) / fire pea (2) can cross the body
    // tier mid-jump, so fire when health passes <= the plain-zombie pool.
    if (type != NORMAL_TYPE && health <= *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin() * health_scale)
    {
        type = NORMAL_TYPE;
        degrade_steps = 0; // the normal-zombie tiers (arm at 5) still apply
        if (spec.drop_dir_w)
        {
            // door: its 196x197 sheets end with the door; the plain body is 166x144
            dir_width = spec.drop_dir_w;
            dir_height = spec.drop_dir_h;
        }
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(spec.reanim_drop_img, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
        else if (spec.sheet_drop_dir != NULL_DIRECTORY)
            zombie_parts.push_back(ZombiePart(spec.sheet_drop_dir, HEAD_ZOMBIE_FRAME, row, x_location + 80,
                                              HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT, cold_time > 0));
    }
}

void ArmoredZombie::render_zombie()
{
    Zombie::render_zombie();
    if (reanimz::ready())
        return; // armor already drawn by the reanim path
    if (armor_dir == NULL_DIRECTORY || !spec.has_overlay)
        return;

    // current frame
    int sframe = frame / ZOMBIE_FRAME + spec.overlay_frame_bias->at(directory_num);
    int x_loc = x_location + spec.overlay_x_bias;
    // The helm rides the body art: the body is bottom-anchored on the ground
    // line, its top `body_lift` px off the old box top.
    const int feet = dir_height > ZOMBIE_HEIGHT ? ZOMBIE_DOOR_FEET_Y : ZOMBIE_SHEET_FEET_Y;
    const int body_lift = feet - ZOMBIE_SHEET_GROUND_Y;
    int y_loc = y_location + spec.overlay_y_bias + roof_slope_offset(x_location) - body_lift;
    win.draw_nth_frame_bl(armor_dir, sframe, spec.overlay_w, spec.overlay_h,
                          x_loc, y_loc + spec.overlay_h, spec.overlay_w, spec.overlay_h);

    // zombie cold
    if (cold_time)
    {
        win.set_texture_color(blink_of(armor_dir), 0, 75, 255);
        win.set_texture_alpha(blink_of(armor_dir), 120);
        win.draw_nth_frame_bl(blink_of(armor_dir), sframe, spec.overlay_w, spec.overlay_h,
                              x_loc, y_loc + spec.overlay_h, spec.overlay_w, spec.overlay_h);
        win.set_texture_color(blink_of(armor_dir), 255, 255, 255);
        win.set_texture_alpha(blink_of(armor_dir), 70);
    }

    // zombie attacked
    if (attacked_time)
    {
        win.draw_nth_frame_bl(blink_of(armor_dir), sframe, spec.overlay_w, spec.overlay_h,
                              x_loc, y_loc + spec.overlay_h, spec.overlay_w, spec.overlay_h);
    }
}

void ArmoredZombie::display_credited(const int &_minus_x)
{
    if (reanimz::ready() || !spec.has_overlay)
    {
        Zombie::display_credited(_minus_x);
        return;
    }
    // current frame (body feet anchored on the same ground line as the reanim path)
    int sframe = frame / ZOMBIE_FRAME / 2;
    const int feet = dir_height > ZOMBIE_HEIGHT ? ZOMBIE_DOOR_FEET_Y : ZOMBIE_SHEET_FEET_Y;
    win.draw_nth_frame_bl(directory_num, sframe, dir_width, dir_height,
                          x_location - _minus_x,
                          y_location + ZOMBIE_SHEET_GROUND_Y + dir_height - feet,
                          dir_width, dir_height);

    sframe += spec.overlay_frame_bias->at(directory_num);
    // the helm rides the body art, whose top lifted by (feet - ground)
    const int body_lift = feet - ZOMBIE_SHEET_GROUND_Y;
    int x_loc = x_location + spec.overlay_x_bias - _minus_x;
    int y_loc = y_location + spec.overlay_y_bias - body_lift + spec.overlay_h;
    win.draw_nth_frame_bl(armor_dir, sframe, spec.overlay_w, spec.overlay_h,
                          x_loc, y_loc, spec.overlay_w, spec.overlay_h);

    // zombie next frame
    if (++frame >= 2 * ZOMBIE_FRAME * all_img[directory_num].n_sheet)
    {
        frame = 0;
    }
}
