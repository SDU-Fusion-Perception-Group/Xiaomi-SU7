#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_MAX_FRAME 64U

uint16_t modbus_crc16(const uint8_t *data, size_t length);
size_t modbus_build_read_holding(uint8_t *frame, size_t capacity, uint8_t address,
                                 uint16_t register_address, uint16_t register_count);
size_t modbus_build_write_single(uint8_t *frame, size_t capacity, uint8_t address,
                                 uint16_t register_address, uint16_t value);
size_t modbus_build_write_multiple(uint8_t *frame, size_t capacity, uint8_t address,
                                   uint16_t register_address, const uint16_t *values, uint16_t count);
int modbus_parse_read_response(const uint8_t *frame, size_t length, uint8_t address,
                               uint16_t *values, size_t value_capacity, size_t *value_count);
int modbus_parse_write_response(const uint8_t *frame, size_t length, uint8_t address,
                                uint8_t function, uint16_t register_address);

#ifdef __cplusplus
}
#endif

#endif
