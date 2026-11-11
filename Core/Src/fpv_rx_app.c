#include "main.h"

#include "fpv_rx_app.h"
#include "fpv_protocol.h"
#include "e32_lora.h"
#include "ssd1306_min.h"

extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c1;

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

static detector_entry_t
detectors[DETECTOR_TABLE_SIZE];

static e32_t lora;

static uint8_t my_id = 0;

static uint32_t last_alarm_time = 0;
static uint32_t last_oled_update = 0;

static uint8_t read_receiver_dip_gnd(void)
{
    uint8_t v = 0;

    if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==GPIO_PIN_RESET) v|=1<<0;
    if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1)==GPIO_PIN_RESET) v|=1<<1;
    if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2)==GPIO_PIN_RESET) v|=1<<2;

    return v & 0x07;
}

static void buzzer_on(void)
{
    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_10,
        GPIO_PIN_SET);
}

static void buzzer_off(void)
{
    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_10,
        GPIO_PIN_RESET);
}

static void detector_table_update(
    fpv_packet_t *p)
{
    int free_index = -1;
    int oldest_index = 0;

    for(int i=0;i<DETECTOR_TABLE_SIZE;i++)
    {
        if(detectors[i].active &&
           detectors[i].detector_id ==
           p->detector_id)
        {
            detectors[i].threat =
                p->threat;

            detectors[i].rms =
                p->rms;

            detectors[i].fpv_score =
                p->fpv_score;

            detectors[i].noise_floor =
                p->noise_floor;

            detectors[i].last_seen =
                HAL_GetTick();

            return;
        }

        if(!detectors[i].active &&
           free_index < 0)
        {
            free_index = i;
        }

        if(detectors[i].last_seen <
           detectors[oldest_index].last_seen)
        {
            oldest_index = i;
        }
    }

    int idx =
        free_index >= 0 ?
        free_index :
        oldest_index;

    detectors[idx].active = 1;

    detectors[idx].detector_id =
        p->detector_id;

    detectors[idx].threat =
        p->threat;

    detectors[idx].rms =
        p->rms;

    detectors[idx].fpv_score =
        p->fpv_score;

    detectors[idx].noise_floor =
        p->noise_floor;

    detectors[idx].last_seen =
        HAL_GetTick();
}

static void draw_detector_table(void)
{
    ssd1306_clear();

    ssd1306_print(
        0,
        0,
        "RX");

    ssd1306_print_num(
        18,
        0,
        ":",
        my_id);

    for(int i=0;i<DETECTOR_TABLE_SIZE;i++)
    {
        uint8_t y =
            10 + i*9;

        if(!detectors[i].active)
        {
            ssd1306_print(
                0,
                y,
                "--");

            continue;
        }

        uint32_t age =
            (HAL_GetTick() -
            detectors[i].last_seen)
            /1000;

        ssd1306_print_num(
            0,
            y,
            "",
            detectors[i].detector_id);

        if(detectors[i].threat)
        {
            ssd1306_print(
                24,
                y,
                "ALR");
        }
        else
        {
            ssd1306_print(
                24,
                y,
                "OK");
        }

        ssd1306_print_num(
            54,
            y,
            "",
            detectors[i].fpv_score);

        ssd1306_print_num(
            96,
            y,
            "",
            age);
    }

    ssd1306_update();
}

void FPV_RX_App_Init(void)
{
    lora.huart = &huart1;

    lora.m0_port = GPIOB;
    lora.m0_pin  = GPIO_PIN_0;

    lora.m1_port = GPIOB;
    lora.m1_pin  = GPIO_PIN_1;

    lora.aux_port = GPIOB;
    lora.aux_pin  = GPIO_PIN_2;

    my_id =
        read_receiver_dip_gnd();

    e32_normal(&lora);

    ssd1306_init(&hi2c1);

    ssd1306_clear();

    ssd1306_print(
        0,
        0,
        "FPV RX READY");

    ssd1306_print_num(
        0,
        16,
        "ID:",
        my_id);

    ssd1306_update();
}

void FPV_RX_App_Loop(void)
{
    fpv_packet_t packet;

    if(e32_receive(
        &lora,
        (uint8_t*)&packet,
        FPV_PACKET_SIZE,
        50) == HAL_OK)
    {
        if(fpv_packet_check(&packet) &&
           packet.receiver_id == my_id)
        {
            detector_table_update(
                &packet);

            draw_detector_table();

            if(packet.threat)
            {
                buzzer_on();

                last_alarm_time =
                    HAL_GetTick();
            }
        }
    }

    if(HAL_GetTick() -
       last_alarm_time > 1500)
    {
        buzzer_off();
    }

    if(HAL_GetTick() -
       last_oled_update > 1000)
    {
        draw_detector_table();

        last_oled_update =
            HAL_GetTick();
    }
}
