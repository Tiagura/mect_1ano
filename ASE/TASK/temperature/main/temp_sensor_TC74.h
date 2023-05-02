#pragma once
#include "driver/i2c.h"
#include "esp_log.h"
#include "driver/gpio.h"

extern const char* TAG;

#define I2C_MASTER_SCL_IO GPIO_NUM_22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_21               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM    I2C_NUM_0                 /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000                   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */

#define TC74_CONF_REG 0x01                    /*!< I2C master configuration register */
#define TC74_CONF_STANDBY 0x80                /*!< I2C master configuration standby */
#define TC74_CONF_NORMAL 0x00                 /*!< I2C master configuration normal */
#define TC74_SLAVE_ADDR 0x4D                  /*!< I2C master slave address */ 
#define TC74_CONF_RDY 0x40                    /*!< I2C master configuration ready */
#define TC74_TEMP_REG 0x00                    /*!< I2C master temperature register */

#define I2C_ACK_CHECK_EN 0x1                            /*!< I2C master will check ack from slave*/
#define I2C_ACK_CHECK_DIS 0x0                           /*!< I2C master will not check ack from slave */
#define I2C_ACK_VAL 0x0                                 /*!< I2C ack value */
#define I2C_NACK_VAL 0x1                                /*!< I2C nack value */
#define I2C_WRITE_BIT I2C_MASTER_WRITE                  /*!< I2C master write */
#define I2C_READ_BIT  I2C_MASTER_READ                   /*!< I2C master read */

esp_err_t tc74_init(i2c_port_t i2cPort, int sdaPin, int sclPin, uint32_t clkSpeedHz);

esp_err_t tc74_free(i2c_port_t i2cPort);

esp_err_t tc74_standy(i2c_port_t i2cPort, uint8_t sensAddr, TickType_t timeOut);

esp_err_t tc74_wakeup(i2c_port_t i2cPort, uint8_t sensAddr, TickType_t timeOut);

bool tc74_is_temperature_ready(i2c_port_t i2cPort, uint8_t sensAddr, TickType_t timeOut);

esp_err_t tc74_wakeup_and_read_temp(i2c_port_t i2cPort, uint8_t sensAddr,TickType_t timeOut, uint8_t* pData);

esp_err_t tc74_read_temp_after_cfg(i2c_port_t i2cPort, uint8_t sensAddr,TickType_t timeOut, uint8_t* pData);

esp_err_t tc74_read_temp_after_temp(i2c_port_t i2cPort, uint8_t sensAddr,TickType_t timeOut, uint8_t* pData);
