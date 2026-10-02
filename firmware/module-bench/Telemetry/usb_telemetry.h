#ifndef USB_TELEMETRY_H
#define USB_TELEMETRY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float yaw_rad;
    float pitch_rad;
    float roll_rad;
    float gyro_dps[3];
    float accel_g[3];
    float temperature_c;
    uint8_t imu_valid;
    uint8_t motor_online;
    uint16_t motor_alarm;
    int32_t motor_position;
    int16_t motor_speed_raw;
    uint32_t rs485_frames;
    uint32_t rs485_errors;
    uint32_t can_rx;
    uint32_t can_tx;
    uint32_t can_errors;
    uint32_t uptime_ms;
} telemetry_snapshot_t;

void usb_telemetry_init(void);
int usb_telemetry_send(const telemetry_snapshot_t *snapshot);
int usb_telemetry_take_command(char *line, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
