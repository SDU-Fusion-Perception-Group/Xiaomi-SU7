#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 * DM-MC02 module bench configuration.
 * USART3 is the only RS485 port used by this template.
 * FDCAN3 is the only CAN port used by this template.
 */

#define BOARD_RS485_UART              huart3
#define BOARD_RS485_BAUDRATE          115200UL
#define BOARD_RS485_DEFAULT_ADDRESS   1U

#define BOARD_CAN                     hfdcan3
#define BOARD_CAN_NOMINAL_BPS         1000000UL
#define BOARD_CAN_TEST_ID             0x123U

#define BOARD_USB_TELEMETRY_HZ        10U
#define BOARD_IMU_SAMPLE_HZ           1000U
#define BOARD_RS485_POLL_HZ           20U
#define BOARD_CAN_TEST_HZ             10U

#define BOARD_RS485_ATTACHED          0U
#define BOARD_CAN_ATTACHED            1U

#define BOARD_WHEEL_DIAMETER_MM       75U
#define BOARD_WHEEL_TRACK_MM          200U
#define BOARD_WHEEL_PULSES_PER_REV    51200UL

#define BOARD_BUZZER_TIMER            htim12
#define BOARD_BUZZER_CHANNEL          TIM_CHANNEL_2

#endif
