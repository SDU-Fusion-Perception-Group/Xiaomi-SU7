#include "command_shell.h"
#include "chassis_motion.h"
#include "dji_3508.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void reply(const char *text)
{
    if ((text != NULL) && (text[0] != '\0'))
    {
        (void)CDC_Transmit_HS((uint8_t *)text, (uint16_t)strlen(text));
    }
}

static int parse_u16(const char *text, unsigned long maximum, uint16_t *value)
{
    char *end = NULL;
    unsigned long parsed;
    if ((text == NULL) || (value == NULL))
    {
        return -1;
    }
    parsed = strtoul(text, &end, 10);
    if ((end == text) || (*end != '\0') || (parsed > maximum))
    {
        return -1;
    }
    *value = (uint16_t)parsed;
    return 0;
}

void command_shell_execute(const char *line, lyx_motor_t *motor)
{
    char buffer[96];
    char *token;
    char *argument;
    uint16_t rpm = 60U;

    if ((line == NULL) || (motor == NULL))
    {
        return;
    }
    strncpy(buffer, line, sizeof(buffer) - 1U);
    buffer[sizeof(buffer) - 1U] = '\0';
    token = strtok(buffer, " ");
    argument = strtok(NULL, " ");
    if (token == NULL)
    {
        return;
    }

    if ((strcmp(token, "help") == 0) || (strcmp(token, "?") == 0))
    {
        reply("nsh: help | status | fw [rpm] | rv [rpm] | drive <cm> | cw <deg> | ccw <deg> | m3508 <current> | stop | ping | move\r\n");
    }
    else if (strcmp(token, "status") == 0)
    {
        char text[96];
        snprintf(text, sizeof(text), "motor addr=%u online=%u alarm=%u pos=%ld rpm=%d\r\n",
                 motor->address, motor->online, motor->alarm, (long)motor->position,
                 (int)(motor->speed_raw / 256));
        reply(text);
    }
    else if ((strcmp(token, "fw") == 0) || (strcmp(token, "rv") == 0))
    {
        if ((argument != NULL) && (parse_u16(argument, 15000UL, &rpm) != 0))
        {
            reply("ERR rpm 1..15000\r\n");
            return;
        }
        if ((lyx_motor_write(motor->address, 0x003AU, rpm > 30U ? 30U : rpm) != 0) ||
            (lyx_motor_write(motor->address, 0x003BU, rpm) != 0) ||
            (lyx_motor_write(motor->address, 0x003CU, 60U) != 0) ||
            (lyx_motor_write(motor->address, 0x003FU, strcmp(token, "fw") == 0 ? 2U : 3U) != 0) ||
            (lyx_motor_write(motor->address, 0x0040U, 1U) != 0))
        {
            reply("ERR motor command\r\n");
            return;
        }
        reply("OK\r\n");
    }
    else if ((strcmp(token, "drive") == 0) || (strcmp(token, "cw") == 0) || (strcmp(token, "ccw") == 0))
    {
        char *end = NULL;
        long value = argument == NULL ? 0L : strtol(argument, &end, 10);
        int result = -1;
        if ((argument == NULL) || (end == argument) || (*end != '\0'))
        {
            reply("ERR number required\r\n");
            return;
        }
        if (strcmp(token, "drive") == 0)
        {
            result = chassis_motion_drive_cm(motor, motor, (int32_t)value);
        }
        else
        {
            result = chassis_motion_rotate_deg(motor, motor, strcmp(token, "cw") == 0 ? (int32_t)value : -(int32_t)value);
        }
        reply(result == 0 ? "OK\r\n" : "ERR busy\r\n");
    }
    else if (strcmp(token, "m3508") == 0)
    {
        char *end = NULL;
        long current = argument == NULL ? 0L : strtol(argument, &end, 10);
        const dji_3508_t *status;
        dji_3508_bus_status_t bus;
        char text[192];
        if ((argument == NULL) || (end == argument) || (*end != '\0') || (current > 16384L) || (current < -16384L))
        {
            reply("ERR current -16384..16384\r\n");
            return;
        }
        dji_3508_set_current(DJI_3508_MOTOR_INDEX, (int16_t)current);
        dji_3508_get_bus_status(&bus);
        status = dji_3508_status(DJI_3508_MOTOR_INDEX);
        snprintf(text, sizeof(text), "3508 id=2 online=%u angle=%u rpm=%d current=%d temp=%u tx=%lu rx=%lu txerr=%lu\r\n",
                 status->online, status->angle_raw, status->speed_rpm, status->current_raw,
                 status->temperature_c, (unsigned long)bus.tx_count,
                 (unsigned long)bus.rx_count, (unsigned long)bus.tx_errors);
        reply(text);
    }
    else if (strcmp(token, "stop") == 0)
    {
        dji_3508_set_current(DJI_3508_MOTOR_INDEX, 0);
        reply(lyx_motor_stop(motor) == 0 ? "OK\r\n" : "ERR stop\r\n");
    }
    else if (strcmp(token, "ping") == 0)
    {
        reply("pong\r\n");
    }
    else if (strcmp(token, "move") == 0)
    {
        reply(lyx_motor_move_position(motor, 0U, 30U, 60U, 60U, 51200U) == 0 ? "OK\r\n" : "ERR move\r\n");
    }
    else
    {
        reply("ERR unknown command\r\n");
    }
}
