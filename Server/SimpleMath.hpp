#pragma once

inline const short bitwise_abs_short(const short x)noexcept
{
	const short mask = x >> (sizeof(short) * 8 - 1);
	return (x + mask) ^ mask;
}

inline const float bitwise_absf(float num)noexcept
{
	int& numAsInt = reinterpret_cast<int&>(num);
	numAsInt &= 0x7FFFFFFF;
	return num;
}

inline const int bitwise_absi(const int num)noexcept
{
	const int mask = num >> (sizeof(int) * 8 - 1);
	return (num + mask) ^ mask;
}

inline const float Q_rsqrt(const float number)noexcept
{
	long i;
	float x2, y;
	constexpr const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long*)&y;                    
	i = 0x5f3759df - (i >> 1);            
	y = *(float*)&i;
	y = y * (threehalfs - (x2 * y * y)); 
	
	return y;
}

inline const bool IsFloatZero(const float number)noexcept
{
	return (number >= -FLT_EPSILON && number <= FLT_EPSILON);
}

inline const float Q_sqrt(const float number)noexcept
{
	const float invSqrt = Q_rsqrt(number);
	return IsFloatZero(number) ? 0.f : 1.0f / invSqrt;
}