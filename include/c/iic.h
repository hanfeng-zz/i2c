/*
 * Copyright (C) 2025-09-23  zheng zhao
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v3.0 or later.
 * See <https://www.gnu.org/licenses/>.
 */


#ifndef IIC_H
#define IIC_H

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "iiccommon.h"
/*
 * Function: iic_open
 * Description: Support for repeated open, and the information is stored global(struct list_node list_head;)
 * Return: if success, return file descriptor, otherwise return (-errno),
 *         used to indicate open faild or apply for memory failed.
 */
int iic_open(const char *const device,
             const struct iic_config config);

int iic_read_ioctl(const int fd,
                   const uint16_t deviceAddr,
                   const uint32_t internalAddr,
                   uint8_t *buf,
                   const uint16_t len);

int iic_write_ioctl(const int fd,
                    const uint16_t deviceAddr,
                    const uint32_t internalAddr,
                    uint8_t *buf,
                    const uint16_t len);

int iic_close(const int fd);

void iic_debug();
#ifdef __cplusplus
}
#endif
#endif //IIC_H
