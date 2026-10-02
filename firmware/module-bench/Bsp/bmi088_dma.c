#include "bmi088_dma.h"
#include "BMI088driver.h"
#include "BMI088reg.h"
#include "main.h"
#include "spi.h"

#define BMI_ACCEL_LENGTH 8U
#define BMI_GYRO_LENGTH 7U
#define BMI_TEMP_LENGTH 4U

static DMA_HandleTypeDef spi2_rx_dma;
static DMA_HandleTypeDef spi2_tx_dma;
static uint8_t tx_buffer[8];
static uint8_t rx_buffer[8];
static float sample_gyro[3];
static float sample_accel[3];
static float sample_temperature;
static volatile uint8_t sample_ready;
static volatile uint8_t stage;
static volatile uint8_t busy;

static void select_device(uint8_t accelerometer)
{
    HAL_GPIO_WritePin(ACC_CS_GPIO_Port, ACC_CS_Pin, accelerometer ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GYRO_CS_GPIO_Port, GYRO_CS_Pin, accelerometer ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void release_devices(void)
{
    HAL_GPIO_WritePin(ACC_CS_GPIO_Port, ACC_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GYRO_CS_GPIO_Port, GYRO_CS_Pin, GPIO_PIN_SET);
}

static void start_transfer(uint8_t register_address, uint8_t length, uint8_t accelerometer)
{
    for (uint8_t index = 0U; index < sizeof(tx_buffer); index++)
    {
        tx_buffer[index] = 0x55U;
        rx_buffer[index] = 0U;
    }
    tx_buffer[0] = (uint8_t)(register_address | 0x80U);
    select_device(accelerometer);
    busy = 1U;
    if (HAL_SPI_TransmitReceive_DMA(&hspi2, tx_buffer, rx_buffer, length) != HAL_OK)
    {
        release_devices();
        busy = 0U;
        stage = 0U;
    }
}

void bmi088_dma_init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    spi2_rx_dma.Instance = DMA1_Stream1;
    spi2_rx_dma.Init.Request = DMA_REQUEST_SPI2_RX;
    spi2_rx_dma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    spi2_rx_dma.Init.PeriphInc = DMA_PINC_DISABLE;
    spi2_rx_dma.Init.MemInc = DMA_MINC_ENABLE;
    spi2_rx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi2_rx_dma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi2_rx_dma.Init.Mode = DMA_NORMAL;
    spi2_rx_dma.Init.Priority = DMA_PRIORITY_HIGH;
    spi2_rx_dma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&spi2_rx_dma);
    __HAL_LINKDMA(&hspi2, hdmarx, spi2_rx_dma);

    spi2_tx_dma.Instance = DMA1_Stream2;
    spi2_tx_dma.Init.Request = DMA_REQUEST_SPI2_TX;
    spi2_tx_dma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    spi2_tx_dma.Init.PeriphInc = DMA_PINC_DISABLE;
    spi2_tx_dma.Init.MemInc = DMA_MINC_ENABLE;
    spi2_tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi2_tx_dma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi2_tx_dma.Init.Mode = DMA_NORMAL;
    spi2_tx_dma.Init.Priority = DMA_PRIORITY_HIGH;
    spi2_tx_dma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&spi2_tx_dma);
    __HAL_LINKDMA(&hspi2, hdmatx, spi2_tx_dma);

    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
    HAL_NVIC_SetPriority(SPI2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
    release_devices();
}

void bmi088_dma_start(void)
{
    if (busy == 0U)
    {
        stage = 1U;
        start_transfer(BMI088_ACCEL_XOUT_L, BMI_ACCEL_LENGTH, 1U);
    }
}

int bmi088_dma_take(float gyro[3], float accel[3], float *temperature)
{
    uint32_t primask;
    if ((gyro == NULL) || (accel == NULL) || (temperature == NULL))
    {
        return -1;
    }
    primask = __get_PRIMASK();
    __disable_irq();
    if (sample_ready == 0U)
    {
        if (primask == 0U)
        {
            __enable_irq();
        }
        return -1;
    }
    for (int axis = 0; axis < 3; axis++)
    {
        gyro[axis] = sample_gyro[axis];
        accel[axis] = sample_accel[axis];
    }
    *temperature = sample_temperature;
    sample_ready = 0U;
    if (primask == 0U)
    {
        __enable_irq();
    }
    return 0;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    int16_t raw[3];
    if (hspi != &hspi2)
    {
        return;
    }
    release_devices();
    if (stage == 1U)
    {
        for (int axis = 0; axis < 3; axis++)
        {
            raw[axis] = (int16_t)((rx_buffer[2U + axis * 2U] << 8U) | rx_buffer[1U + axis * 2U]);
            sample_accel[axis] = (float)raw[axis] * BMI088_ACCEL_3G_SEN;
        }
        stage = 2U;
        start_transfer(BMI088_GYRO_X_L, BMI_GYRO_LENGTH, 0U);
    }
    else if (stage == 2U)
    {
        for (int axis = 0; axis < 3; axis++)
        {
            raw[axis] = (int16_t)((rx_buffer[2U + axis * 2U] << 8U) | rx_buffer[1U + axis * 2U]);
            sample_gyro[axis] = (float)raw[axis] * BMI088_GYRO_2000_SEN;
        }
        stage = 3U;
        start_transfer(BMI088_TEMP_M, BMI_TEMP_LENGTH, 1U);
    }
    else
    {
        uint16_t temperature = (uint16_t)(((uint16_t)rx_buffer[2] << 3U) | (rx_buffer[3] >> 5U));
        sample_temperature = ((float)temperature * 0.125f) + 23.0f;
        sample_ready = 1U;
        stage = 0U;
        busy = 0U;
    }
}

void DMA1_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&spi2_rx_dma);
}

void DMA1_Stream2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&spi2_tx_dma);
}

void SPI2_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi2);
}
