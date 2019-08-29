# antirez/rax API说明

## raxInsert

函数声明：

```c
int raxInsert(rax *rax, unsigned char *s, size_t len, void *data, void **old);
```
当key有值的时候，只将**data**的指针保存到`raxNode`的`data`中，所以如果**data**被回收，那么数据就丢失了。出现的问题：

```c
static PyObject * PyRax_insert(PyRaxObject *self, PyObject *args, PyObject *kw) {
    char* kwlist[] = {"key", "data", NULL};
    char *key;
    char *data;
    int ret;

    if(!PyArg_ParseTupleAndKeywords(args, kw, "s|s", kwlist, &key, &data)) {
        ret = -1;
        return PyLong_FromSize_t(ret);
    }
    ret = raxInsert(self->rt, (unsigned char *)key, strlen(key), data, NULL);
    return PyLong_FromSize_t(ret);
}
```
当在`PyRax_insert`函数中调用`raxInsert`，函数返回的时候，**data**的值将会被销毁。对应的**raxNode**中的指针地址不变，但是数据已经改变。
