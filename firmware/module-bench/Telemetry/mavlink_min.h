#ifndef MAVLINK_MIN_H
#define MAVLINK_MIN_H

#include <stddef.h>
#include <stdint.h>

#define MAVLINK_SYSTEM_ID 1U
#define MAVLINK_COMPONENT_ID 1U

size_t mavlink_encode_attitude(uint8_t *frame, size_t capacity, uint32_t time_ms,
                               float roll, float pitch, float yaw,
                               float rollspeed, float pitchspeed, float yawspeed);
size_t mavlink_encode_named_value(uint8_t *frame, size_t capacity, uint32_t time_ms,
                                  const char *name, float value);
int mavlink_decode_command_long(const uint8_t *data, size_t length, uint16_t *command, float *param1);

#endif
