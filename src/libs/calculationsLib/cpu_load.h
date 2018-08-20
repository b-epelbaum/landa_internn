#pragma once

static void cpu_load_fibbonacci(int num)
{
	volatile int c = 0;
	volatile int first = 0;
	volatile int second = 1;
	volatile int next = 0;

	for (c = 0; c < num; c++)
	{
		if (c <= 1)
			next = c;
		else
		{
			next = first + second;
			first = second;
			second = next;
		}
	}
}
