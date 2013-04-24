#include<stdio.h>
#include<stdlib.h>

#include "gpio.h"


int main(void)
{
	int ret;

	ret = setup_io();
	if (ret < 0) {
		printf("%d\n", ret);
		return;
	}

	init_lcd();
	init_switch();
	init_led();

	LCD_Write_1st("Hello,");
	LCD_Write_2nd("World!");

	while(1) {
		if (get_switch()) {
			LED_On();
		} else {
			LED_Off();
		}
		usleep(100);
	}
	return 0;
}


