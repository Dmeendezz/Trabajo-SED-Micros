


#include "parking3.h"

static TIM_HandleTypeDef htim4; // Temporizador para medir pulsos
static float distance3 = 0.0f;  // Distancia medida en cm
static uint32_t elapsed_time3 = 0; // Tiempo acumulado en microsegundos
static uint32_t tiempo_plaza3 = 0; // Último tiempo registrado
static uint8_t object_near3 = 0;    // Bandera de objeto cercano
static float tiempo_segundos3 = 0.0f;


/* Función estática para medir el ancho del pulso ECHO */
static uint32_t Measure_Pulse_Width(void) {
    uint32_t start = 0, stop = 0;

    while (HAL_GPIO_ReadPin(ECHO_PORT3, ECHO_PIN3) == GPIO_PIN_RESET);
    start = __HAL_TIM_GET_COUNTER(&htim4);

    while (HAL_GPIO_ReadPin(ECHO_PORT3, ECHO_PIN3) == GPIO_PIN_SET);
    stop = __HAL_TIM_GET_COUNTER(&htim4);

    return (stop >= start) ? (stop - start) : (65535 - start + stop + 1);
}

/* Inicialización del sistema de estacionamiento */

/* Procesar la lógica de estacionamiento */
void Parking_Process3(void) {
    // Generar pulso TRIG
    HAL_GPIO_WritePin(TRIG_PORT3, TRIG_PIN3, GPIO_PIN_SET);
    Delay_us3(10); // Pulso de 10 us
    HAL_GPIO_WritePin(TRIG_PORT3, TRIG_PIN3, GPIO_PIN_RESET);

    // Medir el pulso ECHO y calcular la distancia
    uint32_t duration3 = Measure_Pulse_Width();
    distance3 = (duration3 / 2.0) * 0.0343;// Distancia en centímetros

    if (distance3 <= 5.0) {
        // Objeto cercano
        HAL_GPIO_WritePin(LED_RED_PORT3, LED_RED_PIN3, GPIO_PIN_SET);       // Encender LED rojo
        HAL_GPIO_WritePin(LED_GREEN_PORT3, LED_GREEN_PIN3, GPIO_PIN_RESET); // Apagar LED verde

        if (object_near3 == 0) {
            object_near3 = 1;  // Objeto detectado por primera vez
            elapsed_time3 = 0; // Reiniciar el tiempo acumulado
        } else {
            elapsed_time3 += duration3/2; // Acumular tiempo transcurrido
        }
    } else {
        // Objeto alejado
        HAL_GPIO_WritePin(LED_RED_PORT3, LED_RED_PIN3, GPIO_PIN_RESET); // Apagar LED rojo
        HAL_GPIO_WritePin(LED_GREEN_PORT3, LED_GREEN_PIN3, GPIO_PIN_SET);   // Encender LED verde

        if (object_near3 == 1) {
            object_near3 = 0;         // Objeto ya no está cerca
            tiempo_plaza3 = elapsed_time3; // Guardar el tiempo acumulado
            tiempo_segundos3=tiempo_plaza3/1000.0f;
            elapsed_time3 = 0;        // Reiniciar el tiempo acumulado
        }
    }

    //HAL_Delay(100); // Pausa entre mediciones
}
void Parking_Init3(void) {

    // Inicialización de GPIO
    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = TRIG_PIN3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TRIG_PORT3, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ECHO_PIN3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ECHO_PORT3, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_RED_PIN3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_RED_PORT3, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_GREEN_PIN3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GREEN_PORT3, &GPIO_InitStruct);

    // Inicialización del temporizador TIM4
    __HAL_RCC_TIM4_CLK_ENABLE();



     /* USER CODE BEGIN TIM4_Init 1 */

     /* USER CODE END TIM4_Init 1 */
     htim4.Instance = TIM4;
     htim4.Init.Prescaler = 99;
     htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
     htim4.Init.Period = 65535;
     htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
     htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;


     if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
            while (1); // Manejo de error
        }

    HAL_TIM_Base_Start(&htim4); // Iniciar temporizador
}


/* Función de delay en microsegundos */
void Delay_us3(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim4, 0); // Reiniciar contador
    while (__HAL_TIM_GET_COUNTER(&htim4) < us);
}



uint8_t get_object_near3(){return object_near3;}
