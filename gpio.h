
void init_switch(void);
int  get_switch(void);
void init_led(void);
void led_ctl(int on);
void init_lcd(void);
void write_str(char line, char *str);
int  setup_io(void);


#define LED_On()					led_ctl(1);
#define LED_Off()					led_ctl(0);
#define LCD_Write_1st(_str_)		write_str(1, _str_);
#define LCD_Write_2nd(_str_)		write_str(2, _str_);
