#include "math.h"

#define abstract_min(a, b) (a < b ? a : b)
#define abstract_max(a, b) (a > b ? a : b)

float min(float a, float b)
{
	return abstract_min(a, b);
}

double min(double a, double b)
{
	return abstract_min(a, b);
}

u8 min(u8 a, u8 b)
{
	return abstract_min(a, b);
}

u16 min(u16 a, u16 b)
{
	return abstract_min(a, b);
}

u32 min(u32 a, u32 b)
{
	return abstract_min(a, b);
}

u64 min(u64 a, u64 b)
{
	return abstract_min(a, b);
}

float max(float a, float b)
{
	return abstract_max(a, b);
}

double max(double a, double b)
{
	return abstract_max(a, b);
}

u8 max(u8 a, u8 b)
{
	return abstract_max(a, b);
}

u16 max(u16 a, u16 b)
{
	return abstract_max(a, b);
}

u32 max(u32 a, u32 b)
{
	return abstract_max(a, b);
}

u64 max(u64 a, u64 b)
{
	return abstract_max(a, b);
}

// @Source: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
u32 math_u32_next_power_of_2(u32 v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}