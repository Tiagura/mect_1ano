#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/ledc.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/uart.h"


static const char *TAG = "OUTPUT";

uint8_t led_state = 0;

#define PIN_OUT 5
#define PIN_IN 4

// Tasks SEND stuff
#define STACK_SIZE 200
StaticTask_t task_send_buffer;
StackType_t task_send_stack[STACK_SIZE];

int setup_uart() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // check issue E (325) uart: uart_param_config(748): Invalid src_clk
    uart_param_config(UART_NUM_0, &uart_config);
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0));

    return 0;
}

char get_char() {
    // flush stdout to make sure prints before this function are printed
    fflush(stdout);

    char data[1];
    uint32_t len = 0;
    while (1) {
        len = uart_read_bytes(UART_NUM_0, (uint8_t*)data, 1, 100);

        // new line is being sent as 0x0d (CR) instead of 0x0a (LF)
        if (data[0] == 0x0d) data[0] = 0x0a;

        // print char back to terminal for readability purposes
        // only print and return when there is something there
        if (len > 0) {
            printf("%c", data[0]);
            return data[0];
        }
    }
    
    return '\0';
}

int get_string(char *data) {
    char c;
    uint32_t i = 0;
    while ((c = get_char()) != '\n') {
        data[i++] = c;
    }
    // add null terminator
    data[i] = '\0';

    // print newline for readability purposes
    printf("\n");

    return 0;
}

int get_freq() {
    uint16_t freq = 0;
    while (freq < 1 || freq > 100) {
        printf("Freq (1-100Hz): ");
        
        char data[3];
        get_string(data);

        freq = atoi(data);
        
        if (freq < 1 || freq > 100)
            printf("Invalid frequency value\n\n");
    }
    return freq;
}

void setup_pins(){
    //configure PIN OUTPUT
    gpio_reset_pin(PIN_OUT);
    gpio_set_direction(PIN_OUT, GPIO_MODE_OUTPUT);

    //configure PIN INPUT
    gpio_reset_pin(PIN_IN);
    gpio_set_direction(PIN_IN, GPIO_MODE_INPUT);
}

void output_task(void *pvParameters){
    //turn LED on and off with the signal frequency
    int freq = get_freq();
    float period = 1.0 / freq;
    ESP_LOGI(TAG, "Period: %f", period);
    ESP_LOGI(TAG, "Frequency: %d", freq);

    while (1)
    {
        led_state = !led_state;
        gpio_set_level(PIN_OUT, led_state);
        vTaskDelay(period * 1000 / portTICK_PERIOD_MS);
    }
}

void input_task(void *pvParameters){
    uint32_t start_time = 0;
    uint32_t current_time = 0;

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));

        //if(polling_sig==1){
            while (gpio_set_level(PIN_IN) == 0)
            
            start_time = xTaskGetTickCountFromISR();

            while (gpio_set_level(PIN_IN) == 1)

            while (gpio_set_level(PIN_IN) == 0)

            current_time = xTaskGetTickCountFromISR();

            uint32_t freq = 1000 / (current_time - start_time) / 10;

            polling_signal = 0;

            printf("Frequency: %lu Hz", freq);
        //}
    }

    vTaskDelete(NULL);
    
}

void app_main(void)
{
    setup_uart();
    setup_pins();

    while (1)
    {
        xTaskCreate(output_task, "freq_send", STACK_SIZE, NULL, 10, NULL);
        xTaskCreate(input_task, "freq_recv", STACK_SIZE, NULL, 10, NULL);
    }
}