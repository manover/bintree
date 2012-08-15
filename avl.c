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
#define MAX(a,b) (a > b ? a : b)

typedef struct Node {
    PyObject_HEAD
    struct Node *left;
    struct Node *right;
    PyObject *key;
    struct Node *parent;
    int bf;
} Node;

Node * Node__new(PyTypeObject *type,
                 PyObject *key,
                 Node *left,
                 Node *right,
                 Node *parent
                )
{
    Node *node;

    node = (Node *)type->tp_new(type, Py_None, Py_None);

    node->key = key;
    node->left = left;
    node->right = right;
    node->parent = parent;

    Py_INCREF(key);
    Py_INCREF(left);
    Py_INCREF(right);
    Py_INCREF(parent);

    return node;
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

static void Node__update_bf_on_increase(Node *self, int delta, int dont_rebalance)
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
            Node__update_bf_on_increase(
                parent,
                Node__get_child_place(parent, self),
                abs(bf) > 1 || dont_rebalance
            );
    }
    if (abs(bf) > 1 && !dont_rebalance)
        Node__rebalance(self);
}

static void Node__update_bf_on_decrease(Node *self, int delta, int dont_rebalance)
{
    int bf;
    Node *parent;

    self->bf += delta;
    bf = self->bf;

    if (bf == 0 || SIGN(bf) != SIGN(delta)) {
        parent = self->parent;
        if (parent != (Node *)Py_None)
            Node__update_bf_on_decrease(
                parent,
                -Node__get_child_place(parent, self),
                (abs(bf) > 1 || dont_rebalance)
            );
    }
    if (abs(bf) > 1 && !dont_rebalance)
        Node__rebalance(self);
}

static void Node__disconnect(Node *self, Node *node)
{
    if (self->left == node) {
        Py_DECREF(self->left);
        Py_INCREF(Py_None);
        self->left = (Node *)Py_None;
    } else {
        Py_DECREF(self->right);
        Py_INCREF(Py_None);
        self->right = (Node *)Py_None;
    }
}

static Node * Node__rightmost(Node *self)
{
    if (self->right != (Node *)Py_None)
        return Node__rightmost(self->right);
    else
        return self;
}

PyObject * Node__insert(Node *self, PyObject *key)
{
    Node *p, *n;

    p = Node__search(self, key);

    if (!PyObject_Compare(p->key, key)) {
        PyErr_SetString(PyExc_KeyError, "key already present");
        return NULL;
    } else {
        n = Node__new(self->ob_type, key, (Node *)Py_None, (Node *)Py_None, self);
        Node__update_bf_on_increase(p, Node__connect_to_parent(n, p), 0);
        Py_DECREF(n);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static uint Node__height(Node *self)
{
    uint h_left=0, h_right=0;

    if (self->left != (Node *)Py_None)
        h_left = Node__height(self->left);

    if (self->right != (Node *)Py_None)
        h_right = Node__height(self->right);

    return 1 + MAX(h_left, h_right);
}

static int Node__calc_bf(Node *self)
{
    uint h_left=0, h_right=0;

    if (self->left != (Node *)Py_None)
        h_left = Node__height(self->left);

    if (self->right != (Node *)Py_None)
        h_right = Node__height(self->right);

    return h_left - h_right;
}

static Node * Node__from_list_raw(PyTypeObject *type, PyObject *l, Node *parent)
{
    PyObject *key, *left, *right;
    Node *node, *tnode;

    if (!PyArg_ParseTuple(l, "OOO", &key, &left, &right)) {
        return NULL;
    }

    node = Node__new(type, key, (Node *)Py_None, (Node *)Py_None, parent);

    if (left != Py_None) {
        tnode = Node__from_list_raw(type, left, node);
        if (tnode) {
            Py_DECREF(node->left);
            node->left = tnode;
        }
    }

    if (right != Py_None) {
        tnode = Node__from_list_raw(type, right, node);
        if (tnode) {
            Py_DECREF(node->right);
            node->right = tnode;
        }
    }

    node->bf = Node__calc_bf(node);
    return node;
}

static void Node__move(Node *self, Node *node)
{
    node->key = self->key;
    node->left = self->left;
    node->right = self->right;
    node->bf = self->bf;
}

#if 0
                    PARENT                  PARENT
                    /    \                  /     \
                RIGHT           =>      PIVOT
                /   \                   /   \
            PIVOT    A              LEFT    RIGHT
            /   \                          /    \
        LEFT     B                        B      A
#endif

static int Node__rotate_cw(Node *self)
{
    Node *right = self->parent;
    Node *a, *pivot, *parent;
    PyObject *r_key;
    int old_bf, delta;

    if (right == (Node *)Py_None) {
        PyErr_SetString(PyExc_RuntimeError, "can't rotate root node");
        return -1;
    } else if (right->right == self) {
        PyErr_SetString(PyExc_RuntimeError, "can't rotate right subtree CW");
        return -1;
    }

    // Save old subtree bf
    old_bf = right->bf;
    // Save the subtree
    a = right->right;
    r_key = right->key;
    // Move PIVOT into RIGHT
    Node__move(self, right);

    // Manually reconnect LEFT to the new PIVOT
    if (self->left != (Node *)Py_None) {
        // Disconnect old PIVOT from LEFT
        Py_DECREF(self->left->parent);
        // Reconnect LEFT to the new PIVOT, no incref needed
        self->left->parent = right;
    }
    right->left = self->left;

    // old PIVOT is the new RIGHT
    self->key = r_key;
    right->right = self;
    // Move B to the left subtree
    self->left = self->right;

    // Reconnect A to the new RIGHT
    if (a != (Node *)Py_None) {
        a->parent = self;
        Py_INCREF(a->parent);
    }
    self->right = a;

    // Update bf's
    // Redefine variables to catch up with the rotation changes
    pivot = right;
    right = self;
    parent = pivot->parent;
    // RIGHT's left subtree is 1 node shorter now (minus PIVOT)
    right->bf = old_bf - 1;
    if (pivot->bf > 0)
        // If PIVOT bf > 0, PIVOT subtree height was represented by LEFT
        // LEFT is no longer RIGHT's successor, so compensate for it
        right->bf -= pivot->bf;
    // PIVOT's right subtree is 1 node longer now (plus RIGHT)
    pivot->bf -= 1;
    if (right->bf < 0)
        // If RIGHT bf < 0, RIGHT subtree height is represented by A
        // A is now PIVOT's successor, update bf accordingly
        pivot->bf += right->bf;

    delta = abs(pivot->bf) - abs(old_bf);
    if (parent != (Node *)Py_None) {
        // When rotating, every height change in one node is accounted
        // for double change in bf, e.g. when rotating tree with bf = 2 CW,
        // the new bf will be 0, height will decrease by 1
        if (delta > 0)
            // Subtree height increased
            Node__update_bf_on_increase(parent, delta/2 * Node__get_child_place(parent, pivot), 0);
        else if (delta < 0)
            // Subtree height decreased
            Node__update_bf_on_decrease(parent, delta/2 * Node__get_child_place(parent, pivot), 0);
    }

    return 0;
}

#if 0
        PARENT                    PARENT
        /    \                   /     \
             LEFT         =>          PIVOT
            /    \                    /   \
           A    PIVOT              LEFT    RIGHT
               /     \            /    \
              B     RIGHT        A      B
#endif

static int Node__rotate_ccw(Node *self)
{
    Node *left = self->parent;
    Node *a, *pivot, *parent;
    PyObject *l_key;
    int old_bf, delta;

    if (left == (Node *)Py_None) {
        PyErr_SetString(PyExc_RuntimeError, "can't rotate root node");
        return -1;
    } else if (left->left == self) {
        PyErr_SetString(PyExc_RuntimeError, "can't rotate left subtree CCW");
        return -1;
    }

    // Save old subtree bf
    old_bf = left->bf;
    // Save the subtree
    a = left->left;
    l_key = left->key;
    // Move PIVOT into LEFT
    Node__move(self, left);

    // Manually reconnect RIGHT to the new PIVOT
    if (self->right != (Node *)Py_None) {
        // Disconnect old PIVOT from RIGHT
        Py_DECREF(self->right->parent);
        // Reconnect RIGHT to the new PIVOT, no incref needed
        self->right->parent = left;
    }
    left->right = self->right;

    // old PIVOT is the new LEFT
    self->key = l_key;
    left->left = self;
    // Move B to the right subtree
    self->right = self->left;

    // Reconnect A to the new LEFT
    if (a != (Node *)Py_None) {
        a->parent = self;
        Py_INCREF(a->parent);
    }
    self->left = a;

    // Update bf's
    // Redefine variables to catch up with the rotation changes
    pivot = left;
    left = self;
    parent = pivot->parent;
    // LEFT's right subtree is 1 node shorter now (minus PIVOT)
    left->bf = old_bf + 1;
    if (pivot->bf < 0)
        // If PIVOT bf < 0 PIVOT subtree height was represented by RIGHT
        // RIGHT is no longer LEFT's successor, so compensate for it
        pivot->bf += left->bf;

    // PIVOT's left subtree is 1 node longer now (plus LEFT)
    pivot->bf += 1;
    if (left->bf > 0)
        // If LEFT bf > 0 LEFT subtree height is represented by A
        // A is now PIVOT's successor, update bf accordingly
        pivot->bf += left->bf;

    delta = abs(pivot->bf) - abs(old_bf);
    if (parent != (Node *)Py_None) {
        // When rotating, every height change in one node is accounted
        // for double change in bf, e.g. when rotating tree with bf = 2 CW,
        // the new bf will be 0, height will decrease by 1
        if (delta > 1)
            // Subtree height increased
            Node__update_bf_on_increase(parent, delta/2 * Node__get_child_place(parent, pivot), 0);
        else if (delta < -1)
            // Subtree height decreased
            Node__update_bf_on_decrease(parent, delta/2 * Node__get_child_place(parent, pivot), 0);
    }

    return 0;
}
/********************* Export functions ********************************/

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

    if (!PyArg_ParseTuple(args, "O", &key))
        return NULL;

    return Node__insert(self, key);
}

static PyObject * Node_delete(Node *self, PyObject *args)
{
    Node *node, *p;
    PyObject *key, *nkey, *vargs;
    int bf = 0;

    if (!PyArg_ParseTuple(args, "O", &key))
        return NULL;

    node = Node_search(self, args);
    p = node->parent;

    if (node->left != (Node *)Py_None && node->right != (Node *)Py_None) {
        // Both children exist
        nkey = (Node__rightmost(node->left))->key;
        Py_INCREF(nkey);

        vargs = Py_BuildValue("(O)", nkey);
        Py_DECREF(Node_delete(node->left, vargs));
        Py_DECREF(vargs);

        node->key = nkey;
    } else {
        if (p != (Node *)Py_None)
            bf = Node__get_child_place(p, node);

        if (node->left != (Node *)Py_None) // Only left child exists
            Node__connect_to_parent(node->left, p);
        else if (node->right != (Node *)Py_None) // Only right child exists
            Node__connect_to_parent(node->right, p);
        else if (p != (Node *)Py_None) // No children exist, parent is not None
            Node__disconnect(p, node);
        else // Parent is None
            PyErr_SetString(PyExc_ValueError, "can't remove the last node");

        if (p != (Node *)Py_None)
            Node__update_bf_on_decrease(p, -bf, 0);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Node_from_list(PyTypeObject *type, PyObject *args)
{
    PyObject *o, *l;
    Py_ssize_t len, i;
    PyObject **arr;
    Node *tree;

    if (!PyArg_ParseTuple(args, "O", &o))
        return NULL;

    l = PySequence_Fast(o, "sequence is required");
    if (!l)
        return NULL;

    len = PySequence_Fast_GET_SIZE(l);
    arr = PySequence_Fast_ITEMS(l);

    if (len < 1) {
        Py_INCREF(Py_None);
        return Py_None;
    } else
        tree = Node__new(type, arr[0], (Node *)Py_None,
                (Node *)Py_None, (Node *)Py_None);

    for (i=1; i<len; i++)
        Node__insert(tree, arr[i]);

    return (PyObject *)tree;
}

static PyObject * Node_from_list_raw(PyTypeObject *type, PyObject *args)
{
    PyObject *l, *parent=NULL;
    Node *node;

    if (!PyArg_ParseTuple(args, "O|O", &l, &parent))
        return NULL;

    if (!parent)
        parent = Py_None;

    node = Node__from_list_raw(type, l, (Node *)parent);

    if (node)
        return (PyObject *)node;
    else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static PyObject * Node_to_list(Node *self)
{
    PyObject *left=Py_None, *right=Py_None;

    if (self->left != (Node *)Py_None)
        left = Node_to_list(self->left);
    else
        Py_INCREF(left);

    if (self->right != (Node *)Py_None)
        right = Node_to_list(self->right);
    else
        Py_INCREF(right);

    return Py_BuildValue("ONN", self->key, left, right);
}

static PyObject * Node_rightmost(Node *self)
{
    PyObject * node;

    node = (PyObject *)Node__rightmost(self);
    Py_INCREF(node);

    return node;
}

static PyObject * Node_height(Node *self)
{
    return Py_BuildValue("I", Node__height(self));
}

static PyObject * Node_calc_bf(Node *self)
{
    return Py_BuildValue("i", Node__calc_bf(self));
}

static PyObject * Node_rotate_cw(Node *self)
{
    if (!Node__rotate_cw(self)) {
        Py_INCREF(Py_None);
        return Py_None;
    } else
        return NULL;
}

static PyObject * Node_rotate_ccw(Node *self)
{
    if (!Node__rotate_ccw(self)) {
        Py_INCREF(Py_None);
        return Py_None;
    } else
        return NULL;
}

static PyObject * Node_traverse(Node *self, PyObject *args, PyObject *kwargs)
{
    PyObject *f, *nargs;

    f = PyTuple_GetItem(args, 0);
    if (!f) {
        PyErr_SetString(PyExc_TypeError, "function takes at least 1 argument");
        return NULL;
    }

    if (!PyCallable_Check(f)) {
        PyErr_SetString(PyExc_TypeError, "first parameter must be a callable object");
        return NULL;
    }

    nargs = PyTuple_GetSlice(args, 0, PyTuple_GET_SIZE(args));
    Py_INCREF(self);
    PyTuple_SET_ITEM(nargs, 0, (PyObject *)self);
    return nargs;

    Py_DECREF(PyObject_Call(f, nargs, kwargs));
    Py_DECREF(nargs);

    if (self->left != (Node *)Py_None)
        Py_DECREF(Node_traverse(self->left, args, kwargs));
    if (self->right != (Node *)Py_None)
        Py_DECREF(Node_traverse(self->right, args, kwargs));

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
    PyObject *s;
    Node *self = (Node *)o;
    PyObject *r_key = PyObject_Repr(self->key);
    PyObject *r_left = PyObject_Repr((PyObject *)self->left);
    PyObject *r_right = PyObject_Repr((PyObject *)self->right);

    s = PyString_FromFormat("%s(%s,%s)",
            PyString_AS_STRING(r_key),
            PyString_AS_STRING(r_left),
            PyString_AS_STRING(r_right));

    Py_DECREF(r_key);
    Py_DECREF(r_left);
    Py_DECREF(r_right);

    return s;
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
    {"delete", (PyCFunction)Node_delete, METH_VARARGS,
     "Deletes a key from a tree"
    },
    {"from_list", (PyCFunction)Node_from_list, METH_VARARGS | METH_CLASS,
     "Builds a tree from a sequence"
    },
    {"from_list_raw", (PyCFunction)Node_from_list_raw, METH_VARARGS | METH_CLASS,
     "Builds a tree from a tuple tree"
    },
    {"to_list", (PyCFunction)Node_to_list, METH_NOARGS,
     "Builds a tuple tree"
    },
    {"rightmost", (PyCFunction)Node_rightmost, METH_NOARGS,
     "Returns the rightmost node"
    },
    {"height", (PyCFunction)Node_height, METH_NOARGS,
     "Returns tree height"
    },
    {"calc_bf", (PyCFunction)Node_calc_bf, METH_NOARGS,
     "Calculates tree balance factor"
    },
    {"rotate_cw", (PyCFunction)Node_rotate_cw, METH_NOARGS,
     "Rotates a node clock-wise"
    },
    {"rotate_ccw", (PyCFunction)Node_rotate_ccw, METH_NOARGS,
     "Rotates a node counter clock-wise"
    },
    {"traverse", (PyCFunction)Node_traverse, METH_KEYWORDS,
     "Traverses a tree"
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
