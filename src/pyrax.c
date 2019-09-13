#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "rax.h"

// static PyObject *PyRaxErr;

typedef struct {
    PyObject_HEAD
    rax *rt;
} PyRaxObject;

typedef struct {
    PyObject_HEAD
    raxIterator it;
} PyRaxIterator;


static PyObject * PyRaxIterator_new(PyTypeObject *type, PyObject *args, PyObject **kwds) {
    PyRaxIterator *self;
    self = (PyRaxIterator *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int PyRaxIterator_init(PyRaxIterator *self, PyObject *args, PyObject *kwds) {
    PyObject *rax;
    char *key, *op;

    if (!PyArg_ParseTuple(args, "Oss", &rax, &key, &op)) {
        return 1;
    }

    raxStart(&self->it, ((PyRaxObject *)rax)->rt);
    raxSeek(&self->it, op, (unsigned char *)key, strlen(key));
    
    return 0;
}

static void PyRaxIterator_dealloc(void *self) {
    raxStop(&((PyRaxIterator *)self)->it);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * PyRaxIterator_iter(PyRaxIterator *self) {
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject * PyRaxIterator_next(PyRaxIterator *self) {
    if(raxNext(&self->it)) {
        return Py_BuildValue("s", self->it.key);
    }
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
}

static PyTypeObject PyRaxIteratorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyrax.PyRaxItertor",
    .tp_doc = "Itertor",
    .tp_basicsize = sizeof(PyRaxIterator),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = (newfunc) PyRaxIterator_new,
    .tp_init = (initproc) PyRaxIterator_init,
    .tp_dealloc = (destructor) PyRaxIterator_dealloc,
    .tp_iter = (getiterfunc) PyRaxIterator_iter,
    .tp_iternext = (iternextfunc) PyRaxIterator_next
};

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
    char *kwlist[] = {"key", "data", NULL};
    const char *key;
    const char *data;
    int ret;
    void *old = NULL;
    size_t data_len;

    if(!PyArg_ParseTupleAndKeywords(args, kw, "s|s", kwlist, &key, &data)) {
        ret = -1;
        return PyLong_FromSize_t(ret);
    }

    data_len = strlen(data);
    char *buf = (char *)malloc(data_len+1);
    strcpy(buf, data);

    printf("data len %ld\n", data_len);
    ret = raxInsert(self->rt, (unsigned char *)key, strlen(key), (void *)buf, &old);
    if (ret == 0 && old && old != buf) {
        free(old);
    }
    return PyLong_FromSize_t(ret);
}

static PyObject * PyRax_find(PyRaxObject *self, PyObject *args) {
    char *key;
    void *data;
    
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;
    
    data = raxFind(self->rt, (unsigned char *)key, strlen(key));
    
    if (data == raxNotFound) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return Py_BuildValue("s", (char *)data);
}

static PyObject * PyRax_remove(PyRaxObject *self, PyObject *args) {
    char *key;
    int ret;
    void *old = NULL;

    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;
    
    ret = raxRemove(self->rt, (unsigned char *)key, strlen(key), &old);
    if (ret == 1) {
        free(old);
    }

    return PyLong_FromSize_t(ret);
}

static PyObject * PyRax_size(PyRaxObject *self) {
    unsigned long ret = raxSize(self->rt);
    return Py_BuildValue("k", ret);
}

static PyObject * PyRax_seek(PyRaxObject *self, PyObject *args) {
    // create and return an PyRaxIterator object.
    char *key, *op;

    if (!PyArg_ParseTuple(args, "ss", &key, &op)) {
        return NULL;
    }
    printf("%s %s\n", key, op);

    PyObject *args_list = Py_BuildValue("Oss", self, key, op);
    return PyObject_CallObject((PyObject *) &PyRaxIteratorType, args_list);
}

static void PyRaxNodeFreeCallback(void *data) {
    free(data);
}

static void Pyrax_dealloc(void *self) {
    void (*func)(void *data);
    func = PyRaxNodeFreeCallback;
    raxFreeWithCallback(((PyRaxObject *)self)->rt, func);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyMethodDef PyRax_methods[] = {
    {"insert", (PyCFunction) PyRax_insert, METH_VARARGS|METH_KEYWORDS},
    {"find", (PyCFunction) PyRax_find, METH_VARARGS},
    {"remove", (PyCFunction) PyRax_remove, METH_VARARGS},
    {"size", (PyCFunction) PyRax_size, METH_NOARGS},
    {"seek", (PyCFunction) PyRax_seek, METH_VARARGS},
    {NULL}
};

static PyTypeObject PyRaxType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyrax.PyRax",
    .tp_doc = "Radix tree",
    .tp_basicsize = sizeof(PyRaxObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = (newfunc) PyRax_new,
    .tp_init = (initproc) PyRax_init,
    .tp_dealloc = (destructor) Pyrax_dealloc,
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

    if (PyType_Ready(&PyRaxIteratorType) < 0)
        return NULL;
  
    m = PyModule_Create(&pyraxmodule);
    if (m == NULL)
        return NULL;
    
    Py_INCREF(&PyRaxIteratorType);
    PyModule_AddObject(m, "PyRaxIterator", (PyObject *) &PyRaxIteratorType);

    Py_INCREF(&PyRaxType);
    PyModule_AddObject(m, "PyRax", (PyObject *) &PyRaxType);
    return m;
}
