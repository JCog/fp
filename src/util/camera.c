#include "fp.h"
#include "gu.h"
#include "sys/input.h"
#include "util/geometry.h"
#include <math.h>
#include <n64.h>

static const f32 joyMspeed = 0.25f;
static const f32 joyRspeed = 0.0007f;
static const s32 joyMax = 60.f;
static const f32 pitchLim = M_PI / 2.f - joyRspeed;
static const f32 folMspeed = 1.f / 3.f;
static const f32 folRspeed = 1.f / 3.f;

s32 adjustJoystick(s32 v) {
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

static void camManual(void) {
    if (!fp.lockCam) {
        s32 x = adjustJoystick(inputX());
        s32 y = adjustJoystick(inputY());

        Vec3f vf;
        Vec3f vr;
        Vec3f move;
        vec3fPy(&vf, fp.camPitch, fp.camYaw);
        vec3fPy(&vr, 0.f, fp.camYaw - M_PI / 2.f);

        if (inputPad() & BUTTON_Z) {
            vec3fScale(&move, &vf, y * joyMspeed);
            vec3fAdd(&fp.camPos, &fp.camPos, &move);

            vec3fScale(&move, &vr, x * joyMspeed);
            vec3fAdd(&fp.camPos, &fp.camPos, &move);

            if (inputPad() & BUTTON_C_UP) {
                fp.camPos.y += joyMax * joyMspeed;
            }
            if (inputPad() & BUTTON_C_DOWN) {
                fp.camPos.y += -joyMax * joyMspeed;
            }
            if (inputPad() & BUTTON_C_RIGHT) {
                fp.camYaw -= joyMax * joyRspeed;
            }
            if (inputPad() & BUTTON_C_LEFT) {
                fp.camYaw -= -joyMax * joyRspeed;
            }
        } else {
            fp.camPitch += y * joyRspeed;
            fp.camYaw -= x * joyRspeed;

            if (inputPad() & BUTTON_C_UP) {
                vec3fScale(&move, &vf, joyMax * joyMspeed);
                vec3fAdd(&fp.camPos, &fp.camPos, &move);
            }
            if (inputPad() & BUTTON_C_DOWN) {
                vec3fScale(&move, &vf, -joyMax * joyMspeed);
                vec3fAdd(&fp.camPos, &fp.camPos, &move);
            }
            if (inputPad() & BUTTON_C_RIGHT) {
                vec3fScale(&move, &vr, joyMax * joyMspeed);
                vec3fAdd(&fp.camPos, &fp.camPos, &move);
            }
            if (inputPad() & BUTTON_C_LEFT) {
                vec3fScale(&move, &vr, -joyMax * joyMspeed);
                vec3fAdd(&fp.camPos, &fp.camPos, &move);
            }
        }

        if (fp.camPitch > pitchLim) {
            fp.camPitch = pitchLim;
        } else if (fp.camPitch < -pitchLim) {
            fp.camPitch = -pitchLim;
        }
    }
}

static void camBirdseye(void) {
    Vec3f vt;
    vt.x = pm_gPlayerStatus.position.x;
    vt.y = pm_gPlayerStatus.position.y;
    vt.z = pm_gPlayerStatus.position.z;

    Vec3f vd;
    vec3fSub(&vd, &vt, &fp.camPos);

    f32 pitch, yaw;
    vec3fPyangles(&vd, &pitch, &yaw);

    f32 dPitch = angleDif(pitch, fp.camPitch);
    if (fabsf(dPitch) < .001f) {
        fp.camPitch = pitch;
    } else {
        fp.camPitch += dPitch * folRspeed;
    }

    f32 dYaw = angleDif(yaw, fp.camYaw);
    if (fabsf(dYaw) < .001f) {
        fp.camYaw = yaw;
    } else {
        fp.camYaw += dYaw * folRspeed;
    }

    f32 dist = vec3fMag(&vd);
    if (dist < fp.camDistMin) {
        Vec3f move;
        vec3fPy(&move, fp.camPitch, fp.camYaw);
        vec3fScale(&move, &move, (dist - fp.camDistMin) * folMspeed);
        vec3fAdd(&fp.camPos, &fp.camPos, &move);
    } else if (dist > fp.camDistMax) {
        Vec3f move;
        vec3fPy(&move, fp.camPitch, fp.camYaw);
        vec3fScale(&move, &move, (dist - fp.camDistMax) * folMspeed);
        vec3fAdd(&fp.camPos, &fp.camPos, &move);
    }

    camManual();
}

static void camRadial(void) {
    Vec3f vf;
    vec3fPy(&vf, fp.camPitch, fp.camYaw);

    Vec3f vt;
    vt.x = pm_gPlayerStatus.position.x;
    vt.y = pm_gPlayerStatus.position.y;
    vt.z = pm_gPlayerStatus.position.z;

    Vec3f vd;
    vec3fSub(&vd, &vt, &fp.camPos);

    f32 dist = vec3fDot(&vd, &vf);
    Vec3f vp;
    vec3fScale(&vp, &vf, dist);

    Vec3f vr;
    vec3fSub(&vr, &vd, &vp);
    {
        Vec3f move;
        vec3fScale(&move, &vr, folMspeed);
        vec3fAdd(&fp.camPos, &fp.camPos, &move);
    }

    if (dist < fp.camDistMin) {
        f32 norm = 1.f / dist;
        Vec3f move;
        vec3fScale(&move, &vp, (dist - fp.camDistMin) * folMspeed * norm);
        vec3fAdd(&fp.camPos, &fp.camPos, &move);
    } else if (dist > fp.camDistMax) {
        f32 norm = 1.f / dist;
        Vec3f move;
        vec3fScale(&move, &vp, (dist - fp.camDistMax) * folMspeed * norm);
        vec3fAdd(&fp.camPos, &fp.camPos, &move);
    }

    if (!fp.lockCam) {
        s32 x = adjustJoystick(inputX());
        s32 y = adjustJoystick(inputY());

        if (inputPad() & BUTTON_Z) {
            dist -= y * joyMspeed;
            if (inputPad() & BUTTON_C_UP) {
                fp.camPitch += joyMax * joyRspeed;
            }
            if (inputPad() & BUTTON_C_DOWN) {
                fp.camPitch += -joyMax * joyRspeed;
            }
        } else {
            fp.camPitch += y * joyRspeed;
            if (inputPad() & BUTTON_C_UP) {
                dist -= joyMax * joyMspeed;
            }
            if (inputPad() & BUTTON_C_DOWN) {
                dist -= -joyMax * joyMspeed;
            }
        }
        fp.camYaw -= x * joyRspeed;
        if (inputPad() & BUTTON_C_LEFT) {
            fp.camYaw -= joyMax * joyRspeed;
        }
        if (inputPad() & BUTTON_C_RIGHT) {
            fp.camYaw -= -joyMax * joyRspeed;
        }

        if (fp.camPitch > pitchLim) {
            fp.camPitch = pitchLim;
        } else if (fp.camPitch < -pitchLim) {
            fp.camPitch = -pitchLim;
        }

        Vec3f vfoc;
        vec3fAdd(&vfoc, &fp.camPos, &vp);

        Vec3f move;
        vec3fPy(&move, fp.camPitch, fp.camYaw);
        vec3fScale(&move, &move, -dist);
        vec3fAdd(&fp.camPos, &vfoc, &move);
    }
}

void fpUpdateCam(void) {
    switch (fp.camBhv) {
        case CAMBHV_MANUAL: camManual(); break;
        case CAMBHV_BIRDSEYE: camBirdseye(); break;
        case CAMBHV_RADIAL: camRadial(); break;
    }
}