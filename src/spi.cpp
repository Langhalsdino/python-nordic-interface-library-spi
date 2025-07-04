/*
 * @file	spi_Nordic_Pi.h
 * @date	15 Jan 2019
 * @version	0.0.1
 * @brief
 *
 */
/*! @file spi_Nordic_Pi.h
 @brief File for setting up spi Connection */

#include <spi.h>

#include <fcntl.h>             //Needed for SPI port
#include <linux/spi/spidev.h>  //Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>  //Needed for SPI port
#include <unistd.h>     //Needed for SPI port
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <list>
#include <string>

#include <time.h>

#ifndef DEBUG_SPI
#define DEBUG_SPI 0
#endif

int spi_cs0_fd;
int spi_cs1_fd;
unsigned char spi_mode;
unsigned char spi_bitsPerWord;
unsigned int spi_speed;

//***********************************
//***********************************
//********** SPI OPEN PORT **********
//***********************************
//***********************************
// spi_device    0=CS0, 1=CS1
int spi_open(int spi_device) {
    int status_value = -1;
    int *spi_cs_fd;

#if DEBUG_SPI
    printf("%lu: Opening spi device.", clock());
#endif

    //----- SET SPI MODE -----
    // SPI_MODE_0 (0,0)     CPOL = 0, CPHA = 0, Clock idle low, data is clocked
    // in on rising edge, output data (change) on falling edge SPI_MODE_1 (0,1)
    // CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge,
    // output data (change) on rising edge SPI_MODE_2 (1,0)     CPOL = 1, CPHA =
    // 0, Clock idle high, data is clocked in on falling edge, output data
    // (change) on rising edge SPI_MODE_3 (1,1)     CPOL = 1, CPHA = 1, Clock
    // idle high, data is clocked in on rising, edge output data (change) on
    // falling edge
    spi_mode = SPI_MODE_0;

    //----- SET BITS PER WORD -----
    spi_bitsPerWord = 8;

    //----- SET SPI BUS SPEED -----
    spi_speed = 1000000;  // 1000000 = 1MHz (0.5uS per bit)

    if (spi_device)
        spi_cs_fd = &spi_cs1_fd;
    else
        spi_cs_fd = &spi_cs0_fd;

    if (spi_device)
        *spi_cs_fd =
            open(std::string("/dev/spidev0.1").c_str(), O_RDWR | O_SYNC);
    else
        *spi_cs_fd =
            open(std::string("/dev/spidev0.0").c_str(), O_RDWR | O_SYNC);

    if (*spi_cs_fd < 0) {
        perror("Error - Could not open SPI device");
        exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
    if (status_value < 0) {
        perror("Could not set SPIMode (WR)...ioctl fail");
        exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
    if (status_value < 0) {
        perror("Could not set SPIMode (RD)...ioctl fail");
        exit(1);
    }

    status_value =
        ioctl(*spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
    if (status_value < 0) {
        perror("Could not set SPI bitsPerWord (WR)...ioctl fail");
        exit(1);
    }

    status_value =
        ioctl(*spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
    if (status_value < 0) {
        perror("Could not set SPI bitsPerWord(RD)...ioctl fail");
        exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if (status_value < 0) {
        perror("Could not set SPI speed (WR)...ioctl fail");
        exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    if (status_value < 0) {
        perror("Could not set SPI speed (RD)...ioctl fail");
        exit(1);
    }
    return (status_value);
}

//************************************
//************************************
//********** SPI CLOSE PORT **********
//************************************
//************************************
int spi_close(int spi_device) {
    int status_value = -1;
    int *spi_cs_fd;

#if DEBUG_SPI
    printf("%lu: Closing spi device.", clock());
#endif

    if (spi_device)
        spi_cs_fd = &spi_cs1_fd;
    else
        spi_cs_fd = &spi_cs0_fd;

    status_value = close(*spi_cs_fd);
    if (status_value < 0) {
        perror("Error - Could not close SPI device");
        exit(1);
    }
    return (status_value);
}

//*******************************************
//*******************************************
//********** SPI WRITE & READ DATA **********
//*******************************************
//*******************************************
// data        Bytes to write.  Contents is overwritten with bytes read.
static uint16_t delay = 20;
static uint32_t speed = 1000000;
static uint8_t bits   = 8;

int spi_xfer(int spi_device, unsigned char *tx_data, unsigned char *rx_data,
             int length) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_data,
        .rx_buf = (unsigned long)rx_data,
        .len    = (unsigned int)length,
    };

    tr.delay_usecs   = delay;
    tr.speed_hz      = speed;
    tr.bits_per_word = bits;

    int retVal = -1;
    int *spi_cs_fd;

    if (spi_device)
        spi_cs_fd = &spi_cs1_fd;
    else
        spi_cs_fd = &spi_cs0_fd;

    retVal = ioctl(*spi_cs_fd, SPI_IOC_MESSAGE(1), &tr);

#if DEBUG_SPI
#warning Debug enabled!
    printf(" SPI_TX: ");
    for (int i = 0; i < length; i++) {
        printf("%3u ", tx_data[i]);
    }
    printf("\r\n SPI_RX: ");
    for (int i = 0; i < length; i++) {
        printf("%3u ", rx_data[i]);
    }
    printf("\r\n");
#endif

    if (retVal < 0) {
        perror("Error - Problem transmitting spi data..ioctl");
        exit(1);
    }

    return retVal;
}

/** @}*/
/** @}*/
