/*
 * device_set.c
 *
 * Copyright © 2015 Michael Mohr <akihana@gmail.com>.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

#include "device_common.h"

/*
 *  Functions which operate directly on the hdhomerun_device_t pointer
 */

const char HDHR_DOC_set_device[] = "Set the device to which this object points.";
PyObject *py_hdhr_set_device(py_hdhr_object *self, PyObject *args, PyObject *kwds) {
    unsigned int device_id = 0;
    unsigned int device_ip = 0;
    char *kwlist[] = {"device_id", "device_ip", NULL};
    int success;

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "II", kwlist, &device_id, &device_ip))
        return NULL;

    success = hdhomerun_device_set_device(self->hd, (uint32_t)device_id, (uint32_t)device_ip);
    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(hdhomerun_device_error, "failed to set device parameters");
        return NULL;
    } else if(success == 1) {
        Py_RETURN_NONE;
    } else {
        PyErr_SetString(hdhomerun_device_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
}

const char HDHR_DOC_set_multicast[] = "Set the device's multicast parameters.";
PyObject *py_hdhr_set_multicast(py_hdhr_object *self, PyObject *args, PyObject *kwds) {
    unsigned int multicast_ip;
    unsigned int multicast_port;
    char *kwlist[] = {"multicast_ip", "multicast_port", NULL};
    int success;

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "II", kwlist, &multicast_ip, &multicast_port))
        return NULL;

    success = hdhomerun_device_set_multicast(self->hd, (uint32_t)multicast_ip, (uint16_t)multicast_port);
    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(hdhomerun_device_error, "failed to set multicast parameters");
        return NULL;
    } else if(success == 1) {
        Py_RETURN_NONE;
    } else {
        PyErr_SetString(hdhomerun_device_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
}

const char HDHR_DOC_set_tuner[] = "Set the tuner which this object references.";
PyObject *py_hdhr_set_tuner(py_hdhr_object *self, PyObject *args, PyObject *kwds) {
    unsigned int tuner = 0;
    char *kwlist[] = {"tuner", NULL};
    int success;

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "I", kwlist, &tuner))
        return NULL;

    success = hdhomerun_device_set_tuner(self->hd, tuner);
    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(hdhomerun_device_error, "failed to set tuner number");
        return NULL;
    } else if(success == 1) {
        Py_RETURN_NONE;
    } else {
        PyErr_SetString(hdhomerun_device_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
}

const char HDHR_DOC_set_tuner_from_str[] = "Set the tuner which this object references.";
PyObject *py_hdhr_set_tuner_from_str(py_hdhr_object *self, PyObject *args, PyObject *kwds) {
    const char *tuner = NULL;
    char *kwlist[] = {"tuner", NULL};
    int success;

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &tuner))
        return NULL;

    success = hdhomerun_device_set_tuner_from_str(self->hd, tuner);
    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(hdhomerun_device_error, "failed to set tuner from string");
        return NULL;
    } else if(success == 1) {
        Py_RETURN_NONE;
    } else {
        PyErr_SetString(hdhomerun_device_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
}

/*
 *  Functions which operate on a HDHomeRun device
 */

const char HDHR_DOC_set_var[] = "Set a named control variable on the device.";
PyObject *py_hdhr_set_var(py_hdhr_object *self, PyObject *args, PyObject *kwds) {
    char *ret_error = "the set operation was rejected by the device";
    char *item = NULL;
    char *value = NULL;
    int success;
    char *kwlist[] = {"item", "value", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, &item, &value))
        return NULL;

    success = hdhomerun_device_set_var(self->hd, item, value, NULL, &ret_error);
    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(hdhomerun_device_error, ret_error);
        return NULL;
    } else if(success == 1) {
        Py_RETURN_NONE;
    } else {
        PyErr_SetString(hdhomerun_device_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
}
