// Copyright (C) 2015       Hykem <hykem@hotmail.com>
// Licensed under the terms of the GNU GPL, version 3
// http://www.gnu.org/licenses/gpl-3.0.txt

#include "utils.h"

// Auxiliary functions.
u8 be8(u8 *p)
{
	return *p;
}

u16 be16(u8 *p)
{
	u16 a;

	a  = p[0] << 8;
	a |= p[1];

	return a;
}

u32 be32(u8 *p)
{
	u32 a;

	a  = p[0] << 24;
	a |= p[1] << 16;
	a |= p[2] <<  8;
	a |= p[3] <<  0;

	return a;
}

u64 be64(u8 *p)
{
	u32 a, b;

	a = be32(p);
	b = be32(p + 4);

	return ((u64)a<<32) | b;
}

u16 se16(u16 i)
{
	return (((i & 0xFF00) >> 8) | ((i & 0xFF) << 8));
}

u32 se32(u32 i)
{
	return ((i & 0xFF000000) >> 24) | ((i & 0xFF0000) >>  8) | ((i & 0xFF00) <<  8) | ((i & 0xFF) << 24);
}

u64 se64(u64 i)
{
	return ((i & 0x00000000000000FF) << 56) | ((i & 0x000000000000FF00) << 40) |
		((i & 0x0000000000FF0000) << 24) | ((i & 0x00000000FF000000) <<  8) |
		((i & 0x000000FF00000000) >>  8) | ((i & 0x0000FF0000000000) >> 24) |
		((i & 0x00FF000000000000) >> 40) | ((i & 0xFF00000000000000) >> 56);
}

// ZLIB inflate.
int inflate_data(u8 *in, u32 in_len, u8 *out, u32 out_len)
{
	int ret = 0;
	
	u8 *tmp = malloc (out_len * sizeof(u8));
	if (tmp == NULL)
	{
		printf("Could not allocate memory!");
		return -1;
	}

	z_stream s;
	memset(&s, 0, sizeof(s));

	s.zalloc = Z_NULL;
	s.zfree = Z_NULL;
	s.opaque = Z_NULL;

	ret = inflateInit(&s);
	if (ret != Z_OK)
	{
		printf("Inflate failed: %d", ret);
		return ret;
	}
		
	s.avail_in = in_len;
	s.next_in = in;

	s.avail_out = out_len;
	s.next_out = tmp;

	ret = inflate(&s, Z_FINISH);
	if (ret != Z_OK && ret != Z_STREAM_END)
	{
		printf("Inflate failed: %d", ret);
		return ret;
	}
		
	inflateEnd(&s);
	memcpy(out, tmp, out_len);
	free(tmp);
		
	return ret;
}