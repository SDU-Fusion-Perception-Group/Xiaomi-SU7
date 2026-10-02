#ifndef BMI088_DMA_H
#define BMI088_DMA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void bmi088_dma_init(void);
void bmi088_dma_start(void);
int bmi088_dma_take(float gyro[3], float accel[3], float *temperature);

#ifdef __cplusplus
}
#endif

#endif
