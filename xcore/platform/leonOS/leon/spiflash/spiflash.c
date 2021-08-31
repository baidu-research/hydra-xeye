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
#include <stdlib.h>
#include "mv_types.h"
#include "spiflash.h"
#include <bsp.h>
#include <icb_defines.h>
#include <fcntl.h>

#include "OsDrvSpiBus.h"
#include "DrvTimer.h"
#include "DrvGpio.h"

#include <bsp/irq.h>
#include <rtems/libi2c.h>
#include <rtems/libio.h>
#include "OsDrvCpr.h"

#include <SpiFlashN25QDevice.h>

#include "swcShaveLoaderPrivate.h"
#include "swcMemoryTransfer.h"
#include <swcLeonUtils.h>
#include <DrvLeonL2C.h>
#include <DrvLeonL2CDefines.h>
#include <swcCrc.h>
#include "spi_i2c_config.h"

#include <VcsHooksApi.h>
#include <DrvRegUtilsDefines.h>
#include <registersMyriadMa2x5x.h>
#define MVLOG_UNIT_NAME xeye_spiflash
#include <mvLog.h>

static int spi_fd = 0;
int erase_spi_flash(int fd, struct tFlashParams* p) {
    int status;

    struct spiFlashN25QEraseArgs_t eraseArgs = {
        .offset = p->offset,
        .size   = p->size,
    };

    status = ioctl(fd, FLASH_CMD_ERASE, &eraseArgs);

    if (status) {
        mvLog(MVLOG_ERROR, "Unable to erase device, err = %d", status);
    }

    return status;
}

int erase_spi_flash_Ex(struct tFlashParams* p) {
    return erase_spi_flash(spi_fd, p);
}

int write_spi_flash(int fd, struct tFlashParams* p) {
    int status;

    status =  lseek(fd, p->offset, SEEK_SET);

    if (status != (int)p->offset) {
        perror("seek error: ");
        return -1;
    }

    status = write(fd, p->inBuff, p->size);

    if (status != (int)p->size) {
        mvLog(MVLOG_ERROR, "Unable to write all %u bytes to device. %d sent ", (unsigned int)p->size, status);
        return -1;
    }

    return 0;
}

int write_spi_flash_Ex(struct tFlashParams* p) {
    return write_spi_flash(spi_fd, p);
}

int read_spi_flash(int fd, struct tFlashParams* p) {
    int status = 0;

    status =  lseek(fd, p->offset, SEEK_SET);

    if (status != (int)p->offset) {
        perror("seek error: ");
        return -1;
    }

    status = read(fd, p->outBuff, p->size);

    if (status != (int)p->size) {
        mvLog(MVLOG_ERROR, "Unable to read all %u bytes from device. %d sent ", (unsigned int)p->size, status);
        return -1;
    }

    return 0;
}

int read_spi_flash_Ex(struct tFlashParams* p) {
    return read_spi_flash(spi_fd, p);
}

int32_t flash_init(void) {
    int fd = 0;
    int ret = 0;
    v_myr2_spi_test_pin_config();

    if (i_myr2_register_libi2c_spi_bus() != 0) {
        mvLog(MVLOG_ERROR, "init error");
    }

    const char* devName = SPI_BUS_NAME "." SPI_FLASH_NAME;

    fd = open(devName, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd < 0) {
        mvLog(MVLOG_ERROR, "spi open failed");
        ret = ERR_IO_DRV;
    }

    spi_fd = fd;
    return ret;
}

int update_firmware(const char* buf, int size, const u8* param, u8* resp) {
    int erno = 0;
    assert(resp != NULL || param != NULL);
    struct tFlashParams myFlash;
    myFlash.offset = ALIGN_TO_SUBSECTOR(0);
    myFlash.size = ALIGN_TO_SUBSECTOR(((size / 10240) + 1) * 10240);
    mvLog(MVLOG_INFO, "erase size:%d", myFlash.size);
    erase_spi_flash_Ex(&myFlash);
    mvLog(MVLOG_INFO, "erase spiflash Done");

    int wsize = 10 * 1024;
    size = ((size / 10240) + 1) * 10240;
    int total_size = size;
    int statis_size = 0;
    int skip = 0;
    myFlash.offset = 0;
    myFlash.inBuff = buf;
    mvLog(MVLOG_INFO, "start to write flash...");

    while (wsize < size) {
        myFlash.size = wsize;
        write_spi_flash_Ex(&myFlash);
        statis_size += wsize;

        if (skip % 10 == 0) {
            mvLog(MVLOG_INFO, "Write %d/%d", statis_size, total_size);
        }

        skip++;
        size -= wsize;
        myFlash.offset += wsize;
        myFlash.inBuff += wsize;
    }

    myFlash.size = size;
    write_spi_flash_Ex(&myFlash);
    statis_size += wsize;
    mvLog(MVLOG_INFO, "Write %d/%d", total_size, total_size);
    mvLog(MVLOG_INFO, "Write flash OK....");

FIRMAPPLY_RETURN:

    if (erno == 0) {
        mvLog(MVLOG_INFO, "Firmapply successfully");
    } else {
        mvLog(MVLOG_ERROR, "Firmapply failed");
    }

    return 0;
}

int  verify_firmware(unsigned char* buf, int size) {
    int erno = 0;
    struct tFlashParams myFlash;
    myFlash.offset = ALIGN_TO_SUBSECTOR(0);
    myFlash.size = ALIGN_TO_SUBSECTOR(((size / 10240) + 1) * 10240);
    mvLog(MVLOG_INFO, "verify firmware read size:%d", myFlash.size);

    int rsize = 10 * 1024;
    size = ((size / 10240) + 1) * 10240;
    int total_size = size;
    int statis_size = 0;
    int skip = 0;
    myFlash.offset = 0;
    myFlash.outBuff = buf;
    mvLog(MVLOG_INFO, "verify firmware start to read flash...");

    while (rsize < size) {
        myFlash.size = rsize;
        read_spi_flash_Ex(&myFlash);
        statis_size += rsize;

        if (skip % 10 == 0) {
            mvLog(MVLOG_INFO, "verify firmware Read %d/%d", statis_size, total_size);
        }

        skip++;
        size -= rsize;
        myFlash.offset += rsize;
        myFlash.outBuff += rsize;
    }

    myFlash.size = size;
    read_spi_flash_Ex(&myFlash);
    statis_size += rsize;
    mvLog(MVLOG_INFO, "verify firmware Read %d/%d", total_size, total_size);
    mvLog(MVLOG_INFO, "verfify firmware Read flash OK....");
FIRMAPPLY_RETURN:

    if (erno == 0) {
        mvLog(MVLOG_INFO, "Firmapply read successfully");
    } else {
        mvLog(MVLOG_ERROR, "Firmapply  read  failed");
    }

    return 0;
}

int read_spiflashfirm(u8* buf, int32_t size) {
    struct tFlashParams myFlash;
    myFlash.offset = ALIGN_TO_SUBSECTOR(0);
    myFlash.size = ALIGN_TO_SUBSECTOR(size);
    myFlash.outBuff = buf;
    return read_spi_flash_Ex(&myFlash);
}

int system_reset(void) {
    SET_REG_WORD(CPR_MAS_RESET_ADR, 0x00);
    return 0;
}


