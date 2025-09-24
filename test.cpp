/*
 * Copyright (C) 2025-09-23  zheng zhao
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v3.0 or later.
 * See <https://www.gnu.org/licenses/>.
 */


#include "iic.h"
#include "iicc++.h"
#include <cassert>
#include <cstdio>

#define LASER_BUFFER_LEN 128

void test_print(uint8_t *buf, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if (((i + 1) % 16) == 0) {
            printf("\n");
        }
    }
    printf("----------------end---------------------\n");
}

int main(int argc, char *argv[]) {

    char *dev = "/dev/i2c-0";
    char *dev1 = "/dev/i2c-1";
    char *dev2 = "/dev/i2c-2";
    struct iic_config config = {0};
    uint8_t buf[LASER_BUFFER_LEN] = {0}, wbuf[LASER_BUFFER_LEN] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    uint32_t addr = 0x80;

    config.internalAddrBytes = 1;
    config.pageBytes = 8;

    class IicDriver iic;

    int rc = iic.open(dev1, config);
    assert(rc == 0);

    rc = iic.read_ioctl( 0x51, addr, buf, LASER_BUFFER_LEN);
    assert(rc >= 0);

    test_print(buf, LASER_BUFFER_LEN);

    rc = iic.write_ioctl(0x51, addr, wbuf, 16);
    assert(rc >= 0);


    rc = iic.read_ioctl( 0x51, addr, buf, LASER_BUFFER_LEN);
    assert(rc >= 0);

    test_print(buf, LASER_BUFFER_LEN);

#if 0
    int fd = iic_open(dev, config);
    assert(fd >= 0);

    int fd1 = iic_open(dev1, config);
    assert(fd >= 0);

    int fd2 = iic_open(dev2, config);
    assert(fd >= 0);

    int ret = iic_read_ioctl(fd, 0x51, addr, buf, 128);
    assert(ret >= 0);

    test_print(buf, LASER_BUFFER_LEN);

    printf("----------------read----------------\n");
    ret = iic_write_ioctl(fd, 0x51, addr, wbuf, 16);
    assert(ret >= 0);

    ret = iic_close(fd);
    assert(ret == 0);

    iic_debug();
    ret = iic_close(fd);
    assert(ret >= 0);
    iic_close(fd1);
    assert(ret >= 0);
    iic_close(fd2);
    assert(ret >= 0);
#endif
}