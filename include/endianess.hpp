//
// Copyright (C) 2020 ActiveVideo Inc. Reproduction in whole or in part
// without written permission is prohibited. All rights reserved.
//
#pragma once

#include <stdint.h>
#include <stddef.h> 

inline uint32_t get16le(const uint8_t *data)
{
    return (uint32_t(data[0]) << 0)
            | (uint32_t(data[1]) << 8);
}

inline uint32_t get24le(const uint8_t *data)
{
    return (uint32_t(data[0]) <<  0)
            | (uint32_t(data[1]) <<  8)
            | (uint32_t(data[2]) << 16);
}
