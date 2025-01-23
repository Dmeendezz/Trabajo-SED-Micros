#ifndef PARKING_H
#define PARKING_H

#include "stm32f4xx_hal.h"

/* Definiciones de pines */
#define TRIG_PIN GPIO_PIN_6
#define TRIG_PORT GPIOA
#define ECHO_PIN GPIO_PIN_7
#define ECHO_PORT GPIOA

#define LED_RED_PIN GPIO_PIN_5
#define LED_RED_PORT GPIOA
#define LED_GREEN_PIN GPIO_PIN_0
#define LED_GREEN_PORT GPIOB

/* Prototipos de funciones */
void Parking_Init(void);
void Parking_Process(void);
void Delay_us(uint32_t us);

//float Get_Stable_Distance(void);

uint8_t get_object_near();

#endif /* PARKING_H */

