#include "gpio.h"
#include "main.h"
#include "cmsis_os.h"
#include "led.h"

led_show_t led_green;

void led_check(void)
{
    if(led_green.sw == LED_ON){
        if(led_green.counter > LED_TO){
            led_green.counter = 0;
            led_green.sw = LED_OFF;
            /*Configure GPIO pin Output Level */
            HAL_GPIO_WritePin(GPIOE, Sys_led_Pin, GPIO_PIN_SET);
        }else{
            led_green.counter++;
            HAL_GPIO_WritePin(GPIOE, Sys_led_Pin, GPIO_PIN_RESET);
        }
    }
}

void led_routine(void)
{
    if(led_green.sw == LED_OFF){
        led_green.sw = LED_ON;
    }
    osDelay(1000);
}

/* End of this file. */
