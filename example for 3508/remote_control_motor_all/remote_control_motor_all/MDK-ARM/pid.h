#ifndef __PID_H
#define __PID_H

#include "stdint.h"

typedef struct {
    float Kp, Ki, Kd;
    float setpoint;
    float feedback;
    float prev_error;
    float integral;
    float output;
    float out_max, out_min;
    float integral_limit;
    uint8_t enable;
} PID_TypeDef;

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd,
              float out_max, float out_min, float integral_limit);
float PID_Calc(PID_TypeDef *pid, float setpoint, float feedback);
void PID_Reset(PID_TypeDef *pid);

#endif