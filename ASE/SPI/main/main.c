#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "inttypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#define SPI_MASTER_HOST SPI3_HOST

#define PIN_SPI_MOSI 23
#define PIN_SPI_MISO 19
#define PIN_SPI_CLK 18
#define PIN_SPI_CS 5

void spi_tx_data(spi_device_handle_t spiHandle, const uint8_t *pBuffer, int buffSize)
{
    esp_err_t ret;
    spi_transaction_t spiTrans;

    if(buffSize <= 0){
        return;
    }
    memset(&spiTrans, 0, sizeof(spiTrans));
    spiTrans.length = buffSize * 8;
    spiTrans.tx_buffer = pBuffer;
    ret = spi_device_polling_transmit(spiHandle, &spiTrans);
    assert (ret == ESP_OK);
}

void spi_tx_rx_data(spi_device_hande_t spiHandle, const uint8_t *pTxBuffer, int txBuffSize, uint8_t * pRxBuffer, int rxBuffSize){
    esp_err_t ret;
    spi_transaction_t spiTrans;

    if(txBuffSize <= 0){
        return;
    }
    memset(&spiTrans, 0, sizeof(spiTrans));
    spiTrans.length = (txBuffSize + rxBuffSize) * 8;
    spiTrans.tx_buffer = pTxBuffer;
    spiTrans.rxlength = rxBuffSize * 8;
    spiTrans.rx_buffer = pRxBuffer;
    ret = spi_device_polling_transmit(spiHandle, &spiTrans);

    assert (ret == ESP_OK);
}

void app_main(void)
{
    esp_err_t ret;
    spi_device_handle_t spiHandle;

    spi_bus_config_t spiBusCfg = {
        .miso_io_num = PIN_SPI_MOSI,
        .mosi_io_num = PIN_SPI_MISO,
        .sclk_io_num = PIN_SPI_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };

    spi_device_interface_config_t masterCfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .clock_speed_hz = 1000000,
        .spics_io_num = PIN_SPI_CS,
        .queue_size = 1,
    };

    ret = spi_bus_initialize(SPI_MASTER_HOST, &spiBusCfg, SPI_DMA_DISABLED);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(SPI_MASTER_HOST, &masterCfg, &spiHandle);
    ESP_ERROR_CHECK(ret);

    uint8_t txData1[2] = {0x01, 0x02};

    spi_tx_data(spiHandle, txData1, sizeof(txData1));

    uint8_t txData2 = 0x05;
    uint8_t rxData;

    while(1){
        //spi_tx_data(spiHandle, &txData1, sizeof(txData1));
        spi_tx_rx_data(spiHandle, &txData2, sizeof(txData2), &rxData, sizeof(rxData));
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}