#include "fp.h"
#include "geometry.h"
#include "gu.h"
#include "input.h"
#include <math.h>
#include <n64.h>

static const f32 joy_mspeed = 0.25f;
static const f32 joy_rspeed = 0.0007f;
static const s32 joy_max = 60.f;
static const f32 pitch_lim = M_PI / 2.f - joy_rspeed;
static const f32 fol_mspeed = 1.f / 3.f;
static const f32 fol_rspeed = 1.f / 3.f;

s32 zu_adjust_joystick(s32 v) {
    if (v < 0) {
        if (v > -8) {
            return 0;
        } else if (v < -66) {
            return -60;
        } else {
            return v + 7;
        }
    } else {
        if (v < 8) {
            return 0;
        } else if (v > 66) {
            return 60;
        } else {
            return v - 7;
        }
    }
}

static void cam_manual(void) {
    if (!fp.lock_cam) {
        s32 x = zu_adjust_joystick(input_x());
        s32 y = zu_adjust_joystick(input_y());

        vec3f_t vf;
        vec3f_t vr;
        vec3f_t move;
        vec3f_py(&vf, fp.cam_pitch, fp.cam_yaw);
        vec3f_py(&vr, 0.f, fp.cam_yaw - M_PI / 2.f);

        if (input_pad() & BUTTON_Z) {
            vec3f_scale(&move, &vf, y * joy_mspeed);
            vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);

            vec3f_scale(&move, &vr, x * joy_mspeed);
            vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);

            if (input_pad() & BUTTON_C_UP) {
                fp.cam_pos.y += joy_max * joy_mspeed;
            }
            if (input_pad() & BUTTON_C_DOWN) {
                fp.cam_pos.y += -joy_max * joy_mspeed;
            }
            if (input_pad() & BUTTON_C_RIGHT) {
                fp.cam_yaw -= joy_max * joy_rspeed;
            }
            if (input_pad() & BUTTON_C_LEFT) {
                fp.cam_yaw -= -joy_max * joy_rspeed;
            }
        } else {
            fp.cam_pitch += y * joy_rspeed;
            fp.cam_yaw -= x * joy_rspeed;

            if (input_pad() & BUTTON_C_UP) {
                vec3f_scale(&move, &vf, joy_max * joy_mspeed);
                vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
            }
            if (input_pad() & BUTTON_C_DOWN) {
                vec3f_scale(&move, &vf, -joy_max * joy_mspeed);
                vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
            }
            if (input_pad() & BUTTON_C_RIGHT) {
                vec3f_scale(&move, &vr, joy_max * joy_mspeed);
                vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
            }
            if (input_pad() & BUTTON_C_LEFT) {
                vec3f_scale(&move, &vr, -joy_max * joy_mspeed);
                vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
            }
        }

        if (fp.cam_pitch > pitch_lim) {
            fp.cam_pitch = pitch_lim;
        } else if (fp.cam_pitch < -pitch_lim) {
            fp.cam_pitch = -pitch_lim;
        }
    }
}

static void cam_birdseye(void) {
    vec3f_t vt;
    vt.x = pm_player.position.x;
    vt.y = pm_player.position.y;
    vt.z = pm_player.position.z;

    vec3f_t vd;
    vec3f_sub(&vd, &vt, &fp.cam_pos);

    f32 pitch, yaw;
    vec3f_pyangles(&vd, &pitch, &yaw);

    f32 d_pitch = angle_dif(pitch, fp.cam_pitch);
    if (fabsf(d_pitch) < .001f) {
        fp.cam_pitch = pitch;
    } else {
        fp.cam_pitch += d_pitch * fol_rspeed;
    }

    f32 d_yaw = angle_dif(yaw, fp.cam_yaw);
    if (fabsf(d_yaw) < .001f) {
        fp.cam_yaw = yaw;
    } else {
        fp.cam_yaw += d_yaw * fol_rspeed;
    }

    f32 dist = vec3f_mag(&vd);
    if (dist < fp.cam_dist_min) {
        vec3f_t move;
        vec3f_py(&move, fp.cam_pitch, fp.cam_yaw);
        vec3f_scale(&move, &move, (dist - fp.cam_dist_min) * fol_mspeed);
        vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
    } else if (dist > fp.cam_dist_max) {
        vec3f_t move;
        vec3f_py(&move, fp.cam_pitch, fp.cam_yaw);
        vec3f_scale(&move, &move, (dist - fp.cam_dist_max) * fol_mspeed);
        vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
    }

    cam_manual();
}

static void cam_radial(void) {
    vec3f_t vf;
    vec3f_py(&vf, fp.cam_pitch, fp.cam_yaw);

    vec3f_t vt;
    vt.x = pm_player.position.x;
    vt.y = pm_player.position.y;
    vt.z = pm_player.position.z;

    vec3f_t vd;
    vec3f_sub(&vd, &vt, &fp.cam_pos);

    f32 dist = vec3f_dot(&vd, &vf);
    vec3f_t vp;
    vec3f_scale(&vp, &vf, dist);

    vec3f_t vr;
    vec3f_sub(&vr, &vd, &vp);
    {
        vec3f_t move;
        vec3f_scale(&move, &vr, fol_mspeed);
        vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
    }

    if (dist < fp.cam_dist_min) {
        f32 norm = 1.f / dist;
        vec3f_t move;
        vec3f_scale(&move, &vp, (dist - fp.cam_dist_min) * fol_mspeed * norm);
        vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
    } else if (dist > fp.cam_dist_max) {
        f32 norm = 1.f / dist;
        vec3f_t move;
        vec3f_scale(&move, &vp, (dist - fp.cam_dist_max) * fol_mspeed * norm);
        vec3f_add(&fp.cam_pos, &fp.cam_pos, &move);
    }

    if (!fp.lock_cam) {
        s32 x = zu_adjust_joystick(input_x());
        s32 y = zu_adjust_joystick(input_y());

        if (input_pad() & BUTTON_Z) {
            dist -= y * joy_mspeed;
            if (input_pad() & BUTTON_C_UP) {
                fp.cam_pitch += joy_max * joy_rspeed;
            }
            if (input_pad() & BUTTON_C_DOWN) {
                fp.cam_pitch += -joy_max * joy_rspeed;
            }
        } else {
            fp.cam_pitch += y * joy_rspeed;
            if (input_pad() & BUTTON_C_UP) {
                dist -= joy_max * joy_mspeed;
            }
            if (input_pad() & BUTTON_C_DOWN) {
                dist -= -joy_max * joy_mspeed;
            }
        }
        fp.cam_yaw -= x * joy_rspeed;
        if (input_pad() & BUTTON_C_LEFT) {
            fp.cam_yaw -= joy_max * joy_rspeed;
        }
        if (input_pad() & BUTTON_C_RIGHT) {
            fp.cam_yaw -= -joy_max * joy_rspeed;
        }

        if (fp.cam_pitch > pitch_lim) {
            fp.cam_pitch = pitch_lim;
        } else if (fp.cam_pitch < -pitch_lim) {
            fp.cam_pitch = -pitch_lim;
        }

        vec3f_t vfoc;
        vec3f_add(&vfoc, &fp.cam_pos, &vp);

        vec3f_t move;
        vec3f_py(&move, fp.cam_pitch, fp.cam_yaw);
        vec3f_scale(&move, &move, -dist);
        vec3f_add(&fp.cam_pos, &vfoc, &move);
    }
}

void fp_update_cam(void) {
    switch (fp.cam_bhv) {
        case CAMBHV_MANUAL: cam_manual(); break;
        case CAMBHV_BIRDSEYE: cam_birdseye(); break;
        case CAMBHV_RADIAL: cam_radial(); break;
    }
}
