#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include "gpio.h"

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE	(4*1024)
#define BLOCK_SIZE	(4*1024)

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
#define GPIO_GET *(gpio+13) // read   bits

#define GPIO_PIN_SWITCH		(4)
#define GPIO_PIN_LED		(17)
#define GPIO_PIN_LCD_RS		(11)
#define GPIO_PIN_LCD_E		(22)
#define GPIO_PIN_LCD_DB4	(7)
#define GPIO_PIN_LCD_DB5	(8)
#define GPIO_PIN_LCD_DB6	(9)
#define GPIO_PIN_LCD_DB7	(10)

#define DR_2LINEMODE	(0x28)
#define DR_DISP_OFF		(0x08)
#define DR_DISP_ON		(0x0C)
#define DR_DISP_CLR		(0x01)
#define DR_ENTRY_SHIFT	(0x04)
#define DR_ENTRY_INC	(0x06)
#define DR_POS_1ST		(0x80)
#define DR_POS_2ND		(0xC0)
#define DR_CUR_R		(0x14)


int  mem_fd;
char *gpio_mem, *gpio_map;
volatile unsigned *gpio;

void init_switch(void);
int get_switch(void);
void init_led(void);
void led_ctl(int on);
void init_lcd(void);
static void set_ctl(unsigned char value);
static void write_char(char value);
void write_str(char line, char *str);
static void setup_lcd_4bit(void);
static void setup_lcd_8bit(void);
int setup_io(void);

#define enable_reg()	\
{						\
	GPIO_SET = 1 << 22;	\
	usleep(20);			\
	GPIO_CLR = 1 << 22;	\
}

void init_switch(void)
{
	OUT_GPIO(GPIO_PIN_LED);
	INP_GPIO(GPIO_PIN_LED);
	usleep(1);
}

int get_switch(void)
{
	return GPIO_GET & (0x01 << GPIO_PIN_SWITCH);
}

void init_led(void)
{
	INP_GPIO(GPIO_PIN_LED);
	OUT_GPIO(GPIO_PIN_LED);
	usleep(1);
}

void led_ctl(int on)
{
	if(on > 0) {
		GPIO_SET = 1 << 17;
	} else {
		GPIO_CLR = 1 << 17;
	}
}

void init_lcd(void)
{
	INP_GPIO(GPIO_PIN_LCD_RS);
	OUT_GPIO(GPIO_PIN_LCD_RS);

	INP_GPIO(GPIO_PIN_LCD_E);
	OUT_GPIO(GPIO_PIN_LCD_E);

	INP_GPIO(GPIO_PIN_LCD_DB4);
	OUT_GPIO(GPIO_PIN_LCD_DB4);

	INP_GPIO(GPIO_PIN_LCD_DB5);
	OUT_GPIO(GPIO_PIN_LCD_DB5);

	INP_GPIO(GPIO_PIN_LCD_DB6);
	OUT_GPIO(GPIO_PIN_LCD_DB6);

	INP_GPIO(GPIO_PIN_LCD_DB7);
	OUT_GPIO(GPIO_PIN_LCD_DB7);

	usleep(1);

	setup_lcd_4bit();

	// 2gyo-mode
	set_ctl(DR_2LINEMODE);
	usleep(50);

	// dispOff, CurOff, BlinkOff
	set_ctl(DR_DISP_OFF);
	usleep(50);

	// display clear
	set_ctl(DR_DISP_CLR);
	usleep(3*1000);

	//entrymode setting
	set_ctl(DR_ENTRY_INC | DR_ENTRY_SHIFT);
	usleep(50);
	
	set_ctl(DR_CUR_R);
	usleep(50);

	//display on
	set_ctl(DR_DISP_ON);
	usleep(50);

}

static void set_ctl(unsigned char value)
{
	GPIO_CLR = 0x10 << 7; // rs:0(ctl mode)
	usleep(1);

	GPIO_CLR = 0x0F << 7;
	GPIO_SET = (value >> 4) << 7;
	enable_reg();

	GPIO_CLR = 0x0F << 7;
	GPIO_SET = (value & 0x0F) << 7;
	enable_reg();

	usleep(50);
//	printf("value : %X%X\n", (value >> 4), (value & 0x0F));
}

static void write_char(char value)
{
	GPIO_SET = 0x10 << 7; // rs:1(char mode)
	usleep(1);

	GPIO_CLR = 0x0F << 7;
	GPIO_SET = (value >> 4) << 7;
	enable_reg();

	GPIO_CLR = 0x0F << 7;
	GPIO_SET = (value & 0x0F) << 7;
	enable_reg();

	usleep(50);
//	printf("value : %X%X\n", (value >> 4), (value & 0x0F));
}

void write_str(char line, char *str)
{
	int i;
	if (line==1) {
		set_ctl(DR_POS_1ST);
	} else {
		set_ctl(DR_POS_2ND);
	}
	usleep(50);

	for (i=0; i<16; i++) {
		if (*str != '\0') {
			write_char(*str);
			str++;
		} else {
			write_char(' ');
		}
	}
}

static void setup_lcd_4bit(void)
{
	GPIO_CLR = 0x01 << 22;	// enable bit off
	GPIO_CLR = 0x10 << 7;	// register select:control

	usleep(15*1000);
	setup_lcd_8bit();
	usleep(5*1000);
	setup_lcd_8bit();
	usleep(200);
	setup_lcd_8bit();
	usleep(80);

	GPIO_CLR = 0x0F << 7;
	GPIO_SET = 0x02 << 7;
	enable_reg();

	usleep(80);
}

static void setup_lcd_8bit(void)
{
	GPIO_CLR = 0x0F << 7;
//	enable_reg();
	GPIO_SET = 0x03 << 7;
	enable_reg();
}

int setup_io(void)
{
	gpio_mem = malloc(BLOCK_SIZE + (PAGE_SIZE -1));
	if (gpio_mem == NULL) {
		return -1;
	}

	// メモリ境界へ移動
	if ((unsigned long)gpio_mem % PAGE_SIZE) {
		gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);
	}

	mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
	if ((long)mem_fd < 0) {
		printf("mem_fd %d", mem_fd);
		return -2;
	}

	gpio_map = (char *)mmap((caddr_t)gpio_mem, 
							 BLOCK_SIZE, 
							 PROT_READ|PROT_WRITE, 
							 MAP_SHARED|MAP_FIXED, 
							 mem_fd, GPIO_BASE);
	if ((long)gpio_map < 0) {
		return -3;
	}

	gpio = (volatile unsigned*)gpio_map;
	return 0;
}

