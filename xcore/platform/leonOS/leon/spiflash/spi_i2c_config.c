/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

// 1: Includes
// ----------------------------------------------------------------------------
#include "mv_types.h"
#include "OsDrvCpr.h"
#include "OsDrvGpio.h"
#include "OsDrvSpiBus.h"
#include "OsDrvI2cBus.h"
#include "spi_i2c_config.h"
#include <SpiFlashN25QDevice.h>
#define MVLOG_UNIT_NAME xeye_spi_i2c_config
#include <mvLog.h>

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define SPI_BUS_NAME   "/dev/spi"
#define SPI_FLASH_NAME "flash"

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 4: Static Local Data
// ----------------------------------------------------------------------------

// Declare the SPI bus
DECLARE_SPI_BUS_GPIO_SS(myr2_spi_0, 1, 0, 0, 1, 1, 1 * 1000 * 1000, 8);

// declare I2C bus
#define INTERRUPTPRIORITY   8   //priority of the I2C bus
#define ADDR10BIT           0   //addr10bit_ can be 0 for 7 bits address or 1 for 10 bits address
#define PMIC_I2C_ADDRESS        (0x6C>>1)

// Declare the I2C bus
DECLARE_I2C_BUS(myr2_i2c, 3, I2C_SPEED_SS, ADDR10BIT, INTERRUPTPRIORITY);

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
// 6: Functions Implementation
// ----------------------------------------------------------------------------


/**********************************************************************
 * Function: void myr2_spi_test_pin_config(void)
 * Description: function called at init to configure SPI port
 ***********************************************************************/
void v_myr2_spi_test_pin_config(void) {
    DrvGpioModeRange(74, 77, 0 |  D_GPIO_DIR_OUT);
}


/**********************************************************************
 * Function: int myr2_register_libi2c_spi_bus(void)
 * Return Val:  - OS_MYR_DRV_SUCCESS (0) if all initialization functions executed correctly,
 *              - different then 0 if any init function failed.
 ***********************************************************************/
int i_myr2_register_libi2c_spi_bus(void) {
    int ret_code = RTEMS_SUCCESSFUL;
    int spi0_busno;
    u32 spi_flash_minor;

    /*MSS_CP_TIM
     * init I2C library (if not already done)
     */
    ret_code = (int)OsDrvLibI2CInitialize();

    if (ret_code != (int)RTEMS_SUCCESSFUL) {
        mvLog(MVLOG_ERROR, "rtems_libi2c_initialize FAILED %d", ret_code);
        goto exit_myr2_register_libi2c_spi_bus;
    }

    /*
     * register first I2C bus
     */

    ret_code = (int)OsDrvLibI2CRegisterBus(SPI_BUS_NAME,
                                           (rtems_libi2c_bus_t*)&myr2_spi_0);

    if (ret_code < 0) {
        mvLog(MVLOG_ERROR, "Could not register the bus");
        goto exit_myr2_register_libi2c_spi_bus;
    }

    spi0_busno = ret_code;

    // Returns minor
    ret_code = (int)OsDrvLibI2CRegisterDevice(SPI_FLASH_NAME,
               spi_flash_N25Q_driver_descriptor,
               spi0_busno, 77);

    if (ret_code < 0) {
        mvLog(MVLOG_ERROR, "Could not register the spi device");
        goto exit_myr2_register_libi2c_spi_bus;
    }

    spi_flash_minor = ret_code;

    if ((ret_code = (int)OsDrvIOInitialize(rtems_libi2c_major, spi_flash_minor,
                                           NULL)) != (int)RTEMS_SUCCESSFUL) {
        mvLog(MVLOG_ERROR, "rtems_io_initialize failed with sc %d", ret_code);
        goto exit_myr2_register_libi2c_spi_bus;
    }

exit_myr2_register_libi2c_spi_bus:
    return (ret_code);
}

/* End of File */
