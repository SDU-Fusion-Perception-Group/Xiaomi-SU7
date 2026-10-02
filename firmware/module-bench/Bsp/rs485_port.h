#ifndef RS485_PORT_H
#define RS485_PORT_H

#include <stdint.h>
#include <stddef.h>
#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RS485_RX_CAPACITY 256U

typedef struct
{
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t frame_count;
    uint32_t error_count;
    uint32_t last_rx_tick;
    uint16_t last_frame_length;
} rs485_status_t;

void rs485_port_init(void);
int rs485_port_send(const uint8_t *data, size_t length, uint32_t timeout_ms);
int rs485_port_take_frame(uint8_t *data, size_t capacity, size_t *length);
void rs485_port_get_status(rs485_status_t *status);

#ifdef __cplusplus
}
#endif

#endif
