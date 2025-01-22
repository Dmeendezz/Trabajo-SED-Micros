#include "main.h"
#include "ADC.h"
#include "stm32f4xx_hal.h"
#include "parking.h"
#include "parking2.h"

uint16_t adcval;
 uint8_t buttonPressed = 0;    // Variable para marcar si se presionó el botón
volatile uint8_t ledOn = 0;         // Indica si el LED está encendido
uint32_t ledStartTime = 0;          // Tiempo en que el LED se encendió

/* Prototipos de funciones */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
int debouncer(GPIO_TypeDef* GPIO_port, uint16_t GPIO_number);

/* Función de callback para interrupciones EXTI */


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        buttonPressed = 1; // Marca que el botón fue presionado
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13); // Indicador en D12 para depuración
    }
}






/* Función de debouncing */
int debouncer(GPIO_TypeDef* GPIO_port, uint16_t GPIO_number) {
    static uint32_t last_tick = 0;
    static uint8_t last_state = GPIO_PIN_SET;

    uint8_t current_state = HAL_GPIO_ReadPin(GPIO_port, GPIO_number);

    if (last_state == GPIO_PIN_SET && current_state == GPIO_PIN_RESET) {
        if (HAL_GetTick() - last_tick > 100) { // 100 ms para debouncing
            last_tick = HAL_GetTick();
            last_state = current_state;
            return 1; // Pulsación válida
        }
    }

    last_state = current_state;
    return 0; // No hay evento
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init(); // Inicializar ADC
    Parking_Init();
    Parking_Init2();


    while (1) {
        Parking_Process();
        Parking_Process2();

        adcval = ADC_Read(); // Leer valor del ADC

        // Lógica para manejar el LED de emergencia
        if ((((adcval < 2400) || (buttonPressed))) && !ledOn) {
            buttonPressed = 0; // Reinicia la bandera tras usarla
            ledOn = 1;         // Marca que el LED está encendido
            ledStartTime = HAL_GetTick(); // Registra el tiempo de inicio
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET); // Enciende LED
        }

        if (ledOn && (HAL_GetTick() - ledStartTime >= 5000)) {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); // Apaga LED
            ledOn = 0; // Marca que el LED está apagado
        }

        // Control del LED externo (PA4) según el valor del ADC
        if (adcval < 2400) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Enciende LED externo
        } else {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Apaga LED externo
        }
    }
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Habilitar relojes para los puertos
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // Configurar pin A4 como salida (LED externo)
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configurar pin A0 como entrada analógica (ADC)
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configurar pin A0 como entrada con interrupción (botón)
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configurar pin D13 como salida (LED de emergencia)
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    // Configurar interrupción EXTI para el pin A0
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0); // Prioridad 2
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);         // Habilitar interrupción
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 100;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 8;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void) {
    while (1) {
        // Aquí puedes implementar lógica adicional para depurar errores
    }
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
