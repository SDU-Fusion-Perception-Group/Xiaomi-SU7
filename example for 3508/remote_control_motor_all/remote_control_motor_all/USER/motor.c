#include "motor.h"
#include "remote_control.h"
#include "CAN_receive.h"
#include <math.h>

extern RC_ctrl_t rc_ctrl;

float lyf1;
float lyf2;
float lyf3;
float lyf4;
float yaw_target_ecd; // yaw轴目标角度（ecd单位）
static uint8_t yaw_init_flag = 0; // 无扰启动标志

// 四个底盘电机的PID控制器
pid_t chassis_motor_pid[4];
// 云台PID：[0]yaw角度环，[1]yaw速度环，[2]pitch，[3]trigger
pid_t gimbal_motor_pid[4];

/**
 * @brief  底盘电机PID初始化
 * @param  无
 * @retval 无
 */
void chassis_motor_init(void)
{
    uint8_t i;
    // 初始化底盘3508电机速度环PID
    for (i = 0; i < 4; i++)
    {
        PID_init(&chassis_motor_pid[i],
                 PID_POSITION,
                 CHASSIS_KP,
                 CHASSIS_KI,
                 CHASSIS_KD,
                 CHASSIS_MAX_OUT,
                 CHASSIS_MAX_IOUT);
    }

    /************************ yaw轴GM6020串级双环PID初始化 ************************/
    // 角度外环：ecd输入，输出目标转速rpm
    PID_init(&gimbal_motor_pid[0],
             PID_POSITION,
             YAW_ANG_KP,
             YAW_ANG_KI,
             YAW_ANG_KD,
             YAW_ANG_OUT,
             YAW_ANG_IOUT);
    // 速度内环：rpm输入，输出GM6020控制电压
    PID_init(&gimbal_motor_pid[1],
             PID_POSITION,
             YAW_SPD_KP,
             YAW_SPD_KI,
             YAW_SPD_KD,
             YAW_SPD_OUT,
             YAW_SPD_IOUT);

    // pitch/trigger其他云台电机PID初始化（保留原有参数）
    for (i = 2; i < 4; i++)
    {
        PID_init(&gimbal_motor_pid[i],
                 PID_POSITION,
                 GIMBAL_KP,
                 GIMBAL_KI,
                 GIMBAL_KD,
                 GIMBAL_MAX_OUT,
                 GIMBAL_MAX_IOUT);
    }
    // 初始化标志清零，上电第一次运行做无扰切换
    yaw_init_flag = 0;
}

/**
 * @brief  底盘电机驱动（速度闭环）
 * @param  无
 * @retval 无
 */
void chassis_motor_drive(void)
{
    float target_speed[4];  // 目标转速（rpm）
    float current_out[4];   // PID输出电流
    const motor_measure_t *motor_measure;
    uint8_t i;

    // 计算四个电机的目标转速（摇杆映射）
    // ch[3]: 前后  ch[2]: 左右平移  ch[0]: 底盘旋转
    target_speed[0] =  rc_ctrl.rc.ch[3] / base * maxspeed
                      + rc_ctrl.rc.ch[2] / base * maxspeed
                      + rc_ctrl.rc.ch[0] / base * maxspeed;

    target_speed[1] =  rc_ctrl.rc.ch[3] / base * maxspeed
                      - rc_ctrl.rc.ch[2] / base * maxspeed
                      + rc_ctrl.rc.ch[0] / base * maxspeed;

    target_speed[2] = -rc_ctrl.rc.ch[3] / base * maxspeed
                      - rc_ctrl.rc.ch[2] / base * maxspeed
                      + rc_ctrl.rc.ch[0] / base * maxspeed;

    target_speed[3] = -rc_ctrl.rc.ch[3] / base * maxspeed
                      + rc_ctrl.rc.ch[2] / base * maxspeed
                      + rc_ctrl.rc.ch[0] / base * maxspeed;

    // 对每个电机进行PID闭环计算
    for (i = 0; i < 4; i++)
    {
        // 获取电机反馈数据
        motor_measure = get_chassis_motor_measure_point(i);

        // PID计算：目标转速 vs 实际转速 → 输出电流
        current_out[i] = PID_calc(&chassis_motor_pid[i],
                                  target_speed[i],
                                  (float)motor_measure->speed_rpm);
    }



    // 发送电流控制指令
    CAN_cmd_chassis((int16_t)current_out[0],
                    (int16_t)current_out[1],
                    (int16_t)current_out[2],
                    (int16_t)current_out[3]);
}

/**
 * @brief  yaw轴GM6020大疆标准增量式位置控制
 * @note  拨ch1转，回中锁在当前位置不回零，外力掰不动，串级双环
 * @param  无
 * @retval 无
 */
void gimbal_motor_drive(void)
{
    int16_t ch1_val = rc_ctrl.rc.ch[1];
    float ecd_add;           // 本周期角度增量
    float real_ecd;          // 电机实际ecd值
    float feedback_ecd;      // 归一化后的反馈ecd（处理过零）
    float err;               // 角度误差
    float target_rpm;        // 角度环输出目标转速
    float real_rpm;          // 电机实际转速
    float yaw_volt_out;      // 速度环输出电压
    const motor_measure_t *yaw_motor = get_yaw_gimbal_motor_measure_point();


    real_ecd = (float)yaw_motor->ecd;
    real_rpm = (float)yaw_motor->speed_rpm;

    /************************ 无扰启动：第一次运行目标角度=当前角度 ************************/
    if(yaw_init_flag == 0)
    {
        yaw_target_ecd = real_ecd;
        PID_clear(&gimbal_motor_pid[0]);
        PID_clear(&gimbal_motor_pid[1]);
        yaw_init_flag = 1;
    }

    /************************ 输入处理：摇杆值转角度增量 ************************/
    if(abs(ch1_val) < YAW_RC_DEADZONE)
    {
        ecd_add = 0.0f; // 回中增量为0，目标角度锁死
    }
    else
    {
        // 摇杆值映射本周期ecd增量，偏移越大增量越大，转速越快
        ecd_add = (ch1_val / base) * YAW_ECD_ADD_MAX;
    }

    /************************ 目标角度累加+限幅 ************************/
    yaw_target_ecd += ecd_add;
    // 角度限幅，防止无限转到结构极限
    //if(yaw_target_ecd > real_ecd + YAW_ECD_LIMIT)
    //    yaw_target_ecd = real_ecd + YAW_ECD_LIMIT;
    //if(yaw_target_ecd < real_ecd - YAW_ECD_LIMIT)
    //   yaw_target_ecd = real_ecd - YAW_ECD_LIMIT;

    /************************ 单圈角度误差归一化（处理过零跳变） ************************/
		err = yaw_target_ecd - real_ecd;
		while(err > 4096)  err -= 8192;
		while(err < -4096) err += 8192;
		feedback_ecd = yaw_target_ecd - err;

    /************************ 串级双环PID计算 ************************/
    // 角度外环：角度误差 → 目标转速
    target_rpm = PID_calc(&gimbal_motor_pid[0], yaw_target_ecd, feedback_ecd);
    // 速度内环：转速误差 → GM6020控制电压
    yaw_volt_out = PID_calc(&gimbal_motor_pid[1], target_rpm, real_rpm);

    // 电压限幅 ±25000
    if(yaw_volt_out > YAW_SPD_OUT)
        yaw_volt_out = YAW_SPD_OUT;
    if(yaw_volt_out < -YAW_SPD_OUT)
        yaw_volt_out = -YAW_SPD_OUT;
		
		lyf1 = yaw_target_ecd;
		lyf2 = real_ecd;
		lyf3 = feedback_ecd;
		lyf4 = yaw_volt_out;

    // 发送CAN控制指令
    CAN_cmd_gimbal((int16_t)yaw_volt_out, (int16_t)yaw_volt_out, (int16_t)yaw_volt_out, 0);
}
