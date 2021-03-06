#include <stdio.h>
#include <stdarg.h>
#include "hd44780.h"
#include "delay.h"
#include "cmsis_os.h"

extern TIM_HandleTypeDef LCD_VEE_TIM;

void lcd_print(char* _string, ...)
{
	char _str[255];
	va_list ap;
	va_start(ap, _string);
	
	vsprintf(_str, _string, ap);

	lcd_puts(_str);
	va_end(ap);
}



#define LCD_STROBE() HAL_GPIO_WritePin(LCD_EN_PORT,LCD_EN_PIN,GPIO_PIN_SET),HAL_GPIO_WritePin(LCD_EN_PORT,LCD_EN_PIN,GPIO_PIN_RESET)

static volatile uint8_t lcd_led_enabled = 0;
void lcd_led_on(void)
{
	if(!lcd_led_enabled)
	{
		lcd_led_enabled = 1;
		HAL_GPIO_WritePin(LCD_LED_PORT, LCD_LED_PIN, GPIO_PIN_SET);
	}
}
void lcd_led_off(void)
{
	if(lcd_led_enabled)
	{
		lcd_led_enabled = 0;
		HAL_GPIO_WritePin(LCD_LED_PORT, LCD_LED_PIN, GPIO_PIN_RESET);
	}
}

void hd44780_msp_init(void)
{
	/*
	GPIO_InitTypeDef LCD_GPIO_Struct;
	LCD_GPIO_Struct.Pull = GPIO_NOPULL;

	__GPIOC_CLK_ENABLE();
	__GPIOF_CLK_ENABLE();

	LCD_GPIO_Struct.Mode = GPIO_MODE_OUTPUT_PP;
	LCD_GPIO_Struct.Speed = GPIO_SPEED_HIGH;

	LCD_GPIO_Struct.Pin = LCD_EN_PIN;
	HAL_GPIO_Init(LCD_EN_PORT, &LCD_GPIO_Struct);

	LCD_GPIO_Struct.Pin = LCD_RS_PIN;
	HAL_GPIO_Init(LCD_RS_PORT, &LCD_GPIO_Struct);

	LCD_GPIO_Struct.Pin = LCD_4b_PIN;
	HAL_GPIO_Init(LCD_4b_PORT, &LCD_GPIO_Struct);

	LCD_GPIO_Struct.Pin = LCD_5b_PIN;
	HAL_GPIO_Init(LCD_5b_PORT, &LCD_GPIO_Struct);

	LCD_GPIO_Struct.Pin = LCD_6b_PIN;
	HAL_GPIO_Init(LCD_6b_PORT, &LCD_GPIO_Struct);

	LCD_GPIO_Struct.Pin = LCD_7b_PIN;
	HAL_GPIO_Init(LCD_7b_PORT, &LCD_GPIO_Struct);

	LCD_GPIO_Struct.Mode = GPIO_MODE_OUTPUT_OD;
	LCD_GPIO_Struct.Speed = GPIO_SPEED_LOW;
	LCD_GPIO_Struct.Pin = LCD_LED_PIN;
	HAL_GPIO_Init(LCD_LED_PORT, &LCD_GPIO_Struct);
	*/
}


void lcd_write(unsigned char c)
{
	uint32_t uxSavedInterruptStatus = 0;
	DelayUs(50);
	
	if(__get_IPSR() != 0) uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	else taskENTER_CRITICAL();
	if (testbit(c,4)) HAL_GPIO_WritePin(LCD_4b_PORT, LCD_4b_PIN, GPIO_PIN_SET); else HAL_GPIO_WritePin(LCD_4b_PORT, LCD_4b_PIN, GPIO_PIN_RESET);
	if (testbit(c,5)) HAL_GPIO_WritePin(LCD_5b_PORT, LCD_5b_PIN, GPIO_PIN_SET); else HAL_GPIO_WritePin(LCD_5b_PORT, LCD_5b_PIN, GPIO_PIN_RESET);
	if (testbit(c,6)) HAL_GPIO_WritePin(LCD_6b_PORT, LCD_6b_PIN, GPIO_PIN_SET); else HAL_GPIO_WritePin(LCD_6b_PORT, LCD_6b_PIN, GPIO_PIN_RESET);
	if (testbit(c,7)) HAL_GPIO_WritePin(LCD_7b_PORT, LCD_7b_PIN, GPIO_PIN_SET); else HAL_GPIO_WritePin(LCD_7b_PORT, LCD_7b_PIN, GPIO_PIN_RESET);
	LCD_STROBE();
	if (testbit(c,0)) HAL_GPIO_WritePin(LCD_4b_PORT, LCD_4b_PIN, GPIO_PIN_SET); else HAL_GPIO_WritePin(LCD_4b_PORT, LCD_4b_PIN, GPIO_PIN_RESET);
	if (testbit(c,1)) HAL_GPIO_WritePin(LCD_5b_PORT, LCD_5b_PIN, GPIO_PIN_SET); else HAL_GPIO_WritePin(LCD_5b_PORT, LCD_5b_PIN, GPIO_PIN_RESET);
	if (testbit(c,2)) HAL_GPIO_WritePin(LCD_6b_PORT, LCD_6b_PIN, GPIO_PIN_SET); else HAL_GPIO_WritePin(LCD_6b_PORT, LCD_6b_PIN, GPIO_PIN_RESET);
	if (testbit(c,3)) HAL_GPIO_WritePin(LCD_7b_PORT, LCD_7b_PIN, GPIO_PIN_SET); else HAL_GPIO_WritePin(LCD_7b_PORT, LCD_7b_PIN, GPIO_PIN_RESET);
	LCD_STROBE();
	if(__get_IPSR() != 0) taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	else taskEXIT_CRITICAL();
 
}

 
void lcd_clear(void)
{
	HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
	lcd_write(0x1);
	DelayMs(2);
}
 
 
void lcd_puts(const char * s)
{
	HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET);
	while(*s)
	lcd_write(lcd_decode_ansii(*s++));
}
 
 
void lcd_putch(char c)
{
	HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET);
	lcd_write(c);
}
 
void lcd_goto(unsigned char pos)
{
	HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
	lcd_write(0x80+pos);
}

void lcd_cursor(unsigned char cnf)
{
	HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
	lcd_write(cnf);
}

#define BIN(n) \
(\
((n >> 21) & 0x80) | \
((n >> 18) & 0x40) | \
((n >> 15) & 0x20) | \
((n >> 12) & 0x10) | \
((n >> 9) & 0x08) | \
((n >> 6) & 0x04) | \
((n >> 3) & 0x02) | \
((n ) & 0x01) \
) 


static const char lcd_userchars[]=
{
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),

		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
		BIN(1011111),
	0x00
};

void lcd_enable(void)
{
	HAL_TIM_PWM_Start(&LCD_VEE_TIM, TIM_CHANNEL_1);
}

void lcd_disable(void)
{
	HAL_TIM_PWM_Stop(&LCD_VEE_TIM, TIM_CHANNEL_1);
}

void lcd_init(void)
{	 
	hd44780_msp_init();
	lcd_led_off();
	lcd_disable();
	HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_EN_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
	DelayMs(15);
	HAL_GPIO_WritePin(LCD_4b_PORT, LCD_4b_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_5b_PORT, LCD_5b_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_6b_PORT, LCD_6b_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_7b_PORT, LCD_7b_PIN, GPIO_PIN_RESET);
	LCD_STROBE();
	DelayMs(5);
	LCD_STROBE();
	DelayUs(200);
	LCD_STROBE();
	DelayUs(200);
	HAL_GPIO_WritePin(LCD_4b_PORT, LCD_4b_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_5b_PORT, LCD_5b_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_6b_PORT, LCD_6b_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_7b_PORT, LCD_7b_PIN, GPIO_PIN_RESET);
	LCD_STROBE();
	lcd_write(0x28);
	lcd_write(0xC);
	lcd_clear();
	lcd_write(0x4);

	lcd_clear();
	lcd_write(0x40);
	lcd_puts(lcd_userchars);
	
	lcd_clear();
	lcd_clear();
}

uint8_t lcd_decode_ansii(uint8_t ansii)
{
	uint8_t res = ansii;
	if(ansii >= 0xC0 && ansii <= 0xFF)
	{
		res = '*';
		
		switch(ansii)
		{
			case 0xC0 : res = 0x41; break;
			case 0xC1 : res = 0xA0; break;
			case 0xC2 : res = 0x42; break;
			case 0xC3 : res = 0xA1; break;
			case 0xC4 : res = 0xE0; break;
			case 0xC5 : res = 0x45; break;
			case 0xC6 : res = 0xA3; break;
			case 0xC7 : res = 0xA4; break;
			case 0xC8 : res = 0xA5; break;
			case 0xC9 : res = 0xA6; break;
			case 0xCA : res = 0x4B; break;
			case 0xCB : res = 0xA7; break;
			case 0xCC : res = 0x4D; break;
			case 0xCD : res = 0x48; break;
			case 0xCE : res = 0x4F; break;
			case 0xCF : res = 0xA8; break;
			case 0xD0 : res = 0x50; break;
			case 0xD1 : res = 0x43; break;
			case 0xD2 : res = 0x54; break;
			case 0xD3 : res = 0xA9; break;
			case 0xD4 : res = 0xAA; break;
			case 0xD5 : res = 0x58; break;
			case 0xD6 : res = 0xE1; break;
			case 0xD7 : res = 0xAB; break;
			case 0xD8 : res = 0xAC; break;
			case 0xD9 : res = 0xE2; break;
			case 0xDA : res = 0xAD; break;
			case 0xDB : res = 0xAE; break;
			case 0xDC : res = 0xC4; break;
			case 0xDD : res = 0xAF; break;
			case 0xDE : res = 0xB0; break;
			case 0xDF : res = 0xB1; break;
			case 0xE0 : res = 0x61; break;
			case 0xE1 : res = 0xB2; break;
			case 0xE2 : res = 0xB3; break;
			case 0xE3 : res = 0xB4; break;
			case 0xE4 : res = 0xE3; break;
			case 0xE5 : res = 0x65; break;
			case 0xE6 : res = 0xB6; break;
			case 0xE7 : res = 0xB7; break;
			case 0xE8 : res = 0xB8; break;
			case 0xE9 : res = 0xB9; break;
			case 0xEA : res = 0xBA; break;
			case 0xEB : res = 0xBB; break;
			case 0xEC : res = 0xBC; break;
			case 0xED : res = 0xBD; break;
			case 0xEE : res = 0x6F; break;
			case 0xEF : res = 0xBE; break;
			case 0xF0 : res = 0x70; break;
			case 0xF1 : res = 0x63; break;
			case 0xF2 : res = 0xBF; break;
			case 0xF3 : res = 0x79; break;
			case 0xF4 : res = 0xE4; break;
			case 0xF5 : res = 0x78; break;
			case 0xF6 : res = 0xE5; break;
			case 0xF7 : res = 0xC0; break;
			case 0xF8 : res = 0xC1; break;
			case 0xF9 : res = 0xE6; break;
			case 0xFA : res = 0xC2; break;
			case 0xFB : res = 0xC3; break;
			case 0xFC : res = 0xC4; break;
			case 0xFD : res = 0xC5; break;
			case 0xFE : res = 0xC6; break;
			case 0xFF : res = 0xC7; break;
		}
		
	}
	return res;
}
