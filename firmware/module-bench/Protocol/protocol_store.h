#ifndef PROTOCOL_STORE_H
#define PROTOCOL_STORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_FLASH_BASE        0x90000000UL
#define PROTOCOL_FLASH_CAPACITY    0x00800000UL
#define PROTOCOL_MAGIC             0x4C595850UL
#define PROTOCOL_VERSION           2U

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint32_t baudrate;
    uint8_t default_address;
    uint8_t enable_active_high;
    uint8_t control_mode;
    uint8_t reserved;
    uint16_t start_rpm;
    uint16_t maximum_rpm;
    uint16_t acceleration_rpm_s;
    uint16_t pulses_per_revolution;
    uint32_t checksum;
} protocol_profile_t;

int protocol_store_mount(void);
const protocol_profile_t *protocol_store_profile(void);

#ifdef __cplusplus
}
#endif

#endif
