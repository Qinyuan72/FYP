#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* --- 1) 环形缓冲大小 --- */
#define RING_BUFFER_SIZE 256

/* --- 2) 用于环形缓冲的全局数组和读写指针 --- */
/* 为防止编译器优化，在多任务或中断场景下，建议使用 volatile */
static volatile uint8_t ringBuffer[RING_BUFFER_SIZE];
static volatile uint16_t ringHead = 0;
static volatile uint16_t ringTail = 0;

/* --- 3) 用于中断接收的临时字节 --- */
static uint8_t rxData = 0;

/**
  * @brief  往环形缓冲写入 1 字节 (在中断回调中调用)
  */
static void RingBuffer_WriteByte(uint8_t data)
{
    uint16_t next = (ringHead + 1) % RING_BUFFER_SIZE;

    // 如果 next == ringTail，说明缓冲满了 (队列将溢出)
    // 简单处理：覆盖最早的数据
    if (next == ringTail)
    {
        // 丢弃最旧字节
        ringTail = (ringTail + 1) % RING_BUFFER_SIZE;
    }

    ringBuffer[ringHead] = data;
    ringHead = next;
}

/**
  * @brief  从环形缓冲读 1 字节 (在任务里循环调用)
  * @retval >=0 读到的有效字节； -1 表示缓冲为空
  */
static int RingBuffer_ReadByte(void)
{
    if (ringTail == ringHead)
    {
        // 缓冲为空
        return -1;
    }
    else
    {
        uint8_t data = ringBuffer[ringTail];
        ringTail = (ringTail + 1) % RING_BUFFER_SIZE;
        return data;
    }
}


/* 用于中断接收的临时字节 */
/* 全局变量，用于存储解析到的整数 */
volatile int number = 65535;

/* Define task handles */
osThreadId_t displayTaskHandle;
osThreadId_t helloTaskHandle;
osThreadId_t USARTComHandle;

/* GPIO Pin Definitions for SN74HC595 */
#define SER_PIN    GPIO_PIN_6  // PB6 -> Serial Data Input
#define SRCLK_PIN  GPIO_PIN_6  // PA6 -> Shift Register Clock
#define RCLK_PIN   GPIO_PIN_7  // PA7 -> Latch Clock
#define SER_PORT   GPIOB
#define SRCLK_PORT GPIOA
#define RCLK_PORT  GPIOA
#define MY_DELAY 10

/* Array for displaying numbers 0 to 9 on the 7-segment display */

/* Function prototypes */
void SN74HC595_Init(void);
void SN74HC595_SendData(int16_t data);
void vTaskSN74HC595(void *argument);
void vTaskHello(void *argument);
void vTask_USART_Com(void *argument);
void parseDollarNumber(const char *str);
void SystemClock_Config(void);
void MX_USART2_UART_Init(void);
void MX_GPIO_Init(void);
void my_delay(int value);

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

    /* 先启动一次中断接收，准备收第 1 个字节 */
    HAL_UART_Receive_IT(&huart2, &rxData, 1);

    char *testMsg = "Bare-metal print before RTOS starts...\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)testMsg, strlen(testMsg), 100);

    /* Initialize SN74HC595 */
    SN74HC595_Init();

    /* Initialize FreeRTOS */
    osKernelInitialize();

    /* Create the SN74HC595 display task */
    const osThreadAttr_t displayTask_attributes = {
        .name = "DisplayTask",
        .stack_size = 256 * 4,
        .priority = (osPriority_t) osPriorityRealtime,
    };
    displayTaskHandle = osThreadNew(vTaskSN74HC595, NULL, &displayTask_attributes);

    const osThreadAttr_t helloTask_attributes = {
        .name = "helloTask",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityNormal,
    };
    helloTaskHandle = osThreadNew(vTaskHello, NULL, &helloTask_attributes);

    const osThreadAttr_t Task_USART_Com_attributes = {
        .name = "vTask_USART_Com",
        .stack_size = 512 * 4,
        .priority = (osPriority_t) osPriorityNormal,
    };
    USARTComHandle = osThreadNew(vTask_USART_Com, NULL, &Task_USART_Com_attributes);
    /* Start FreeRTOS scheduler */

    if (USARTComHandle == NULL)
    {
        char *err = "Error: Failed to create vTask_USART_Com!\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)err, strlen(err), 100);
    }

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
    HAL_GPIO_WritePin(SER_PORT, SER_PIN, GPIO_PIN_SET); // Disable output initially
}

/**
  * @brief Send data to SN74HC595 shift register.
  * @param data: 8-bit data to be sent
  */
/* 优化后的数据发送函数（确保唯一存在） */

void my_delay(int value)
{

	for (int i=0; i<value; i++)
	{
		__NOP();
	}

}
void SN74HC595_SendData(int16_t  data)
{

    // Initial values
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // SER
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); // RCLK
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // SRCLK

    // Loop through each bit in data (from LSB to MSB)
    for (int i=0; i<16; i++)
    {
        // Set SER pin based on the current bit
        if (data & (1<<i))
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // SER = 1
        } else
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // SER = 0
        }
        my_delay(MY_DELAY);

        // Pulse the clock to shift in the bit
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);   // SRCLK rising edge
        my_delay(MY_DELAY);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // SRCLK falling edge
        my_delay(MY_DELAY);
    }

    // Latch the shifted data to the output
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);   // ST_CP (Latch) rising edge
    my_delay(MY_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); // ST_CP (Latch) falling edge

    // Final values
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // SER
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); // RCLK
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // SRCLK

	my_delay(MY_DELAY);

}


/**
  * @brief  Task to manage the SN74HC595 shift register display.
  * @param  argument: Not used
  */
void vTaskSN74HC595(void *argument)
{
    /* Infinite loop to update display */
    for (;;)
    {
        /* Send current count value to 74HC595 */
        SN74HC595_SendData(number);
        /* Delay for a while (replace HAL_Delay with FreeRTOS delay) */
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

/* 这是我们要运行的任务函数 */
void vTaskHello(void *argument)
{
    for (;;)
    {
        /* 每隔 1s 通过串口打印一次 */
        char *msg = "Status Check: RTOS Running\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        /* 休眠 1000ms (FreeRTOS 写法) */
        osDelay(5000);
    }
}

void vTask_USART_Com(void *argument)
{
    char msg2[64];
    char lineBuffer[128];
    int lineIndex = 0;
    static char lastLine[128] = {0};  // 保存上一条完整字符串

    for (;;)
    {
        while (1)
        {
            int c = RingBuffer_ReadByte();
            if (c < 0) break;

            // 如果你希望调试时能看到字符流，可启用以下语句：
            // sprintf(msg2, "[RX] Char: 0x%02X ('%c')\r\n", c, (c >= 32 && c <= 126) ? c : '.');
            // HAL_UART_Transmit(&huart2, (uint8_t*)msg2, strlen(msg2), HAL_MAX_DELAY);

            if (c == ';')
            {
                if (lineIndex > 0)
                {
                    lineBuffer[lineIndex] = '\0';

                    // 比较是否和上一条一样
                    if (strcmp(lineBuffer, lastLine) != 0)
                    {
                        sprintf(msg2, ">>> Complete line: '%s'\r\n", lineBuffer);
                        HAL_UART_Transmit(&huart2, (uint8_t*)msg2, strlen(msg2), HAL_MAX_DELAY);

                        // 更新 lastLine
                        strncpy(lastLine, lineBuffer, sizeof(lastLine));
                        lastLine[sizeof(lastLine) - 1] = '\0'; // 保证结尾安全

                        // 调用解析函数
                        parseDollarNumber(lineBuffer);
                    }

                    // 无论是否重复，都清空 lineIndex 准备下一轮
                    lineIndex = 0;
                }
            }
            else
            {
                if (lineIndex < (int)sizeof(lineBuffer) - 1)
                {
                    lineBuffer[lineIndex++] = (char)c;
                }
                else
                {
                    lineIndex = 0; // 缓冲区溢出时重置
                }
            }
        }

        osDelay(50); // 减少空转
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
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2); // 这一句很关键！必须调用 HAL 的处理逻辑
}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
/* GPIO Pin Definitions for SN74HC595 (修正后的引脚定义) */
#define SER_PIN    GPIO_PIN_6  // PB6 -> Serial Data Input
#define SRCLK_PIN  GPIO_PIN_6  // PA6 -> Shift Register Clock
#define RCLK_PIN   GPIO_PIN_7  // PA7 -> Latch Clock
#define SER_PORT   GPIOB
#define SRCLK_PORT GPIOA
#define RCLK_PORT  GPIOA

/* 修改后的GPIO初始化 */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*-- 统一配置所有控制引脚 --*/
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /* SER (PB6) */
    GPIO_InitStruct.Pin = SER_PIN;
    HAL_GPIO_Init(SER_PORT, &GPIO_InitStruct);

    /* SRCLK (PA6) */
    GPIO_InitStruct.Pin = SRCLK_PIN;
    HAL_GPIO_Init(SRCLK_PORT, &GPIO_InitStruct);

    /* RCLK (PA7) */
    GPIO_InitStruct.Pin = RCLK_PIN;
    HAL_GPIO_Init(RCLK_PORT, &GPIO_InitStruct);

    /* 初始状态设置 */
    HAL_GPIO_WritePin(SER_PORT, SER_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SRCLK_PORT, SRCLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RCLK_PORT, RCLK_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  UART 接收完成回调
  * @note   每收到 1 个字节就会进这个函数 (若 HAL_UART_Receive_IT 配置为1字节)
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        RingBuffer_WriteByte(rxData); // 写入环形缓冲区
        HAL_UART_Receive_IT(&huart2, &rxData, 1); // 重新启用中断
    }
}



/**
  * @brief  在给定字符串中搜索 '$' 成对出现的片段，将其转为整数，存到全局 number
  *         若找到多段，也可以只取第一段，或循环处理。
  */
void parseDollarNumber(const char *str)
{
    char debugMsg[64];
    sprintf(debugMsg, "[parseDollarNumber] Input: '%s'\r\n", str);
    HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), HAL_MAX_DELAY);

    const char *start = strchr(str, '$');
    if (!start) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"No start $ found\r\n", 17, HAL_MAX_DELAY);
        return;
    }

    const char *end = strchr(start + 1, '$');
    if (!end) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"No end $ found\r\n", 16, HAL_MAX_DELAY);
        return;
    }

    int length = end - (start + 1);
    if (length <= 0) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Invalid length\r\n", 16, HAL_MAX_DELAY);
        return;
    }

    char temp[32];
    if (length >= (int)sizeof(temp)) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Temp buffer overflow\r\n", 21, HAL_MAX_DELAY);
        return;
    }

    memcpy(temp, start + 1, length);
    temp[length] = '\0';

    int val = atoi(temp);
    sprintf(debugMsg, "Extracted: '%s' -> %d\r\n", temp, val);
    HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), HAL_MAX_DELAY);

    number = val;
}

/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}


