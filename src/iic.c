// Copyright (c) 2025 Shifei Technologies. All rights reserved.
//
// This file is part of internal projects, unauthorized copying of this file
// via any medium is strictly prohibited.

//
// Created by zheng zhao on 2025/9/17.
//

#include "iic.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <malloc.h>
#include <unistd.h>
#include <netinet/in.h>
#include <endian.h>
#include <string.h>
#include <errno.h>

#define IIC_GET_WRITE_SIZE(p, i, r) (p ? (((i % p + r) > p) ? p : (p - i % p)) : r)

struct iic_reverse {
    union {
        uint32_t addr;
        uint8_t msg[IIC_IDDR_LEN];
    };
};

struct entry {
    int fd;
    char dev[50];
    struct iic_config config;
    LIST_ENTRY(entry) entries;
};

LIST_HEAD(list_node, entry);

struct list_node list_head;

static struct entry *match_fd(const int fd) {
    struct entry *node = LIST_FIRST(&list_head);
    while (NULL != node) {
        if (node->fd == fd) {
            break;
        }
        node = LIST_NEXT(node, entries);
    };
    return node;
}

static void iic_free(struct entry *node) {
    LIST_REMOVE(node, entries);
    free(node);
    node = NULL;
}

int iic_open(const char *const device,
             const struct iic_config config) {

    int fd = open(device, O_RDWR);
    if (fd < 0) {
        return -errno;
    }

    struct entry *node = (struct entry *)calloc(1, sizeof(struct entry));
    if (NULL == node) {
        close(fd);
        return -errno;
    }

    node->fd        = fd;
    node->config    = config;
    memcpy(node->dev, device, strlen(device));

    LIST_INSERT_HEAD(&list_head, node, entries);
    return fd;
}

int iic_read_ioctl(const int fd,
                   const uint16_t deviceAddr,
                   const uint32_t internalAddr,
                   uint8_t *buf,
                   const uint16_t len) {
    struct entry *node = match_fd(fd);
    if (!node) {
        return IIC_ERR;
    }

    uint16_t flag                           = (node->config.tenBit ? I2C_M_TEN : 0 ) | I2C_M_RD;
    struct i2c_msg ioctl_msg[2]             = {0};
    struct i2c_rdwr_ioctl_data ioctl_data   = {0};
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    struct iic_reverse reverse_internalAddr = {.addr = internalAddr};
#else
    struct iic_reverse reverse_internalAddr = {.addr = htonl(internalAddr)};
#endif

    if (node->config.internalAddrBytes) {

        ioctl_msg[0].len    = node->config.internalAddrBytes;
        ioctl_msg[0].addr	= deviceAddr;
        ioctl_msg[0].flags	= 0;
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        ioctl_msg[0].buf    = reverse_internalAddr.msg;
#else
        ioctl_msg[0].buf    = reverse_internalAddr.msg + (IIC_IDDR_LEN - node->config.internalAddrBytes);
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

    return ioctl(fd, I2C_RDWR, &ioctl_data);
}

int iic_write_ioctl(const int fd,
                    const uint16_t deviceAddr,
                    const uint32_t internalAddr,
                    uint8_t *buf,
                    const uint16_t len) {

    int ret = IIC_ERR;
    struct entry *node = match_fd(fd);

    if (!node) {
        return ret;
    }

    uint8_t tx[IIC_PAGE_MAX];
    uint32_t iAddr                          = internalAddr;
    struct i2c_msg ioctl_msg                = {0};
    struct i2c_rdwr_ioctl_data ioctl_data   = {0};
    struct iic_reverse reverse_internalAddr = {0};
    uint16_t flag = node->config.tenBit ? I2C_M_TEN : 0, start = 0, remain = len, wSize;

    do {
        //update internal address
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        reverse_internalAddr.addr           = iAddr;
        memcpy(tx, reverse_internalAddr.msg, node->config.internalAddrBytes);
#else
        reverse_internalAddr.addr           = htonl(iAddr);
        memcpy(tx, reverse_internalAddr.msg + (IIC_IDDR_LEN - node->config.internalAddrBytes), node->config.internalAddrBytes);
#endif

//        wSize = node->config.pageBytes ?
//                ((iAddr % node->config.pageBytes + remain > node->config.pageBytes)
//                ? node->config.pageBytes : (node->config.pageBytes - (iAddr % node->config.pageBytes))) : remain;
        wSize = IIC_GET_WRITE_SIZE(node->config.pageBytes, iAddr, remain);

        memcpy(tx + node->config.internalAddrBytes, buf + start, wSize);

        ioctl_msg.len	                    = node->config.internalAddrBytes + wSize;
        ioctl_msg.addr	                    = deviceAddr;
        ioctl_msg.buf	                    = tx;
        ioctl_msg.flags	                    = flag;

        ioctl_data.nmsgs                    = 1;
        ioctl_data.msgs	                    = &ioctl_msg;

        ret = ioctl(fd, I2C_RDWR, &ioctl_data);
        if (ret == -1) {
            return -errno;
        }

        start   += wSize;
        remain  -= wSize;
        iAddr   += wSize;
    } while (remain);

    return ret;
}


int iic_close(const int fd) {
    int ret = IIC_ERR;
    struct entry *node = match_fd(fd);

    if (node) {
        ret = close(fd);
        if (ret == -1) {
            return -errno;
        }
        iic_free(node);
    }
    return ret;
}

void iic_debug() {
    struct entry *node = LIST_FIRST(&list_head);
    while (node) {
        printf("dev:%s fd:%d pageBytes:%d internalAddrBytes:%d tenBit:%d delayUs:%d\n",
               node->dev,
               node->fd,
               node->config.pageBytes,
               node->config.internalAddrBytes,
               node->config.tenBit,
               node->config.delayUs);
        node = LIST_NEXT(node, entries);
    };
}