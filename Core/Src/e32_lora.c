#include "e32_lora.h"

static int bypass_aux = 0;
static int bypass_aux_permanent = 0;




void e32_wait_aux(e32_t *e)
{

    uint32_t start = HAL_GetTick(); // Або millis() для Arduino

    // Чекаємо, поки AUX не стане HIGH (1)
    while (HAL_GPIO_ReadPin(e->aux_port, e->aux_pin) == GPIO_PIN_RESET) {

    	if (bypass_aux)  // Count timeout
    	{
    	    if ((HAL_GetTick() - start) >= bypass_aux) {
                break; // Помилка: вийшов час очікування
            }
    	}
    	HAL_Delay(1);
    }

    HAL_Delay(2);
    if (!bypass_aux_permanent) bypass_aux = 0; // Reset bypass_aux value if permanent is not set
}

void e32_normal(e32_t *e)
{
    HAL_GPIO_WritePin(e->m0_port, e->m0_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(e->m1_port, e->m1_pin, GPIO_PIN_RESET);

    HAL_Delay(10);
    e32_wait_aux(e);
}

void e32_config(e32_t *e)
{
    HAL_GPIO_WritePin(e->m0_port, e->m0_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(e->m1_port, e->m1_pin, GPIO_PIN_SET);

    HAL_Delay(10);
    e32_wait_aux(e);
}

HAL_StatusTypeDef e32_send_fixed(
    e32_t *e,
    uint16_t address,
    uint8_t channel,
    uint8_t *payload,
    uint16_t len
)
{
    uint8_t header[3];

    header[0] = address >> 8;
    header[1] = address & 0xFF;
    header[2] = channel;



    e32_wait_aux(e);

    if (HAL_UART_Transmit(e->huart, header, 3, 200) != HAL_OK)
        return HAL_ERROR;

    if (HAL_UART_Transmit(e->huart, payload, len, 300) != HAL_OK)
        return HAL_ERROR;

    e32_wait_aux(e);

    return HAL_OK;
}

HAL_StatusTypeDef e32_send_fixed2(
    e32_t *e,
    uint16_t address,
    uint8_t channel,
    uint8_t *payload,
    uint16_t len
)
{
    uint8_t header[3];

    header[0] = address >> 8;
    header[1] = address & 0xFF;
    header[2] = channel;

    uint8_t buth[64];
    int ptr = 0;
    int i;

    for (i=0; i<3; i++)
    	buth[ptr++] = header[i];

    for (i=0; i<len; i++)
    	buth[ptr++] = payload[i];


    e32_wait_aux(e);

    if (HAL_UART_Transmit(e->huart, buth, ptr, 200) != HAL_OK)
        return HAL_ERROR;

    e32_wait_aux(e);

    return HAL_OK;
}


HAL_StatusTypeDef e32_receive(
    e32_t *e,
    uint8_t *data,
    uint16_t len,
    uint32_t timeout
)
{
    return HAL_UART_Receive(e->huart, data, len, timeout);
}

void e32_bypass_aux(int i_bypass_aux, int i_permanent)
{
	bypass_aux = i_bypass_aux; // Set bypass_aux mode
	bypass_aux_permanent = i_permanent; // No reset after wait execution
}

void e32_read_config(e32_t *e, uint8_t *lora_config) {

	uint8_t cmd_read_params[4] = {0xC1, 0xC1, 0xC1};
	e32_config(e);

    // 3. Відправляємо команду читання параметрів (3 байти 0xC1)
    HAL_UART_Transmit(e->huart, cmd_read_params, 3, 100);

    // 4. Отримуємо 6 байтів відповіді від модуля
    if (HAL_UART_Receive(e->huart, lora_config, 6, 500) == HAL_OK) {
        // Успішно прочитано! Дані знаходяться в масиві rx_params[0..5]
    }

   e32_normal(e);

}

void e32_restart(e32_t *e)
{
	uint8_t cmd_read_params[3] = {0xC4, 0xC4, 0xC4};

	//e32_config(e);

    // 3. Відправляємо команду читання параметрів (3 байти 0xC4)
    HAL_UART_Transmit(e->huart, cmd_read_params, 3, 100);

   // e32_normal(e);

}
