#include <MCXA153.h>
#include "App/app.h"

int main(void)
{
    app_init();

    while (1)
    {
        app_update();
    }
}

void SysTick_Handler(void)
{
    app_tick_1ms();
}