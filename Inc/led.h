#ifndef LED_H
#define LED_H

#define LED_ON  1
#define LED_OFF 0
#define LED_TO  5   /* 3ms. */

typedef struct{
	uint8_t sw;
	uint8_t counter;
}led_show_t;

void led_routine(void);
void led_check(void);

#endif
/* End of this file. */
