#include "status_indicator.h"
#include "bsp_buzzer.h"
#include "ws2812.h"

static status_code_t current_status = STATUS_BOOT;
static uint32_t last_refresh_ms;

static void show_color(uint8_t red, uint8_t green, uint8_t blue)
{
    WS2812_Ctrl(red, green, blue);
}

void status_indicator_set(status_code_t status)
{
    current_status = status;
    last_refresh_ms = 0U;
}

void status_indicator_update(uint32_t now_ms)
{
    uint8_t phase;
    if ((last_refresh_ms != 0U) && ((now_ms - last_refresh_ms) < 100U))
    {
        return;
    }
    last_refresh_ms = now_ms;
    phase = (uint8_t)((now_ms / 250U) & 1U);

    switch (current_status)
    {
    case STATUS_BOOT:
        show_color(0U, 0U, 32U);
        break;
    case STATUS_SELF_TEST:
        show_color(32U, 32U, 0U);
        break;
    case STATUS_IMU_ERROR:
        show_color(phase == 0U ? 48U : 0U, 0U, 0U);
        break;
    case STATUS_LINKS_BOTH_OK:
        show_color(0U, 32U, 0U);
        break;
    case STATUS_RS485_ONLY_ERROR:
        show_color(0U, 32U, 32U);
        break;
    case STATUS_CAN_ONLY_ERROR:
        show_color(32U, 0U, 32U);
        break;
    case STATUS_LINKS_BOTH_ERROR:
        show_color(32U, 16U, 0U);
        break;
    default:
        show_color(0U, 0U, 0U);
        break;
    }
}
