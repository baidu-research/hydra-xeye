/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_LED_H
#define _XEYE_LED_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LED_OFF = 0,
    LED_ON, // not valid for bi-color led
    LED_GREEN,
    LED_RED
} led_status_t;

void XeyeDrvLedInit(void);
void XeyeDrvLedOn(void);
void XeyeDrvLedOff(void);
void XeyeDrvLedToggle(void);
void XeyeDrvLedTwinkleNTimes(u8 times, u32 period);
// bi-color led api
void xeye_led_init(void);
void xeye_led_set_status(led_status_t status);
void xeye_led_twinkle_n_times(led_status_t color, u8 times, u32 period);
//rtems_task xeye_led_demo_task(rtems_task_argument unused);

#ifdef __cplusplus
}
#endif
#endif
