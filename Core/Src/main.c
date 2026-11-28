#include "main.h"
#include "fpv_protocol.h"
#include "e32_lora.h"
#include "ssd1306_min.h"
#include "ssd1306.h"
#include "fonts.h"
#include "e32_config.h"
//#include "alarm.h"
#include "delayer.h"
#include "button2.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


UART_HandleTypeDef huart1;
I2C_HandleTypeDef hi2c1;

#define DETECTOR_TABLE_SIZE 6

typedef struct
{
    uint8_t detector_id;
    uint8_t threat;
    uint16_t rms;
    uint16_t fpv_score;
    uint16_t noise_floor;
    uint32_t last_seen;
    uint8_t active;
} detector_entry_t;

detector_entry_t detectors[DETECTOR_TABLE_SIZE];

static delay_t delay;
static button_t button;
static uint8_t bstate = 0;

static uint8_t muted = 0;


__weak void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */
  if (GPIO_Pin != button.h_button_pin)
	  return;

  uint8_t pressed = button_pressed(&button); // 0 - unpressed, 1 - pressed
  button_irq_handler(&button, pressed);
}



void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);

static uint8_t read_receiver_dip(void)
{
    uint8_t v = 0;

    if (HAL_GPIO_ReadPin(ADDR0_RX_GPIO_Port, ADDR0_RX_Pin) == GPIO_PIN_SET) v |= 1 << 0;
    if (HAL_GPIO_ReadPin(ADDR1_RX_GPIO_Port, ADDR1_RX_Pin) == GPIO_PIN_SET) v |= 1 << 1;
    if (HAL_GPIO_ReadPin(ADDR2_RX_GPIO_Port, ADDR2_RX_Pin) == GPIO_PIN_SET) v |= 1 << 2;

    return v & 0x07;
}


static void buzzer_on(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
}

static void buzzer_off(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}


static void detector_table_update(fpv_packet_t *p)
{
    int free_index = -1;
    int oldest_index = 0;

    for (int i = 0; i < DETECTOR_TABLE_SIZE; i++)
    {
        if (detectors[i].active && detectors[i].detector_id == p->detector_id)
        {
            detectors[i].threat = p->threat;
            detectors[i].rms = p->rms;
            detectors[i].fpv_score = p->fpv_score;
            detectors[i].noise_floor = p->noise_floor;
            detectors[i].last_seen = HAL_GetTick();
            return;
        }

        if (!detectors[i].active && free_index < 0)
            free_index = i;

        if (detectors[i].last_seen < detectors[oldest_index].last_seen)
            oldest_index = i;
    }

    int idx = free_index >= 0 ? free_index : oldest_index;

    detectors[idx].active = 1;
    detectors[idx].detector_id = p->detector_id;
    detectors[idx].threat = p->threat;
    detectors[idx].rms = p->rms;
    detectors[idx].fpv_score = p->fpv_score;
    detectors[idx].noise_floor = p->noise_floor;
    detectors[idx].last_seen = HAL_GetTick();
}

static void draw_detector_table(uint8_t my_id)
{
    ssd1306_clear();

    ssd1306_print(0, 0, "RX:");
    ssd1306_print_num(20, 0, "", my_id);
    ssd1306_print(42, 0, "ID S SCORE");
	ssd1306_update();


    for (int i = 0; i < DETECTOR_TABLE_SIZE; i++)
    {
        uint8_t y = 10 + i * 9;

        if (!detectors[i].active)
        {
        	ssd1306_print(0, y, "--");
            continue;
        }

        uint32_t age = (HAL_GetTick() - detectors[i].last_seen) / 1000;

        ssd1306_print_num(0, y, "", detectors[i].detector_id);

        char msg[5];


        if (detectors[i].threat)
            sprintf(msg,"ALR");
        else
        	sprintf(msg,"OK ");

        if (muted)
        	sprintf(msg,"X%2d",muted);

        ssd1306_print(24, y, msg);

        ssd1306_print_num(54, y, "", detectors[i].fpv_score);
        ssd1306_print_num(100, y, "", age);
    }

    ssd1306_update();
}

int main(void)
{

	HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();



    e32_t lora =
    {
        .huart = &huart1,
        .m0_port = LoRa_M0_GPIO_Port,
        .m0_pin = LoRa_M0_Pin,
        .m1_port = LoRa_M1_GPIO_Port,
        .m1_pin = LoRa_M1_Pin,
        .aux_port = LoRa_AUX_GPIO_Port,
        .aux_pin = LoRa_AUX_Pin
    };

    //e32_wait_aux(&lora);

	e32_write_config_fixed(&lora, 2, 0x17);

    ssd1306_init(&hi2c1);

    ssd1306_clear();
    ssd1306_set_current_font(SSD1306_MEDIUM_FONT,1);
    ssd1306_print(0, 0, "FPV RX");
    ssd1306_print(0,19, "READY !!!");

    ssd1306_update();
    HAL_Delay(500);

    ssd1306_set_current_font(SSD1306_DEFAULT_FONT,0);

    uint8_t my_id = read_receiver_dip();

    uint32_t last_alarm_time = 0;
    uint32_t last_oled_update = 0;


    button.h_button_port = GPIOB;
    button.h_button_pin = GPIO_PIN_13;

    button.s_max_pressed_count = 3;

    init_button(&button);

    //alarm_init();
    //uint8_t g_rx_byte;
    //HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);



    while (1)
    {
        fpv_packet_t packet;

        if (e32_receive(&lora, (uint8_t *)&packet, FPV_PACKET_SIZE, 100) == HAL_OK)
        {
            if (fpv_packet_check(&packet))
            {
                if (packet.receiver_id == my_id)
                {
                    detector_table_update(&packet);
                    draw_detector_table(my_id);

                    if (packet.threat && !muted)
                    {
                        buzzer_on();
                        last_alarm_time = HAL_GetTick();
                    }

                    //alarm_update(packet.seq, packet.threat);

                }


            }
        }

        if (HAL_GetTick() - last_alarm_time > 1500)
            buzzer_off();

        if (HAL_GetTick() - last_oled_update > 1000)
        {
            draw_detector_table(my_id);
            last_oled_update = HAL_GetTick();
        }

        button_query(&button);
        bstate = button_result(&button);

        if (bstate)
        {
            if (bstate >=4 )
            {
                reset_delay(&delay);
                muted = 0;
            }
            else
            {
            	muted = bstate*15;  // Once - 15 sec, Twice - 30 sec, Three times - 45 sec, HOLD - unmute
            	set_delay(&delay, muted*1000);
            }

        }

        if (get_delay(&delay))
        	muted = 0;



      //  alarm_button_poll();
    }

}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
        Error_Handler();
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
        Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOB, LoRa_M0_Pin | LoRa_M1_Pin | BUZZER_Pin, GPIO_PIN_RESET);

    // Set all to 1
    GPIO_InitStruct.Pin = ADDR0_RX_Pin | ADDR1_RX_Pin | ADDR2_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LoRa_M0_Pin | LoRa_M1_Pin | BUZZER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LoRa_AUX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : KNOBKA_Int_Pin */
  GPIO_InitStruct.Pin = KNOBKA_Int_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KNOBKA_Int_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);


}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
