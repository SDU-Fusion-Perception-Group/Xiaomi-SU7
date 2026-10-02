#ifndef COMMAND_SHELL_H
#define COMMAND_SHELL_H

#include "lyx_motor.h"

#ifdef __cplusplus
extern "C" {
#endif

void command_shell_execute(const char *line, lyx_motor_t *motor);

#ifdef __cplusplus
}
#endif

#endif
