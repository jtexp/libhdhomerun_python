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

#ifndef _DEVICE_COMMON_H
#define _DEVICE_COMMON_H

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <Python.h>
#include <structmember.h>
#include <libhdhomerun/hdhomerun.h>

typedef struct {
    PyObject_HEAD
    struct hdhomerun_device_t *hd;
    unsigned int tuner_count;
    unsigned int locked;
} py_hdhr_object;

/* String constants for use when raising exceptions */
extern const char * const HDHR_ERR_REJECTED_OP;
extern const char * const HDHR_ERR_COMMUNICATION;
extern const char * const HDHR_ERR_UNDOCUMENTED;

/* Internal */
PyObject *build_tuner_status_dict(struct hdhomerun_tuner_status_t *);

#endif
