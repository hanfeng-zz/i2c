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

#define IIC_IDDR_LEN sizeof(uint32_t)

struct iic_reverse {
    union {
        uint32_t addr;
        uint8_t msg[IIC_IDDR_LEN];
    };
};

struct entry {
    int fd;
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
        return fd;
    }

    struct entry *node = (struct entry *)calloc(1, sizeof(struct entry));
    if (NULL == node) {
        close(fd);
        return -2;
    }

    node->fd        = fd;
    node->config    = config;

    LIST_INSERT_HEAD(&list_head, node, entries);
    return fd;
}

int iic_read(const int fd, const uint16_t deviceAddr, const uint32_t internalAddr, uint8_t *buf, const uint16_t len) {

    int ret = -2;
    struct entry *node = match_fd(fd);

    if (node) {

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

        ret = ioctl(fd, I2C_RDWR, &ioctl_data);
    }

    return ret;
}

int iic_write(const int fd, const uint16_t deviceAddr, const uint8_t *internalAddr, uint8_t *buf, const uint16_t len) {
    int ret = -2;
    struct entry *node = match_fd(fd);

    if (node) {

        uint16_t flag                           = node->config.tenBit ? I2C_M_TEN : 0;
        struct i2c_msg ioctl_msg[2]             = {0};
        struct i2c_rdwr_ioctl_data ioctl_data   = {0};

        if (node->config.internalAddrBytes) {
            //todo
            ret = -3;
        } else {
            ioctl_msg[0].len    = len;
            ioctl_msg[0].addr	= deviceAddr;
            ioctl_msg[0].buf    = buf;
            ioctl_msg[0].flags	= flag;

            ioctl_data.nmsgs	= 1;
            ioctl_data.msgs		= ioctl_msg;

            ret = ioctl(fd, I2C_RDWR, &ioctl_data);
        }
    }

    return ret;
}


int iic_close(const int fd) {
    int ret = -2;
    struct entry *node = match_fd(fd);

    if (node) {
        ret = close(fd);
        iic_free(node);
    }
    return ret;
}