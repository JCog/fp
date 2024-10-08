#include "fp.h"
#include "gu.h"
#include "sys/input.h"
#include "util/geometry.h"
#include <math.h>
#include <n64.h>

static const f32 joyMax = 60.f;
static const f32 folMspeed = 1.f / 3.f;
static const f32 folRspeed = 1.f / 3.f;

static f32 getFreeCamMoveSpeed() {
    return 0.001f * fp.freeCamMoveSpeed;
}
static f32 getFreeCamPanSpeed() {
    return 0.00001f * fp.freeCamPanSpeed;
}

#define PITCH_LIM (M_PI / 2.f - getFreeCamPanSpeed())

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
        vec3fPy(&vf, fp.cam.pitch, fp.cam.yaw);
        vec3fPy(&vr, 0.f, fp.cam.yaw - M_PI / 2.f);
        f32 joyMspeed = getFreeCamMoveSpeed();
        f32 joyRspeed = getFreeCamPanSpeed();

        if (inputPad().z) {
            vec3fScale(&move, &vf, y * joyMspeed);
            vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);

            vec3fScale(&move, &vr, x * joyMspeed);
            vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);

            if (inputPad().cUp) {
                fp.cam.eye.y += joyMax * joyMspeed;
            }
            if (inputPad().cDown) {
                fp.cam.eye.y += -joyMax * joyMspeed;
            }
            if (inputPad().cRight) {
                fp.cam.yaw -= joyMax * joyRspeed;
            }
            if (inputPad().cLeft) {
                fp.cam.yaw -= -joyMax * joyRspeed;
            }
        } else {
            fp.cam.pitch += y * joyRspeed;
            fp.cam.yaw -= x * joyRspeed;

            if (inputPad().cUp) {
                vec3fScale(&move, &vf, joyMax * joyMspeed);
                vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
            }
            if (inputPad().cDown) {
                vec3fScale(&move, &vf, -joyMax * joyMspeed);
                vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
            }
            if (inputPad().cRight) {
                vec3fScale(&move, &vr, joyMax * joyMspeed);
                vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
            }
            if (inputPad().cLeft) {
                vec3fScale(&move, &vr, -joyMax * joyMspeed);
                vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
            }
        }

        if (fp.cam.pitch > PITCH_LIM) {
            fp.cam.pitch = PITCH_LIM;
        } else if (fp.cam.pitch < -PITCH_LIM) {
            fp.cam.pitch = -PITCH_LIM;
        }
    }
}

static void camBirdseye(void) {
    Vec3f vt;
    vt.x = pm_gPlayerStatus.position.x;
    vt.y = pm_gPlayerStatus.position.y;
    vt.z = pm_gPlayerStatus.position.z;

    Vec3f vd;
    vec3fSub(&vd, &vt, &fp.cam.eye);

    f32 pitch, yaw;
    vec3fPyangles(&vd, &pitch, &yaw);

    f32 dPitch = angleDif(pitch, fp.cam.pitch);
    if (fabsf(dPitch) < .001f) {
        fp.cam.pitch = pitch;
    } else {
        fp.cam.pitch += dPitch * folRspeed;
    }

    f32 dYaw = angleDif(yaw, fp.cam.yaw);
    if (fabsf(dYaw) < .001f) {
        fp.cam.yaw = yaw;
    } else {
        fp.cam.yaw += dYaw * folRspeed;
    }

    f32 dist = vec3fMag(&vd);
    if (dist < fp.camDistMin) {
        Vec3f move;
        vec3fPy(&move, fp.cam.pitch, fp.cam.yaw);
        vec3fScale(&move, &move, (dist - fp.camDistMin) * folMspeed);
        vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
    } else if (dist > fp.camDistMax) {
        Vec3f move;
        vec3fPy(&move, fp.cam.pitch, fp.cam.yaw);
        vec3fScale(&move, &move, (dist - fp.camDistMax) * folMspeed);
        vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
    }

    camManual();
}

static void camRadial(void) {
    Vec3f vf;
    vec3fPy(&vf, fp.cam.pitch, fp.cam.yaw);

    Vec3f vt;
    vt.x = pm_gPlayerStatus.position.x;
    vt.y = pm_gPlayerStatus.position.y;
    vt.z = pm_gPlayerStatus.position.z;

    Vec3f vd;
    vec3fSub(&vd, &vt, &fp.cam.eye);

    f32 dist = vec3fDot(&vd, &vf);
    Vec3f vp;
    vec3fScale(&vp, &vf, dist);

    Vec3f vr;
    vec3fSub(&vr, &vd, &vp);
    {
        Vec3f move;
        vec3fScale(&move, &vr, folMspeed);
        vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
    }

    if (dist < fp.camDistMin) {
        f32 norm = 1.f / dist;
        Vec3f move;
        vec3fScale(&move, &vp, (dist - fp.camDistMin) * folMspeed * norm);
        vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
    } else if (dist > fp.camDistMax) {
        f32 norm = 1.f / dist;
        Vec3f move;
        vec3fScale(&move, &vp, (dist - fp.camDistMax) * folMspeed * norm);
        vec3fAdd(&fp.cam.eye, &fp.cam.eye, &move);
    }

    if (!fp.lockCam) {
        s32 x = adjustJoystick(inputX());
        s32 y = adjustJoystick(inputY());
        f32 joyMspeed = getFreeCamMoveSpeed();
        f32 joyRspeed = getFreeCamPanSpeed();

        if (inputPad().z) {
            dist -= y * joyMspeed;
            if (inputPad().cUp) {
                fp.cam.pitch += joyMax * joyRspeed;
            }
            if (inputPad().cDown) {
                fp.cam.pitch += -joyMax * joyRspeed;
            }
        } else {
            fp.cam.pitch += y * joyRspeed;
            if (inputPad().cUp) {
                dist -= joyMax * joyMspeed;
            }
            if (inputPad().cDown) {
                dist -= -joyMax * joyMspeed;
            }
        }
        fp.cam.yaw -= x * joyRspeed;
        if (inputPad().cLeft) {
            fp.cam.yaw -= joyMax * joyRspeed;
        }
        if (inputPad().cRight) {
            fp.cam.yaw -= -joyMax * joyRspeed;
        }

        if (fp.cam.pitch > PITCH_LIM) {
            fp.cam.pitch = PITCH_LIM;
        } else if (fp.cam.pitch < -PITCH_LIM) {
            fp.cam.pitch = -PITCH_LIM;
        }

        Vec3f vfoc;
        vec3fAdd(&vfoc, &fp.cam.eye, &vp);

        Vec3f move;
        vec3fPy(&move, fp.cam.pitch, fp.cam.yaw);
        vec3fScale(&move, &move, -dist);
        vec3fAdd(&fp.cam.eye, &vfoc, &move);
    }
}

void fpUpdateCam(void) {
    switch (fp.camBhv) {
        case CAMBHV_MANUAL: camManual(); break;
        case CAMBHV_BIRDSEYE: camBirdseye(); break;
        case CAMBHV_RADIAL: camRadial(); break;
    }
}
