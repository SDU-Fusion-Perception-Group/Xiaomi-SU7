#include "usb_telemetry.h"
#include "board_config.h"
#include "mavlink_min.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>

#define COMMAND_CAPACITY 128U

static char command[COMMAND_CAPACITY];
static volatile uint16_t command_length;
static volatile uint8_t command_ready;

void usb_telemetry_init(void)
{
    command_length = 0U;
    command_ready = 0U;
}

int usb_telemetry_send(const telemetry_snapshot_t *snapshot)
{
    uint8_t frame[64];
    size_t length;
    static uint8_t slice;
    static uint32_t last_attitude_ms;
    const char *names[] = {"gx", "gy", "gz", "ax", "ay", "az", "temp", "motor", "alarm", "pos", "rpm", "rsf", "rse", "crx", "ctx", "cerr"};
    float values[16];

    if (snapshot == NULL)
    {
        return -1;
    }
    values[0] = snapshot->gyro_dps[0];
    values[1] = snapshot->gyro_dps[1];
    values[2] = snapshot->gyro_dps[2];
    values[3] = snapshot->accel_g[0];
    values[4] = snapshot->accel_g[1];
    values[5] = snapshot->accel_g[2];
    values[6] = snapshot->temperature_c;
    values[7] = snapshot->motor_online;
    values[8] = snapshot->motor_alarm;
    values[9] = (float)snapshot->motor_position;
    values[10] = snapshot->motor_speed_raw;
    values[11] = (float)snapshot->rs485_frames;
    values[12] = (float)snapshot->rs485_errors;
    values[13] = (float)snapshot->can_rx;
    values[14] = (float)snapshot->can_tx;
    values[15] = (float)snapshot->can_errors;

    if ((snapshot->uptime_ms - last_attitude_ms) >= (1000U / BOARD_USB_TELEMETRY_HZ))
    {
        last_attitude_ms = snapshot->uptime_ms;
        length = mavlink_encode_attitude(frame, sizeof(frame), snapshot->uptime_ms,
                                         snapshot->roll_rad, snapshot->pitch_rad, snapshot->yaw_rad,
                                         snapshot->gyro_dps[0] * (3.14159265f / 180.0f),
                                         snapshot->gyro_dps[1] * (3.14159265f / 180.0f),
                                         snapshot->gyro_dps[2] * (3.14159265f / 180.0f));
    }
    else
    {
        uint8_t value_index = slice;
        if (value_index >= 16U)
        {
            value_index = 0U;
        }
        length = mavlink_encode_named_value(frame, sizeof(frame), snapshot->uptime_ms,
                                            names[value_index], values[value_index]);
    }
    slice = (uint8_t)((slice + 1U) % 17U);
    if (length == 0U)
    {
        return -1;
    }
    return CDC_Transmit_HS(frame, (uint16_t)length) == USBD_OK ? 0 : -1;
}

int usb_telemetry_take_command(char *line, size_t capacity)
{
    uint32_t primask;
    size_t length;

    if ((line == NULL) || (capacity < 2U))
    {
        return -1;
    }
    primask = __get_PRIMASK();
    __disable_irq();
    if (command_ready == 0U)
    {
        if (primask == 0U)
        {
            __enable_irq();
        }
        return -1;
    }
    length = command_length;
    if (length >= capacity)
    {
        length = capacity - 1U;
    }
    memcpy(line, command, length);
    line[length] = '\0';
    command_ready = 0U;
    command_length = 0U;
    if (primask == 0U)
    {
        __enable_irq();
    }
    return 0;
}

void usb_telemetry_rx(const uint8_t *data, uint32_t length)
{
    for (uint32_t index = 0U; index < length; index++)
    {
        char value = (char)data[index];
        if ((value == '\r') || (value == '\n'))
        {
            if (command_length > 0U)
            {
                command_ready = 1U;
            }
            continue;
        }
        if (command_ready == 0U && command_length < (COMMAND_CAPACITY - 1U))
        {
            command[command_length++] = value;
        }
    }
}
