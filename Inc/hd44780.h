#include "main.h"
 
#define	LCD_LED_PORT LCD_LED_GPIO_Port
#define	LCD_RS_PORT LCD_RS_GPIO_Port
#define	LCD_EN_PORT LCD_EN_GPIO_Port
#define	LCD_4b_PORT LCD_D4_GPIO_Port
#define	LCD_5b_PORT LCD_D5_GPIO_Port
#define	LCD_6b_PORT LCD_D6_GPIO_Port
#define	LCD_7b_PORT LCD_D7_GPIO_Port

#define	LCD_LED_PIN LCD_LED_Pin
#define	LCD_RS_PIN LCD_RS_Pin
#define	LCD_EN_PIN LCD_EN_Pin
#define	LCD_4b_PIN LCD_D4_Pin
#define	LCD_5b_PIN LCD_D5_Pin
#define	LCD_6b_PIN LCD_D6_Pin
#define	LCD_7b_PIN LCD_D7_Pin

#define LCD_VEE_TIM htim3

#define LCD_CURSOR_OFF 0x0C
#define LCD_CURSOR_ON 0x0E
#define LCD_CURSOR_BLINK 0x0F

#define LCD_STRING_LENGTH 16


#define testbit(data,bitno) ((data>>bitno)&0x01)

extern void lcd_write(unsigned char c);
extern void lcd_clear(void);
extern void lcd_puts(const char * s);
extern void lcd_putch(char c);
extern void lcd_print(char* _string, ...);
extern void lcd_goto(unsigned char pos);
extern void lcd_cursor(unsigned char cnf);
extern void lcd_init(void);
extern void lcd_led_on(void);
extern void lcd_led_off(void);
extern void lcd_enable(void);
extern void lcd_disable(void);
extern uint8_t lcd_decode_ansii(uint8_t ansii);

