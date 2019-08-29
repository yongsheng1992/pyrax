#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "rax.h"

// static PyObject *PyRaxErr;

typedef struct {
    PyObject_HEAD
    rax *rt;
} PyRaxObject;


static PyObject * PyRax_new(PyTypeObject *type, PyObject *args, PyObject **kwds) {
    PyRaxObject *self;
    self = (PyRaxObject *)type->tp_alloc(type, 0);
    return (PyObject *) self;
}

static int PyRax_init(PyRaxObject *self, PyObject *args, PyObject *kwds) {
    self->rt = raxNew();
    return 0;
}

static PyObject * PyRax_insert(PyRaxObject *self, PyObject *args, PyObject *kw) {
    char* kwlist[] = {"key", "data", NULL};
    char *key;
    char *data;
    int ret;
    PyObject *py_bytes;
    PyObject *old;

    if(!PyArg_ParseTupleAndKeywords(args, kw, "s|s", kwlist, &key, &data)) {
        ret = -1;
        return PyLong_FromSize_t(ret);
    }

    py_bytes = PyBytes_FromString(data);

    Py_INCREF(py_bytes);
    ret = raxInsert(self->rt, (unsigned char *)key, strlen(key), (void *)py_bytes, (void *)old);
    Py_DECREF(old);
    
    return PyLong_FromSize_t(ret);
}

static PyObject * PyRax_find(PyRaxObject *self, PyObject *args) {
    char *key;
    void *ret;
    
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;
    
    ret = raxFind(self->rt, (unsigned char *)key, strlen(key));
    
    if (ret == raxNotFound) {
        return Py_None;
    }
    
    Py_INCREF((PyObject *)ret);
    return (PyObject *)ret;
}

static PyObject * PyRax_remove(PyRaxObject *self, PyObject *args) {
    char *key;
    int ret;
    PyObject *old;

    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;
    
    ret = raxRemove(self->rt, (unsigned char *)key, strlen(key), old);
    Py_DECREF(old);

    return PyLong_FromSize_t(ret);
}

static void Pyrax_dealloc(PyRaxObject *self) {
    PyObject_GC_UnTrack(self);
    raxFree(self->rt);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyMethodDef PyRax_methods[] = {
    {"insert", (PyCFunction) PyRax_insert, METH_VARARGS|METH_KEYWORDS},
    {"find", (PyCFunction) PyRax_find, METH_VARARGS},
    {NULL}
};

static PyTypeObject PyRaxType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyrax.PyRax",
    .tp_doc = "Radix tree",
    .tp_basicsize = sizeof(PyRaxObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyRax_new,
    .tp_init = (initproc) PyRax_init,
    .tp_dealloc = Pyrax_dealloc,
    .tp_methods = PyRax_methods
};


static PyModuleDef pyraxmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pyrax",
    .m_doc = "Radix tree for Python",
    .m_size = -1
};



PyMODINIT_FUNC PyInit_pyrax(void) {
    PyObject *m;

    if (PyType_Ready(&PyRaxType) < 0)
        return NULL;
  
    m = PyModule_Create(&pyraxmodule);
    if (m == NULL)
        return NULL;
    
    Py_INCREF(&PyRaxType);
    PyModule_AddObject(m, "PyRax", (PyObject *) &PyRaxType);
    // PyRaxErr = PyErr_NewException("pyrax.error", NULL, NULL);

    // PyModule_AddObject(m, "error", PyRaxErr);
    
    return m;
}
