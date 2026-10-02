#ifndef LYX_MOTOR_H
#define LYX_MOTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 0x21 is only effective while the hardware MOD pin is low. MOD high forces open loop. */
typedef enum
{
    LYX_CONTROL_OPEN_LOOP = 0,
    LYX_CONTROL_NORMAL_CLOSED_LOOP = 1,
    LYX_CONTROL_SUPER_CLOSED_LOOP = 2,
    LYX_CONTROL_SERVO_CLOSED_LOOP = 3,
    LYX_CONTROL_TORQUE_CURRENT = 4
} lyx_control_mode_t;

typedef enum
{
    LYX_MOTION_POSITION_FORWARD = 0,
    LYX_MOTION_POSITION_REVERSE = 1,
    LYX_MOTION_SPEED_FORWARD = 2,
    LYX_MOTION_SPEED_REVERSE = 3,
    LYX_MOTION_HOME_STALL_FORWARD = 4,
    LYX_MOTION_HOME_STALL_REVERSE = 5
} lyx_motion_mode_t;

typedef enum
{
    LYX_ALARM_NONE = 0,
    LYX_ALARM_OVERCURRENT = 1,
    LYX_ALARM_MOTOR_DISCONNECTED = 2,
    LYX_ALARM_COIL_ABNORMAL = 3,
    LYX_ALARM_FOLLOWING_ERROR = 4,
    LYX_ALARM_STALL = 5
} lyx_alarm_t;

typedef struct
{
    uint16_t start_rpm;
    uint16_t maximum_rpm;
    uint16_t acceleration_rpm_s;
    uint32_t pulses;
    lyx_motion_mode_t mode;
} lyx_motion_t;

typedef struct
{
    uint16_t stall_angle;
    uint8_t stall_output_enable;
    uint16_t following_error_angle;
    uint8_t following_error_stop_enable;
    uint16_t activation_or_integral_separation_rpm;
    uint8_t base_current_div_128;
    uint8_t lead_coefficient;
    uint8_t position_integral;
    uint8_t super_filter;
} lyx_closed_loop_config_t;

typedef struct
{
    uint8_t address;
    uint8_t online;
    uint16_t alarm;
    uint16_t control_mode;
    uint16_t motion_active;
    int16_t error_angle;
    int16_t speed_raw;
    int32_t position;
    uint32_t timeout_count;
} lyx_motor_t;

void lyx_motor_init(lyx_motor_t *motor, uint8_t address);
int lyx_motor_read(uint8_t address, uint16_t register_address, uint16_t count, uint16_t *values);
int lyx_motor_write(uint8_t address, uint16_t register_address, uint16_t value);
int lyx_motor_save(uint8_t address);

int lyx_motor_set_control_mode(lyx_motor_t *motor, lyx_control_mode_t mode, uint8_t save);
int lyx_motor_configure_closed_loop(lyx_motor_t *motor, const lyx_closed_loop_config_t *config, uint8_t save);
int lyx_motor_poll(lyx_motor_t *motor);
int lyx_motor_command_pending(void);
int lyx_motor_start_motion(lyx_motor_t *motor, const lyx_motion_t *motion);
int lyx_motor_stop(lyx_motor_t *motor);
int lyx_motor_move_position(lyx_motor_t *motor, uint8_t reverse, uint16_t start_rpm,
                            uint16_t max_rpm, uint16_t acceleration_rpm_s, uint32_t pulses);

#ifdef __cplusplus
}
#endif

#endif
