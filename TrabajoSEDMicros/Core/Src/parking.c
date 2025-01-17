/*
 * parking.c
 *
 *  Created on: Jan 17, 2025
 *      Author: gasco
 */

#include "parking.h"

/* Variables globales */
static TIM_HandleTypeDef htim2; // Temporizador para medir pulsos
static float distance = 0.0f;  // Distancia medida en cm
static uint32_t elapsed_time = 0; // Tiempo acumulado en microsegundos
static uint32_t tiempo_plaza1 = 0; // Último tiempo registrado
static uint8_t object_near = 0;    // Bandera de objeto cercano

/* Función estática para medir el ancho del pulso ECHO */
static uint32_t Measure_Pulse_Width(void) {
    uint32_t start = 0, stop = 0;

    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_RESET);
    start = __HAL_TIM_GET_COUNTER(&htim2);

    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET);
    stop = __HAL_TIM_GET_COUNTER(&htim2);

    return (stop >= start) ? (stop - start) : (0xFFFFFFFF - start + stop + 1);
}

/* Inicialización del sistema de estacionamiento */
void Parking_Init(void) {
    // Inicialización de GPIO
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = TRIG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TRIG_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ECHO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ECHO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_RED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_GREEN_PIN;
    HAL_GPIO_Init(LED_GREEN_PORT, &GPIO_InitStruct);

    // Inicialización del temporizador TIM2
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 100 - 1;  // Frecuencia de 1 MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF; // Periodo máximo (32 bits)
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        while (1); // Manejo de error
    }

    HAL_TIM_Base_Start(&htim2); // Iniciar temporizador
}

/* Procesar la lógica de estacionamiento */
void Parking_Process(void) {
    // Generar pulso TRIG
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    Delay_us(10); // Pulso de 10 us
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

    // Medir el pulso ECHO y calcular la distancia
    uint32_t duration = Measure_Pulse_Width();
    distance = (duration / 2.0) * 0.0343; // Distancia en centímetros

    if (distance <= 5.0) {
        // Objeto cercano
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);       // Encender LED rojo
        HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET); // Apagar LED verde

        if (object_near == 0) {
            object_near = 1;  // Objeto detectado por primera vez
            elapsed_time = 0; // Reiniciar el tiempo acumulado
        } else {
            elapsed_time += duration; // Acumular tiempo transcurrido
        }
    } else {
        // Objeto alejado
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET); // Apagar LED rojo
        HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);   // Encender LED verde

        if (object_near == 1) {
            object_near = 0;         // Objeto ya no está cerca
            tiempo_plaza1 = elapsed_time; // Guardar el tiempo acumulado
            elapsed_time = 0;        // Reiniciar el tiempo acumulado
        }
    }

    HAL_Delay(100); // Pausa entre mediciones
}

/* Función de delay en microsegundos */
void Delay_us(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0); // Reiniciar contador
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

