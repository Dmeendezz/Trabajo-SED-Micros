#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "ADC.h"
#include "stm32f4xx_hal.h"
#include "parking.h"
#include "parking2.h"
#include "lcd.h"


uint16_t adcval;
uint8_t buttonPressed = 0;    // Variable para marcar si se presionó el botón
volatile uint8_t ledOn = 0;         // Indica si el LED está encendido
uint32_t ledStartTime = 0;          // Tiempo en que el LED se encendió
uint32_t ledEntrada = 0;
uint16_t plazas;
LCD_t lcd;


/* Prototipos de funciones */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void plazas_libres();
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
   //Parking_Init3();
   // LCD SETTINGS
        lcd.RS_port = LCD_RS_GPIO_Port;
        lcd.RS_pin = LCD_RS_Pin;
        //lcd.RW_port = LCD_RW_GPIO_Port;
        //lcd.RW_pin = LCD_RW_Pin;
        lcd.EN_port = LCD_EN_GPIO_Port;
        lcd.EN_pin = LCD_EN_Pin;
        lcd.D4_port = D4_GPIO_Port;
        lcd.D4_pin = D4_Pin;
        lcd.D5_port = D5_GPIO_Port;
        lcd.D5_pin = D5_Pin;
        lcd.D6_port = D6_GPIO_Port;
        lcd.D6_pin = D6_Pin;
        lcd.D7_port = D7_GPIO_Port;
        lcd.D7_pin = D7_Pin;
        // inicializamos el LCD
        lcd_begin(&lcd, 16, 2, LCD_5x8DOTS);
        /* Escribimos un mensaje inicial */
        lcd_setCursor(&lcd, 0, 0);
        lcd_print(&lcd, "BIENVENIDOS ");
        lcd_setCursor(&lcd, 0, 1);
        lcd_print(&lcd, "AL PARKING");
        HAL_Delay(3000);
        lcd_noDisplay(&lcd); // Apagar el display
        lcd_display(&lcd);   // Encender el display
        lcd_clear(&lcd);     // Limpiar el LCD
        lcd_home(&lcd);      // Llevar el cursor al inicio
        lcd_print(&lcd, "Puedes aparcar");
        HAL_Delay(1000);
        lcd_cursor(&lcd);    // Mostrar el cursor
        HAL_Delay(1000);
        lcd_blink(&lcd);     // Hacer parpadear el cursor
        HAL_Delay(3000);
        lcd_noBlink(&lcd);   // Detener el parpadeo
        lcd_noCursor(&lcd);  // Ocultar el cursor
        HAL_Delay(100);
        for (int i = 0; i < 10; ++i) {
            lcd_scrollDisplayRight(&lcd); // Desplazar texto a la derecha
            HAL_Delay(300);
        }
        for (int i = 0; i < 10; ++i) {
              lcd_scrollDisplayLeft(&lcd); // Desplazar texto a la izquierda
              HAL_Delay(300);
          }
   while (1) {
       Parking_Process();
       Parking_Process2();
       //Parking_Process3();
       adcval = ADC_Read(); // Leer valor del ADC
       // Lógica para manejar el LED de emergencia
       if ((((adcval < 2400) || (buttonPressed))) && !ledOn) {
           buttonPressed = 0; // Reinicia la bandera tras usarla
           ledOn = 1;         // Marca que el LED está encendido
           ledStartTime = HAL_GetTick(); // Registra el tiempo de inicio
           HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET); // Enciende LED
       }
       if (ledOn && (HAL_GetTick() - ledStartTime >= 7000)) {
           HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); // Apaga LED
           ledOn = 0; // Marca que el LED está apagado
       }
       // Control del LED externo (PA4) según el valor del ADC
       if (adcval < 2400) {
           HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Enciende LED externo
           ledEntrada = HAL_GetTick();
       }
       if(HAL_GetTick() - ledEntrada >= 1500){
                   	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
       }
       plazas_libres();
   }
}
void plazas_libres(){
	plazas = 4 - get_object_near() - get_object_near2();
	char plazas_char[5]; // Espacio para 2 dígitos + '\0'
	    sprintf(plazas_char, "%u", plazas);
	    lcd_setCursor(&lcd, 0, 0);
	    lcd_print(&lcd, "Plazas libres: ");
	    lcd_print(&lcd, plazas_char);
}
/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
 RCC_OscInitTypeDef RCC_OscInitStruct = {0};
 RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
 /** Configure the main internal regulator output voltage
 */
 __HAL_RCC_PWR_CLK_ENABLE();
 __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
 /** Initializes the RCC Oscillators according to the specified parameters
 * in the RCC_OscInitTypeDef structure.
 */
 RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
 RCC_OscInitStruct.HSIState = RCC_HSI_ON;
 RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
 RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
 RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
 RCC_OscInitStruct.PLL.PLLM = 8;
 RCC_OscInitStruct.PLL.PLLN = 100;
 RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
 RCC_OscInitStruct.PLL.PLLQ = 8;
 if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
 {
   Error_Handler();
 }
 /** Initializes the CPU, AHB and APB buses clocks
 */
 RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                             |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
 RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
 RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
 RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
 RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
 if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
 {
   Error_Handler();
 }
}
static void MX_GPIO_Init(void)
{
 GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */
 /* GPIO Ports Clock Enable */
 __HAL_RCC_GPIOE_CLK_ENABLE();
 __HAL_RCC_GPIOC_CLK_ENABLE();
 __HAL_RCC_GPIOA_CLK_ENABLE();
 __HAL_RCC_GPIOB_CLK_ENABLE();
 __HAL_RCC_GPIOD_CLK_ENABLE();
 /*Configure GPIO pin Output Level */
 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_RESET);
 /*Configure GPIO pin Output Level */
 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_RESET);
 /*Configure GPIO pin Output Level */
 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);
 /*Configure GPIO pin Output Level */
 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|LCD_EN_Pin|LCD_RS_Pin|D7_Pin
                         |D6_Pin|D5_Pin|D4_Pin, GPIO_PIN_RESET);
 /*Configure GPIO pin Output Level */
 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
 /*Configure GPIO pins : PE2 PE3 PE4 */
 GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
 HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
 /*Configure GPIO pin : PE5 */
 GPIO_InitStruct.Pin = GPIO_PIN_5;
 GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
 /*Configure GPIO pins : PC0 PC1 PC2 */
 GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
 HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
 /*Configure GPIO pin : PC3 */
 GPIO_InitStruct.Pin = GPIO_PIN_3;
 GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
 /*Configure GPIO pin : PA0 */
 GPIO_InitStruct.Pin = GPIO_PIN_0;
 GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 /*Configure GPIO pins : PA4 PA5 PA6 */
 GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
 HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 /*Configure GPIO pin : PA7 */
 GPIO_InitStruct.Pin = GPIO_PIN_7;
 GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 /*Configure GPIO pins : PB0 LCD_EN_Pin LCD_RS_Pin D7_Pin
                          D6_Pin D5_Pin D4_Pin */
 GPIO_InitStruct.Pin = GPIO_PIN_0|LCD_EN_Pin|LCD_RS_Pin|D7_Pin
                         |D6_Pin|D5_Pin|D4_Pin;
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
 HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 /*Configure GPIO pin : PD13 */
 GPIO_InitStruct.Pin = GPIO_PIN_13;
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 GPIO_InitStruct.Pull = GPIO_NOPULL;
 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
 HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
 /* EXTI interrupt init*/
 HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
 HAL_NVIC_EnableIRQ(EXTI0_IRQn);
/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}
/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
 /* USER CODE BEGIN Error_Handler_Debug */
 /* User can add his own implementation to report the HAL error return state */
 __disable_irq();
 while (1)
 {
 }
 /* USER CODE END Error_Handler_Debug */
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

