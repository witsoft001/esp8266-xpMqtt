#ifndef __MQTT_USER_TASTS__
#define __MQTT_USER_TASTS__

#include "c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef TaskDelay
#define TaskDelay(n)    vTaskDelay( n / portTICK_RATE_MS )
#endif


/***********************************************************************************/
//                  º¯ÊýAPI
/***********************************************************************************/
void MQTT_USER_TAST_Init( uint16 usStackDepth, uint16 uxPriority );



#endif

