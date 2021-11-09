#include "main.h"

#define BT_PIN_LEN 5
#define BT_NAME_LEN 16
#define WIFI_SSID_LEN 16
#define WIFI_KEY_LEN 16
#define WIFI_LINK_NAME 16
#define WIFI_LINK_LEN 128
#define WIFI_LINK_CNT 8

typedef struct
{
	uint8_t BT_Enabled;
	uint8_t WIFI_Enabled;
	char BT_Pin[BT_PIN_LEN];
	char BT_Name[BT_NAME_LEN];
	char WIFI_SSID[WIFI_SSID_LEN];
	char WIFI_Key[WIFI_SSID_LEN];
	char WIFI_Link[WIFI_LINK_CNT][WIFI_LINK_LEN];
	char WIFI_LinkName[WIFI_LINK_CNT][WIFI_LINK_NAME];
	
}ParametersTypeDef;

extern ParametersTypeDef Parameters;

extern void Params_SaveConfiguration(void);
extern HAL_StatusTypeDef Params_LoadConfiguration(void);
