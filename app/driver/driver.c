#include "driver.h"



void Driver_Init( void )
{
    //拉低GPIO16(关掉板上业务功能)
    gpio16_output_conf();
    gpio16_output_set(0);

    Uart0_Init();
    d_printf( "\n\nstart...." );

}





