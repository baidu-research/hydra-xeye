/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/


// Includes
// ----------------------------------------------------------------------------
#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

#ifdef __cplusplus
extern "C" {
#endif
extern void SampleTemperature();
extern void TemperatureInit();
extern float GetCssTemp();
extern float GetMssTemp();
// TODO(hyx): Correct the name of uap to upa
extern float GetUpa0Temp();
extern float GetUpa1Temp();

#ifdef __cplusplus
}
#endif
#endif

