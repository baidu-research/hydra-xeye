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

#include <stdio.h>
#include <OsDrvCpr.h>
#include <DrvTempSensor.h>
#define MVLOG_UNIT_NAME xeye_tsens
#include <mvLog.h>

s32 Tsens_clk_enable(void) {
    tyAuxClkDividerCfg tsens_Clk[] = {
        {
            .auxClockEnableMask = (u32)(1 << CSS_AUX_TSENS),
            .auxClockSource = CLK_SRC_REFCLK0,
            .auxClockDivNumerator = 1,
            .auxClockDivDenominator = 10,
        },
        {0, 0, 0, 0}, // Null Terminated List
    };
    s32 sc = OsDrvCprAuxClockArrayConfig(tsens_Clk);

    if (sc) {
        return sc;
    }
}

void SampleTemperature(void) {
    int32_t tempSensorStat = 0;
    float temp_MSS;
    float temp_CSS;
    float temp_UPA0;
    float temp_UPA1;

    tempSensorStat = DrvTempSensorGetSample(TSENS_CSS, &temp_CSS);
    tempSensorStat += DrvTempSensorGetSample(TSENS_MSS, &temp_MSS);
    tempSensorStat += DrvTempSensorGetSample(TSENS_UPA0, &temp_UPA0);
    tempSensorStat += DrvTempSensorGetSample(TSENS_UPA1, &temp_UPA1);
    mvLog(MVLOG_INFO, "Status:%d,Temperature CSS=%2.1fC, MSS=%2.1fC, UPA0=%2.1fC, UPA1=%2.1fC\n", tempSensorStat,\
                    temp_CSS, temp_MSS, temp_UPA0, temp_UPA1);
}

void TemperatureInit(void) {
    DrvTempSensConfig tempParam = {1};
    int32_t tempSensorStat = 0;

    Tsens_clk_enable();
    DrvTempSensorInitialise(&tempParam);
    //config temperature sensor
    tempSensorStat = DrvTempSensorSetMode(TSENS_CSS, TSENS_CONTINUOUS_MODE, TSENS_SAMPLE_TEMP);
    tempSensorStat += DrvTempSensorSetMode(TSENS_MSS, TSENS_CONTINUOUS_MODE, TSENS_SAMPLE_TEMP);
    tempSensorStat += DrvTempSensorSetMode(TSENS_UPA0, TSENS_CONTINUOUS_MODE, TSENS_SAMPLE_TEMP);
    tempSensorStat += DrvTempSensorSetMode(TSENS_UPA1, TSENS_CONTINUOUS_MODE, TSENS_SAMPLE_TEMP);
    mvLog(MVLOG_INFO, "Temperature sensor configuration status:%d\n", tempSensorStat);
}

float GetCssTemp(void) {
    float temp;
    DrvTempSensorGetSample(TSENS_CSS, &temp);
    return temp;
}

float GetMssTemp(void) {
    float temp;
    DrvTempSensorGetSample(TSENS_MSS, &temp);
    return temp;
}

float GetUpa0Temp(void) {
    float temp;
    DrvTempSensorGetSample(TSENS_UPA0, &temp);
    return temp;
}

float GetUpa1Temp(void) {
    float temp;
    DrvTempSensorGetSample(TSENS_UPA1, &temp);
    return temp;
}

