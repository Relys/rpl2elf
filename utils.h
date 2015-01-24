// Copyright (C) 2015       Hykem <hykem@hotmail.com>
// Licensed under the terms of the GNU GPL, version 3
// http://www.gnu.org/licenses/gpl-3.0.txt

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zlib.h"

typedef unsigned long long u64;
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef long sint64_t;
typedef int sint32_t;
typedef short sint16_t;
typedef char sint8_t;

typedef u64 uint64_t;
typedef u32 uint32_t;
typedef u16 uint16_t;
typedef u8 uint8_t;

u8 be8(u8 *p);
u16 be16(u8 *p);
u32 be32(u8 *p);
u64 be64(u8 *p);

u16 se16(u16 i);
u32 se32(u32 i);
u64 se64(u64 i);

int inflate_data(u8 *in, u32 in_len, u8 *out, u32 out_len);