#pragma once
#ifdef USE_HARDWARE_SENSOR

#include <cstring>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

class SPIDriver {
public:
    explicit SPIDriver(const char *spiDevicePath);
    ~SPIDriver();

    int8_t write(uint8_t        registerAddress,
                 const uint8_t *dataBuffer,
                 uint16_t       length);

    int8_t read(uint8_t   registerAddress,
                uint8_t  *dataBuffer,
                uint16_t  length);

    int8_t transfer(const uint8_t *transmitBuffer,
                    uint8_t       *receiveBuffer,
                    uint16_t       length);

private:
    int      fileDescriptor_ = -1;
    uint32_t clockSpeed_     = 5000000;
    uint8_t  spiMode_        = SPI_MODE_0;
    uint8_t  bitsPerWord_    = 8;
};

inline SPIDriver::SPIDriver(const char *spiDevicePath) {
    fileDescriptor_ = open(spiDevicePath, O_RDWR);
    if (fileDescriptor_ < 0) { perror("[SPIDriver] open"); return; }

    if (ioctl(fileDescriptor_, SPI_IOC_WR_MODE,          &spiMode_)    < 0)
        perror("[SPIDriver] set mode");
    if (ioctl(fileDescriptor_, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord_) < 0)
        perror("[SPIDriver] set bits per word");
    if (ioctl(fileDescriptor_, SPI_IOC_WR_MAX_SPEED_HZ,  &clockSpeed_) < 0)
        perror("[SPIDriver] set clock speed");
}

inline SPIDriver::~SPIDriver() {
    if (fileDescriptor_ >= 0) close(fileDescriptor_);
}

inline int8_t SPIDriver::transfer(const uint8_t *transmitBuffer,
                                   uint8_t       *receiveBuffer,
                                   uint16_t       length) {

    struct spi_ioc_transfer spiTransfer;
    memset(&spiTransfer, 0, sizeof(spiTransfer));
    spiTransfer.tx_buf        = reinterpret_cast<uint64_t>(transmitBuffer);
    spiTransfer.rx_buf        = reinterpret_cast<uint64_t>(receiveBuffer);
    spiTransfer.len           = length;
    spiTransfer.speed_hz      = clockSpeed_;
    spiTransfer.bits_per_word = bitsPerWord_;

    if (ioctl(fileDescriptor_, SPI_IOC_MESSAGE(1), &spiTransfer) < 0) {
        perror("[SPIDriver] transfer");
        return 1;
    }
    return 0;
}

inline int8_t SPIDriver::write(uint8_t        registerAddress,
                                const uint8_t *dataBuffer,
                                uint16_t       length) {

    uint8_t writeAddress = registerAddress & 0x7F;

    std::vector<uint8_t> transmitBuffer(length + 1);
    std::vector<uint8_t> receiveBuffer (length + 1);
    transmitBuffer[0] = writeAddress;
    std::memcpy(transmitBuffer.data() + 1, dataBuffer, length);

    return transfer(transmitBuffer.data(),
                    receiveBuffer.data(),
                    static_cast<uint16_t>(transmitBuffer.size()));
}

inline int8_t SPIDriver::read(uint8_t   registerAddress,
                               uint8_t  *dataBuffer,
                               uint16_t  length) {

    uint8_t readAddress = registerAddress | 0x80;

    std::vector<uint8_t> transmitBuffer(length + 1, 0x00);
    std::vector<uint8_t> receiveBuffer (length + 1);
    transmitBuffer[0] = readAddress;

    int8_t result = transfer(transmitBuffer.data(),
                             receiveBuffer.data(),
                             static_cast<uint16_t>(transmitBuffer.size()));

    if (result == 0) {

        std::memcpy(dataBuffer, receiveBuffer.data() + 1, length);
    }
    return result;
}

#endif

