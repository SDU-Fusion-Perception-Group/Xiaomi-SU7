#include "lyx_motor.h"
#include "modbus_rtu.h"
#include "rs485_port.h"

#define LYX_TIMEOUT_MS 40U

#define LYX_REG_SAVE             0x0000U
#define LYX_REG_ALARM            0x0008U
#define LYX_REG_ERROR_ANGLE      0x0010U
#define LYX_REG_CONTROL_MODE     0x0021U
#define LYX_REG_STALL_ANGLE      0x0022U
#define LYX_REG_STALL_OUTPUT     0x0023U
#define LYX_REG_FOLLOW_ANGLE     0x0024U
#define LYX_REG_FOLLOW_STOP      0x0025U
#define LYX_REG_SPEED_THRESHOLD  0x0026U
#define LYX_REG_BASE_CURRENT     0x0027U
#define LYX_REG_LEAD             0x0028U
#define LYX_REG_POSITION_I       0x002BU
#define LYX_REG_SUPER_FILTER     0x002CU
#define LYX_REG_START_RPM        0x003AU
#define LYX_REG_MAX_RPM          0x003BU
#define LYX_REG_ACCELERATION     0x003CU
#define LYX_REG_PULSE_HIGH       0x003DU
#define LYX_REG_MOTION_MODE      0x003FU
#define LYX_REG_MOTION_START     0x0040U

typedef struct
{
    uint8_t active;
    uint8_t address;
    uint8_t request[MODBUS_MAX_FRAME];
    size_t request_length;
    uint32_t start_tick;
} lyx_transaction_t;

static lyx_transaction_t transaction;

static int exchange_start(uint8_t address, const uint8_t *request, size_t request_length)
{
    if ((transaction.active != 0U) || (request == NULL) || (request_length == 0U) ||
        (request_length > sizeof(transaction.request)))
    {
        return -1;
    }
    for (size_t index = 0U; index < request_length; index++)
    {
        transaction.request[index] = request[index];
    }
    transaction.address = address;
    transaction.request_length = request_length;
    transaction.start_tick = HAL_GetTick();
    transaction.active = 1U;
    if (rs485_port_send(transaction.request, transaction.request_length, 0U) != 0)
    {
        transaction.active = 0U;
        return -1;
    }
    return 0;
}

static int exchange_poll(uint8_t *response, size_t response_capacity, size_t *response_length)
{
    if (transaction.active == 0U)
    {
        return -1;
    }
    if (rs485_port_take_frame(response, response_capacity, response_length) == 0)
    {
        transaction.active = 0U;
        return 0;
    }
    if ((HAL_GetTick() - transaction.start_tick) >= LYX_TIMEOUT_MS)
    {
        transaction.active = 0U;
        return -1;
    }
    return 1;
}

void lyx_motor_init(lyx_motor_t *motor, uint8_t address)
{
    if (motor == NULL)
    {
        return;
    }
    motor->address = address;
    motor->online = 0U;
    motor->alarm = LYX_ALARM_NONE;
    motor->control_mode = LYX_CONTROL_NORMAL_CLOSED_LOOP;
    motor->motion_active = 0U;
    motor->error_angle = 0;
    motor->speed_raw = 0;
    motor->position = 0;
    motor->timeout_count = 0U;
}

int lyx_motor_read(uint8_t address, uint16_t register_address, uint16_t count, uint16_t *values)
{
    uint8_t request[8];
    uint8_t response[MODBUS_MAX_FRAME];
    size_t request_length;
    size_t response_length = 0U;
    size_t value_count = 0U;
    int state;

    request_length = modbus_build_read_holding(request, sizeof(request), address, register_address, count);
    if ((request_length == 0U) || (exchange_start(address, request, request_length) != 0))
    {
        return -1;
    }
    state = exchange_poll(response, sizeof(response), &response_length);
    if (state != 0)
    {
        return state;
    }
    return (modbus_parse_read_response(response, response_length, address, values, count, &value_count) == 0) &&
           (value_count == count) ? 0 : -1;
}

int lyx_motor_write(uint8_t address, uint16_t register_address, uint16_t value)
{
    uint8_t request[8];
    uint8_t response[8];
    size_t request_length;
    size_t response_length = 0U;

    request_length = modbus_build_write_single(request, sizeof(request), address, register_address, value);
    if ((request_length == 0U) || (exchange_start(address, request, request_length) != 0))
    {
        return -1;
    }
    if (exchange_poll(response, sizeof(response), &response_length) != 0)
    {
        return 1;
    }
    return modbus_parse_write_response(response, response_length, address, 0x06U, register_address) == 0 ? 0 : -1;
}

int lyx_motor_save(uint8_t address)
{
    return lyx_motor_write(address, LYX_REG_SAVE, 1U);
}

int lyx_motor_set_control_mode(lyx_motor_t *motor, lyx_control_mode_t mode, uint8_t save)
{
    if ((motor == NULL) || (mode > LYX_CONTROL_TORQUE_CURRENT))
    {
        return -1;
    }
    if (lyx_motor_write(motor->address, LYX_REG_CONTROL_MODE, (uint16_t)mode) != 0)
    {
        return -1;
    }
    motor->control_mode = (uint16_t)mode;
    return save ? lyx_motor_save(motor->address) : 0;
}

int lyx_motor_configure_closed_loop(lyx_motor_t *motor, const lyx_closed_loop_config_t *config, uint8_t save)
{
    if ((motor == NULL) || (config == NULL))
    {
        return -1;
    }
    if ((lyx_motor_write(motor->address, LYX_REG_STALL_ANGLE, config->stall_angle) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_STALL_OUTPUT, config->stall_output_enable) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_FOLLOW_ANGLE, config->following_error_angle) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_FOLLOW_STOP, config->following_error_stop_enable) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_SPEED_THRESHOLD, config->activation_or_integral_separation_rpm) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_BASE_CURRENT, config->base_current_div_128) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_LEAD, config->lead_coefficient) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_POSITION_I, config->position_integral) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_SUPER_FILTER, config->super_filter) != 0))
    {
        return -1;
    }
    return save ? lyx_motor_save(motor->address) : 0;
}

int lyx_motor_command_pending(void)
{
    return transaction.active ? 1 : 0;
}

int lyx_motor_poll(lyx_motor_t *motor)
{
    uint16_t status[9];
    uint16_t motion;
    static uint8_t phase;
    int result;

    if (motor == NULL)
    {
        return -1;
    }
    if (phase == 0U)
    {
        result = lyx_motor_read(motor->address, LYX_REG_ALARM, 9U, status);
        if (result == 1)
        {
            return 1;
        }
        if (result != 0)
        {
            motor->online = 0U;
            motor->timeout_count++;
            return -1;
        }
        motor->alarm = status[0];
        motor->error_angle = (int16_t)status[8];
        motor->position = (int32_t)(((uint32_t)status[4] << 16U) | status[5]);
        motor->speed_raw = (int16_t)status[6];
        phase = 1U;
        return 1;
    }

    result = lyx_motor_read(motor->address, LYX_REG_MOTION_START, 1U, &motion);
    if (result == 1)
    {
        return 1;
    }
    phase = 0U;
    if (result != 0)

    motor->online = 1U;
    motor->alarm = status[0];
    motor->error_angle = (int16_t)status[8];
    motor->position = (int32_t)(((uint32_t)status[4] << 16U) | status[5]);
    motor->speed_raw = (int16_t)status[6];
    motor->motion_active = motion;
    return 0;
}

int lyx_motor_start_motion(lyx_motor_t *motor, const lyx_motion_t *motion)
{
    uint8_t request[17];
    uint8_t response[8];
    uint16_t pulses[2];
    size_t request_length;
    size_t response_length = 0U;

    if ((motor == NULL) || (motion == NULL) || (motion->mode > LYX_MOTION_HOME_STALL_REVERSE) ||
        (motion->maximum_rpm < motion->start_rpm))
    {
        return -1;
    }
    pulses[0] = (uint16_t)(motion->pulses >> 16U);
    pulses[1] = (uint16_t)motion->pulses;
    request_length = modbus_build_write_multiple(request, sizeof(request), motor->address,
                                                 LYX_REG_PULSE_HIGH, pulses, 2U);
    if ((lyx_motor_write(motor->address, LYX_REG_START_RPM, motion->start_rpm) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_MAX_RPM, motion->maximum_rpm) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_ACCELERATION, motion->acceleration_rpm_s) != 0) ||
        (request_length == 0U) ||
        (exchange_start(motor->address, request, request_length) != 0) ||
        (exchange_poll(response, sizeof(response), &response_length) != 0) ||
        (modbus_parse_write_response(response, response_length, motor->address, 0x10U, LYX_REG_PULSE_HIGH) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_MOTION_MODE, (uint16_t)motion->mode) != 0) ||
        (lyx_motor_write(motor->address, LYX_REG_MOTION_START, 1U) != 0))
    {
        return -1;
    }
    motor->motion_active = 1U;
    return 0;
}

int lyx_motor_stop(lyx_motor_t *motor)
{
    if (motor == NULL)
    {
        return -1;
    }
    if (lyx_motor_write(motor->address, LYX_REG_MOTION_START, 0U) != 0)
    {
        return -1;
    }
    motor->motion_active = 0U;
    return 0;
}

int lyx_motor_move_position(lyx_motor_t *motor, uint8_t reverse, uint16_t start_rpm,
                            uint16_t max_rpm, uint16_t acceleration_rpm_s, uint32_t pulses)
{
    lyx_motion_t motion;
    motion.start_rpm = start_rpm;
    motion.maximum_rpm = max_rpm;
    motion.acceleration_rpm_s = acceleration_rpm_s;
    motion.pulses = pulses;
    motion.mode = reverse ? LYX_MOTION_POSITION_REVERSE : LYX_MOTION_POSITION_FORWARD;
    return lyx_motor_start_motion(motor, &motion);
}
