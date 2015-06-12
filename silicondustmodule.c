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

#include <stdio.h>
#include <string.h>
#include <Python.h>
#include <structmember.h>
#include <libhdhomerun/hdhomerun.h>

PyDoc_STRVAR(silicondust_module_doc,
    "Python bindings for the SiliconDust hdhomerun library");

PyObject *silicondust_hdhr_error = NULL;

typedef struct
{
    PyObject_HEAD
    struct hdhomerun_device_t *hd;
    unsigned int tuner_count;
} py_hdhr_object;

static int py_hdhr_init(py_hdhr_object *self, PyObject *args, PyObject *kwds)
{
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
    return 0;
}

static void py_hdhr_dealloc(py_hdhr_object *self)
{
    hdhomerun_device_destroy(self->hd);
    self->hd = NULL;
    self->tuner_count = 0;
    self->ob_type->tp_free((PyObject*)self);
}

static uint32_t parse_ip_addr(const char *str)
{
    unsigned int a[4];
    if (sscanf(str, "%u.%u.%u.%u", &a[0], &a[1], &a[2], &a[3]) != 4)
        return 0;

    return (uint32_t)((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | (a[3] << 0));
}

PyDoc_STRVAR(HDHR_discover_doc,
    "Locates all HDHomeRun(s) on your network and returns a list of HDHR objects.");

static PyObject *py_hdhr_discover(PyObject *cls, PyObject *args, PyObject *kwds)
{
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
            if(tuner == NULL) return NULL;
            if(PyList_SetItem(result, i, tuner) != 0) return NULL;
        }
    }

    return result;
}

PyDoc_STRVAR(HDHR_get_doc,
    "Get a named control variable on the device.");

static PyObject *py_hdhr_get(py_hdhr_object *self, PyObject *args)
{
    char *ret_value = NULL;
    char *ret_error = NULL;
    char *item = NULL;

    if(!PyArg_ParseTuple(args, "s", &item))
        return NULL;

    if(hdhomerun_device_get_var(self->hd, item, &ret_value, &ret_error) < 0) {
        PyErr_SetString(PyExc_IOError, "communication error sending request to hdhomerun device");
        return NULL;
    }

    if(ret_error) {
        PyErr_SetString(silicondust_hdhr_error, ret_error);
        return NULL;
    }

    return PyString_FromString(ret_value);
}

PyDoc_STRVAR(HDHR_set_doc,
    "Set a named control variable on the device.");

static PyObject *py_hdhr_set(py_hdhr_object *self, PyObject *args)
{
    char *ret_error = NULL;
    char *item = NULL;
    char *value = NULL;

    if(!PyArg_ParseTuple(args, "ss", &item, &value))
        return NULL;

    if(hdhomerun_device_set_var(self->hd, item, value, NULL, &ret_error) < 0) {
        PyErr_SetString(PyExc_IOError, "communication error sending request to hdhomerun device");
        return NULL;
    }

    if(ret_error) {
        PyErr_SetString(silicondust_hdhr_error, ret_error);
        return NULL;
    }

    Py_RETURN_NONE;
}

PyDoc_STRVAR(HDHR_upgrade_doc,
    "Uploads and installs a firmware image on a HDHomeRun device.");

static PyObject *py_hdhr_upgrade(py_hdhr_object *self, PyObject *args)
{
    FILE *fp = NULL;
    char *filename = NULL;
    int count = 0;
    int wait = -1;
    char *version_str;
    PyObject *wait_obj = NULL;

    if(!PyArg_ParseTuple(args, "sO!", &filename, &PyBool_Type, &wait_obj))
        return NULL;

    fp = fopen(filename, "rb");
    if(!fp) {
        PyErr_SetString(PyExc_IOError, "unable to open firmware file");
        return NULL;
    }
    if (hdhomerun_device_upgrade(self->hd, fp) <= 0) {
        PyErr_SetString(silicondust_hdhr_error, "error sending upgrade file to hdhomerun device");
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    wait = PyObject_IsTrue(wait_obj);
    if(wait < 0)
        return NULL;
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

static PyMethodDef py_hdhr_methods[] =
{
    {"discover", (PyCFunction)py_hdhr_discover, METH_VARARGS | METH_CLASS, HDHR_discover_doc},
    {"get",      (PyCFunction)py_hdhr_get,      METH_VARARGS,              HDHR_get_doc},
    {"set",      (PyCFunction)py_hdhr_set,      METH_VARARGS,              HDHR_set_doc},
    {"upgrade",  (PyCFunction)py_hdhr_upgrade,  METH_VARARGS,              HDHR_upgrade_doc},
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyMemberDef py_hdhr_members[] =
{
    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(silicondust_HDHR_type_doc,
    "An object representing a single HDHomeRun device.");

static PyTypeObject silicondust_hdhr_type =
{
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

/* module methods (none for now) */
static PyMethodDef silicondust_methods[] =
{
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

