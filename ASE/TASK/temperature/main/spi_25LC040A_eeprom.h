#pragma once
#include "driver/spi_master.h"

#define SPI_MASTER_HOST SPI3_HOST
#define PIN_SPI_SS      5
#define PIN_SPI_SCLK    18
#define PIN_SPI_MOSI    23
#define PIN_SPI_MISO    19

#define CMD_READ 0x03
#define CMD_WRITE 0x02
#define CMD_WREN 0x06
#define CMD_WRDI 0x04
#define CMD_RDSR 0x05
#define CMD_WRSR 0x01

esp_err_t spi_25LC040_init(spi_host_device_t masterHostId,int csPin, int sckPin, int mosiPin, int misoPin,
                            int clkSpeedHz, spi_device_handle_t* devHandle);

esp_err_t spi_25LC040_free(spi_host_device_t masterHostId, spi_device_handle_t devHandle);

esp_err_t spi_25LC040_read_byte(spi_device_handle_t devHandle,
                                uint16_t address, uint8_t* pData);

esp_err_t spi_25LC040_write_byte(spi_device_handle_t devHandle,
                                 uint16_t address, uint8_t data);

esp_err_t spi_25LC040_write_page(spi_device_handle_t devHandle,
                                 uint16_t address, const uint8_t* pBuffer, uint8_t size);

esp_err_t spi_25LC040_write_enable(spi_device_handle_t devHandle);

esp_err_t spi_25LC040_write_disable(spi_device_handle_t devHandle);

esp_err_t spi_25LC040_read_status(spi_device_handle_t devHandle, uint8_t* pStatus);

esp_err_t spi_25LC040_write_status(spi_device_handle_t devHandle, uint8_t status);

esp_err_t spi_25LC040_dump(spi_device_handle_t devHandle);