/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/
 
#include "DrvGpio.h"
#include "xeye_led.h"
#include <rtems.h>

//Indicate that in case of led turns on what the gpio state is.
#define LED_ON_GPIO_STATE 0
#define LED_GPIO_NUM 27
#define LED_GPIO_NUM_COLOR1 26
#define LED_GPIO_NUM_COLOR2 42

// TODO(zhoury):refactor led driver
void XeyeDrvLedInit(void) {
    DrvGpioSetMode(LED_GPIO_NUM, D_GPIO_DIR_OUT | D_GPIO_MODE_7);
}

void XeyeDrvLedOn(void) {
    DrvGpioSetPin(LED_GPIO_NUM, 1);
}

void XeyeDrvLedOff(void) {
    DrvGpioSetPin(LED_GPIO_NUM, 0);
}

void XeyeDrvLedToggle(void) {
    DrvGpioToggleState(LED_GPIO_NUM);
}

void XeyeDrvLedTwinkleNTimes(u8 times, u32 period) {
    uint8_t i;

    if(period == 0) {
        period = 200;
    }
    for(i = 0; i < times; i++) {
        XeyeDrvLedOn();
        rtems_task_wake_after(period);
        XeyeDrvLedOff();
        rtems_task_wake_after(period);
    }
}

void xeye_led_init(void) {
    DrvGpioSetMode(LED_GPIO_NUM_COLOR1, D_GPIO_DIR_OUT | D_GPIO_MODE_7);
    DrvGpioSetMode(LED_GPIO_NUM_COLOR2, D_GPIO_DIR_OUT | D_GPIO_MODE_7);
}

void xeye_led_set_status(led_status_t status) {
    switch(status) {
        case LED_OFF:
            DrvGpioSetPin(LED_GPIO_NUM_COLOR1, 0);
            DrvGpioSetPin(LED_GPIO_NUM_COLOR2, 0);
            break;
        case LED_GREEN:
            DrvGpioSetPin(LED_GPIO_NUM_COLOR1, 0);
            DrvGpioSetPin(LED_GPIO_NUM_COLOR2, 1);
            break;
        case LED_RED:
            DrvGpioSetPin(LED_GPIO_NUM_COLOR1, 1);
            DrvGpioSetPin(LED_GPIO_NUM_COLOR2, 0);
            break;
        default:
            break;
    }
}

void xeye_led_twinkle_n_times(led_status_t color, u8 times, u32 period) {
    uint8_t i;

    if(period == 0) {
        period = 200;
    }

    for(i = 0; i < times; i++) {
        xeye_led_set_status(color);
        rtems_task_wake_after(period);
        xeye_led_set_status(LED_OFF);
        rtems_task_wake_after(period);
    }
}

