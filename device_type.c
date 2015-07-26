/*
 * device_type.c
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

PyDoc_STRVAR(silicondust_module_doc,
    "Python bindings for the SiliconDust hdhomerun library");

PyObject *silicondust_hdhr_error = NULL;

int py_hdhr_init(py_hdhr_object *self, PyObject *args, PyObject *kwds) {
    unsigned int device_id = 0;
    unsigned int device_ip = 0;
    unsigned int tuner_count = 0;
    char *kwlist[] = {"device_id", "device_ip", "tuner_count", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "III", kwlist, &device_id, &device_ip, &tuner_count))
        return -1;
    self->hd = hdhomerun_device_create(device_id, device_ip, 0, NULL);
    if(!self->hd) {
        PyErr_SetString(silicondust_hdhr_error, "Failed to create HDHR device");
        return -1;
    }
    self->tuner_count = tuner_count;
    self->locked = 0;
    return 0;
}

void py_hdhr_dealloc(py_hdhr_object *self) {
    if(self->locked != 0) {
        /* Try to unlock the tuner, ignore errors */
        hdhomerun_device_tuner_lockkey_release(self->hd);
        self->locked = 0;
    }
    hdhomerun_device_destroy(self->hd);
    self->hd = NULL;
    self->tuner_count = 0;
    self->ob_type->tp_free((PyObject*)self);
}

uint32_t parse_ip_addr(const char *str) {
    unsigned int a[4];
    if (sscanf(str, "%u.%u.%u.%u", &a[0], &a[1], &a[2], &a[3]) != 4)
        return 0;

    return (uint32_t)((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | (a[3] << 0));
}

PyDoc_STRVAR(HDHR_discover_doc,
    "Locates all HDHomeRun(s) on your network and returns a list of HDHR objects.");

PyObject *py_hdhr_discover(PyObject *cls, PyObject *args, PyObject *kwds) {
    PyObject *result = NULL;
    PyObject *tuner = NULL;
    char *target_ip_str = NULL;
    uint32_t target_ip = 0;
    int count = 0, i;
    char *kwlist[] = {"target_ip", NULL};
    struct hdhomerun_discover_device_t result_list[64];

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &target_ip_str))
        return NULL;

    if(target_ip_str) {
        target_ip = parse_ip_addr(target_ip_str);
        if (target_ip == 0) {
            PyErr_SetString(silicondust_hdhr_error, "invalid ip address");
            return NULL;
        }
    }

    count = hdhomerun_discover_find_devices_custom(target_ip, HDHOMERUN_DEVICE_TYPE_TUNER, HDHOMERUN_DEVICE_ID_WILDCARD, result_list, 64);

    if(count < 0) {
        PyErr_SetString(silicondust_hdhr_error, "error sending discover request");
        return NULL;
    }

    result = PyList_New((Py_ssize_t)count);
    if(!result) {
        return NULL;
    }

    if(count > 0) {
        for(i=0; i<count; i++) {
            tuner = PyObject_CallFunction(cls, "III", result_list[i].device_id, result_list[i].ip_addr, result_list[i].tuner_count);
            if(tuner == NULL) { Py_DECREF(result); return NULL; }
            if(PyList_SetItem(result, i, tuner) != 0) { Py_DECREF(result); return NULL; }
        }
    }

    return result;
}

PyDoc_STRVAR(HDHR_upgrade_doc,
    "Uploads and installs a firmware image on a HDHomeRun device.");

PyObject *py_hdhr_upgrade(py_hdhr_object *self, PyObject *args, PyObject *kwds) {
    FILE *fp = NULL;
    char *filename = NULL;
    int count = 0;
    int wait = -1;
    char *version_str;
    PyObject *wait_obj = NULL;
    char *kwlist[] = {"filename", "wait", NULL};
    int success;

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "sO!", kwlist, &filename, &PyBool_Type, &wait_obj))
        return NULL;
    Py_INCREF(wait_obj);
    wait = PyObject_IsTrue(wait_obj);
    Py_DECREF(wait_obj);
    if(wait < 0)
        return NULL;

    fp = fopen(filename, "rb");
    if(!fp) {
        PyErr_SetString(PyExc_IOError, "unable to open firmware file");
        return NULL;
    }
    success = hdhomerun_device_upgrade(self->hd, fp);
    fclose(fp);
    fp = NULL;
    if(success == -1) {
        PyErr_SetString(silicondust_hdhr_error, "error sending upgrade file to hdhomerun device");
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(silicondust_hdhr_error, "the hdhomerun device rejected the firmware upgrade");
        return NULL;
    }

    if(wait > 0) {
        /* Wait for the device to come back online */
        msleep_minimum(10000);
        while (1) {
            if(hdhomerun_device_get_version(self->hd, &version_str, NULL) >= 0)
                break;
            count++;
            if (count > 30) {
                PyErr_SetString(silicondust_hdhr_error, "error finding device after firmware upgrade");
                return NULL;
            }
            msleep_minimum(1000);
        }
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(HDHR_tuner_lockkey_request_doc,
    "Locks a tuner.");

PyObject *py_hdhr_tuner_lockkey_request(py_hdhr_object *self) {
    char *ret_error = "the device rejected the lock request";
    int success;

    success = hdhomerun_device_tuner_lockkey_request(self->hd, &ret_error);

    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(silicondust_hdhr_error, ret_error);
        return NULL;
    } else if(success == 1) {
        self->locked = 1;
    } else {
        PyErr_SetString(silicondust_hdhr_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(HDHR_tuner_lockkey_force_doc,
    "Locks a tuner.");

PyObject *py_hdhr_tuner_lockkey_force(py_hdhr_object *self) {
    int success;

    success = hdhomerun_device_tuner_lockkey_force(self->hd);

    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(silicondust_hdhr_error, "the device rejected the forced lock request");
        return NULL;
    } else if(success == 1) {
        self->locked = 1;
    } else {
        PyErr_SetString(silicondust_hdhr_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(HDHR_tuner_lockkey_release_doc,
    "Unlocks a tuner.");

PyObject *py_hdhr_tuner_lockkey_release(py_hdhr_object *self) {
    int success;

    success = hdhomerun_device_tuner_lockkey_release(self->hd);
    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(silicondust_hdhr_error, "the device rejected the unlock request");
        return NULL;
    } else if(success == 1) {
        self->locked = 0;
    } else {
        PyErr_SetString(silicondust_hdhr_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(HDHR_stream_start_doc,
    "Tell the device to start streaming data.");

PyObject *py_hdhr_stream_start(py_hdhr_object *self) {
    int success;

    success = hdhomerun_device_stream_start(self->hd);
    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(silicondust_hdhr_error, "the device refused to start streaming");
        return NULL;
    } else if(success != 1) {
        PyErr_SetString(silicondust_hdhr_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(HDHR_stream_recv_doc,
    "Receive stream data.");

PyObject *py_hdhr_stream_recv(py_hdhr_object *self, PyObject *args, PyObject *kwds) {
    uint8_t *ptr;
    size_t actual_size;
    unsigned int max_size = VIDEO_DATA_BUFFER_SIZE_1S;
    char *kwlist[] = {"max_size", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|I", kwlist, &max_size))
        return NULL;

    ptr = hdhomerun_device_stream_recv(self->hd, (size_t)max_size, &actual_size);
    if(!ptr) {
        Py_RETURN_NONE;
    }

    return PyByteArray_FromStringAndSize((const char *)ptr, (Py_ssize_t)actual_size);
}

PyDoc_STRVAR(HDHR_stream_flush_doc,
    "Undocumented.");

PyObject *py_hdhr_stream_flush(py_hdhr_object *self) {
    hdhomerun_device_stream_flush(self->hd);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(HDHR_stream_stop_doc,
    "Tell the device to stop streaming data.");

PyObject *py_hdhr_stream_stop(py_hdhr_object *self) {
    hdhomerun_device_stream_stop(self->hd);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(HDHR_wait_for_lock_doc,
    "Wait for tuner lock after channel change.");

PyObject *py_hdhr_wait_for_lock(py_hdhr_object *self) {
    int success;
    struct hdhomerun_tuner_status_t status;

    success = hdhomerun_device_wait_for_lock(self->hd, &status);
    if(success == -1) {
        PyErr_SetString(PyExc_IOError, HDHR_ERR_COMMUNICATION);
        return NULL;
    } else if(success == 0) {
        PyErr_SetString(silicondust_hdhr_error, "the device did not report lock status");
        return NULL;
    } else if(success != 1) {
        PyErr_SetString(silicondust_hdhr_error, HDHR_ERR_UNDOCUMENTED);
        return NULL;
    }

    return build_tuner_status_dict(&status);
}

PyDoc_STRVAR(HDHR_DOC_clone,
    "Clone the HDHR object");

PyObject *py_hdhr_clone(py_hdhr_object *self);

PyMethodDef py_hdhr_methods[] = {
    /* Python methods for the HDHR class, not language bindings for libhdhomerun */
    {"discover",                (PyCFunction)py_hdhr_discover,                METH_KEYWORDS | METH_CLASS, HDHR_discover_doc},
    {"clone",                   (PyCFunction)py_hdhr_clone,                   METH_NOARGS,                HDHR_DOC_clone},
    /* Get operations, defined in device_get.c */
    {"get_name",                (PyCFunction)py_hdhr_get_name,                METH_NOARGS,                HDHR_DOC_get_name},
    {"get_device_id",           (PyCFunction)py_hdhr_get_device_id,           METH_NOARGS,                HDHR_DOC_get_device_id},
    {"get_device_ip",           (PyCFunction)py_hdhr_get_device_ip,           METH_NOARGS,                HDHR_DOC_get_device_ip},
    {"get_device_id_requested", (PyCFunction)py_hdhr_get_device_id_requested, METH_NOARGS,                HDHR_DOC_get_device_id_requested},
    {"get_device_ip_requested", (PyCFunction)py_hdhr_get_device_ip_requested, METH_NOARGS,                HDHR_DOC_get_device_ip_requested},
    {"get_tuner",               (PyCFunction)py_hdhr_get_tuner,               METH_NOARGS,                HDHR_DOC_get_tuner},
    {"get_var",                 (PyCFunction)py_hdhr_get_var,                 METH_KEYWORDS,              HDHR_DOC_get_var},
    {"get_tuner_status",        (PyCFunction)py_hdhr_get_tuner_status,        METH_NOARGS,                HDHR_DOC_get_tuner_status},
    {"get_tuner_vstatus",       (PyCFunction)py_hdhr_get_tuner_vstatus,       METH_NOARGS,                HDHR_DOC_get_tuner_vstatus},
    {"get_tuner_streaminfo",    (PyCFunction)py_hdhr_get_tuner_streaminfo,    METH_NOARGS,                HDHR_DOC_get_tuner_streaminfo},
    {"get_tuner_channel",       (PyCFunction)py_hdhr_get_tuner_channel,       METH_NOARGS,                HDHR_DOC_get_tuner_channel},
    {"get_tuner_vchannel",      (PyCFunction)py_hdhr_get_tuner_vchannel,      METH_NOARGS,                HDHR_DOC_get_tuner_vchannel},
    {"get_tuner_channelmap",    (PyCFunction)py_hdhr_get_tuner_channelmap,    METH_NOARGS,                HDHR_DOC_get_tuner_channelmap},
    {"get_tuner_filter",        (PyCFunction)py_hdhr_get_tuner_filter,        METH_NOARGS,                HDHR_DOC_get_tuner_filter},
    {"get_tuner_program",       (PyCFunction)py_hdhr_get_tuner_program,       METH_NOARGS,                HDHR_DOC_get_tuner_program},
    {"get_tuner_target",        (PyCFunction)py_hdhr_get_tuner_target,        METH_NOARGS,                HDHR_DOC_get_tuner_target},
    {"get_tuner_plotsample",    (PyCFunction)py_hdhr_get_tuner_plotsample,    METH_NOARGS,                HDHR_DOC_get_tuner_plotsample},
    {"get_tuner_lockkey_owner", (PyCFunction)py_hdhr_get_tuner_lockkey_owner, METH_NOARGS,                HDHR_DOC_get_tuner_lockkey_owner},
    {"get_oob_status",          (PyCFunction)py_hdhr_get_oob_status,          METH_NOARGS,                HDHR_DOC_get_oob_status},
    {"get_oob_plotsample",      (PyCFunction)py_hdhr_get_oob_plotsample,      METH_NOARGS,                HDHR_DOC_get_oob_plotsample},
    {"get_ir_target",           (PyCFunction)py_hdhr_get_ir_target,           METH_NOARGS,                HDHR_DOC_get_ir_target},
    {"get_version",             (PyCFunction)py_hdhr_get_version,             METH_NOARGS,                HDHR_DOC_get_version},
    {"get_supported",           (PyCFunction)py_hdhr_get_supported,           METH_KEYWORDS,              HDHR_DOC_get_supported},
    /* Set operations, defined in device_set.c */
    {"set_device",              (PyCFunction)py_hdhr_set_device,              METH_KEYWORDS,              HDHR_DOC_set_device},
    {"set_multicast",           (PyCFunction)py_hdhr_set_multicast,           METH_KEYWORDS,              HDHR_DOC_set_multicast},
    {"set_tuner",               (PyCFunction)py_hdhr_set_tuner,               METH_KEYWORDS,              HDHR_DOC_set_tuner},
    {"set_tuner_from_str",      (PyCFunction)py_hdhr_set_tuner_from_str,      METH_KEYWORDS,              HDHR_DOC_set_tuner_from_str},
    {"set_var",                 (PyCFunction)py_hdhr_set_var,                 METH_KEYWORDS,              HDHR_DOC_set_var},
    /* Misc. operations */
    {"upgrade",                 (PyCFunction)py_hdhr_upgrade,                 METH_KEYWORDS,              HDHR_upgrade_doc},
    {"tuner_lockkey_request",   (PyCFunction)py_hdhr_tuner_lockkey_request,   METH_NOARGS,                HDHR_tuner_lockkey_request_doc},
    {"tuner_lockkey_force",     (PyCFunction)py_hdhr_tuner_lockkey_force,     METH_NOARGS,                HDHR_tuner_lockkey_force_doc},
    {"tuner_lockkey_release",   (PyCFunction)py_hdhr_tuner_lockkey_release,   METH_NOARGS,                HDHR_tuner_lockkey_release_doc},
    {"stream_start",            (PyCFunction)py_hdhr_stream_start,            METH_NOARGS,                HDHR_stream_start_doc},
    {"stream_recv",             (PyCFunction)py_hdhr_stream_recv,             METH_KEYWORDS,              HDHR_stream_recv_doc},
    {"stream_flush",            (PyCFunction)py_hdhr_stream_flush,            METH_NOARGS,                HDHR_stream_flush_doc},
    {"stream_stop",             (PyCFunction)py_hdhr_stream_stop,             METH_NOARGS,                HDHR_stream_stop_doc},
    {"wait_for_lock",           (PyCFunction)py_hdhr_wait_for_lock,           METH_NOARGS,                HDHR_wait_for_lock_doc},
    {NULL,                      NULL,                                         0,                          NULL}  /* Sentinel */
};

PyMemberDef py_hdhr_members[] = {
    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(silicondust_HDHR_type_doc,
    "An object representing a single HDHomeRun device.");

PyTypeObject silicondust_hdhr_type = {
    PyObject_HEAD_INIT(NULL)
    0,                              /* ob_size */
    "silicondust.HDHR",             /* tp_name */
    sizeof(py_hdhr_object),         /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)py_hdhr_dealloc,    /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    silicondust_HDHR_type_doc,      /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    py_hdhr_methods,                /* tp_methods */
    py_hdhr_members,                /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)py_hdhr_init,         /* tp_init */
    (allocfunc)PyType_GenericAlloc, /* tp_alloc */
    (newfunc)PyType_GenericNew,     /* tp_new */
    (freefunc)PyObject_Del,         /* tp_free */
};

PyObject *py_hdhr_clone(py_hdhr_object *self) {
    PyObject *arg_list, *copied_obj;
    uint32_t device_id, device_ip;

    device_id = hdhomerun_device_get_device_id(self->hd);
    device_ip = hdhomerun_device_get_device_ip(self->hd);
    arg_list = Py_BuildValue("(III)", device_id, device_ip, self->tuner_count);
    if(arg_list == NULL) {
        return NULL;
    }
    copied_obj = PyObject_CallObject((PyObject *)&silicondust_hdhr_type, arg_list);
    Py_DECREF(arg_list);
    return copied_obj;
}

/* module methods (none for now) */
PyMethodDef silicondust_methods[] = {
    {NULL}  /* Sentinel */
};

PyMODINIT_FUNC initsilicondust(void) {
    PyObject *m;

    m = Py_InitModule3("silicondust", silicondust_methods, silicondust_module_doc);
    if(!m)
        return;

    /* Finalize the HDHR type object */
    if (PyType_Ready(&silicondust_hdhr_type) < 0)
        return;
    Py_INCREF(&silicondust_hdhr_type);
    if(PyModule_AddObject(m, "HDHR", (PyObject *)&silicondust_hdhr_type) < 0)
        return;

    /* Initialize the HDHRError exception class */
    silicondust_hdhr_error = PyErr_NewException("silicondust.HDHRError", PyExc_Exception, NULL);
    Py_INCREF(silicondust_hdhr_error);
    if(PyModule_AddObject(m, "HDHRError", silicondust_hdhr_error) < 0)
        return;
}
