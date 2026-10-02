#include "mavlink_min.h"
#include <string.h>

#define MAVLINK_STX_V2 0xFDU
#define MAVLINK_ATTITUDE_CRC 39U
#define MAVLINK_NAMED_VALUE_CRC 170U

static uint16_t crc_accumulate(uint8_t data, uint16_t crc)
{
    uint8_t temporary = data ^ (uint8_t)crc;
    temporary ^= (uint8_t)(temporary << 4U);
    return (uint16_t)((crc >> 8U) ^ ((uint16_t)temporary << 8U) ^
                      ((uint16_t)temporary << 3U) ^ (temporary >> 4U));
}

static uint16_t payload_crc(const uint8_t *payload, uint8_t length, uint8_t crc_extra)
{
    uint16_t crc = 0xFFFFU;
    for (uint8_t index = 0U; index < length; index++)
    {
        crc = crc_accumulate(payload[index], crc);
    }
    return crc_accumulate(crc_extra, crc);
}

static size_t encode(uint8_t *frame, size_t capacity, uint32_t message_id,
                     uint8_t crc_extra, const uint8_t *payload, uint8_t payload_length)
{
    uint16_t crc;
    static uint8_t sequence;

    if ((frame == NULL) || (payload == NULL) || (capacity < (size_t)(payload_length + 12U)))
    {
        return 0U;
    }

    frame[0] = MAVLINK_STX_V2;
    frame[1] = payload_length;
    frame[2] = 0U;
    frame[3] = 0U;
    frame[4] = sequence++;
    frame[5] = MAVLINK_SYSTEM_ID;
    frame[6] = MAVLINK_COMPONENT_ID;
    frame[7] = (uint8_t)message_id;
    frame[8] = (uint8_t)(message_id >> 8U);
    frame[9] = (uint8_t)(message_id >> 16U);
    memcpy(&frame[10], payload, payload_length);
    crc = payload_crc(&frame[1], (uint8_t)(payload_length + 9U), crc_extra);
    frame[10U + payload_length] = (uint8_t)crc;
    frame[11U + payload_length] = (uint8_t)(crc >> 8U);
    return (size_t)payload_length + 12U;
}

static void put_float(uint8_t *payload, float value)
{
    memcpy(payload, &value, sizeof(value));
}

size_t mavlink_encode_attitude(uint8_t *frame, size_t capacity, uint32_t time_ms,
                               float roll, float pitch, float yaw,
                               float rollspeed, float pitchspeed, float yawspeed)
{
    uint8_t payload[28] = {0};
    payload[0] = (uint8_t)time_ms;
    payload[1] = (uint8_t)(time_ms >> 8U);
    payload[2] = (uint8_t)(time_ms >> 16U);
    payload[3] = (uint8_t)(time_ms >> 24U);
    put_float(&payload[4], roll);
    put_float(&payload[8], pitch);
    put_float(&payload[12], yaw);
    put_float(&payload[16], rollspeed);
    put_float(&payload[20], pitchspeed);
    put_float(&payload[24], yawspeed);
    return encode(frame, capacity, 30U, MAVLINK_ATTITUDE_CRC, payload, sizeof(payload));
}

size_t mavlink_encode_named_value(uint8_t *frame, size_t capacity, uint32_t time_ms,
                                  const char *name, float value)
{
    uint8_t payload[18] = {0};
    payload[0] = (uint8_t)time_ms;
    payload[1] = (uint8_t)(time_ms >> 8U);
    payload[2] = (uint8_t)(time_ms >> 16U);
    payload[3] = (uint8_t)(time_ms >> 24U);
    put_float(&payload[4], value);
    if (name != NULL)
    {
        for (uint8_t index = 0U; index < 10U; index++)
        {
            payload[8U + index] = (uint8_t)name[index];
            if (name[index] == '\0')
            {
                break;
            }
        }
    }
    return encode(frame, capacity, 251U, MAVLINK_NAMED_VALUE_CRC, payload, sizeof(payload));
}

int mavlink_decode_command_long(const uint8_t *data, size_t length, uint16_t *command, float *param1)
{
    uint16_t received_crc;
    uint16_t calculated_crc;

    if ((data == NULL) || (command == NULL) || (param1 == NULL) || (length < 45U))
    {
        return -1;
    }
    if ((data[0] != MAVLINK_STX_V2) || (data[1] < 30U) || (data[7] != 76U))
    {
        return -1;
    }

    calculated_crc = payload_crc(&data[1], (uint8_t)(data[1] + 9U), 152U);
    received_crc = (uint16_t)data[10U + data[1]] | ((uint16_t)data[11U + data[1]] << 8U);
    if (received_crc != calculated_crc)
    {
        return -1;
    }

    *command = (uint16_t)data[38] | ((uint16_t)data[39] << 8U);
    memcpy(param1, &data[10], sizeof(float));
    return 0;
}
