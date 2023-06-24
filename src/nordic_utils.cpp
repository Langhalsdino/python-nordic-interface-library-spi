#include <Python.h>
#include <protocol.h>

PyObject *nordic_utils_clear_callback(PyObject *self, PyObject *args);

PyObject *nordic_utils_callback_handler = 0;

void nordic_utils_parse_status(uint8_t status) {
    // at every transmission this function is called
    if (nordic_utils_callback_handler != 0) {
        PyObject *dict = PyDict_New();

        if (status & BBP_BAT_LOW_FLAG) {
            PyDict_SetItemString(dict, "batteryLow", PyLong_FromLong(1));
        } else {
            PyDict_SetItemString(dict, "batteryLow", PyLong_FromLong(0));
        }

        PyObject *arglist = Py_BuildValue("(O)", dict);
        PyObject_CallObject(nordic_utils_callback_handler, arglist);
    } else {
    }
}

PyObject *nordic_utils_set_callback(PyObject *self, PyObject *args) {
    PyObject *callback;
    int ret = PyArg_ParseTuple(args, "O", &callback);
    if (ret == 0) {
        PyErr_SetString(PyExc_RuntimeError, "Argument Error.");
        return 0;
    }

    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Argument Error. Needs to be callable.");
        return 0;
    }

    // make sure all memory is dereferenced
    nordic_utils_clear_callback(Py_None, Py_None);

    Py_INCREF(callback);
    nordic_utils_callback_handler = callback;
    return Py_None;
}

PyObject *nordic_utils_clear_callback(PyObject *self, PyObject *args) {
    if (nordic_utils_callback_handler != 0) {
        Py_DECREF(nordic_utils_callback_handler);
        nordic_utils_callback_handler = 0;
    }

    return Py_None;
}

PyObject *nordic_utils_callback_test(PyObject *self, PyObject *args) {
    nordic_utils_parse_status(0xff);

    return Py_None;
}

PyObject *nordic_utils_spi_xfer(PyObject *self, PyObject *args) {
    /* Parse arguments and validate */
    PyObject *arg0;
    int ret = PyArg_ParseTuple(args, "O", &arg0);
    if (ret == 0) {
        PyErr_SetString(PyExc_RuntimeError, "Argument Error.");
        return 0;
    }

    PyObject *m = PyList_New(0);

    if (!PyList_Check(arg0)) {
        PyErr_SetString(PyExc_RuntimeError, "Argument 1 is no list.");
        return 0;
    }

    /* Get the data that will be transmitted */
    int size = PyList_Size(arg0);
    uint8_t tx_data[size];
    uint8_t rx_data[size];

    for (int i = 0; i < size; i++) {
        PyObject *el = PyList_GetItem(arg0, i);
        long x       = PyLong_AsLong(el);
        tx_data[i]   = (uint8_t)x;
    }

    /* Transmit and receive */
    prot_open_stream();
    prot_xfer(SPI_DEVICE, tx_data, rx_data, size);
    prot_close_stream();

    /* Return RX */
    for (int i = 0; i < size; i++) {
        PyList_Append(m, PyLong_FromLong(rx_data[i]));
    }

    return m;
}

PyObject *nordic_utils_exec_command(PyObject *self, PyObject *args) {
    PyObject *arg0;
    int res = PyArg_ParseTuple(args, "O", &arg0);
    if (res == 0) {
        PyErr_SetString(PyExc_RuntimeError, "Argument Error.");
        return 0;
    }

    if (!PyNumber_Check(arg0)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Argument 1 has to be a command id.");
        return 0;
    }

    long x = PyLong_AsLong(PyNumber_Long(arg0));

    prot_ret_t spi_ret = {0};
    spi_ret.error_code = 1;
    if (x >= 0 && x <= 255) {
        spi_ret = prot_exec_command((uint8_t)x);
    }

    nordic_utils_parse_status(spi_ret.status);

    PyObject *ret = PyLong_FromLong(spi_ret.error_code);
    return ret;
}

PyObject *nordic_utils_write_raw(uint8_t _id, uint32_t _data, uint8_t _len) {
    uint8_t tx_data[_len];
    for (uint8_t i = 0; i < _len; i++) {
        tx_data[i] = (_data >> 8 * (_len - i - 1)) & 0xff;
    }

    prot_ret_t spi_ret = {0};
    spi_ret.error_code = 1;
    if (_id >= 0 && _id <= 255) {
        spi_ret = prot_write_data((uint8_t)_id, tx_data, _len);
    }

    nordic_utils_parse_status(spi_ret.status);

    //! Parse return codes
    if (spi_ret.status == 0xff) {
        PyErr_SetString(PyExc_RuntimeError, "Device isn't answering");
        return 0;
    }

    if (spi_ret.error_code != 0) {
        switch (spi_ret.error_code) {
            case 1:
                PyErr_SetString(PyExc_RuntimeError,
                                "The requested device is not clear to send.");
                break;
            case 2:
                PyErr_SetString(PyExc_RuntimeError,
                                "The device couldn't execute the command.");
                break;
        }

        return 0;
    }

    nordic_utils_parse_status(spi_ret.status);

    PyObject *ret = PyLong_FromLong(spi_ret.error_code);
    return ret;
}

PyObject *nordic_utils_read_raw(uint8_t _id, uint8_t _len) {
    uint8_t rx_data[_len];

    prot_ret_t spi_ret = {0};
    spi_ret.error_code = 1;
    if (_id >= 0 && _id <= 255) {
        spi_ret = prot_read_data(_id, rx_data, _len);
    }

    //! Parse return codes
    if (spi_ret.status == 0xff) {
        PyErr_SetString(PyExc_RuntimeError, "Device isn't answering");
        return 0;
    }

    if (spi_ret.error_code != 0) {
        switch (spi_ret.error_code) {
            case 1:
                PyErr_SetString(PyExc_RuntimeError,
                                "The requested device is not clear to send.");
                break;
            case 2:
                PyErr_SetString(PyExc_RuntimeError,
                                "Data couldn't be supplied on the device.");
                break;
            case 3:
                PyErr_SetString(PyExc_RuntimeError, "Data readout failed.");
                break;
        }

        return 0;
    }

    uint32_t data = 0;
    for (uint8_t i = 0; i < _len; i++) {
        data <<= 8;
        data |= rx_data[i];
    }

    nordic_utils_parse_status(spi_ret.status);

    PyObject *ret = PyLong_FromLong(data);
    return ret;
}

PyObject *nordic_utils_read_status(PyObject *self, PyObject *args) {
    int res = PyArg_ParseTuple(args, "");
    if (res == 0) {
        PyErr_SetString(PyExc_RuntimeError, "Argument Error.");
        return 0;
    }

    prot_ret_t ret = prot_exec_command(0);

    if (ret.status == 0xff) {
        PyErr_SetString(PyExc_RuntimeError, "The device doesn't answer.");
        return 0;
    }

    nordic_utils_parse_status(ret.status);

    PyObject *ret_ = PyLong_FromLong(ret.status);
    return ret_;
}

/**
 * General read write
 */
PyObject *nordic_utils_write(PyObject *self, PyObject *args) {
    PyObject *arg0;
    PyObject *arg1;
    PyObject *arg2;
    int res = PyArg_ParseTuple(args, "OOO", &arg0, &arg1, &arg2);
    if (res == 0) {
        PyErr_SetString(PyExc_RuntimeError, "Argument Error.");
        return 0;
    }

    if (!PyNumber_Check(arg0)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Argument 1 has to be a command number.");
        return 0;
    }

    if (!PyNumber_Check(arg1)) {
        PyErr_SetString(PyExc_RuntimeError, "Argument 2 has to be a number.");
        return 0;
    }

    if (!PyNumber_Check(arg2)) {
        PyErr_SetString(PyExc_RuntimeError, "Argument 3 has to be a number.");
        return 0;
    }

    long command_id = PyLong_AsLong(PyNumber_Long(arg0));
    long value      = PyLong_AsLong(PyNumber_Long(arg1));
    long length     = PyLong_AsLong(PyNumber_Long(arg2));

    PyObject *ret = nordic_utils_write_raw(command_id, (uint32_t)value, length);
    return ret;
}

PyObject *nordic_utils_read(PyObject *self, PyObject *args) {
    PyObject *arg0;
    PyObject *arg1;
    int res = PyArg_ParseTuple(args, "OO", &arg0, &arg1);
    if (res == 0) {
        PyErr_SetString(PyExc_RuntimeError, "Argument Error.");
        return 0;
    }

    if (!PyNumber_Check(arg0)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Argument 1 has to be a command number.");
        return 0;
    }

    if (!PyNumber_Check(arg1)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Argument 2 has to be a command number.");
        return 0;
    }

    long command_id = PyLong_AsLong(PyNumber_Long(arg0));
    long length     = PyLong_AsLong(PyNumber_Long(arg1));

    PyObject *ret = nordic_utils_read_raw(command_id, length);

    return ret;
}

PyObject *nordic_utils_write_array(PyObject *self, PyObject *args) {
    PyObject *arr, *id;
    int res = PyArg_ParseTuple(args, "OO", &id, &arr);
    if (res == 0) {
        PyErr_SetString(PyExc_RuntimeError, "Argument Error.");
        return 0;
    }

    if (!PyNumber_Check(id)) {
        PyErr_SetString(PyExc_RuntimeError, "Argument 1 has to be a number.");
        return 0;
    }
    if (!PyList_Check(arr)) {
        PyErr_SetString(PyExc_RuntimeError, "Argument 2 has to be a list.");
        return 0;
    }

    long command_id = PyLong_AsLong(PyNumber_Long(id));
    PyObject *iter  = PyObject_GetIter(arr);
    if (!iter) {
        PyErr_SetString(PyExc_RuntimeError, "No iterator found.");
        return 0;
    }

    Py_ssize_t length = PyList_Size(arr);
    uint8_t arr_sent[length];
    for (Py_ssize_t i = 0; i < length; i++) {
        PyObject *it = PyList_GetItem(arr, i);
        int val = Py_SAFE_DOWNCAST(PyLong_AsLong(PyNumber_Long(it)), long, int);
        arr_sent[i] = (uint8_t)(val & 0xff);
    }

    prot_ret_t spi_ret = {0};
    spi_ret.error_code = 1;
    if (command_id >= 0 && command_id <= 255) {
        spi_ret = prot_write_data((uint8_t)command_id, arr_sent, length);
    }

    //! Parse return codes
    if (spi_ret.status == 0xff) {
        PyErr_SetString(PyExc_RuntimeError, "Device isn't answering");
        return 0;
    }

    if (spi_ret.error_code != 0) {
        switch (spi_ret.error_code) {
            case 1:
                PyErr_SetString(PyExc_RuntimeError,
                                "The requested device is not clear to send.");
                break;
            case 2:
                PyErr_SetString(PyExc_RuntimeError,
                                "The device couldn't process the data.");
                break;
        }

        return 0;
    }

    nordic_utils_parse_status(spi_ret.status);

    PyObject *ret = PyLong_FromLong(spi_ret.error_code);
    return ret;
}

PyObject *nordic_utils_read_array(PyObject *self, PyObject *args) {
    PyObject *_length, *_id;
    int res = PyArg_ParseTuple(args, "OO", &_id, &_length);
    if (res == 0) {
        PyErr_SetString(PyExc_RuntimeError, "Argument Error.");
        return 0;
    }

    if (!(PyNumber_Check(_length) && PyNumber_Check(_id))) {
        PyErr_SetString(PyExc_RuntimeError, "Argument 1 has to be a number.");
        return 0;
    }

    long id     = PyLong_AsLong(PyNumber_Long(_id));
    long length = PyLong_AsLong(PyNumber_Long(_length));

    uint8_t data[length];
    prot_ret_t spi_ret;
    spi_ret.error_code = 1;
    spi_ret            = prot_read_data(id, data, length);

    //! Parse return codes
    if (spi_ret.status == 0xff) {
        PyErr_SetString(PyExc_RuntimeError, "Device isn't answering");
        return 0;
    }

    if (spi_ret.error_code != 0) {
        switch (spi_ret.error_code) {
            case 1:
                PyErr_SetString(PyExc_RuntimeError,
                                "The requested device is not clear to send.");
                break;
            case 2:
                PyErr_SetString(PyExc_RuntimeError,
                                "Data couldn't be supplied on the device.");
                break;
            case 3:
                PyErr_SetString(PyExc_RuntimeError, "Data readout failed.");
                break;
        }

        return 0;
    }

    nordic_utils_parse_status(spi_ret.status);

    if (spi_ret.error_code == 1) {
        PyErr_SetString(PyExc_RuntimeError, "Command failed");
        return 0;
    }

    PyObject *out = PyList_New(0);
    for (long i = 0; i < length; i++) {
        PyObject *_data = PyLong_FromLong(data[i]);
        PyList_Append(out, _data);
    }

    return out;
}

/**
 * Python Bindings
 */
static PyMethodDef nordic_utils_methods[] = {
    {"_spi_xfer", nordic_utils_spi_xfer, METH_VARARGS, ""},
    {"exec_command", nordic_utils_exec_command, METH_VARARGS, ""},
    {"write_bytes", nordic_utils_write, METH_VARARGS, ""},
    {"write_byte_array", nordic_utils_write_array, METH_VARARGS, ""},
    {"read_byte_array", nordic_utils_read_array, METH_VARARGS, ""},
    {"read_bytes", nordic_utils_read, METH_VARARGS, ""},
    {"get_status", nordic_utils_read_status, METH_VARARGS, ""},
    {"set_callback", nordic_utils_set_callback, METH_VARARGS, ""},
    {"clear_callback", nordic_utils_clear_callback, METH_VARARGS, ""},
    {"_test_callback", nordic_utils_callback_test, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL},
};

static struct PyModuleDef nordic_utils = {
    PyModuleDef_HEAD_INIT, "nordicUtils", NULL, -1, nordic_utils_methods,
};

PyObject *m_nordic_utils;

PyMODINIT_FUNC PyInit_nordicUtils(void) {
    m_nordic_utils = PyModule_Create(&nordic_utils);

    // need to update here
    PyModule_AddStringConstant(m_nordic_utils, "__version__", "0.1.2");

    return m_nordic_utils;
}
