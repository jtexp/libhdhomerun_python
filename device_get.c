/*
 * silicondustmodule.c
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

#include "device_get.h"

/*
 *  Functions which operate directly on the hdhomerun_device_t pointer
 */

const char HDHR_DOC_get_name[] = "Get the device name.";
PyObject *py_hdhr_get_name(py_hdhr_object *self) {
    const char *name;

    name = hdhomerun_device_get_name(self->hd);
    return PyString_FromString(name);
}

const char HDHR_DOC_get_device_id[] = "Get the device ID.";
PyObject *py_hdhr_get_device_id(py_hdhr_object *self) {
    uint32_t device_id;

    device_id = hdhomerun_device_get_device_id(self->hd);
    return PyLong_FromUnsignedLong((unsigned long)device_id);
}

const char HDHR_DOC_get_device_ip[] = "Get the device IP.";
PyObject *py_hdhr_get_device_ip(py_hdhr_object *self) {
    uint32_t device_ip;

    device_ip = hdhomerun_device_get_device_ip(self->hd);
    return PyLong_FromUnsignedLong((unsigned long)device_ip);
}

const char HDHR_DOC_get_device_id_requested[] = "Get the requested device ID.";
PyObject *py_hdhr_get_device_id_requested(py_hdhr_object *self) {
    uint32_t device_id;

    device_id = hdhomerun_device_get_device_id_requested(self->hd);
    return PyLong_FromUnsignedLong((unsigned long)device_id);
}

const char HDHR_DOC_get_device_ip_requested[] = "Get the requested device IP.";
PyObject *py_hdhr_get_device_ip_requested(py_hdhr_object *self) {
    uint32_t device_ip;

    device_ip = hdhomerun_device_get_device_ip_requested(self->hd);
    return PyLong_FromUnsignedLong((unsigned long)device_ip);
}

const char HDHR_DOC_get_tuner[] = "Get the tuner number that this HDHR object references.";
PyObject *py_hdhr_get_tuner(py_hdhr_object *self) {
    unsigned int tuner_number;

    tuner_number = hdhomerun_device_get_tuner(self->hd);
    return PyLong_FromUnsignedLong((unsigned long)tuner_number);
}
