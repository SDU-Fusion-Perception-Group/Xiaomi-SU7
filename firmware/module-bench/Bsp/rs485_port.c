#include "rs485_port.h"
#include "board_config.h"
#include "cmsis_os.h"

static uint8_t rx_stream[RS485_RX_CAPACITY];
static uint8_t tx_buffer[RS485_RX_CAPACITY];
static uint8_t frame[RS485_RX_CAPACITY];
static volatile uint16_t frame_length;
static volatile uint8_t frame_ready;
static volatile uint8_t tx_busy;
static rs485_status_t status;
static DMA_HandleTypeDef usart3_tx_dma;

void rs485_port_init(void)
{
    frame_length = 0U;
    frame_ready = 0U;
    tx_busy = 0U;

    __HAL_RCC_DMA1_CLK_ENABLE();
    usart3_tx_dma.Instance = DMA1_Stream3;
    usart3_tx_dma.Init.Request = DMA_REQUEST_USART3_TX;
    usart3_tx_dma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    usart3_tx_dma.Init.PeriphInc = DMA_PINC_DISABLE;
    usart3_tx_dma.Init.MemInc = DMA_MINC_ENABLE;
    usart3_tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart3_tx_dma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart3_tx_dma.Init.Mode = DMA_NORMAL;
    usart3_tx_dma.Init.Priority = DMA_PRIORITY_MEDIUM;
    usart3_tx_dma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&usart3_tx_dma);
    __HAL_LINKDMA(&BOARD_RS485_UART, hdmatx, usart3_tx_dma);
    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
    HAL_UARTEx_ReceiveToIdle_DMA(&BOARD_RS485_UART, rx_stream, RS485_RX_CAPACITY);
}

int rs485_port_send(const uint8_t *data, size_t length, uint32_t timeout_ms)
{
    HAL_StatusTypeDef result;

    if ((data == NULL) || (length == 0U) || (length > 255U))
    {
        status.error_count++;
        return -1;
    }

    (void)timeout_ms;
    if (tx_busy != 0U)
    {
        status.error_count++;
        return -1;
    }
    for (size_t index = 0U; index < length; index++)
    {
        tx_buffer[index] = data[index];
    }
    tx_busy = 1U;
    result = HAL_UART_Transmit_DMA(&BOARD_RS485_UART, tx_buffer, (uint16_t)length);
    if (result != HAL_OK)
    {
        tx_busy = 0U;
        status.error_count++;
        return -1;
    }

    status.tx_count++;
    return 0;
}

int rs485_port_take_frame(uint8_t *data, size_t capacity, size_t *length)
{
    uint16_t copy_length;
    uint32_t primask;

    if ((data == NULL) || (length == NULL))
    {
        return -1;
    }

    primask = __get_PRIMASK();
    __disable_irq();
    if (frame_ready == 0U)
    {
        if (primask == 0U)
        {
            __enable_irq();
        }
        return -1;
    }

    copy_length = frame_length;
    if (copy_length > capacity)
    {
        copy_length = (uint16_t)capacity;
    }
    for (uint16_t index = 0U; index < copy_length; index++)
    {
        data[index] = frame[index];
    }
    frame_ready = 0U;
    if (primask == 0U)
    {
        __enable_irq();
    }

    *length = copy_length;
    return 0;
}

void rs485_port_get_status(rs485_status_t *snapshot)
{
    if (snapshot != NULL)
    {
        *snapshot = status;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if ((huart != &BOARD_RS485_UART) || (size == 0U) || (size > RS485_RX_CAPACITY))
    {
        return;
    }

    for (uint16_t index = 0U; index < size; index++)
    {
        frame[index] = rx_stream[index];
    }
    frame_length = size;
    frame_ready = 1U;
    status.rx_count += size;
    status.frame_count++;
    status.last_frame_length = size;
    status.last_rx_tick = osKernelSysTick();
    HAL_UARTEx_ReceiveToIdle_DMA(&BOARD_RS485_UART, rx_stream, RS485_RX_CAPACITY);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &BOARD_RS485_UART)
    {
        status.error_count++;
        tx_busy = 0U;
        HAL_UARTEx_ReceiveToIdle_DMA(&BOARD_RS485_UART, rx_stream, RS485_RX_CAPACITY);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &BOARD_RS485_UART)
    {
        tx_busy = 0U;
    }
}

void DMA1_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&usart3_tx_dma);
}
