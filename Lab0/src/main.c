/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
			

int main(void)
{
	int x = 0;
	int y = 0;

	for(;;)
	{
		x += 1;
		y -= 1;
	}
}
