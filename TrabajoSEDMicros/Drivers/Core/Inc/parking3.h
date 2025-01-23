/*
 * parking2.h
 *
 *  Created on: Jan 19, 2025
 *      Author: gasco
 */

#ifndef PARKING3_H
#define PARKING3_H

#include "stm32f4xx_hal.h"

// Declaración de funciones públicas

#define TRIG_PIN3 GPIO_PIN_4
#define TRIG_PORT3 GPIOE
#define ECHO_PIN3 GPIO_PIN_5
#define ECHO_PORT3 GPIOE

#define LED_RED_PIN3 GPIO_PIN_2
#define LED_RED_PORT3 GPIOE
#define LED_GREEN_PIN3 GPIO_PIN_3
#define LED_GREEN_PORT3 GPIOE



void Parking_Init3(void);
void Parking_Process3(void);
void Delay_us3(uint32_t us);

//float Get_Stable_Distance(void);

uint8_t get_object_near3();






#endif // PARKING2_H
