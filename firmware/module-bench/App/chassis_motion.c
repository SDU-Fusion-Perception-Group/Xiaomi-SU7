#include "chassis_motion.h"
#include "board_config.h"

#define CHASSIS_RPM 60U
#define CHASSIS_ACCELERATION 60U

typedef struct
{
    uint8_t active;
    uint8_t reverse;
    uint8_t step;
    uint8_t motor_index;
    uint32_t pulses;
} chassis_job_t;

static chassis_job_t jobs[2];

static uint32_t pulses_for_distance(uint32_t millimeters)
{
    return (uint32_t)(((uint64_t)millimeters * BOARD_WHEEL_PULSES_PER_REV) /
                      (3141UL * (uint32_t)BOARD_WHEEL_DIAMETER_MM / 1000UL));
}

int chassis_motion_poll(lyx_motor_t *left, lyx_motor_t *right)
{
    lyx_motor_t *motors[2] = {left, right};
    for (uint8_t index = 0U; index < 2U; index++)
    {
        if ((jobs[index].active == 0U) || (lyx_motor_command_pending() != 0))
        {
            continue;
        }
        if (jobs[index].step == 0U)
        {
            if (lyx_motor_move_position(motors[index], jobs[index].reverse, 30U, CHASSIS_RPM,
                                        CHASSIS_ACCELERATION, jobs[index].pulses) == 0)
            {
                jobs[index].active = 0U;
            }
        }
        else if (lyx_motor_stop(motors[index]) == 0)
        {
            jobs[index].active = 0U;
        }
    }
    return (jobs[0].active != 0U) || (jobs[1].active != 0U);
}

static int queue_motion(uint32_t left_pulses, uint8_t left_reverse, uint32_t right_pulses, uint8_t right_reverse)
{
    if ((jobs[0].active != 0U) || (jobs[1].active != 0U))
    {
        return -1;
    }
    jobs[0] = (chassis_job_t){1U, left_reverse, 0U, 0U, left_pulses};
    jobs[1] = (chassis_job_t){1U, right_reverse, 0U, 1U, right_pulses};
    return 0;
}

int chassis_motion_drive_cm(lyx_motor_t *left, lyx_motor_t *right, int32_t centimeters)
{
    uint32_t pulses;
    uint8_t reverse;
    (void)left;
    (void)right;
    reverse = centimeters < 0 ? 1U : 0U;
    pulses = pulses_for_distance((uint32_t)(centimeters < 0 ? -centimeters : centimeters) * 10UL);
    return queue_motion(pulses, reverse, pulses, reverse);
}

int chassis_motion_rotate_deg(lyx_motor_t *left, lyx_motor_t *right, int32_t degrees)
{
    uint32_t arc;
    uint32_t pulses;
    (void)left;
    (void)right;
    arc = (uint32_t)(((uint64_t)(degrees < 0 ? -degrees : degrees) * 3141UL * BOARD_WHEEL_TRACK_MM) / 360000UL);
    pulses = pulses_for_distance(arc);
    return degrees >= 0 ? queue_motion(pulses, 0U, pulses, 1U) : queue_motion(pulses, 1U, pulses, 0U);
}

int chassis_motion_stop(lyx_motor_t *left, lyx_motor_t *right)
{
    (void)left;
    (void)right;
    jobs[0] = (chassis_job_t){1U, 0U, 1U, 0U, 0U};
    jobs[1] = (chassis_job_t){1U, 0U, 1U, 1U, 0U};
    return 0;
}
