#ifndef DJI_3508_H
#define DJI_3508_H

#include <stdint.h>
#include "fdcan.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DJI_3508_MOTOR_INDEX     1U
#define DJI_3508_CONTROL_ID      0x200U
#define DJI_3508_FEEDBACK_BASE   0x201U
#define DJI_3508_CURRENT_LIMIT   16384

typedef struct
{
    uint8_t online;
    uint16_t angle_raw;
    int16_t speed_rpm;
    int16_t current_raw;
    uint8_t temperature_c;
    uint32_t last_tick;
} dji_3508_t;

typedef struct
{
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t tx_errors;
} dji_3508_bus_status_t;

void dji_3508_init(void);
void dji_3508_set_current(uint8_t index, int16_t current);
void dji_3508_poll(void);
void dji_3508_on_rx(const FDCAN_RxHeaderTypeDef *header, const uint8_t data[8]);
const dji_3508_t *dji_3508_status(uint8_t index);
void dji_3508_get_bus_status(dji_3508_bus_status_t *status);

#ifdef __cplusplus
}
#endif

#endif
