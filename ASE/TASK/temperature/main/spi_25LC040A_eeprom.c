#include "spi_25LC040A_eeprom.h"
#include "esp_log.h"
#include "string.h"

esp_err_t spi_tx_data(spi_device_handle_t spiHandle, const uint8_t *pBuffer, int bufSize){
    esp_err_t ret = ESP_OK;
    spi_transaction_t spiTrans;

    if (bufSize <= 0)
    {
        return ESP_FAIL;
    }
    memset(&spiTrans, 0, sizeof(spiTrans));
    spiTrans.length = bufSize * 8;
    spiTrans.tx_buffer = pBuffer;
    ret = spi_device_polling_transmit(spiHandle, &spiTrans);
    while (1)
    {
        uint8_t status;
        spi_25LC040_read_status(spiHandle, &status);
        if ((status & 0x01) == 0)
        {
            break;
        }
    }
    return ret;
}

esp_err_t spi_tx_rx_data(spi_device_handle_t spiHandle, const uint8_t *pTxBuffer, int txBufSize, uint8_t *pRxBuffer, int rxBufSize)
{
    esp_err_t ret = ESP_OK;
    spi_transaction_t spiTrans;

    if (txBufSize <= 0)
    {
        return ESP_FAIL;
    }
    // printf("txBufSize: %d rxBufSize: %d\n\r ", txBufSize, rxBufSize);
    memset(&spiTrans, 0, sizeof(spiTrans));
    // spiTrans.length = (txBufSize + rxBufSize) * 8;
    spiTrans.length = txBufSize * 8;    
    spiTrans.tx_buffer = pTxBuffer;
    spiTrans.rxlength = rxBufSize * 8;
    spiTrans.rx_buffer = pRxBuffer;
        
    ret = spi_device_polling_transmit(spiHandle, &spiTrans);
    return ret;
}

esp_err_t spi_25LC040_init(spi_host_device_t masterHostId, int csPin, int sckPin, int mosiPin, int misoPin, int clkSpeedHz, spi_device_handle_t *devHandle){
    esp_err_t ret = ESP_OK;

    spi_bus_config_t spiBusCfg = {
        .mosi_io_num = mosiPin,
        .miso_io_num = misoPin,
        .sclk_io_num = sckPin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
        .flags = SPICOMMON_BUSFLAG_MASTER};

    spi_device_interface_config_t spiDeviceCfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .clock_speed_hz = clkSpeedHz,
        .spics_io_num = csPin,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX};

    ret = spi_bus_initialize(masterHostId, &spiBusCfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK)
    {
        return ret;
    }
    ret = spi_bus_add_device(masterHostId, &spiDeviceCfg, devHandle);
    if (ret != ESP_OK)
    {
        return ret;
    }
    return ret;
}

esp_err_t spi_25LC040_write_enable(spi_device_handle_t devHandle){

    esp_err_t ret = ESP_OK;
    uint8_t txData = CMD_WREN;

    ret = spi_tx_data(devHandle, &txData, sizeof(txData));
    if (ret != ESP_OK)
    {
        return ret;
    }
    return ret;
}

esp_err_t spi_25LC040_write_disable(spi_device_handle_t devHandle){
    esp_err_t ret = ESP_OK;
    uint8_t txData = CMD_WRDI;

    ret = spi_tx_data(devHandle, &txData, sizeof(txData));
    if (ret != ESP_OK)
    {
        return ret;
    }
    return ret;
}

esp_err_t spi_25LC040_read_status(spi_device_handle_t devHandle, uint8_t *pStatus)
{
    esp_err_t ret = ESP_OK;
    uint8_t txData = CMD_RDSR;
    uint8_t rxData;

    ret = spi_tx_rx_data(devHandle, &txData, sizeof(txData), &rxData, sizeof(rxData));
    if (ret != ESP_OK)
    {
        return ret;
    }
    *pStatus = rxData;
    return ret;
}

esp_err_t spi_25LC040_write_status(spi_device_handle_t devHandle, uint8_t status){
    esp_err_t ret = ESP_OK;
    uint8_t txData[2];
    txData[0] = CMD_WRSR;
    txData[1] = status;

    ret = spi_25LC040_write_enable(devHandle);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = spi_tx_data(devHandle, txData, sizeof(txData));
    if (ret != ESP_OK)
    {
        return ret;
    }
    while (1)
    {
        ret = spi_25LC040_read_status(devHandle, &status);
        if (ret != ESP_OK)
        {
            return ret;
        }
        if ((status & 0x01) == 0)
        {
            break;
        }
    }
    return ret;
}

esp_err_t spi_25LC040_free(spi_host_device_t masterHostId, spi_device_handle_t devHandle){
    esp_err_t ret = ESP_OK;
    ret = spi_bus_remove_device(devHandle);
    if (ret != ESP_OK)
    {
        return ret;
    }
    ret = spi_bus_free(masterHostId);
    if (ret != ESP_OK)
    {
        return ret;
    }
    return ret;
}

esp_err_t spi_25LC040_read_byte(spi_device_handle_t devHandle, uint16_t address, uint8_t *pData)
{
    esp_err_t ret = ESP_OK;
    uint8_t txData[2]; 
    uint8_t rxData;
    // read command
    txData[0] = CMD_READ;
    // address
    if (address & 0x100)
    {
        txData[0] |= 0x08;
    }
    txData[1] = address & 0xFF;
    // printf("READ_BYTE\n\r");
    ret = spi_tx_rx_data(devHandle, txData, sizeof(txData), &rxData, sizeof(rxData));
    if (ret != ESP_OK)
    {
        return ret;
    }
    *pData = rxData;
    return ret;
}

esp_err_t spi_25LC040_write_byte(spi_device_handle_t devHandle, uint16_t address, uint8_t data){
    esp_err_t ret = ESP_OK;
    uint8_t txData[3];
    // read command
    txData[0] = CMD_WRITE;
    // address
    if (address & 0x100)
    {
        txData[0] |= 0x08;
    }
    txData[1] = address & 0xFF;
    txData[2] = data;
    ret = spi_25LC040_write_enable(devHandle);
    if (ret != ESP_OK)
    {
        return ret;
    }
    ret = spi_tx_data(devHandle, txData, sizeof(txData));
    if (ret != ESP_OK)
    {
        return ret;
    }
    return ret;
}

esp_err_t spi_25LC040_write_page(spi_device_handle_t devHandle, uint16_t address, const uint8_t *pBuffer, uint8_t size){
    esp_err_t ret = ESP_OK;
    uint8_t txData[3];
    // read command
    txData[0] = CMD_WRITE;
    // address
    if (address & 0x100)
    {
        txData[0] |= 0x08;
    }
    txData[1] = address & 0xFF;
    ret = spi_25LC040_write_enable(devHandle);
    if (ret != ESP_OK)
    {
        return ret;
    }
    ret = spi_tx_data(devHandle, txData, sizeof(txData));
    if (ret != ESP_OK)
    {
        return ret;
    }
    ret = spi_tx_data(devHandle, pBuffer, size);
    if (ret != ESP_OK)
    {
        return ret;
    }
    return ret;
}

esp_err_t spi_25LC040_dump(spi_device_handle_t devHandle){
    esp_err_t ret = ESP_OK;
    uint8_t data;
    for (uint16_t i = 0; i < 0x200; i++)
    {
        ret = spi_25LC040_read_byte(devHandle, i, &data);
        if (ret != ESP_OK)
        {
            return ret;
        }
        // print the value in decimal
        printf("%03d ", data);
        if ((i & 0x0F) == 0x0F)
        {
            printf("\r\n");
        }
    }
    return ret;
}