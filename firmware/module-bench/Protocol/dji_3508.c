#include "dji_3508.h"
#include "fdcan.h"
#include <stddef.h>

static dji_3508_t motors[4];
static int16_t command_current[4];
static dji_3508_bus_status_t bus_status;

static int16_t clamp_current(int16_t current)
{
    if (current > DJI_3508_CURRENT_LIMIT)
    {
        return DJI_3508_CURRENT_LIMIT;
    }
    if (current < -DJI_3508_CURRENT_LIMIT)
    {
        return -DJI_3508_CURRENT_LIMIT;
    }
    return current;
}

void dji_3508_init(void)
{
    FDCAN_FilterTypeDef filter = {0};
    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0U;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    HAL_FDCAN_ConfigFilter(&hfdcan1, &filter);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0,
                                 FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U);
    HAL_FDCAN_Start(&hfdcan1);
}

void dji_3508_set_current(uint8_t index, int16_t current)
{
    if (index < 4U)
    {
        command_current[index] = clamp_current(current);
    }
}

void dji_3508_poll(void)
{
    uint8_t data[8];
    FDCAN_TxHeaderTypeDef header = {0};
    if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) == 0U)
    {
        return;
    }
    for (uint8_t index = 0U; index < 4U; index++)
    {
        data[index * 2U] = (uint8_t)(command_current[index] >> 8);
        data[index * 2U + 1U] = (uint8_t)command_current[index];
        if ((HAL_GetTick() - motors[index].last_tick) > 100U)
        {
            motors[index].online = 0U;
        }
    }
    header.Identifier = DJI_3508_CONTROL_ID;
    header.IdType = FDCAN_STANDARD_ID;
    header.TxFrameType = FDCAN_DATA_FRAME;
    header.DataLength = FDCAN_DLC_BYTES_8;
    header.FDFormat = FDCAN_CLASSIC_CAN;
    header.BitRateSwitch = FDCAN_BRS_OFF;
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &header, data) == HAL_OK)
    {
        bus_status.tx_count++;
    }
    else
    {
        bus_status.tx_errors++;
    }
}

void dji_3508_on_rx(const FDCAN_RxHeaderTypeDef *header, const uint8_t data[8])
{
    uint32_t index;
    if ((header == NULL) || (data == NULL) || (header->IdType != FDCAN_STANDARD_ID) ||
        (header->Identifier < DJI_3508_FEEDBACK_BASE) ||
        (header->RxFrameType != FDCAN_DATA_FRAME) || (header->DataLength != FDCAN_DLC_BYTES_8))
    {
        return;
    }
    index = header->Identifier - DJI_3508_FEEDBACK_BASE;
    if (index < 4U)
    {
        bus_status.rx_count++;
        motors[index].angle_raw = (uint16_t)(((uint16_t)data[0] << 8U) | data[1]);
        motors[index].speed_rpm = (int16_t)(((uint16_t)data[2] << 8U) | data[3]);
        motors[index].current_raw = (int16_t)(((uint16_t)data[4] << 8U) | data[5]);
        motors[index].temperature_c = data[6];
        motors[index].last_tick = HAL_GetTick();
        motors[index].online = 1U;
    }
}

const dji_3508_t *dji_3508_status(uint8_t index)
{
    return index < 4U ? &motors[index] : NULL;
}

void dji_3508_get_bus_status(dji_3508_bus_status_t *snapshot)
{
    if (snapshot != NULL)
    {
        *snapshot = bus_status;
    }
}
