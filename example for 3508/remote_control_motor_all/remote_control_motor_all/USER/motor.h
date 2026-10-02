#ifndef __motor_H__
#define __motor_H__

#include "pid.h"

#define base 660.0f
#define maxspeed 5000.0f      // 底盘最大目标转速（rpm）
#define MAX_CURRENT 16384.0f  // 3508电机最大电流值

// 底盘电机PID参数（速度环）
#define CHASSIS_KP  12.0f
#define CHASSIS_KI  0.3f
#define CHASSIS_KD  0.2f
#define CHASSIS_MAX_OUT   16384.0f   // 输出限幅（电流值）
#define CHASSIS_MAX_IOUT  5000.0f    // 积分限幅

/************************ yaw轴GM6020大疆标准增量式位置控制 ************************/
#define YAW_RC_DEADZONE 60.0f        // 遥控器摇杆死区
#define YAW_MAX_RPM 180.0f           // 满偏最大转速
#define YAW_CTRL_PERIOD 0.005f       // 控制周期5ms（200Hz）
// 每周期最大ecd增量：180rpm换算为5ms周期下的ecd变化量
#define YAW_ECD_ADD_MAX ((YAW_MAX_RPM / 60.0f) * 8192.0f * YAW_CTRL_PERIOD)
#define YAW_ECD_LIMIT 8192.0f        // 角度限幅±360°（±8192ecd）
// 角度环PID（ecd输入，rpm输出）
#define YAW_ANG_KP 1.2f
#define YAW_ANG_KI 0.01f
#define YAW_ANG_KD 0.2f
#define YAW_ANG_OUT YAW_MAX_RPM      // 角度环输出限幅=最大转速
#define YAW_ANG_IOUT 50.0f           // 角度环积分限幅
// 速度环PID（rpm输入，电压输出）
#define YAW_SPD_KP 25.0f
#define YAW_SPD_KI 0.4f
#define YAW_SPD_KD 2.5f
#define YAW_SPD_OUT 25000.0f         // GM6020最大电压±25000
#define YAW_SPD_IOUT 6000.0f

// pitch/trigger其他云台电机参数（保留原有）
#define GIMBAL_KP  5.0f
#define GIMBAL_KI  0.0f
#define GIMBAL_KD  0.4f
#define GIMBAL_MAX_OUT   16384.0f
#define GIMBAL_MAX_IOUT  5000.0f

void chassis_motor_init(void);
void chassis_motor_drive(void);
void gimbal_motor_drive(void);

extern float lyf1;
extern float lyf2;
extern float lyf3;
extern float lyf4;
extern float yaw_target_ecd; // yaw轴目标角度（ecd单位）

// 四个底盘电机的PID控制器
extern pid_t chassis_motor_pid[4];
// 云台PID：[0]yaw角度环，[1]yaw速度环，[2]pitch，[3]trigger
extern pid_t gimbal_motor_pid[4];

#endif
