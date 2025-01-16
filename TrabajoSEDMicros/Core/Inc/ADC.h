/*
* ADC.h
*
*  Created on: Jan 16, 2025
*      Author: David
*/
#ifndef INC_ADC_H_
#define INC_ADC_H_
#include "main.h"
// Declaración de funciones
void MX_ADC1_Init(void);      // Inicializa el ADC
uint32_t ADC_Read(void);  // Lee el valor del ADC
#endif /* INC_ADC_H_ */
