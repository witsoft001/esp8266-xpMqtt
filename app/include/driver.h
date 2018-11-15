#ifndef __DRIVER_H__
#define __DRIVER_H__


#include "esp_common.h"
#include "gpio.h"
#include "uart.h"



void Driver_Init( void );


#define d_printf( format, ... )         printf( "D: "format"\n", ##__VA_ARGS__ )







#endif
