#ifndef _UTILITY_FUNC_H
#define _UTILITY_FUNC_H

size_t get_least_order_number(size_t number)
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