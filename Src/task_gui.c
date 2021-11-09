#include "task_gui.h"
#include "hd44780.h"
#include "parameters.h"
#include "delay.h"
#include "mixer.h"
#include <math.h>
#include <string.h>

extern TIM_HandleTypeDef htim2;
extern RTC_DateTypeDef sDate;
extern RTC_TimeTypeDef sTime;
osThreadId guiTaskHandle;

static void menu_unsupported(void);
static void menu_changestring(char * str);
static void menu_wificonfig(void);
static void menu_setyesno(uint8_t * val);

#define LCD_STRING_YES "YES"
#define LCD_STRING_NO  "NO "

#define LCD_CONFIG_FROM ' ' 
#define LCD_CONFIG_TO 'z' 
	
typedef enum
{
	STATUS_INVALID = -1,
	STATUS_CLOCK_LEVEL = 0,
	STATUS_VOLUME,
	STATUS_WIFI_PLAY,
	STATUS_BT_CONFIG,
	STATUS_WIFI_CONFIG,
	STATUS_EQ_CONFIG,
	STATUS_CLOCK_CONFIG,
	STATUS_INVALID_LAST
} StatusTypeDef;

StatusTypeDef Status = STATUS_CLOCK_LEVEL;

volatile uint8_t BUT_ENTER = 0;
volatile uint8_t BUT_NEXT = 0;
volatile uint8_t BUT_PREV = 0;
volatile uint8_t BUT_CANCEL = 0;

volatile uint8_t BUT_ENTER_PRESS = 0;
volatile uint8_t BUT_NEXT_PRESS = 0;
volatile uint8_t BUT_PREV_PRESS = 0;
volatile uint8_t BUT_CANCEL_PRESS = 0;

volatile int32_t BUT_ENTER_PRESS_TIME = 0;
volatile int32_t BUT_NEXT_PRESS_TIME = 0;
volatile int32_t BUT_PREV_PRESS_TIME = 0;
volatile int32_t BUT_CANCEL_PRESS_TIME = 0;

volatile uint8_t DAC_Volume = 0;
volatile uint8_t DAC_HP_Volume = 0;

volatile ModeTypeDef MixerMode = MODE_NONE;

static void ButtonsClear(void)
{
	BUT_ENTER = 0;
	BUT_PREV = 0;
	BUT_NEXT = 0;
	BUT_CANCEL = 0;
}

static void StartGuiTask(const void * argument)
{
	StatusTypeDef oldStatus = STATUS_INVALID;
	uint8_t changed = 0;
	Params_LoadConfiguration();
	while(1)
	{
		if(BUT_NEXT)
		{
			if(++Status >= STATUS_INVALID_LAST)
				Status = STATUS_INVALID + 1;
			BUT_NEXT = 0;
		}
		if(BUT_PREV)
		{
			if(--Status <= STATUS_INVALID)
				Status = STATUS_INVALID_LAST - 1;
			BUT_PREV = 0;
		}
		if(BUT_CANCEL)
		{
			BUT_CANCEL = 0;
			Status = STATUS_INVALID + 1;
		}
		
		if(oldStatus != Status)
		{
			changed = 1;
			ButtonsClear();
			lcd_clear();
			oldStatus = Status;
		}
		switch(Status)
		{
			case STATUS_CLOCK_LEVEL :				
				lcd_goto(0);
				lcd_print("%02d:%02d:%02d %d.%02d ", sTime.Hours,sTime.Minutes,sTime.Seconds, sDate.Date,sDate.Month);
				lcd_goto(0x40);
				lcd_puts("Source:");
				switch(MixerMode)
				{
					case MODE_NONE :
						lcd_puts("Idle     ");
						break;
					case MODE_BT :
						lcd_puts("Bluetooth");
						break;
					case MODE_USB :
						lcd_puts("USB Audio");
						break;
					case MODE_WIFI :
						lcd_puts("WiFi     ");
						break;
						
					default:
						lcd_puts("Invalid  ");
						break;
				}
				break;
			
			case STATUS_VOLUME :		
				if(changed)
				{
					lcd_goto(0x00);
					lcd_puts("Volume: ");
					lcd_goto(0x40);
					lcd_puts("HP Vol: ");
				}
				lcd_goto(0x08);
				lcd_print("%4.1fdB  ", (float)-(roundf(115 - (DAC_Volume / 2.21f))) / 2.0f);
				lcd_goto(0x48);
				lcd_print("%4.1fdB  ", (float)-(roundf(115 - (DAC_HP_Volume / 2.21f))) / 2.0f);
				break;
			case STATUS_BT_CONFIG :
				if(changed)
				{
					lcd_goto(0);
					lcd_puts("Bluetooth Config");
					lcd_goto(0x41);
					lcd_puts("Press Enter");
				}
				
				if(BUT_ENTER)
				{
					menu_unsupported();
					oldStatus = STATUS_INVALID;
				}
				
				break;
			case STATUS_WIFI_CONFIG :
				if(changed)
				{
					lcd_goto(0);
					lcd_puts("WiFi Config");
					lcd_goto(0x41);
					lcd_puts("Press Enter");
				}
				
				if(BUT_ENTER)
				{
					menu_wificonfig();
					oldStatus = STATUS_INVALID;
				}
				
				break;
			case STATUS_WIFI_PLAY :
				if(changed)
				{
					lcd_goto(0);
					lcd_puts("WiFi Player");
					lcd_goto(0x40);
					lcd_puts("<List empty>");
				}
				
				if(BUT_ENTER)
				{
					menu_unsupported();
					oldStatus = STATUS_INVALID;
				}
				
				break;
			case STATUS_EQ_CONFIG :
				if(changed)
				{
					lcd_goto(0);
					lcd_puts("Equalizer Config");
					lcd_goto(0x41);
					lcd_puts("Press Enter");
				}
				
				if(BUT_ENTER)
				{
					menu_unsupported();
					oldStatus = STATUS_INVALID;
				}
				
				break;
			case STATUS_CLOCK_CONFIG :
				if(changed)
				{
					lcd_goto(0);
					lcd_puts("Clock Config");
					lcd_goto(0x41);
					lcd_puts("Press Enter");
				}
				
				if(BUT_ENTER)
				{
					menu_unsupported();
					oldStatus = STATUS_INVALID;
				}
				
				break;
			default : 
				break;
		}
		changed = 0;
		osDelay(10);
	}
}

static void menu_unsupported(void)
{
	lcd_goto(0x40);
	lcd_puts("Unsupported now");
	osDelay(1000);
}

static void menu_wificonfig(void)
{
	uint8_t changed = 0;
	int8_t menuitem = 0;
	int8_t oldmenuitem = -1;
	while(1)
	{
		if(BUT_NEXT)
			if(++menuitem > 3) menuitem = 0;
		
		if(BUT_PREV)
			if(--menuitem < 0) menuitem = 3;
		
		if(menuitem != oldmenuitem)
		{
			oldmenuitem = menuitem;
			lcd_clear();
			ButtonsClear();
			changed = 1;
		}
		if(BUT_CANCEL)
		{
			ButtonsClear();
			return;
		}
		switch(menuitem)
		{
			case 0 :
				if(changed)
				{
					lcd_goto(0);
					lcd_print("WiFi Enabled");
					lcd_goto(0x40);
					lcd_print(Parameters.WIFI_Enabled ? LCD_STRING_YES : LCD_STRING_NO);
				}
				if(BUT_ENTER)
				{
					ButtonsClear();
					menu_setyesno(&Parameters.WIFI_Enabled);
					Params_SaveConfiguration();
					oldmenuitem = -1;
				}
				break;
			case 1 :
				if(changed)
				{
					lcd_goto(0);
					lcd_print("WiFi SSID");
					lcd_goto(0x40);
					lcd_print(Parameters.WIFI_SSID);
				}
				if(BUT_ENTER)
				{
					ButtonsClear();
					menu_changestring(Parameters.WIFI_SSID);
					Params_SaveConfiguration();
					oldmenuitem = -1;
				}
				
				break;
			case 2 :
				if(changed)
				{
					lcd_goto(0);
					lcd_print("WiFi Password");
					lcd_goto(0x40);
					lcd_print(Parameters.WIFI_Key);
				}
				if(BUT_ENTER)
				{
					ButtonsClear();
					menu_changestring(Parameters.WIFI_Key);
					Params_SaveConfiguration();
					oldmenuitem = -1;
				}
				
				break;
			case 3 :
				if(changed)
				{
					lcd_goto(0);
					lcd_print("WiFi Links");
					lcd_goto(0x40);
					lcd_print("Press Enter");
				}
				
				break;
		}
		changed = 0;
	}
}

static void menu_setyesno(uint8_t * val)
{
	DelayStartCount(5);
	uint8_t newval = *val;
	uint8_t maxstrlen = strlen(LCD_STRING_YES) > strlen(LCD_STRING_NO) ? strlen(LCD_STRING_YES) : strlen(LCD_STRING_NO);
	
	while(1)
	{
		if(DelayStopCount(5) % 1000000 < 500000)
		{
			lcd_goto(0x40);
			lcd_print(newval ? LCD_STRING_YES : LCD_STRING_NO);
		}
		else
		{
			lcd_goto(0x40);
			for(uint8_t i = 0; i < maxstrlen; i++)
				lcd_putch(' ');
		}
		if(BUT_CANCEL)
		{
			ButtonsClear();
			return;
		}
		if(BUT_ENTER)
		{
			*val = newval;
			ButtonsClear();
			return;
		}
		if(BUT_NEXT || BUT_PREV)
		{
			newval ^= 1;
			ButtonsClear();
			DelayStartCount(5);
		}
	}
}

static void menu_changestring(char * str)
{
	static char buffer[WIFI_LINK_LEN];
	strncpy(buffer, str, WIFI_LINK_LEN-1);
	
	for(uint8_t i = 0; i < WIFI_LINK_LEN - 1; i++)
		if(buffer[i] == 0) buffer[i] = ' ';
	buffer[WIFI_LINK_LEN - 1] = 0;
	
	uint8_t pointer = 0;
	uint8_t lcdpointer = 0;
	uint8_t lcdchar = 0;
	lcd_goto(0x40);
	lcd_print(str);
	lcd_cursor(LCD_CURSOR_ON);
	lcd_goto(0x40);
	
	while(1)
	{
		if(BUT_CANCEL)
		{
			lcd_cursor(LCD_CURSOR_OFF);
			ButtonsClear();
			return;
		}
		if(BUT_ENTER)
		{
			while(BUT_ENTER_PRESS)
			{
				if(BUT_ENTER_PRESS_TIME >= 500)
				{
					lcd_cursor(LCD_CURSOR_OFF);
					for(uint8_t i = 1; i < WIFI_LINK_LEN; i++)
					{
						if(buffer[WIFI_LINK_LEN - i] == 0x20)
							buffer[WIFI_LINK_LEN - i] = 0;
						else if(buffer[WIFI_LINK_LEN - i] != 0) break;
					}
					strncpy(str, buffer, WIFI_LINK_LEN-1);
					ButtonsClear();
					return;
				}
				else osDelay(1);
			}
			ButtonsClear();
			DelayStartCount(6);
			char chr = buffer[pointer];
			while(1)
			{
				if(DelayStopCount(6) % 1000000 < 500000)
				{
					lcd_goto(0x40 + lcdchar);
					lcd_putch(chr);
					lcd_goto(0x40 + lcdchar);
				}
				else
				{
					lcd_goto(0x40 + lcdchar);
					lcd_putch(' ');
					lcd_goto(0x40 + lcdchar);
				}
				if(BUT_CANCEL)
				{
					lcd_goto(0x40 + lcdchar);
					lcd_putch(buffer[pointer]);
					lcd_goto(0x40 + lcdchar);
					ButtonsClear();
					break;
				}
				if(BUT_ENTER)
				{
					buffer[pointer] = chr;
					lcd_goto(0x40 + lcdchar);
					lcd_putch(chr);
					lcd_goto(0x40 + lcdchar);
					ButtonsClear();
					break;
				}
				if(BUT_NEXT || (BUT_NEXT_PRESS && BUT_NEXT_PRESS_TIME > 300))
				{
					BUT_NEXT_PRESS_TIME -= 150;
					DelayStartCount(6);
					ButtonsClear();
					if(chr >= LCD_CONFIG_TO) chr = LCD_CONFIG_FROM;
					else chr++;
					lcd_goto(0x40 + lcdchar);
					lcd_putch(chr);
					lcd_goto(0x40 + lcdchar);
					
				}
				if(BUT_PREV || (BUT_PREV_PRESS && BUT_PREV_PRESS_TIME > 300))
				{
					BUT_PREV_PRESS_TIME -= 150;
					DelayStartCount(6);
					ButtonsClear();
					if(chr <= LCD_CONFIG_FROM) chr = LCD_CONFIG_TO;
					else chr--;
					lcd_goto(0x40 + lcdchar);
					lcd_putch(chr);
					lcd_goto(0x40 + lcdchar);
					
				}
				osDelay(5);
			}
		}
		
		osDelay(5);
	}
}


void GUI_TimerCallback(void)
{
	uint8_t but_enter = !HAL_GPIO_ReadPin(BUT_ENTER_GPIO_Port, BUT_ENTER_Pin);
	uint8_t but_next = !HAL_GPIO_ReadPin(BUT_NEXT_GPIO_Port, BUT_NEXT_Pin);
	uint8_t but_prev = !HAL_GPIO_ReadPin(BUT_PREV_GPIO_Port, BUT_PREV_Pin);
	uint8_t but_cancel = !HAL_GPIO_ReadPin(BUT_CANCEL_GPIO_Port, BUT_CANCEL_Pin);
	
	static uint8_t but_enter_prev = 0;
	static uint32_t but_enter_time = 0;
	if(but_enter == but_enter_prev) but_enter_time = 0; 
	else if(but_enter_time++ > 15) but_enter_prev = but_enter, 
		but_enter_time = 0;
	
	static uint8_t but_next_prev = 0;
	static uint32_t but_next_time = 0;
	if(but_next == but_next_prev) but_next_time = 0; 
	else if(but_next_time++ > 15) but_next_prev = but_next,
		but_next_time = 0;
	
	static uint8_t but_prev_prev = 0;
	static uint32_t but_prev_time = 0;
	if(but_prev == but_prev_prev) but_prev_time = 0; 
	else if(but_prev_time++ > 15) but_prev_prev = but_prev, 
		but_prev_time = 0;
	
	static uint8_t but_cancel_prev = 0;
	static uint32_t but_cancel_time = 0;
	if(but_cancel == but_cancel_prev) but_cancel_time = 0; 
	else if(but_cancel_time++ > 15) but_cancel_prev = but_cancel,
		but_cancel_time = 0;
	
	if(but_enter_prev)
	{
		if(BUT_ENTER_PRESS == 0)
		{
			BUT_ENTER_PRESS_TIME = 0;
			BUT_ENTER = 1;
		}
		BUT_ENTER_PRESS = 1;
		BUT_ENTER_PRESS_TIME++;
	}
	else BUT_ENTER_PRESS = 0;
	
	if(but_next_prev)
	{
		if(BUT_NEXT_PRESS == 0)
		{
			BUT_NEXT_PRESS_TIME = 0;
			BUT_NEXT = 1;
		}
		BUT_NEXT_PRESS = 1;
		BUT_NEXT_PRESS_TIME++;
	}
	else BUT_NEXT_PRESS = 0;
	
	if(but_prev_prev)
	{
		if(BUT_PREV_PRESS == 0)
		{
			BUT_PREV_PRESS_TIME = 0;
			BUT_PREV = 1;
		}
		BUT_PREV_PRESS = 1;
		BUT_PREV_PRESS_TIME++;
	}
	else BUT_PREV_PRESS = 0;
	
	if(but_cancel_prev)
	{
		if(BUT_CANCEL_PRESS == 0)
		{
			BUT_CANCEL_PRESS_TIME = 0;
			BUT_CANCEL = 1;
		}
		BUT_CANCEL_PRESS = 1;
		BUT_CANCEL_PRESS_TIME++;
	}
	else BUT_CANCEL_PRESS = 0;
}

void GUI_Init(void)
{
	HAL_TIM_Base_Start(&htim2);
	lcd_init();
	lcd_goto(2);
	lcd_print("Lanzar MCU");
	lcd_goto(0x40);
	lcd_print("Maksym Naumchuk");
	lcd_enable();
	lcd_led_on();
	
  osThreadDef(guiTask, StartGuiTask, osPriorityLow, 0, 512);
  guiTaskHandle = osThreadCreate(osThread(guiTask), NULL);
}

void GUI_SetVolume(uint8_t val)
{
	DAC_Volume = val;
}

void GUI_SetHPVolume(uint8_t val)
{
	DAC_HP_Volume = val;
}

void GUI_LedOff(void)
{
	lcd_led_off();
}

void GUI_LedOn(void)
{
	lcd_led_on();
}

void GUI_SetMixerMode(ModeTypeDef mode)
{
	MixerMode = mode;
}

