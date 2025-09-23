// Copyright (c) 2025 Shifei Technologies. All rights reserved.
//
// This file is part of internal projects, unauthorized copying of this file
// via any medium is strictly prohibited.

//
// Created by zheng zhao on 2025/9/16.
//

#ifndef IIC_COMMON_H
#define IIC_COMMON_H

#include <stdint.h>

#define IIC_DELAY_MICROSECOND       500
#define IIC_IDDR_LEN                sizeof(uint32_t)
#define IIC_PAGE_MAX                (256 + IIC_IDDR_LEN)

#define IIC_ERR                     (-200)

#define IIC_GET_WRITE_SIZE(p, i, r) (p ? (((i % p + r) > p) ? p : (p - i % p)) : r)

enum iicMode {
    IIC_MODE_IOCTL,
    IIC_MODE_SMBUS,
};

struct iic_config {
    /* max number of bytes per page,
     * 1K/2K        8
     * 4K/8K/16K    16,
     * 32K/64K      32
     * no page      0 etc.*/
    uint32_t    pageBytes;
    /* device internal(word) address bytes,
     * such as: 24C04 1 byte, 24C64 2 bytes.
     * Default 1 byte */
    uint8_t     internalAddrBytes;
    /* 10-bit address, default 7-bit address */
    uint8_t     tenBit;
    /* operation delay, unit microsecond,
     * default */
    int         delayUs;
};

struct iic_reverse {
    union {
        uint32_t addr;
        uint8_t msg[IIC_IDDR_LEN];
    };
};


#endif //IIC_COMMON_H
