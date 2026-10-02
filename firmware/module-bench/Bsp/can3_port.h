#ifndef CAN3_PORT_H
#define CAN3_PORT_H

#include <stdint.h>
#include "fdcan.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t identifier;
    uint8_t length;
    uint8_t data[8];
    uint32_t tick;
} can3_frame_t;

typedef struct
{
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t error_count;
    uint32_t protocol_status;
    can3_frame_t last_rx;
} can3_status_t;

void can3_port_init(void);
int can3_port_send(uint32_t identifier, const uint8_t *data, uint8_t length);
int can3_port_take(can3_frame_t *frame);
void can3_port_get_status(can3_status_t *status);

#ifdef __cplusplus
}
#endif

#endif
