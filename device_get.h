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

#ifndef _DEVICE_GET_H
#define _DEVICE_GET_H

#include "device_common.h"

extern const char HDHR_DOC_get_name[];
PyObject *py_hdhr_get_name(py_hdhr_object *);

extern const char HDHR_DOC_get_device_id[];
PyObject *py_hdhr_get_device_id(py_hdhr_object *);

extern const char HDHR_DOC_get_device_ip[];
PyObject *py_hdhr_get_device_ip(py_hdhr_object *);

extern const char HDHR_DOC_get_device_id_requested[];
PyObject *py_hdhr_get_device_id_requested(py_hdhr_object *);

extern const char HDHR_DOC_get_device_ip_requested[];
PyObject *py_hdhr_get_device_ip_requested(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner[];
PyObject *py_hdhr_get_tuner(py_hdhr_object *);

extern const char HDHR_DOC_get_var[];
PyObject *py_hdhr_get_var(py_hdhr_object *, PyObject *, PyObject *);

extern const char HDHR_DOC_get_tuner_status[];
PyObject *py_hdhr_get_tuner_status(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_vstatus[];
PyObject *py_hdhr_get_tuner_vstatus(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_streaminfo[];
PyObject *py_hdhr_get_tuner_streaminfo(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_channel[];
PyObject *py_hdhr_get_tuner_channel(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_vchannel[];
PyObject *py_hdhr_get_tuner_vchannel(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_channelmap[];
PyObject *py_hdhr_get_tuner_channelmap(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_filter[];
PyObject *py_hdhr_get_tuner_filter(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_program[];
PyObject *py_hdhr_get_tuner_program(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_target[];
PyObject *py_hdhr_get_tuner_target(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_plotsample[];
PyObject *py_hdhr_get_tuner_plotsample(py_hdhr_object *);

extern const char HDHR_DOC_get_tuner_lockkey_owner[];
PyObject *py_hdhr_get_tuner_lockkey_owner(py_hdhr_object *);

extern const char HDHR_DOC_get_oob_status[];
PyObject *py_hdhr_get_oob_status(py_hdhr_object *);

extern const char HDHR_DOC_get_oob_plotsample[];
PyObject *py_hdhr_get_oob_plotsample(py_hdhr_object *);

extern const char HDHR_DOC_get_ir_target[];
PyObject *py_hdhr_get_ir_target(py_hdhr_object *);

extern const char HDHR_DOC_get_version[];
PyObject *py_hdhr_get_version(py_hdhr_object *);

extern const char HDHR_DOC_get_supported[];
PyObject *py_hdhr_get_supported(py_hdhr_object *, PyObject *, PyObject *);

#endif
