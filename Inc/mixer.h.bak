#include "main.h"

extern void Mixer_AppendUSBBuffer(uint8_t * data, uint32_t size);
extern void Mixer_AppendBTBuffer(uint16_t * data, uint32_t size);
extern void Mixer_AppendWIFIBuffer(uint16_t * data, uint32_t size);
extern void Mixer_TimerCallback(void);
extern void Mixer_SyncUSB(void);
extern void Mixer_SyncBT(void);
extern void Mixer_SyncWIFI(void);

typedef enum
{
	MODE_NONE,
	MODE_BT,
	MODE_USB,
	MODE_WIFI
} ModeTypeDef;

