#include "temp_sensor_TC74.h"


// private functions
static esp_err_t tc74_mode_config(i2c_port_t i2cPort,uint8_t sensAddr,uint8_t mode, TickType_t timeOut)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, sensAddr << 1 | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, TC74_CONF_REG, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, mode, I2C_ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2cPort, cmd, timeOut / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t tc74_mode_read(i2c_port_t i2cPort,uint8_t sensAddr, uint8_t *mode, TickType_t timeOut)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, sensAddr << 1 | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, TC74_CONF_REG, I2C_ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, sensAddr << 1 | I2C_READ_BIT, I2C_ACK_CHECK_EN);
    i2c_master_read_byte(cmd, mode, I2C_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2cPort, cmd, timeOut / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}


// public functions
esp_err_t tc74_init(i2c_port_t i2cPort, int sdaPin, int sclPin, uint32_t clkSpeedHz) {
    i2c_config_t conf;
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = sdaPin;
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_io_num       = sclPin;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = clkSpeedHz;

    esp_err_t ret = i2c_param_config(i2cPort, &conf);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = i2c_driver_install(i2cPort, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    return ESP_OK;
}

esp_err_t tc74_free(i2c_port_t i2cPort){
    return i2c_driver_delete(i2cPort);
}

esp_err_t tc74_standy(i2c_port_t i2cPort, uint8_t sensAddr, TickType_t timeOut){
    // uint8_t operation_mode;
    // tc74_mode_read(I2C_MASTER_NUM, sensAddr, &operation_mode, timeOut);
    // printf("1 Operation mode: 0x%.2x\n\r", operation_mode);
    esp_err_t ret = tc74_mode_config(i2cPort, sensAddr, TC74_CONF_STANDBY, timeOut);
    // tc74_mode_read(I2C_MASTER_NUM, sensAddr, &operation_mode, timeOut);
    // printf("2 Operation mode: 0x%.2x\n\r", operation_mode);
    return ret;
}

esp_err_t tc74_wakeup(i2c_port_t i2cPort, uint8_t sensAddr, TickType_t timeOut){
    return tc74_mode_config(i2cPort, sensAddr, TC74_CONF_NORMAL, timeOut);
}

bool tc74_is_temperature_ready(i2c_port_t i2cPort, uint8_t sensAddr, TickType_t timeOut){
    uint8_t operation_mode;
    tc74_mode_read(i2cPort, sensAddr, &operation_mode, timeOut);
    return (operation_mode & TC74_CONF_RDY) == TC74_CONF_RDY;
}

esp_err_t tc74_wakeup_and_read_temp(i2c_port_t i2cPort, uint8_t sensAddr,TickType_t timeOut, uint8_t* pData){
    esp_err_t ret = tc74_wakeup(i2cPort, sensAddr, timeOut);
    if (ret != ESP_OK) {
        return ret;
    }

    while (!tc74_is_temperature_ready(i2cPort, sensAddr, timeOut)) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, sensAddr << 1 | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, TC74_TEMP_REG, I2C_ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, sensAddr << 1 | I2C_READ_BIT, I2C_ACK_CHECK_EN);
    i2c_master_read_byte(cmd, pData, I2C_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2cPort, cmd, timeOut / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t tc74_read_temp_after_temp(i2c_port_t i2cPort, uint8_t sensAddr,TickType_t timeOut, uint8_t* pData){
    // read 3 times with 100ms interval to get the average value of the temperature
    uint8_t temp[3];
    esp_err_t ret = ESP_OK;
    for (int i = 0; i < 3; i++) {
        ret = tc74_wakeup_and_read_temp(i2cPort, sensAddr, timeOut, &temp[i]);
        if (ret != ESP_OK) {
            return ret;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    // get average value
    
    *pData = (temp[0] + temp[1] + temp[2]) / 3;
    printf("Temperatura : %d %d %d -> %d\n\r", temp[0], temp[1], temp[2], *pData);
    return ret;
}

esp_err_t tc74_read_temp_after_cfg(i2c_port_t i2cPort, uint8_t sensAddr,TickType_t timeOut, uint8_t* pData){
    esp_err_t ret = ESP_OK;
    while (!tc74_is_temperature_ready(i2cPort, sensAddr, timeOut)) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, sensAddr << 1 | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, TC74_TEMP_REG, I2C_ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, sensAddr << 1 | I2C_READ_BIT, I2C_ACK_CHECK_EN);
    i2c_master_read_byte(cmd, pData, I2C_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2cPort, cmd, timeOut / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}