#include "pid.h"

// 绝对值宏
#define ABS(x) ((x) > 0 ? (x) : -(x))

/**
 * @brief  PID初始化
 */
void PID_init(pid_t *pid, pid_mode_e mode, float kp, float ki, float kd, float max_out, float max_iout)
{
    pid->mode = mode;

    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    pid->set = 0.0f;
    pid->fdb = 0.0f;
    pid->err = 0.0f;
    pid->err_last = 0.0f;
    pid->err_before_last = 0.0f;

    pid->p_out = 0.0f;
    pid->i_out = 0.0f;
    pid->d_out = 0.0f;
    pid->out = 0.0f;

    pid->max_out = max_out;
    pid->max_iout = max_iout;

    pid->deadband = 0.0f;
}

/**
 * @brief  输出限幅
 */
static float output_limit(float value, float max_value)
{
    if (value > max_value)
        return max_value;
    if (value < -max_value)
        return -max_value;
    return value;
}

/**
 * @brief  PID计算
 */
float PID_calc(pid_t *pid, float set, float fdb)
{
    pid->set = set;
    pid->fdb = fdb;

    pid->err = set - fdb;

    // 死区处理
    if (ABS(pid->err) < pid->deadband)
    {
        pid->p_out = 0.0f;
        pid->i_out = 0.0f;
        pid->d_out = 0.0f;
        pid->out = 0.0f;
        pid->err_last = pid->err;
        pid->err_before_last = pid->err_last;
        return 0.0f;
    }

    if (pid->mode == PID_POSITION)
    {
        // 位置式PID
        pid->p_out = pid->Kp * pid->err;

        pid->i_out += pid->Ki * pid->err;
        pid->i_out = output_limit(pid->i_out, pid->max_iout);

        pid->d_out = pid->Kd * (pid->err - pid->err_last);

        pid->out = pid->p_out + pid->i_out + pid->d_out;
        pid->out = output_limit(pid->out, pid->max_out);
    }
    else // PID_DELTA 增量式
    {
        // 增量式PID
        float delta_out;

        pid->p_out = pid->Kp * (pid->err - pid->err_last);

        pid->i_out = pid->Ki * pid->err;
        pid->i_out = output_limit(pid->i_out, pid->max_iout);

        pid->d_out = pid->Kd * (pid->err - 2.0f * pid->err_last + pid->err_before_last);

        delta_out = pid->p_out + pid->i_out + pid->d_out;
        pid->out += delta_out;
        pid->out = output_limit(pid->out, pid->max_out);
    }

    // 更新误差历史
    pid->err_before_last = pid->err_last;
    pid->err_last = pid->err;

    return pid->out;
}

/**
 * @brief  PID清零
 */
void PID_clear(pid_t *pid)
{
    pid->set = 0.0f;
    pid->fdb = 0.0f;
    pid->err = 0.0f;
    pid->err_last = 0.0f;
    pid->err_before_last = 0.0f;

    pid->p_out = 0.0f;
    pid->i_out = 0.0f;
    pid->d_out = 0.0f;
    pid->out = 0.0f;
}

/**
 * @brief  设置PID死区
 */
void PID_set_deadband(pid_t *pid, float deadband)
{
    pid->deadband = deadband;
}
