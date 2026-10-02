#ifndef STATUS_INDICATOR_H
#define STATUS_INDICATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    STATUS_BOOT = 0,
    STATUS_SELF_TEST,
    STATUS_IMU_ERROR,
    STATUS_LINKS_BOTH_OK,
    STATUS_RS485_ONLY_ERROR,
    STATUS_CAN_ONLY_ERROR,
    STATUS_LINKS_BOTH_ERROR
} status_code_t;

void status_indicator_set(status_code_t status);
void status_indicator_update(uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif
