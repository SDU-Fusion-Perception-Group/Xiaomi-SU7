#ifndef __PID_H__
#define __PID_H__

#include <stdint.h>

typedef enum
{
    PID_POSITION = 0,  // 位置式PID
    PID_DELTA          // 增量式PID
} pid_mode_e;

typedef struct
{
    pid_mode_e mode;       // PID模式

    float Kp;              // 比例系数
    float Ki;              // 积分系数
    float Kd;              // 微分系数

    float set;             // 目标值
    float fdb;             // 反馈值
    float err;             // 当前误差
    float err_last;        // 上一次误差
    float err_before_last; // 上上次误差

    float p_out;           // 比例输出
    float i_out;           // 积分输出
    float d_out;           // 微分输出
    float out;             // 总输出

    float max_out;         // 输出限幅
    float max_iout;        // 积分输出限幅

    float deadband;        // 死区，误差小于此值时输出为0
} pid_t;

/**
 * @brief  PID初始化
 * @param  pid: PID结构体指针
 * @param  mode: PID模式（位置式/增量式）
 * @param  kp: 比例系数
 * @param  ki: 积分系数
 * @param  kd: 微分系数
 * @param  max_out: 输出最大值
 * @param  max_iout: 积分输出最大值
 * @retval 无
 */
void PID_init(pid_t *pid, pid_mode_e mode, float kp, float ki, float kd, float max_out, float max_iout);

/**
 * @brief  PID计算
 * @param  pid: PID结构体指针
 * @param  set: 目标值
 * @param  fdb: 反馈值
 * @retval PID输出
 */
float PID_calc(pid_t *pid, float set, float fdb);

/**
 * @brief  PID清零
 * @param  pid: PID结构体指针
 * @retval 无
 */
void PID_clear(pid_t *pid);

/**
 * @brief  设置PID死区
 * @param  pid: PID结构体指针
 * @param  deadband: 死区值
 * @retval 无
 */
void PID_set_deadband(pid_t *pid, float deadband);

#endif
