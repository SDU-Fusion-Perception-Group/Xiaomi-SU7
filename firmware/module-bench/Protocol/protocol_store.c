#include "protocol_store.h"
#include "w25q64.h"

static protocol_profile_t cached_profile;

static const protocol_profile_t external_profile = {
    PROTOCOL_MAGIC,
    PROTOCOL_VERSION,
    (uint16_t)sizeof(protocol_profile_t),
    115200UL,
    1U,
    1U,
    2U,
    0U,
    30U,
    60U,
    60U,
    51200U,
    0UL
};

static uint32_t checksum(const protocol_profile_t *profile)
{
    const uint8_t *bytes = (const uint8_t *)profile;
    uint32_t value = 0xFFFFFFFFUL;
    for (uint32_t index = 0U; index < (sizeof(*profile) - sizeof(profile->checksum)); index++)
    {
        value ^= bytes[index];
        for (uint8_t bit = 0U; bit < 8U; bit++)
        {
            value = ((value & 1U) != 0U) ? ((value >> 1U) ^ 0xEDB88320UL) : (value >> 1U);
        }
    }
    return ~value;
}

static int valid(const protocol_profile_t *profile)
{
    return (profile->magic == PROTOCOL_MAGIC) &&
           (profile->version == PROTOCOL_VERSION) &&
           (profile->size == sizeof(*profile)) &&
           (profile->checksum == checksum(profile));
}

int protocol_store_mount(void)
{
    protocol_profile_t stored;

    if ((OSPI_W25Qxx_Init() != OSPI_W25Qxx_OK) ||
        (OSPI_W25Qxx_ReadID() != W25Qxx_FLASH_ID) ||
        (OSPI_W25Qxx_ReadBuffer((uint8_t *)&stored, 0U, sizeof(stored)) != OSPI_W25Qxx_OK) ||
        (valid(&stored) == 0))
    {
        return -1;
    }
    cached_profile = stored;
    return 0;
}

const protocol_profile_t *protocol_store_profile(void)
{
    return &cached_profile;
}
