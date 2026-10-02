#include "modbus_rtu.h"

uint16_t modbus_crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFFU;

    for (size_t index = 0U; index < length; index++)
    {
        crc ^= data[index];
        for (uint8_t bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
            }
            else
            {
                crc >>= 1U;
            }
        }
    }
    return crc;
}

static size_t append_crc(uint8_t *frame, size_t length)
{
    uint16_t crc = modbus_crc16(frame, length);
    frame[length++] = (uint8_t)(crc & 0xFFU);
    frame[length++] = (uint8_t)(crc >> 8U);
    return length;
}

size_t modbus_build_read_holding(uint8_t *frame, size_t capacity, uint8_t address,
                                 uint16_t register_address, uint16_t register_count)
{
    if ((frame == NULL) || (capacity < 8U) || (register_count == 0U) || (register_count > 16U))
    {
        return 0U;
    }

    frame[0] = address;
    frame[1] = 0x03U;
    frame[2] = (uint8_t)(register_address >> 8U);
    frame[3] = (uint8_t)(register_address & 0xFFU);
    frame[4] = (uint8_t)(register_count >> 8U);
    frame[5] = (uint8_t)(register_count & 0xFFU);
    return append_crc(frame, 6U);
}

size_t modbus_build_write_single(uint8_t *frame, size_t capacity, uint8_t address,
                                 uint16_t register_address, uint16_t value)
{
    if ((frame == NULL) || (capacity < 8U))
    {
        return 0U;
    }

    frame[0] = address;
    frame[1] = 0x06U;
    frame[2] = (uint8_t)(register_address >> 8U);
    frame[3] = (uint8_t)(register_address & 0xFFU);
    frame[4] = (uint8_t)(value >> 8U);
    frame[5] = (uint8_t)(value & 0xFFU);
    return append_crc(frame, 6U);
}

size_t modbus_build_write_multiple(uint8_t *frame, size_t capacity, uint8_t address,
                                   uint16_t register_address, const uint16_t *values, uint16_t count)
{
    size_t length;

    if ((frame == NULL) || (values == NULL) || (count == 0U) || (count > 16U) ||
        (capacity < (size_t)(9U + (count * 2U))))
    {
        return 0U;
    }

    frame[0] = address;
    frame[1] = 0x10U;
    frame[2] = (uint8_t)(register_address >> 8U);
    frame[3] = (uint8_t)(register_address & 0xFFU);
    frame[4] = (uint8_t)(count >> 8U);
    frame[5] = (uint8_t)(count & 0xFFU);
    frame[6] = (uint8_t)(count * 2U);
    for (uint16_t index = 0U; index < count; index++)
    {
        frame[7U + (index * 2U)] = (uint8_t)(values[index] >> 8U);
        frame[8U + (index * 2U)] = (uint8_t)(values[index] & 0xFFU);
    }
    length = 7U + ((size_t)count * 2U);
    return append_crc(frame, length);
}

int modbus_parse_write_response(const uint8_t *frame, size_t length, uint8_t address,
                                uint8_t function, uint16_t register_address)
{
    uint16_t received_crc;
    uint16_t calculated_crc;

    if ((frame == NULL) || (length != 8U) || (frame[0] != address) || (frame[1] != function))
    {
        return -1;
    }
    if ((((uint16_t)frame[2] << 8U) | frame[3]) != register_address)
    {
        return -1;
    }
    received_crc = (uint16_t)frame[6] | ((uint16_t)frame[7] << 8U);
    calculated_crc = modbus_crc16(frame, 6U);
    return received_crc == calculated_crc ? 0 : -1;
}

int modbus_parse_read_response(const uint8_t *frame, size_t length, uint8_t address,
                               uint16_t *values, size_t value_capacity, size_t *value_count)
{
    uint16_t received_crc;
    uint16_t calculated_crc;
    size_t count;

    if ((frame == NULL) || (values == NULL) || (value_count == NULL) || (length < 5U))
    {
        return -1;
    }
    if ((frame[0] != address) || (frame[1] != 0x03U))
    {
        return -1;
    }

    count = frame[2] / 2U;
    if ((frame[2] != (count * 2U)) || ((5U + frame[2]) != length) || (count > value_capacity))
    {
        return -1;
    }

    received_crc = (uint16_t)frame[length - 2U] | ((uint16_t)frame[length - 1U] << 8U);
    calculated_crc = modbus_crc16(frame, length - 2U);
    if (received_crc != calculated_crc)
    {
        return -1;
    }

    for (size_t index = 0U; index < count; index++)
    {
        size_t offset = 3U + (index * 2U);
        values[index] = ((uint16_t)frame[offset] << 8U) | frame[offset + 1U];
    }
    *value_count = count;
    return 0;
}
