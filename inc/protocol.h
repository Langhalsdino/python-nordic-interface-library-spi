#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <spi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <spi_commands.h>
#include <time.h>
#include <unistd.h>


#define SPI_DEVICE 0
#define SPI_INTERMISSION_TIME_US   500
#define INTERMISSION_CYCLES (CLOCKS_PER_SEC * SPI_INTERMISSION_TIME_US / 1000000)

typedef struct prot_ret_s {
    uint8_t status;
    uint8_t error_code;
} prot_ret_t;

int prot_open_stream();
int prot_close_stream();
prot_ret_t prot_exec_command(uint8_t _id);
prot_ret_t prot_read_data(uint8_t _id, uint8_t *out_, uint8_t _len);
prot_ret_t prot_write_data(uint8_t _id, uint8_t *_data, uint8_t _len);
int prot_xfer(int spi_device, unsigned char *tx_data, unsigned char *rx_data, int length);



#endif
