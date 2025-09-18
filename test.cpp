// Copyright (c) 2025 Shifei Technologies. All rights reserved.
//
// This file is part of internal projects, unauthorized copying of this file
// via any medium is strictly prohibited.

//
// Created by zheng zhao on 2025/9/17.
//

#include "iic.h"
#include <cassert>
#include <cstdio>

#define LASER_BUFFER_LEN 128

int main(int argc, char *argv[]) {

    char *dev = "/dev/i2c-3";
    struct iic_config config = {0};
    uint8_t buf[LASER_BUFFER_LEN] = {0};
    uint32_t addr = 0x00;

    config.internalAddrBytes = 1;

    int fd = iic_open(dev, config);
    assert(fd >= 0);

    int ret = iic_read(fd, 0x50, addr, buf, 128);
    assert(ret >= 0);

    for (int i = 0; i < LASER_BUFFER_LEN; i++) {
        printf("%02x ", buf[i]);
        if (((i + 1) % 16) == 0) {
            printf("\n");
        }
    }

    ret = iic_close(fd);
    assert(ret == 0);
}