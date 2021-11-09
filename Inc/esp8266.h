#include "stm32f7xx_hal.h"


extern void ESP_TxCallback(UART_HandleTypeDef *huart);
extern void ESP_RxCallback(UART_HandleTypeDef *huart);
extern void ESP_ErrorCallback(UART_HandleTypeDef *huart);

extern void ESP_Start(void);

extern int esp_rxcheck(int fd);
extern int esp_recv(int fd, char * buffer, uint16_t size);
extern int esp_send(int fd, char * buffer, uint16_t size, uint8_t mode);
extern int esp_disconnect(int fd);
extern int esp_connect(char * host, uint16_t port);
extern int esp_gethost(int fd, char * host);
extern int esp_apconnect(char * ssid, char * key);


