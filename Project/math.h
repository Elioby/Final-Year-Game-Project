#pragma once

#include "general.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

float min(float a, float b);
double min(double a, double b);

u8 min(u8 a, u8 b);
u16 min(u16 a, u16 b);
u32 min(u32 a, u32 b);
u64 min(u64 a, u64 b);

float max(float a, float b);
double max(double a, double b);

u8 max(u8 a, u8 b);
u16 max(u16 a, u16 b);
u32 max(u32 a, u32 b);
u64 max(u64 a, u64 b);