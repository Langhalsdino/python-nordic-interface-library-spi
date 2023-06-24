/*
 * @file	spi_Nordic_Pi.h
 * @date	15 Jan 2019
 * @version	0.0.1
 * @brief
 *
 */
/*! @file spi_Nordic_Pi.h
 @brief File for setting up spi Connection */

#include <fcntl.h>                //Needed for SPI port
#include <sys/ioctl.h>            //Needed for SPI port
#include <linux/spi/spidev.h>     //Needed for SPI port
#include <unistd.h>               //Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

#ifndef SPI_NORDIC_PI_H
#define SPI_NORDIC_PI_H

extern int spi_cs0_fd;                //file descriptor for the SPI device
// int spi_cs1_fd;                //file descriptor for the SPI device
extern unsigned char spi_mode;
extern unsigned char spi_bitsPerWord;
extern unsigned int spi_speed;


int spi_open (int spi_device);
int spi_close (int spi_device);
int spi_xfer (int spi_device, unsigned char *tx_data, unsigned char *rx_data, int length);

#endif /* SPI_NORDIC_PI_H */
/** @}*/
/** @}*/
