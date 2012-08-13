/*
    AVL tree
    Python C module


    Code: Denis Bychkov (manover@gmail.com)
    Date: 2012-08-04
*/
#include "Python.h"
#include "structmember.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;

static PyTypeObject NodeType;

#define SIGN(n) ((n >= 0) - (n < 0))

typedef struct Node {
    PyObject_HEAD
    struct Node *left;
    struct Node *right;
    PyObject *key;
    struct Node *parent;
    int bf;
} Node;

static int Node_init(Node *self, PyObject *args)
{
    PyObject *key, *tmp;
    Node *left = NULL;
    Node *right = NULL;
    Node *parent = NULL;
    
    if (!PyArg_ParseTuple(args, "O|OOO", &key, &left, &right, &parent))
        return -1;

    if (!left)
        left = (Node *)Py_None;
    if (!right)
        right = (Node *)Py_None;
    if (!parent)
        parent = (Node *)Py_None;
    
    tmp = self->key;
    Py_INCREF(key);
    self->key = key;
    Py_XDECREF(tmp);
    
    tmp = (PyObject *)self->left;
    Py_INCREF(left);
    self->left = left;
    Py_XDECREF(tmp);

    tmp = (PyObject *)self->right;
    Py_INCREF(right);
    self->right = right;
    Py_XDECREF(tmp);

    tmp = (PyObject *)self->parent;
    Py_INCREF(parent);
    self->parent = parent;
    Py_XDECREF(tmp);
    
    return 0;
}

static Node * Node__search(Node *self, PyObject *key)
{
    /*
        Returns the corresponding node if found, the last checked otherwise
    */
    
    Node *n = self;
    Node *last = NULL;
    
    while (n != (Node *)Py_None) {
        last = n;
        
        switch (PyObject_Compare(key, n->key)) {
            case -1:
                n = n->left;
                break;
            case 1:
                n = n->right;
                break;
            default:
                Py_INCREF(n);
                return n;
        }
    }
    
    Py_INCREF(last);
    return last;
}

static int Node__get_child_place(Node *self, Node *child)
{
    return PyObject_Compare(self->key, child->key);
}

static int Node__connect(Node *self, Node *node)
{
    int bf;
    
    bf = Node__get_child_place(self, node);

    Py_INCREF(node);
    if (bf == 1) {        
        Py_DECREF(self->left);
        self->left = node;
    } else {
        Py_DECREF(self->right);
        self->right = node;
    }
    
    return bf;
}

static int Node__connect_to_parent(Node *self, Node *parent)
{
    int bf;
    
    bf = Node__connect(parent, self);

    Py_DECREF(self->parent);
    Py_INCREF(parent);
    self->parent = parent;
    
    return bf;
}

static void Node__rebalance(Node *self)
{
    return;
}

static void Node__update_bf_on_insert(Node *self, int delta, int dont_rebalance)
{
    int bf;
    Node *parent;
    
    self->bf += delta;
    bf = self->bf;
    
    if (bf == 0 || SIGN(bf) != SIGN(delta))
        return;
    else {
        parent = self->parent;
        if (parent != (Node *)Py_None)
            Node__update_bf_on_insert(
                parent,
                Node__get_child_place(parent, self),
                abs(bf) > 1 || dont_rebalance
            );
    }
    if (abs(bf) > 1 && !dont_rebalance)
        Node__rebalance(self);
}

static void Node__update_bf_on_delete(Node *self, int delta, int dont_rebalance)
{
    return;
}

static Node * Node_search(Node *self, PyObject *args)
{
    Node *n;
    PyObject *key;
    
    if (!PyArg_ParseTuple(args, "O", &key))
        return NULL;
    
    n = Node__search(self, key);
    if (!PyObject_Compare(n->key, key))
        return n;
    else {
        Py_DECREF(n);
        PyErr_SetString(PyExc_KeyError, "key not found");
        return NULL;
    }
}

static PyObject * Node_insert(Node *self, PyObject *args)
{
    PyObject *key;
    Node *p, *n;
    
    if (!PyArg_ParseTuple(args, "O", &key))
        return NULL;
    
    p = Node__search(self, key);
    
    if (!PyObject_Compare(p->key, key)) {
        PyErr_SetString(PyExc_KeyError, "key already present");
        return NULL;
    } else {
        n = (Node *) NodeType.tp_new((PyTypeObject *) &NodeType, Py_None, Py_None);
        Node_init(n, Py_BuildValue("OOOO", key, Py_None, Py_None, self));
        Node__update_bf_on_insert(p, Node__connect_to_parent(n, p), 0);
        Py_DECREF(n);
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMemberDef Node_members[] = {
    {"left", T_OBJECT_EX, offsetof(Node, left), 0, "left child"},
    {"right", T_OBJECT_EX, offsetof(Node, right), 0, "right child"},
    {"key", T_OBJECT_EX, offsetof(Node, key), 0, "node key"},
    {"parent", T_OBJECT_EX, offsetof(Node, parent), 0, "node parent"},
    {"bf", T_INT, offsetof(Node, bf), 0, "balance factor"},
    {NULL}  /* Sentinel */
};

static PyObject * Node_Repr(PyObject *o)
{
    Node *self = (Node *)o;
    PyObject *r_key = PyObject_Repr(self->key);
    PyObject *r_left = PyObject_Repr((PyObject *)self->left);
    PyObject *r_right = PyObject_Repr((PyObject *)self->right);    
    
    return PyString_FromFormat("%s(%s,%s)",
            PyString_AS_STRING(r_key),
            PyString_AS_STRING(r_left),
            PyString_AS_STRING(r_right));
}

static void Node_dealloc(Node *self)
{
    Py_XDECREF(self->left);
    Py_XDECREF(self->right);
    Py_XDECREF(self->parent);
    Py_XDECREF(self->key);
    self->ob_type->tp_free((PyObject *)self);
}

static PyMethodDef Node_methods[] = {
    {"search", (PyCFunction)Node_search, METH_VARARGS,
     "Returns the corresponding node if found, the last checked otherwise"
    },
    {"insert", (PyCFunction)Node_insert, METH_VARARGS,
     "Inserts a new key into a tree"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject NodeType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "avl.Node",                 /*tp_name*/
    sizeof(Node),               /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Node_dealloc,   /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    Node_Repr,                  /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Node object",              /* tp_doc */
    0,	    	               /* tp_traverse */
    0,	                       /* tp_clear */
    0,	                       /* tp_richcompare */
    0,	                       /* tp_weaklistoffset */
    0,	                       /* tp_iter */
    0,	                       /* tp_iternext */
    Node_methods,              /* tp_methods */
    Node_members,              /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Node_init,        /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};

static PyMethodDef avl_methods[] = {
    {NULL}  /* Sentinel */
};

PyMODINIT_FUNC
initavl(void) 
{
    PyObject* m;

    NodeType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&NodeType) < 0)
        return;

    m = Py_InitModule3("avl", avl_methods,
                       "Avl module.");

    Py_INCREF(&NodeType);
    PyModule_AddObject(m, "Node", (PyObject *)&NodeType);
}
