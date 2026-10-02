#ifndef CHASSIS_MOTION_H
#define CHASSIS_MOTION_H

#include "lyx_motor.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int chassis_motion_poll(lyx_motor_t *left, lyx_motor_t *right);
int chassis_motion_drive_cm(lyx_motor_t *left, lyx_motor_t *right, int32_t centimeters);
int chassis_motion_rotate_deg(lyx_motor_t *left, lyx_motor_t *right, int32_t degrees);
int chassis_motion_stop(lyx_motor_t *left, lyx_motor_t *right);

#ifdef __cplusplus
}
#endif

#endif
