/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <stdio.h>
#include <mv_types.h>
#include <string.h>
#include <stdbool.h>
#include "xeye_eeprom.h"
#include "time.h"
#include "OsDrvTimer.h"
#define MVLOG_UNIT_NAME xeye_eeprom
#include <mvLog.h>

bool verifyCheckSum(uint8_t* src, uint8_t size) {
    uint32_t sum = 0;
    uint32_t index;

    for (index = 0; index < size; index++) {
        sum = sum + *(src + index);
    }

    sum = sum & 0xFF;

    if (sum == 0) {
        return true; //checksum is ok
    } else {
        return false; //checksum is not ok
    }
}

uint8_t calcCheckSum(uint8_t* sc, uint8_t size) {
    uint32_t sum = 0;

    for (uint32_t k = 0; k < size; k++) {
        sum += sc[k];
    }

    sum = 0 - (sum & 0xff);

    return (sum & 0xff);
}

// dump memory data
void dumpbuf(uint8_t* buf, uint32_t len) {
    mvLog(MVLOG_INFO, " dump memory buf: %p len: %d", buf, len);

    for (uint32_t i = 0; i < len; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }

        printf("0x%x ", *(buf + i));
    }

    printf("\n");
}

void dump_e2prom_msg(E2promMsg_t* E2promMsg) {
    mvLog(MVLOG_INFO, "E2promMsg->ID: 0x%04x", E2promMsg->ID);
    mvLog(MVLOG_INFO, "E2promMsg->format: 0x%02x", E2promMsg->format);
    mvLog(MVLOG_INFO, "E2promMsg->remaining_bytes: 0x%08x", E2promMsg->remaining_bytes);
    mvLog(MVLOG_INFO, "E2promMsg->board_name[0-5]: %c%c%c%c%c%c", \
          E2promMsg->board_name[0], E2promMsg->board_name[1], E2promMsg->board_name[2],
          E2promMsg->board_name[3], E2promMsg->board_name[4], E2promMsg->board_name[5]);
    mvLog(MVLOG_INFO, "E2promMsg->pcb_revision: 0x%02x", E2promMsg->pcb_revision);
    mvLog(MVLOG_INFO, "E2promMsg->mechanical_revision: 0x%02x", E2promMsg->mechanical_revision);
    mvLog(MVLOG_INFO, "E2promMsg->electrical_revision: 0x%02x", E2promMsg->electrical_revision);
    mvLog(MVLOG_INFO, "E2promMsg->serial_number: 0x%08x", E2promMsg->serial_number);
    mvLog(MVLOG_INFO, "E2promMsg->mac_number: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", \
          E2promMsg->ethernet_mac_address[0], E2promMsg->ethernet_mac_address[1],
          E2promMsg->ethernet_mac_address[2], E2promMsg->ethernet_mac_address[3],
          E2promMsg->ethernet_mac_address[4], E2promMsg->ethernet_mac_address[5]);
    mvLog(MVLOG_INFO, "E2promMsg->feature_flags: 0x%08x", E2promMsg->feature_flags);
    mvLog(MVLOG_INFO, "E2promMsg->reserved: 0x%02x 0x%02x 0x%02x 0x%02x", \
          E2promMsg->reserved[0], E2promMsg->reserved[1],
          E2promMsg->reserved[2], E2promMsg->reserved[3]);
    mvLog(MVLOG_INFO, "E2promMsg->checksum: 0x%02x", E2promMsg->checksum);
}

// read eeprom msg
bool ReadE2promMsg(I2CM_Device* i2cHandle, E2promMsg_t* E2promMsg) {
    uint8_t protocolReadSample2[] = I2C_PROTO_READ_16BA;
    I2CM_StatusType status;
    uint8_t* eeprom_data  = (uint8_t*)E2promMsg;

    memset(eeprom_data, 0, E2PROM_MEMORY_SIZE);
    status = DrvI2cMTransaction(i2cHandle,
                                EEPROM_I2C_SLAVE_ADDRESS,
                                EEPROM_PART_1_OFFSET,
                                protocolReadSample2,
                                (uint8_t*) &eeprom_data[EEPROM_PART_1_OFFSET],
                                EEPROM_PART_1_SIZE);

    if (status == I2CM_STAT_OK) {
        status = DrvI2cMTransaction(i2cHandle,
                                    EEPROM_I2C_SLAVE_ADDRESS,
                                    EEPROM_PART_2_OFFSET,
                                    protocolReadSample2,
                                    (uint8_t*) &eeprom_data[EEPROM_PART_2_OFFSET],
                                    EEPROM_PART_2_SIZE);
        // dumpbuf(eeprom_data, E2PROM_MEMORY_SIZE);
        if (status == I2CM_STAT_OK) {
            return true;
        } else {
            mvLog(MVLOG_ERROR, "EEPROM_PART_2 read error status: %d", status);
            return false;
        }
    } else {
        mvLog(MVLOG_ERROR, "EEPROM_PART_1 read error status: %d", status);
        return false;
    }
}

// read eeprom msg
bool WriteE2promMsg(I2CM_Device* i2cHandle, E2promMsg_t* E2promMsg) {
    uint8_t protocolWriteSample2[] = I2C_PROTO_WRITE_16BA;
    I2CM_StatusType status;
    uint8_t* eeprom_data  = (uint8_t*)E2promMsg;

    status = DrvI2cMTransaction(i2cHandle,
                                EEPROM_I2C_SLAVE_ADDRESS,
                                EEPROM_PART_1_OFFSET,
                                protocolWriteSample2,
                                (uint8_t*) &eeprom_data[EEPROM_PART_1_OFFSET],
                                EEPROM_PART_1_SIZE);

    if (status == I2CM_STAT_OK) {
        // Warning: must to add delay
        DrvTimerSleepMs(500);
        status = DrvI2cMTransaction(i2cHandle,
                                    EEPROM_I2C_SLAVE_ADDRESS,
                                    EEPROM_PART_2_OFFSET,
                                    protocolWriteSample2,
                                    (uint8_t*) &eeprom_data[EEPROM_PART_2_OFFSET],
                                    EEPROM_PART_2_SIZE);
        if (status == I2CM_STAT_OK) {
            return true;
        } else {
            mvLog(MVLOG_ERROR, "EEPROM_PART_2 write error status: %d", status);
            return false;
        }
    } else {
        mvLog(MVLOG_ERROR, "EEPROM_PART_1 write error status: %d", status);
        return false;
    }
}

// read Mac address from eeprom
// TODO(zhoury): we need hide I2C device with eeprom api interface
void XeyeDrvReadMacFromEeprom(I2CM_Device* i2cHandle, char* mac) {
    E2promMsg_t E2promMsg;

    if (ReadE2promMsg(i2cHandle, &E2promMsg)) {
        memcpy(mac, E2promMsg.ethernet_mac_address, MAC_ADDR_LENGTH);
    } else {
        mvLog(MVLOG_ERROR, "read eeprom msg fail");
    }
}

// Burn Mac address to eeprom
void BurnMacToEeprom(I2CM_Device* i2cHandle, char* mac_address) {
    E2promMsg_t E2promMsg;

    if (!ReadE2promMsg(i2cHandle, &E2promMsg)) {
        mvLog(MVLOG_ERROR, "read eeprom msg fail");
    }

    for (int i = 0; i < MAC_ADDR_LENGTH; i++) {
        E2promMsg.ethernet_mac_address[i] = mac_address[i];
    }

    E2promMsg.checksum = calcCheckSum((uint8_t*)&E2promMsg, E2PROM_MEMORY_SIZE - 1);
    // Warning: must to add delay
    DrvTimerSleepMs(500);

    if (!WriteE2promMsg(i2cHandle, &E2promMsg)) {
        mvLog(MVLOG_ERROR, "write eeprom msg fail");
    }

    // Warning: must to add delay
    DrvTimerSleepMs(500);
    memset((char*)&E2promMsg, 0, E2PROM_MEMORY_SIZE);

    if (!ReadE2promMsg(i2cHandle, &E2promMsg)) {
        mvLog(MVLOG_ERROR, "read eeprom msg fail");
    }

    mvLog(MVLOG_INFO, "burn Mac success");
}

int ReadHwVersionFromEeprom(I2CM_Device* i2cHandle) {
    E2promMsg_t E2promMsg;

    if (ReadE2promMsg(i2cHandle, &E2promMsg)) {
        return E2promMsg.pcb_revision;
    } else {
        mvLog(MVLOG_ERROR, "read eeprom msg fail");
        return -1;
    }
}

void WriteHwVersionFromEeprom(I2CM_Device* i2cHandle, int pcb_revision) {
    E2promMsg_t E2promMsg;

    if (ReadE2promMsg(i2cHandle, &E2promMsg)) {
        E2promMsg.pcb_revision = pcb_revision;
        E2promMsg.checksum = calcCheckSum((uint8_t*)&E2promMsg, E2PROM_MEMORY_SIZE - 1);
        // Warning: must to add delay
        DrvTimerSleepMs(500);

        if (!WriteE2promMsg(i2cHandle, &E2promMsg)) {
            mvLog(MVLOG_ERROR, "write eeprom msg fail");
        }

        // Warning: must to add delay
        DrvTimerSleepMs(500);
        memset((char*)&E2promMsg, 0, E2PROM_MEMORY_SIZE);

        if (!ReadE2promMsg(i2cHandle, &E2promMsg)) {
            mvLog(MVLOG_ERROR, "read eeprom msg fail");
        }
    } else {
        mvLog(MVLOG_ERROR, "read eeprom msg fail");
    }
}
