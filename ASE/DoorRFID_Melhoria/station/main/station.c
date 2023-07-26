#include <esp_log.h>
#include <inttypes.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

// own libraries
#include "rc522.h"
#include "leds.h"
#include <time.h>
#include "server.h"
#include "api.h"
#include "uart.h"
#include "buzzer.h"

// Global variables to store SSID and password
char ssid[32];
char password[64];

static const char* TAG = "MAIN";
static rc522_handle_t scanner;

bool acess = false;
bool hasVariables = false;  // Variable to track if all variables are obtained

bool configuration = true;

// Semaphore to control the access to the system
SemaphoreHandle_t mySemaphore;
char rfid[32];  // Assuming the string length is sufficient for the number

void startAplication(){
    int counter = 0;

    while (counter < 4) {
        // blink leds every 500ms to show that the program is running
        play_sound(5000,500);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        counter++;
    }
    play_sound(7500,2000);
}

void config(){
    // Get ESP_WIFI_SSID and ESP_WIFI_PASS from the terminal

    printf("Enter SSID: ");
    uart_read_bytes(UART_NUM_1, (uint8_t*)ssid, sizeof(ssid)-1, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(100));  // Delay for 100 milliseconds
    ssid[sizeof(ssid)-1] = '\0';  // Null-terminate the string
    printf("SSID: %s\n", ssid);
}

bool addallowed_tag(uint64_t tag_id){
    // verify if the tag is already in the array
    if (tag_id == 0) {
        play_sound(1000,1000);
        return false;
    }
    if(api_isuser(tag_id)){
        ESP_LOGW(TAG, "Tag already in the array");
        play_sound(1000,1000);
        return false;
    }else{
        ESP_LOGW(TAG, "Tag added from the array");
        api_adduser(tag_id);
        play_sound(4000, 1000);
    }
    return true;
}

bool remove_Tag(uint64_t tag_id){
    // verify if the tag is already in the array
    if (tag_id == 0) {
        play_sound(1000,1000);
        return false;
    }
    if(api_isuser(tag_id)){
        ESP_LOGW(TAG, "Tag already in the array");
        api_deluser(tag_id);
        play_sound(4000, 1000);
        return true;
    }else{
        ESP_LOGW(TAG, "Tag not in the array");
        play_sound(1000,1000);
        return false;
    }
}

static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data)
{
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;
    // if config = true then add
    // if config = false then remove
    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
            if(config_status != 2) {
                rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
                ESP_LOGI(TAG, "Tag -> (%" PRIu64 ")", tag->serial_number);
                if( configuration ){
                    addallowed_tag(tag->serial_number);
                }else{
                    remove_Tag(tag->serial_number);
                }
            }
        }
        default:
            break;
    }   
}

void getCredentials(){
    printf("Enter SSID: ");
    get_string(ssid, true);
    printf("Enter password: ");
    get_string(password, false);
    printf("--------------------\n");
    printf("SSID: %s\n", ssid);
    printf("Password: %s\n", password);
    printf("--------------------\n");
}

void taskbuttron(){
    gpio_set_direction(33, GPIO_MODE_INPUT);
    while(1){
        if(gpio_get_level(33) == 0){
            // pass
        }else{ // button pressed
            configuration = !configuration;
            play_sound(5000, 500);
            ESP_LOGI(TAG, "Configuration mode: %d", configuration);
            if(configuration){
                turn_on_led(LED_RED);
                turn_off_led(LED_GREEN);
            }else{
                turn_on_led(LED_GREEN);
                turn_off_led(LED_RED);
            }

        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
void app_main()
{
    // configure uart
    uart_init();

        

    // configure buzzer
    configure_buzzer();

    // BLUE LED ON
    turn_on_led(17);

    // configure leds
    configure_leds();

    // configure rc522
    rc522_config_t config = {
        .spi.host = VSPI_HOST,
        .spi.miso_gpio = 25,
        .spi.mosi_gpio = 23,
        .spi.sck_gpio = 19,
        .spi.sda_gpio = 26,
    };
    rc522_create(&config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    rc522_start(scanner);

    // task
    xTaskCreate(taskbuttron, "tasknormalmode", 4096, NULL, 5, NULL);

    // get credentials
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    getCredentials();

    // configure server
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    printf("Starting server\n");
    connect_wifi(ssid, password);
    setup_server();

    startAplication();
}