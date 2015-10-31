#include <iostream>

template<typename T>
void swap(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

template<typename T>
void swap2(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

void main()
{
	int a = 3, b  = 4;
	printf("%d, %d \n" , a, b);
	swap(a, b);
	printf("%d, %d \n" , a, b);
	swap2(a, b);
	printf("%d, %d \n" , a, b);
}