/*
 * parking2.c
 *
 *  Created on: Jan 20, 2025
 *      Author: gasco
 */

#include "parking2.h"
#include <stdio.h> // Para printf si es necesario


// Variables estáticas (locales al módulo)


static TIM_HandleTypeDef htim3; // TEMPORIZADOR TIM 3
static float distance2;          // Distancia medida en cm
static uint32_t elapsed_time2 = 0;
extern uint32_t tiempo_plaza2 = 0;
static float tiempo_segundos2 = 0.0f;
static uint8_t object_near2 = 0;



/* Función estática para medir el ancho del pulso ECHO */
static uint32_t Measure_Pulse_Width(void) {
    uint32_t start = 0, stop = 0;

    while (HAL_GPIO_ReadPin(ECHO_PORT2, ECHO_PIN2) == GPIO_PIN_RESET);
    start = __HAL_TIM_GET_COUNTER(&htim3);

    while (HAL_GPIO_ReadPin(ECHO_PORT2, ECHO_PIN2) == GPIO_PIN_SET);
    stop = __HAL_TIM_GET_COUNTER(&htim3);

    return (stop >= start) ? (stop - start) : (0xFFFFFFFF - start + stop + 1);
}

// INICIALIZACION PARKING 2

void Parking_Init2(void) {

    // INICIALIZACION DE LOS PINES PARKING 2

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = TRIG_PIN2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TRIG_PORT2, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ECHO_PIN2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ECHO_PORT2, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_RED_PIN2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_RED_PORT2, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_GREEN_PIN2;
    HAL_GPIO_Init(LED_GREEN_PORT2, &GPIO_InitStruct);

    // INICIALIZACION DEL TEMPORIZADOR PARKING 2 (TIM3)

    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 100 - 1;  // Frecuencia de 1 MHz
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 65535; // Periodo máximo (32 bits)
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
        while (1); // Manejo de error
    }

    HAL_TIM_Base_Start(&htim3); // Iniciar temporizador
}


//FUNCIONAMINETO PARKING 2

void Parking_Process2(void) {

    // Generar pulso TRIG
    HAL_GPIO_WritePin(TRIG_PORT2, TRIG_PIN2, GPIO_PIN_SET);
    Delay_us2(10); // Pulso de 10 us
    HAL_GPIO_WritePin(TRIG_PORT2, TRIG_PIN2, GPIO_PIN_RESET);

    // Medir el pulso ECHO y calcular la distancia
    uint32_t duration2 = Measure_Pulse_Width();
    distance2 = (duration2 / 2.0) * 0.0343;// Distancia en centímetros

    if (distance2 <= 5.0) {

        // COCHE APARCADO

        HAL_GPIO_WritePin(LED_RED_PORT2, LED_RED_PIN2, GPIO_PIN_SET);       // Encender LED rojo
        HAL_GPIO_WritePin(LED_GREEN_PORT2, LED_GREEN_PIN2, GPIO_PIN_RESET); // Apagar LED verde

        if (object_near2 == 0) {
            object_near2 = 1;  // Objeto detectado por primera vez
            elapsed_time2 = 0; // Reiniciar el tiempo acumulado
        } else {
            elapsed_time2 += duration2;
        }
    } else {

        // SE VA EL COCHE

        HAL_GPIO_WritePin(LED_RED_PORT2, LED_RED_PIN2, GPIO_PIN_RESET); // Apagar LED rojo
        HAL_GPIO_WritePin(LED_GREEN_PORT2, LED_GREEN_PIN2, GPIO_PIN_SET);   // Encender LED verde

        if (object_near2 == 1) {
            object_near2 = 0;
            tiempo_plaza2 = elapsed_time2; // TIEMPO PLAZA 2
            tiempo_segundos2=tiempo_plaza2/1000.0f;
            elapsed_time2 = 0;        // REINICIA EL TEMPORIZADOR
        }
    }

    HAL_Delay(100); // Pausa entre mediciones
}

/* Función de delay en microsegundos */
void Delay_us2(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim3, 0); // REINICIAR CTEMPORIZADOR TIM3
    while (__HAL_TIM_GET_COUNTER(&htim3) < us);
}



uint8_t get_object_near2(){return object_near2;}


