// Copyright (c) 2025 Shifei Technologies. All rights reserved.
//
// This file is part of internal projects, unauthorized copying of this file
// via any medium is strictly prohibited.

//
// Created by zheng zhao on 2025/9/23.
//

#include "iicplusplus.h"

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <endian.h>
#include <cstring>

Iic::~Iic() {
    closed();
}

int Iic::open(const std::string &device, const struct iic_config &config) {
    _fd = ::open(device.c_str(), O_RDWR);
    if (_fd < 0) {
        return -errno;
    }

    _config = config;
    _dev = device;
    return 0;
}

int Iic::read_ioctl(const uint16_t deviceAddr,
                   const uint32_t internalAddr,
                   uint8_t *buf,
                   const uint16_t len) {

    uint16_t flag                           = (_config.tenBit ? I2C_M_TEN : 0 ) | I2C_M_RD;
    struct i2c_msg ioctl_msg[2]             = {0};
    struct i2c_rdwr_ioctl_data ioctl_data   = {0};
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    struct iic_reverse reverse_internalAddr = {.addr = internalAddr};
#else
    struct iic_reverse reverse_internalAddr = {.addr = htonl(internalAddr)};
#endif

    if (_config.internalAddrBytes) {

        ioctl_msg[0].len    = _config.internalAddrBytes;
        ioctl_msg[0].addr	= deviceAddr;
        ioctl_msg[0].flags	= 0;
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        ioctl_msg[0].buf    = reverse_internalAddr.msg;
#else
        ioctl_msg[0].buf    = reverse_internalAddr.msg + (IIC_IDDR_LEN - _config.internalAddrBytes);
#endif

        ioctl_msg[1].len    = len;
        ioctl_msg[1].addr	= deviceAddr;
        ioctl_msg[1].buf    = buf;
        ioctl_msg[1].flags	= flag;

        ioctl_data.nmsgs	= 2;
        ioctl_data.msgs		= ioctl_msg;

    } else {

        ioctl_msg[0].len    = len;
        ioctl_msg[0].addr	= deviceAddr;
        ioctl_msg[0].buf    = buf;
        ioctl_msg[0].flags	= flag;

        ioctl_data.nmsgs	= 1;
        ioctl_data.msgs		= ioctl_msg;
    }

    return ioctl(_fd, I2C_RDWR, &ioctl_data);
}

int Iic::write_ioctl(const uint16_t deviceAddr,
                    const uint32_t internalAddr,
                    uint8_t *buf,
                    const uint16_t len) {

    uint8_t tx[IIC_PAGE_MAX] {};
    int ret                                 = IIC_ERR;
    uint32_t iAddr                          = internalAddr;
    struct i2c_msg ioctl_msg                = {0};
    struct i2c_rdwr_ioctl_data ioctl_data   = {0};
    struct iic_reverse reverse_internalAddr = {0};
    uint16_t flag = _config.tenBit ? I2C_M_TEN : 0, start = 0, remain = len, wSize;

    do {
        //update internal address
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        reverse_internalAddr.addr           = iAddr;
        memcpy(tx, reverse_internalAddr.msg, _config.internalAddrBytes);
#else
        reverse_internalAddr.addr           = htonl(iAddr);
        memcpy(tx, reverse_internalAddr.msg + (IIC_IDDR_LEN - _config.internalAddrBytes), _config.internalAddrBytes);
#endif
        
        wSize = IIC_GET_WRITE_SIZE(_config.pageBytes, iAddr, remain);

        memcpy(tx + _config.internalAddrBytes, buf + start, wSize);

        ioctl_msg.len	                    = _config.internalAddrBytes + wSize;
        ioctl_msg.addr	                    = deviceAddr;
        ioctl_msg.buf	                    = tx;
        ioctl_msg.flags	                    = flag;

        ioctl_data.nmsgs                    = 1;
        ioctl_data.msgs	                    = &ioctl_msg;

        ret = ioctl(_fd, I2C_RDWR, &ioctl_data);
        if (ret == -1) {
            return -errno;
        }

        start   += wSize;
        remain  -= wSize;
        iAddr   += wSize;
    } while (remain);

    return ret;
}


int Iic::closed() {
    return _fd == -1 ? 0 : ::close(_fd);
}

void Iic::debug() {
    std::cout << "dev:" << _dev.c_str()
              << " fd:" << _fd
              << " pageBytes:" << _config.pageBytes
              << " internalAddrBytes:" << _config.internalAddrBytes
              << " tenBit:" << _config.tenBit
              << " delayUs:" << _config.delayUs
              << std::endl;
}