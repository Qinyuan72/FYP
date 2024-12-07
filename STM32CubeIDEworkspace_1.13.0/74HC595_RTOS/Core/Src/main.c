#include "main.h"
#include "cmsis_os.h"

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Define task handles */
osThreadId_t displayTaskHandle;

/* GPIO Pin Definitions for SN74HC595 */
#define SER_PIN    GPIO_PIN_5 // Serial Data Input
#define SRCLK_PIN  GPIO_PIN_7 // Shift Register Clock
#define RCLK_PIN   GPIO_PIN_6 // Storage Register Clock (Latch)
#define OE_PIN     GPIO_PIN_6 // Output Enable (active low)
#define SER_PORT   GPIOA
#define RCLK_PORT  GPIOB
#define OE_PORT    GPIOA

/* Array for displaying numbers 0 to 9 on the 7-segment display */
uint16_t number[10] = {
    0b00000011, 0b10011111, 0b00100101, 0b00001101,
    0b10011001, 0b01001001, 0b01000001, 0b00011011,
    0b00000001, 0b00001001
};

/* Function prototypes */
void SN74HC595_Init(void);
void SN74HC595_SendData(uint8_t data);
void vTaskSN74HC595(void *argument);
void SystemClock_Config(void);
void MX_USART2_UART_Init(void);
void MX_GPIO_Init(void);

/**
  * @brief  Main function to initialize RTOS and tasks.
  * @retval int
  */
int main(void)
{
    /* Initialize HAL library */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    /* Initialize SN74HC595 */
    SN74HC595_Init();

    /* Initialize FreeRTOS */
    osKernelInitialize();

    /* Create the SN74HC595 display task */
    const osThreadAttr_t displayTask_attributes = {
        .name = "DisplayTask",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityNormal,
    };
    displayTaskHandle = osThreadNew(vTaskSN74HC595, NULL, &displayTask_attributes);

    /* Start FreeRTOS scheduler */
    osKernelStart();

    /* Infinite loop (we should never reach here) */
    while (1) {}

    return 0;
}

/**
  * @brief SN74HC595 initialization.
  */
void SN74HC595_Init(void)
{
    HAL_GPIO_WritePin(OE_PORT, OE_PIN, GPIO_PIN_SET); // Disable output initially
}

/**
  * @brief Send data to SN74HC595 shift register.
  * @param data: 8-bit data to be sent
  */
void SN74HC595_SendData(uint8_t data)
{
    // Enable output (active low)
    HAL_GPIO_WritePin(OE_PORT, OE_PIN, GPIO_PIN_RESET);

    // Shift each bit into the shift register
    for (int i = 0; i < 8; i++)
    {
        // Set SER pin based on the current bit
        HAL_GPIO_WritePin(SER_PORT, SER_PIN, (data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        // Pulse SRCLK to shift in the bit
        HAL_GPIO_WritePin(SER_PORT, SRCLK_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(SER_PORT, SRCLK_PIN, GPIO_PIN_RESET);
    }

    // Latch the data into the storage register
    HAL_GPIO_WritePin(RCLK_PORT, RCLK_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RCLK_PORT, RCLK_PIN, GPIO_PIN_RESET);

    // Disable output if required
    HAL_GPIO_WritePin(OE_PORT, OE_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Task to manage the SN74HC595 shift register display.
  * @param  argument: Not used
  */
void vTaskSN74HC595(void *argument)
{
    uint8_t cnt = 0;

    /* Infinite loop to update display */
    for (;;)
    {
        /* Send current count value to 74HC595 */
        SN74HC595_SendData(number[cnt]);

        /* Increment counter and wrap around after 9 */
        cnt = (cnt + 1) % 10;

        /* Delay for a while (replace HAL_Delay with FreeRTOS delay) */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
  * @brief  Configure the system clock.
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pins : SER_PIN, SRCLK_PIN, OE_PIN */
    GPIO_InitStruct.Pin = SER_PIN | SRCLK_PIN | OE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SER_PORT, &GPIO_InitStruct);

    /*Configure GPIO pin : RCLK_PIN */
    GPIO_InitStruct.Pin = RCLK_PIN;
    HAL_GPIO_Init(RCLK_PORT, &GPIO_InitStruct);
}

/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
