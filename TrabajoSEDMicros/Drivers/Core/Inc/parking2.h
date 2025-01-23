/*
 * parking2.h
 *
 *  Created on: Jan 19, 2025
 *      Author: gasco
 */

#ifndef PARKING2_H
#define PARKING2_H

#include "stm32f4xx_hal.h"

// Declaración de funciones públicas

#define TRIG_PIN2 GPIO_PIN_0
#define TRIG_PORT2 GPIOC
#define ECHO_PIN2 GPIO_PIN_3
#define ECHO_PORT2 GPIOC

#define LED_RED_PIN2 GPIO_PIN_1
#define LED_RED_PORT2 GPIOC
#define LED_GREEN_PIN2 GPIO_PIN_2
#define LED_GREEN_PORT2 GPIOC



void Parking_Init2(void);
void Parking_Process2(void);
void Delay_us2(uint32_t us);

//float Get_Stable_Distance(void);

uint8_t get_object_near2();






#endif // PARKING2_H
