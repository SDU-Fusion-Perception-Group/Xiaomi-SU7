#include "can3_port.h"
#include "board_config.h"
#include "dji_3508.h"

#define CAN3_QUEUE_CAPACITY 16U

static can3_frame_t queue_data[CAN3_QUEUE_CAPACITY];
static volatile uint8_t queue_head;
static volatile uint8_t queue_tail;
static can3_status_t status;

static uint32_t data_length_code(uint8_t length)
{
    static const uint32_t codes[9] = {
        FDCAN_DLC_BYTES_0, FDCAN_DLC_BYTES_1, FDCAN_DLC_BYTES_2, FDCAN_DLC_BYTES_3,
        FDCAN_DLC_BYTES_4, FDCAN_DLC_BYTES_5, FDCAN_DLC_BYTES_6, FDCAN_DLC_BYTES_7,
        FDCAN_DLC_BYTES_8
    };
    return codes[length];
}

void can3_port_init(void)
{
    FDCAN_FilterTypeDef filter = {0};
    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0U;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = 0U;
    filter.FilterID2 = 0U;

    HAL_FDCAN_ConfigFilter(&BOARD_CAN, &filter);
    HAL_FDCAN_ConfigGlobalFilter(&BOARD_CAN, FDCAN_ACCEPT_IN_RX_FIFO0,
                                 FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    HAL_FDCAN_ActivateNotification(&BOARD_CAN, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U);
    HAL_FDCAN_Start(&BOARD_CAN);
}

int can3_port_send(uint32_t identifier, const uint8_t *data, uint8_t length)
{
    FDCAN_TxHeaderTypeDef header = {0};

    if ((data == NULL) || (length > 8U))
    {
        status.error_count++;
        return -1;
    }

    header.Identifier = identifier;
    header.IdType = FDCAN_STANDARD_ID;
    header.TxFrameType = FDCAN_DATA_FRAME;
    header.DataLength = data_length_code(length);
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    header.BitRateSwitch = FDCAN_BRS_OFF;
    header.FDFormat = FDCAN_CLASSIC_CAN;
    header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    header.MessageMarker = 0U;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&BOARD_CAN, &header, (uint8_t *)data) != HAL_OK)
    {
        status.error_count++;
        return -1;
    }
    status.tx_count++;
    return 0;
}

int can3_port_take(can3_frame_t *frame)
{
    uint32_t primask;
    if (frame == NULL)
    {
        return -1;
    }

    primask = __get_PRIMASK();
    __disable_irq();
    if (queue_head == queue_tail)
    {
        if (primask == 0U)
        {
            __enable_irq();
        }
        return -1;
    }
    *frame = queue_data[queue_tail];
    queue_tail = (uint8_t)((queue_tail + 1U) % CAN3_QUEUE_CAPACITY);
    if (primask == 0U)
    {
        __enable_irq();
    }
    return 0;
}

void can3_port_get_status(can3_status_t *snapshot)
{
    if (snapshot != NULL)
    {
        status.protocol_status = HAL_FDCAN_GetProtocolStatus(&BOARD_CAN, NULL) == HAL_OK
                                     ? BOARD_CAN.Instance->PSR
                                     : 0U;
        *snapshot = status;
    }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    FDCAN_RxHeaderTypeDef header;
    can3_frame_t frame = {0};
    uint8_t next_head;

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0U)
    {
        return;
    }
    if (hfdcan == &hfdcan1)
    {
        while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &header, frame.data) == HAL_OK)
        {
            dji_3508_on_rx(&header, frame.data);
        }
        return;
    }
    if (hfdcan != &BOARD_CAN)
    {
        return;
    }
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &header, frame.data) != HAL_OK)
    {
        status.error_count++;
        return;
    }

    frame.identifier = header.Identifier;
    frame.length = (uint8_t)(header.DataLength >> 16U);
    if (frame.length > 8U)
    {
        frame.length = 8U;
    }
    frame.tick = HAL_GetTick();
    status.last_rx = frame;
    status.rx_count++;

    next_head = (uint8_t)((queue_head + 1U) % CAN3_QUEUE_CAPACITY);
    if (next_head != queue_tail)
    {
        queue_data[queue_head] = frame;
        queue_head = next_head;
    }
    else
    {
        status.error_count++;
    }
}
