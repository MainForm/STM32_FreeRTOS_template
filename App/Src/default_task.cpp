#include <string.h>

#include "cmsis_os2.h"
#include "usart.h"

extern "C"
void StartDefaultTask(void *argument){
    const char *msg = "msg : Task 1 is running\r\n";

    while(1){
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        osDelay(1000);
    }
}