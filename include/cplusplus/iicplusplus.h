/*
 * Copyright (C) 2025-09-23  zheng zhao
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v3.0 or later.
 * See <https://www.gnu.org/licenses/>.
 */


#ifndef IIC_PLUSPLUS_H
#define IIC_PLUSPLUS_H


#include "iiccommon.h"

#include <iostream>

class Iic {
public:
    Iic(const Iic &) = delete;

    Iic &operator=(const Iic&) = delete;

    Iic(const Iic &&) = delete;

    Iic &operator=(const Iic&&) = delete;

    Iic() = default;

    virtual ~Iic();

    int open(const std::string &device, const struct iic_config &config);

    int read_ioctl(const uint16_t deviceAddr,
                   const uint32_t internalAddr,
                   uint8_t *buf,
                   const uint16_t len);

    int write_ioctl(const uint16_t deviceAddr,
                    const uint32_t internalAddr,
                    uint8_t *buf,
                    const uint16_t len);

    int closed();

    void debug();
private:
    struct iic_config _config {};

    int _fd = -1;

    std::string _dev {};
};

#endif //IIC_PLUSPLUS_H
