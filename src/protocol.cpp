#include <protocol.h>

uint8_t p_stream_opened        = 0;
clock_t last_transmission_time = 0;

int prot_open_stream() {
    if (!p_stream_opened) {
        spi_open(SPI_DEVICE);
        p_stream_opened = 1;
        return 0;
    } else {
        return 0;
    }
}

int prot_close_stream() {
    spi_close(SPI_DEVICE);
    p_stream_opened = 0;
    return 0;
}

int prot_xfer(int spi_device, unsigned char *tx_data, unsigned char *rx_data,
              int length) {
    /* Ensures that an intermission frame of DELAY_US is guaranteed between two
     * transfers */
    clock_t now = clock();
    while ((now - last_transmission_time) < INTERMISSION_CYCLES) {
        now = clock();
    }
    int ret                = spi_xfer(SPI_DEVICE, tx_data, rx_data, length);
    last_transmission_time = now;
    return ret;
}

prot_ret_t prot_write_data(uint8_t _id, uint8_t *_data, uint8_t _len) {
    prot_ret_t ret = {0};
    unsigned char tx[_len + 1];
    unsigned char rx[_len + 1];

    /* Initialize out array */
    tx[0] = _id;
    for (uint8_t i = 1; i <= _len; i++) {
        tx[i] = _data[i - 1];
    }

    prot_open_stream();
    prot_xfer(SPI_DEVICE, tx, rx, _len + 1);

    unsigned char status = rx[0];

    /* Device cannot parse command */
    if (!(status & BBP_CTS_FLAG)) {
        ret.error_code = 1;
        ret.status     = status;
        return ret;
    }

    /* Wait for data to be ready and acknowledged by CTS and RTR flag */
    unsigned char nop = 0;
    status            = 0;
    while (!(status & (BBP_CTS_FLAG))) {
        prot_xfer(SPI_DEVICE, &nop, &status, 1);
    }

    /* Data failed to write */
    if (status & BBP_ERROR_FLAG) {
        prot_close_stream();
        ret.error_code = 2;
        ret.status     = status;
        return ret;
    }

    prot_close_stream();

    ret.error_code = 0;
    ret.status     = status;
    return ret;
}

prot_ret_t prot_read_data(uint8_t _id, uint8_t *out_, uint8_t _len) {
    prot_ret_t ret = {0};
    unsigned char tx[_len + 1];
    unsigned char rx[_len + 1];

    /* Initialize out array */
    tx[0] = _id;
    for (uint8_t i = 1; i <= _len; i++) {
        tx[i] = 0;
    }

    prot_open_stream();
    prot_xfer(SPI_DEVICE, tx, rx, 1);

    unsigned char status = rx[0];

    /* Device cannot parse command */
    if (!(status & BBP_CTS_FLAG)) {
        ret.error_code = 1;
        ret.status     = status;
        return ret;
    }

    /* Wait for data to be ready and acknowledged by CTS and RTR flag */
    unsigned char nop = 0;
    status            = 0;
    while (!(status & (BBP_CTS_FLAG | BBP_RTR_FLAG) ||
             status & (BBP_CTS_FLAG | BBP_ERROR_FLAG))) {
        prot_xfer(SPI_DEVICE, &nop, &status, 1);
    }

    /* Data failed to load */
    if (status & BBP_ERROR_FLAG) {
        prot_close_stream();
        ret.error_code = 2;
        ret.status     = status;
        return ret;
    }

    /* Readout */
    memset(tx, 0, _len + 1);
    tx[0] = BBP_READOUT;
    prot_xfer(SPI_DEVICE, tx, rx, _len + 1);

    /* Command failed to execute */
    if (status & BBP_ERROR_FLAG) {
        prot_close_stream();
        ret.error_code = 3;
        ret.status     = status;
        return ret;
    }

    for (uint8_t i = 0; i < _len; i++) {
        out_[i] = rx[i + 1];
    }

    prot_close_stream();

    ret.error_code = 0;
    ret.status     = status;
    return ret;
}

prot_ret_t prot_exec_command(uint8_t _id) {
    prot_ret_t ret = {0};
    unsigned char status;
    unsigned char tx[1] = {_id};

    prot_open_stream();
    prot_xfer(SPI_DEVICE, tx, &status, 1);

    /* Device cannot execute command */
    if (!(status & BBP_CTS_FLAG)) {
        ret.error_code = 1;
        ret.status     = status;
        return ret;
    }

    /* Wait for command to be executed and acknowledged by CTS and no ERROR flag
     */
    unsigned char nop = 0;
    status            = 0;
    while (!(status & BBP_CTS_FLAG)) {
        prot_xfer(SPI_DEVICE, &nop, &status, 1);
    }

    /* Command failed to execute */
    if (status & BBP_ERROR_FLAG) {
        prot_close_stream();
        ret.error_code = 2;
        ret.status     = status;
        return ret;
    }

    prot_close_stream();

    ret.error_code = 0;
    ret.status     = status;
    return ret;
}
