#ifndef _UTILITY_FUNC_H
#define _UTILITY_FUNC_H

#include <string>

inline size_t make_upper_four_multiply(size_t number)
{
	//number % 4 != 0
	if(number & 0x00000003) return ((number >> 2) + 1) << 2;

	//number is the multiply of 4, then just get the number itself
	return number;
}

inline size_t make_less_four_multiply(size_t number) 
{
	//number % 4 != 0
	if(number & 0x00000003) return ((number >> 2) << 2);

	//number is the multiply of 4, then just get the number itself
	return number;
}

inline size_t get_least_order_number(size_t number)
{
	while(number & (number - 1)) {
		number &= (number - 1);
	}
	int count = 0;
	while(number) {
		++count;
		number >>= 1;
	}

	return (count < 1) ? 0 : (count - 1);
}

#endif