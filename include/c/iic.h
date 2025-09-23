// Copyright (c) 2025 Shifei Technologies. All rights reserved.
//
// This file is part of internal projects, unauthorized copying of this file
// via any medium is strictly prohibited.

//
// Created by zheng zhao on 2025/9/16.
//

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

int iic_read(const int fd, const uint16_t deviceAddr, const uint32_t internalAddr, uint8_t *buf, const uint16_t len);

int iic_write(const int fd, const uint16_t deviceAddr, const uint32_t internalAddr, uint8_t *buf, const uint16_t len);

int iic_close(const int fd);

void iic_debug();
#ifdef __cplusplus
}
#endif
#endif //IIC_H
