#include "bsp_fdcan.h"

extern uint8_t rx_data1[8];
extern uint16_t rec_id1;
volatile uint32_t can1_rx_count __attribute__((used));
volatile uint16_t can1_last_id __attribute__((used));
volatile uint8_t can1_last_data[8] __attribute__((used));

void fdcan1_rx_callback(void)
{
    uint16_t id = 0U;
    uint8_t data[8] = {0};
    if (fdcanx_receive(&hfdcan1, &id, data) != 0U)
    {
        rec_id1 = id;
        for (uint32_t index = 0U; index < 8U; index++)
        {
            rx_data1[index] = data[index];
            can1_last_data[index] = data[index];
        }
        can1_last_id = id;
        can1_rx_count++;
    }
}

uint8_t fdcanx_send_data(hcan_t *hfdcan, uint16_t id, uint8_t *data, uint32_t len)
{
    FDCAN_TxHeaderTypeDef header = {0};
    header.Identifier = id;
    header.IdType = FDCAN_STANDARD_ID;
    header.TxFrameType = FDCAN_DATA_FRAME;
    header.DataLength = len;
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    header.BitRateSwitch = FDCAN_BRS_OFF;
    header.FDFormat = FDCAN_CLASSIC_CAN;
    header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    return HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &header, data) == HAL_OK ? 0U : 1U;
}
