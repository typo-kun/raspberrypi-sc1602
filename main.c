#include<stdio.h>
#include<stdlib.h>

#include "gpio.h"


int main(void)
{
	struct tm *s_time;
	time_t the_time;
	int ret;

	ret = setup_io();
	if (ret < 0) {
		printf("%d\n", ret);
		return;
	}

	init_lcd();
	init_switch();
	init_led();

	time(&the_time);
	s_time = (struct tm *)localtime(&the_time);

	LCD_Write_auto(asctime(s_time));
	printf("%s\n", asctime(s_time));

	while(1) {
		time(&the_time);
		s_time = (struct tm*)localtime(&the_time);
		LCD_Write_auto(asctime(s_time));
		if (get_switch()) {
			LED_On();
		} else {
			LED_Off();
		}
		usleep(100);
	}
	return 0;
}


