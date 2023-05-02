#include "freertos/FreeRTOS.h"
#include "temp_sensor_TC74.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "spi_25LC040A_eeprom.h"


const char *TAG = "TC74";
spi_device_handle_t devHandle;

static void i2c_temperature_task(void *arg){
    esp_err_t ret = ESP_OK;

    // setup the EEPROM
    ret = spi_25LC040_init(SPI_MASTER_HOST, PIN_SPI_SS,PIN_SPI_SCLK, PIN_SPI_MOSI, PIN_SPI_MISO, 1000000, &devHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize EEPROM");
        return;
    }else{
        ESP_LOGI(TAG, "EEPROM initialized");
    }



    // setup the sensor
    ret =  tc74_init(I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return;
    }else{
        ESP_LOGI(TAG, "Sensor initialized");
    }
    
    uint16_t samplesize = 0x200;
    uint8_t temperature_value;
    uint8_t eepromStatus;
    uint8_t value = 0x00;
    uint16_t address = 0x00;
    spi_25LC040_write_status(devHandle,value);
    spi_25LC040_read_status(devHandle,&eepromStatus);
    ESP_LOGI(TAG, "value= 0x%.2x - EEPROM status: 0x%.2x", value,eepromStatus);

    while(address != samplesize){
        ESP_LOGI(TAG, "Reading temperature");
        // Wake up the sensor
        tc74_read_temp_after_temp(I2C_MASTER_NUM, TC74_SLAVE_ADDR, 1000 , &temperature_value);
        
        // write the temperature value in the EEPROM
        spi_25LC040_write_byte(devHandle,address,temperature_value);
        // read the temperature value from the EEPROM
        // spi_25LC040_read_byte(devHandle,address,&valueEprom);
        ESP_LOGI(TAG, "Address: 0x%.4x: Temperature 0x%.2x == 0x%.2x vEprom", address,temperature_value,temperature_value);
        address++;
    
        
        // put the sensor in standby mode
        tc74_standy(I2C_MASTER_NUM, TC74_SLAVE_ADDR, 1000 / portTICK_PERIOD_MS);
        // delay 2 second
        // vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "EEPROM full");

    // dump the EEPROM
    ret = spi_25LC040_dump(devHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to dump EEPROM");
        return;
    }else{
        ESP_LOGI(TAG, "EEPROM dumped");
    }

    // free the EEPROM
    ret = spi_25LC040_free(I2C_MASTER_NUM, devHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to free EEPROM");
        return;
    }else{
        ESP_LOGI(TAG, "EEPROM freed");
    }

    vTaskDelete(NULL); // terminate task
}


void app_main(void)
{
    xTaskCreate(i2c_temperature_task, "i2c_temperature_task", 2048,(void *)0, 10, NULL);
    
}

