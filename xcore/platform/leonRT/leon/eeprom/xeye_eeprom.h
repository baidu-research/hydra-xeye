/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_EEPROM_H
#define _XEYE_EEPROM_H
#include "DrvI2cMaster.h"

#ifdef __cplusplus
extern "C" {
#endif

#define E2PROM_MEMORY_SIZE               35
#define EEPROM_I2C_SLAVE_ADDRESS         0x50
#define EEPROM_PART_1_SIZE               32
#define EEPROM_PART_2_SIZE               3
#define EEPROM_PART_1_OFFSET             0x00
#define EEPROM_PART_2_OFFSET             0x20
#define MAC_IN_EEPROM                    0x14
#define MAC_ADDR_LENGTH                  6
#define I2C_PROTO_WRITE_16BA {S_ADDR_WR, R_ADDR_H, R_ADDR_L, DATAW, LOOP_MINUS_1}
#define I2C_PROTO_READ_16BA  {S_ADDR_WR, R_ADDR_H, R_ADDR_L, S_ADDR_RD, DATAR, LOOP_MINUS_1}
// struct
typedef struct E2promMsg {
    uint16_t ID;
    uint8_t format;
    uint32_t remaining_bytes;
    uint8_t board_name[6];
    uint8_t pcb_revision;
    uint8_t mechanical_revision;
    uint8_t electrical_revision;
    uint32_t serial_number;
    uint8_t ethernet_mac_address[6];
    uint32_t feature_flags;
    uint8_t reserved[4];
    uint8_t checksum;
} __attribute__((packed)) E2promMsg_t;

// gloabl function
void XeyeDrvReadMacFromEeprom(I2CM_Device* i2cHandl, char* mac);
void BurnMacToEeprom(I2CM_Device* i2cHandle, char* mac_address);
void WriteHwVersionFromEeprom(I2CM_Device* i2cHandle, int pcb_revision);
int ReadHwVersionFromEeprom(I2CM_Device* i2cHandle);

#ifdef __cplusplus
}
#endif
#endif
