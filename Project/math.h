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

#define EPSILON 0.001f
#define PI 3.14159

float min(float a, float b);
double min(double a, double b);

u8 min(u8 a, u8 b);
u16 min(u16 a, u16 b);
u32 min(u32 a, u32 b);
u64 min(u64 a, u64 b);

s8 min(s8 a, s8 b);
s16 min(s16 a, s16 b);
s32 min(s32 a, s32 b);
s64 min(s64 a, s64 b);

float max(float a, float b);
double max(double a, double b);

u8 max(u8 a, u8 b);
u16 max(u16 a, u16 b);
u32 max(u32 a, u32 b);
u64 max(u64 a, u64 b);

s8 max(s8 a, s8 b);
s16 max(s16 a, s16 b);
s32 max(s32 a, s32 b);
s64 max(s64 a, s64 b);

u32 math_u32_next_power_of_2(u32 v);