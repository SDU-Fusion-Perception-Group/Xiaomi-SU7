#include "module_bench.h"
#include "command_shell.h"
#include "chassis_motion.h"
#include "board_config.h"
#include "rs485_port.h"
#include "can3_port.h"
#include "lyx_motor.h"
#include "dji_3508.h"
#include "protocol_store.h"
#include "usb_telemetry.h"
#include "bsp_buzzer.h"
#include "status_indicator.h"
#include "bmi088driver.h"
#include "bmi088_dma.h"
#include "MahonyAHRS.h"
#include "cmsis_os.h"
#include <math.h>
#include <string.h>

static lyx_motor_t motor;
static float quaternion[4] = {1.0f, 0.0f, 0.0f, 0.0f};
static float gyro_bias[3];
static telemetry_snapshot_t telemetry;
static volatile uint32_t imu_sequence;
static volatile uint8_t module_ready;

static float radians_to_dps(float radians_per_second)
{
    return radians_per_second * (180.0f / 3.14159265f);
}

static void update_attitude(float dt_seconds)
{
    float gyro[3];
    float accel[3];
    float temperature;
    float corrected[3];

    if (bmi088_dma_take(gyro, accel, &temperature) != 0)
    {
        return;
    }
    for (int axis = 0; axis < 3; axis++)
    {
        corrected[axis] = gyro[axis] - gyro_bias[axis];
        telemetry.gyro_dps[axis] = radians_to_dps(corrected[axis]);
        telemetry.accel_g[axis] = accel[axis];
    }
    telemetry.temperature_c = temperature;

    MahonyAHRSupdateIMU(quaternion,
                        corrected[0], corrected[1], corrected[2],
                        accel[0], accel[1], accel[2]);
    telemetry.yaw_rad = atan2f(2.0f * (quaternion[0] * quaternion[3] + quaternion[1] * quaternion[2]),
                               2.0f * (quaternion[0] * quaternion[0] + quaternion[1] * quaternion[1]) - 1.0f);
    telemetry.pitch_rad = asinf(-2.0f * (quaternion[1] * quaternion[3] - quaternion[0] * quaternion[2]));
    telemetry.roll_rad = atan2f(2.0f * (quaternion[0] * quaternion[1] + quaternion[2] * quaternion[3]),
                                2.0f * (quaternion[0] * quaternion[0] + quaternion[3] * quaternion[3]) - 1.0f);
    telemetry.imu_valid = 1U;
    imu_sequence++;
}

static void calibrate_gyro(void)
{
    float gyro[3];
    float accel[3];
    float temperature;
    float sum[3] = {0.0f, 0.0f, 0.0f};
    const int samples = 400;

    for (int sample = 0; sample < samples; sample++)
    {
        uint32_t started = osKernelSysTick();
        bmi088_dma_start();
        while ((bmi088_dma_take(gyro, accel, &temperature) != 0) && ((osKernelSysTick() - started) < 2U))
        {
        }
        if ((osKernelSysTick() - started) >= 2U)
        {
            continue;
        }
        sum[0] += gyro[0];
        sum[1] += gyro[1];
        sum[2] += gyro[2];
        osDelay(2);
    }
    gyro_bias[0] = sum[0] / (float)samples;
    gyro_bias[1] = sum[1] / (float)samples;
    gyro_bias[2] = sum[2] / (float)samples;
}

void ModuleBench_Init(void)
{
    if (protocol_store_mount() != 0)
    {
        status_indicator_set(STATUS_IMU_ERROR);
        status_indicator_update(osKernelSysTick());
        return;
    }
    rs485_port_init();
    can3_port_init();
    dji_3508_init();
    usb_telemetry_init();
    lyx_motor_init(&motor, BOARD_RS485_DEFAULT_ADDRESS);
    BSP_Buzzer_Init();
    status_indicator_set(STATUS_BOOT);
    status_indicator_update(osKernelSysTick());
    while (BMI088_init() != BMI088_NO_ERROR)
    {
        status_indicator_set(STATUS_IMU_ERROR);
        status_indicator_update(osKernelSysTick());
        osDelay(100);
    }
    status_indicator_set(STATUS_SELF_TEST);
    status_indicator_update(osKernelSysTick());
    calibrate_gyro();
    status_indicator_set(STATUS_LINKS_BOTH_ERROR);
    status_indicator_update(osKernelSysTick());
    BSP_Buzzer_On();
    osDelay(500);
    BSP_Buzzer_Off();
    module_ready = 1U;
}

void ImuTask_Entry(void const *argument)
{
    (void)argument;
    while (module_ready == 0U)
    {
        osDelay(10);
    }
    bmi088_dma_init();
    for (;;)
    {
        update_attitude(1.0f / (float)BOARD_IMU_SAMPLE_HZ);
        bmi088_dma_start();
        osDelay(1000U / BOARD_IMU_SAMPLE_HZ);
    }
}

void Rs485Task_Entry(void const *argument)
{
    (void)argument;
    if (BOARD_RS485_ATTACHED == 0U)
    {
        osThreadTerminate(NULL);
    }
    for (;;)
    {
        lyx_motor_poll(&motor);
        chassis_motion_poll(&motor, &motor);
        osDelay(1000U / BOARD_RS485_POLL_HZ);
    }
}

void Can3Task_Entry(void const *argument)
{
    static uint8_t counter;
    uint8_t payload[8] = {0xA5U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    can3_frame_t frame;
    (void)argument;
    if (BOARD_CAN_ATTACHED == 0U)
    {
        osThreadTerminate(NULL);
    }

    for (;;)
    {
        payload[1] = counter++;
        memcpy(&payload[2], &telemetry.yaw_rad, sizeof(float));
        dji_3508_poll();
        while (can3_port_take(&frame) == 0)
        {
            telemetry.can_rx++;
        }
        osDelay(1000U / BOARD_CAN_TEST_HZ);
    }
}

void TelemetryTask_Entry(void const *argument)
{
    char command[64];
    rs485_status_t rs485;
    can3_status_t can;
    (void)argument;

    for (;;)
    {
        rs485_port_get_status(&rs485);
        can3_port_get_status(&can);
        telemetry.motor_online = motor.online;
        telemetry.motor_alarm = motor.alarm;
        telemetry.motor_position = motor.position;
        telemetry.motor_speed_raw = (int16_t)motor.speed_raw;
        telemetry.rs485_frames = rs485.frame_count;
        telemetry.rs485_errors = rs485.error_count;
        telemetry.can_rx = can.rx_count;
        telemetry.can_tx = can.tx_count;
        telemetry.can_errors = can.error_count;
        telemetry.uptime_ms = osKernelSysTick();
        if (telemetry.imu_valid == 0U)
        {
            status_indicator_set(STATUS_IMU_ERROR);
        }
        else if ((BOARD_RS485_ATTACHED == 0U) && (BOARD_CAN_ATTACHED == 0U))
        {
            status_indicator_set(STATUS_LINKS_BOTH_ERROR);
        }
        else if (BOARD_RS485_ATTACHED == 0U)
        {
            status_indicator_set(STATUS_RS485_ONLY_ERROR);
        }
        else if (BOARD_CAN_ATTACHED == 0U)
        {
            status_indicator_set(STATUS_CAN_ONLY_ERROR);
        }
        else
        {
            status_indicator_set(STATUS_LINKS_BOTH_OK);
        }
        status_indicator_update(telemetry.uptime_ms);
        usb_telemetry_send(&telemetry);

        if (usb_telemetry_take_command(command, sizeof(command)) == 0)
        {
            command_shell_execute(command, &motor);
            if (strcmp(command, "ping") == 0)
            {
                BSP_Buzzer_On();
                osDelay(30);
                BSP_Buzzer_Off();
            }
        }
        osDelay(1000U / BOARD_USB_TELEMETRY_HZ);
    }
}
