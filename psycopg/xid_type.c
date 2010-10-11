/* xid_type.c - python interface to Xid objects
 *
 * Copyright (C) 2008  Canonical Ltd.
 * Copyright (C) 2010  Daniele Varrazzo <daniele.varrazzo@gmail.com>
 *
 * This file is part of psycopg.
 *
 * psycopg2 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link this program with the OpenSSL library (or with
 * modified versions of OpenSSL that use the same license as OpenSSL),
 * and distribute linked combinations including the two.
 *
 * You must obey the GNU Lesser General Public License in all respects for
 * all of the code used other than OpenSSL.
 *
 * psycopg2 is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#define PSYCOPG_MODULE
#include "psycopg/config.h"
#include "psycopg/python.h"
#include "psycopg/psycopg.h"
#include "psycopg/xid.h"

static PyMemberDef xid_members[] = {
    { "format_id", T_OBJECT, offsetof(XidObject, format_id), RO },
    { "gtrid", T_OBJECT, offsetof(XidObject, gtrid), RO },
    { "bqual", T_OBJECT, offsetof(XidObject, bqual), RO },
    { "prepared", T_OBJECT, offsetof(XidObject, prepared), RO },
    { "owner", T_OBJECT, offsetof(XidObject, owner), RO },
    { "database", T_OBJECT, offsetof(XidObject, database), RO },
    { NULL }
};

static PyObject *
xid_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    XidObject *self = (XidObject *)type->tp_alloc(type, 0);

    Py_INCREF(Py_None);
    self->format_id = Py_None;
    Py_INCREF(Py_None);
    self->gtrid = Py_None;
    Py_INCREF(Py_None);
    self->bqual = Py_None;
    Py_INCREF(Py_None);
    self->prepared = Py_None;
    Py_INCREF(Py_None);
    self->owner = Py_None;
    Py_INCREF(Py_None);
    self->database = Py_None;

    return (PyObject *)self;
}

static int
xid_init(XidObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"format_id", "gtrid", "bqual", NULL};
    int format_id, i, gtrid_len, bqual_len;
    const char *gtrid, *bqual;
    PyObject *tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iss", kwlist,
                                     &format_id, &gtrid, &bqual))
        return -1;

    if (format_id < 0 || format_id > 0x7fffffff) {
        PyErr_SetString(PyExc_ValueError,
                        "format_id must be a non-negative 32-bit integer");
        return -1;
    }

    /* make sure that gtrid is no more than 64 characters long and
       made of printable characters (which we're defining as those
       between 0x20 and 0x7f). */
    gtrid_len = strlen(gtrid);
    if (gtrid_len > 64) {
        PyErr_SetString(PyExc_ValueError,
                        "gtrid must be a string no longer than 64 characters");
        return -1;
    }
    for (i = 0; i < gtrid_len; i++) {
        if (gtrid[i] < 0x20 || gtrid[i] >= 0x7f) {
            PyErr_SetString(PyExc_ValueError,
                            "gtrid must contain only printable characters.");
            return -1;
        }
    }
    /* Same for bqual */
    bqual_len = strlen(bqual);
    if (bqual_len > 64) {
        PyErr_SetString(PyExc_ValueError,
                        "bqual must be a string no longer than 64 characters");
        return -1;
    }
    for (i = 0; i < bqual_len; i++) {
        if (bqual[i] < 0x20 || bqual[i] >= 0x7f) {
            PyErr_SetString(PyExc_ValueError,
                            "bqual must contain only printable characters.");
            return -1;
        }
    }

    tmp = self->format_id;
    self->format_id = PyInt_FromLong(format_id);
    Py_XDECREF(tmp);

    tmp = self->gtrid;
    self->gtrid = PyString_FromString(gtrid);
    Py_XDECREF(tmp);

    tmp = self->bqual;
    self->bqual = PyString_FromString(bqual);
    Py_XDECREF(tmp);

    return 0;
}

static int
xid_traverse(XidObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->format_id);
    Py_VISIT(self->gtrid);
    Py_VISIT(self->bqual);
    Py_VISIT(self->prepared);
    Py_VISIT(self->owner);
    Py_VISIT(self->database);
    return 0;
}

static void
xid_dealloc(XidObject *self)
{
    Py_CLEAR(self->format_id);
    Py_CLEAR(self->gtrid);
    Py_CLEAR(self->bqual);
    Py_CLEAR(self->prepared);
    Py_CLEAR(self->owner);
    Py_CLEAR(self->database);

    self->ob_type->tp_free((PyObject *)self);
}

static void
xid_del(PyObject *self)
{
    PyObject_GC_Del(self);
}

static Py_ssize_t
xid_len(XidObject *self)
{
    return 3;
}

static PyObject *
xid_getitem(XidObject *self, Py_ssize_t item)
{
    if (item < 0)
        item += 3;

    switch (item) {
    case 0:
        Py_INCREF(self->format_id);
        return self->format_id;
    case 1:
        Py_INCREF(self->gtrid);
        return self->gtrid;
    case 2:
        Py_INCREF(self->bqual);
        return self->bqual;
    default:
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }
}

static PySequenceMethods xid_sequence = {
    (lenfunc)xid_len,          /* sq_length */
    0,                         /* sq_concat */
    0,                         /* sq_repeat */
    (ssizeargfunc)xid_getitem, /* sq_item */
    0,                         /* sq_slice */
    0,                         /* sq_ass_item */
    0,                         /* sq_ass_slice */
    0,                         /* sq_contains */
    0,                         /* sq_inplace_concat */
    0,                         /* sq_inplace_repeat */
};

static const char xid_doc[] =
    "A transaction identifier used for two phase commit.";

PyTypeObject XidType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "psycopg2.extensions.Xid",
    sizeof(XidObject),
    0,
    (destructor)xid_dealloc, /* tp_dealloc */
    0,          /*tp_print*/

    0,          /*tp_getattr*/
    0,          /*tp_setattr*/

    0,          /*tp_compare*/

    0,          /*tp_repr*/
    0,          /*tp_as_number*/
    &xid_sequence, /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */

    0,          /*tp_call*/
    0,          /*tp_str*/

    0,          /*tp_getattro*/
    0,          /*tp_setattro*/
    0,          /*tp_as_buffer*/

    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_HAVE_GC, /*tp_flags*/
    xid_doc, /*tp_doc*/

    (traverseproc)xid_traverse, /*tp_traverse*/
    0,          /*tp_clear*/

    0,          /*tp_richcompare*/
    0,          /*tp_weaklistoffset*/

    0,          /*tp_iter*/
    0,          /*tp_iternext*/

    /* Attribute descriptor and subclassing stuff */

    0,          /*tp_methods*/
    xid_members, /*tp_members*/
    0,          /*tp_getset*/
    0,          /*tp_base*/
    0,          /*tp_dict*/

    0,          /*tp_descr_get*/
    0,          /*tp_descr_set*/
    0,          /*tp_dictoffset*/

    (initproc)xid_init, /*tp_init*/
    0, /*tp_alloc  will be set to PyType_GenericAlloc in module init*/
    xid_new, /*tp_new*/
    (freefunc)xid_del, /*tp_free  Low-level free-memory routine */
    0,          /*tp_is_gc For PyObject_IS_GC */
    0,          /*tp_bases*/
    0,          /*tp_mro method resolution order */
    0,          /*tp_cache*/
    0,          /*tp_subclasses*/
    0           /*tp_weaklist*/
};


/* Convert a Python object into a proper xid.
 *
 * Return a new reference to the object or set an exception.
 *
 * The idea is that people can either create a xid from connection.xid
 * or use a regular string they have found in PostgreSQL's pg_prepared_xacts
 * in order to recover a transaction not generated by psycopg.
 */
XidObject *xid_ensure(PyObject *oxid)
{
    XidObject *rv = NULL;

    if (PyObject_TypeCheck(oxid, &XidType)) {
        Py_INCREF(oxid);
        rv = (XidObject *)oxid;
    }
    else if (PyString_Check(oxid)) {
        rv = xid_from_string(oxid);
    }
    else {
        PyErr_SetString(PyExc_TypeError,
            "not a valid transaction id");
    }

    return rv;
}


/* Return a base64-encoded string. */

static PyObject *
_xid_encode64(PyObject *s)
{
    PyObject *base64 = NULL;
    PyObject *encode = NULL;
    PyObject *rv = NULL;

    if (!(base64 = PyImport_ImportModule("base64"))) { goto exit; }
    if (!(encode = PyObject_GetAttrString(base64, "b64encode"))) { goto exit; }
    if (!(rv = PyObject_CallFunctionObjArgs(encode, s, NULL))) { goto exit; }

exit:
    Py_XDECREF(encode);
    Py_XDECREF(base64);

    return rv;
}

/* decode a base64-encoded string */

static PyObject *
_xid_decode64(PyObject *s)
{
    PyObject *base64 = NULL;
    PyObject *decode = NULL;
    PyObject *rv = NULL;

    if (!(base64 = PyImport_ImportModule("base64"))) { goto exit; }
    if (!(decode = PyObject_GetAttrString(base64, "b64decode"))) { goto exit; }
    if (!(rv = PyObject_CallFunctionObjArgs(decode, s, NULL))) { goto exit; }

exit:
    Py_XDECREF(decode);
    Py_XDECREF(base64);

    return rv;
}

/* Return the PostgreSQL transaction_id for this XA xid.
 *
 * PostgreSQL wants just a string, while the DBAPI supports the XA standard
 * and thus a triple. We use the same conversion algorithm implemented by JDBC
 * in order to allow some form of interoperation.
 *
 * The function must be called while holding the GIL.
 * Return a buffer allocated with PyMem_Malloc. Use PyMem_Free to free it.
 *
 * see also: the pgjdbc implementation
 *   http://cvs.pgfoundry.org/cgi-bin/cvsweb.cgi/jdbc/pgjdbc/org/postgresql/xa/RecoveredXid.java?rev=1.2
 */
char *
xid_get_tid(XidObject *self)
{
    char *buf = NULL;
    Py_ssize_t bufsize = 0;
    PyObject *egtrid = NULL;
    PyObject *ebqual = NULL;
    PyObject *format = NULL;
    PyObject *args = NULL;
    PyObject *tid = NULL;

    if (Py_None == self->format_id) {
        /* Unparsed xid: return the gtrid. */
        bufsize = 1 + PyString_Size(self->gtrid);
        if (!(buf = (char *)PyMem_Malloc(bufsize))) {
            PyErr_NoMemory();
            goto exit;
        }
        strncpy(buf, PyString_AsString(self->gtrid), bufsize);
    }
    else {
        /* XA xid: mash together the components. */
        if (!(egtrid = _xid_encode64(self->gtrid))) { goto exit; }
        if (!(ebqual = _xid_encode64(self->bqual))) { goto exit; }

        /* tid = "%d_%s_%s" % (format_id, egtrid, ebqual) */
        if (!(format = PyString_FromString("%d_%s_%s"))) { goto exit; }

        if (!(args = PyTuple_New(3))) { goto exit; }
        Py_INCREF(self->format_id);
        PyTuple_SET_ITEM(args, 0, self->format_id);
        PyTuple_SET_ITEM(args, 1, egtrid); egtrid = NULL;
        PyTuple_SET_ITEM(args, 2, ebqual); ebqual = NULL;

        if (!(tid = PyString_Format(format, args))) { goto exit; }

        bufsize = 1 + PyString_Size(tid);
        if (!(buf = (char *)PyMem_Malloc(bufsize))) {
            PyErr_NoMemory();
            goto exit;
        }
        strncpy(buf, PyString_AsString(tid), bufsize);
    }

exit:
    Py_XDECREF(args);
    Py_XDECREF(format);
    Py_XDECREF(egtrid);
    Py_XDECREF(ebqual);
    Py_XDECREF(tid);

    return buf;
}


/* Return the regex object to parse a Xid string.
 *
 * Return a borrowed reference. */

static PyObject *
_xid_get_parse_regex(void) {
    static PyObject *rv;

    if (!rv) {
        PyObject *re_mod = NULL;
        PyObject *comp = NULL;
        PyObject *regex = NULL;

        Dprintf("compiling regexp to parse transaction id");

        if (!(re_mod = PyImport_ImportModule("re"))) { goto exit; }
        if (!(comp = PyObject_GetAttrString(re_mod, "compile"))) { goto exit; }
        if (!(regex = PyObject_CallFunction(comp, "s",
                "^(\\d+)_([^_]*)_([^_]*)$"))) {
            goto exit;
        }

        /* Good, compiled. */
        rv = regex;
        regex = NULL;

exit:
        Py_XDECREF(regex);
        Py_XDECREF(comp);
        Py_XDECREF(re_mod);
    }

    return rv;
}

/* Try to parse a Xid string representation in a Xid object.
 *
 *
 * Return NULL + exception if parsing failed. Else a new Xid object. */

static XidObject *
_xid_parse_string(PyObject *str) {
    PyObject *regex;
    PyObject *m = NULL;
    PyObject *group = NULL;
    PyObject *item = NULL;
    PyObject *format_id = NULL;
    PyObject *egtrid = NULL;
    PyObject *ebqual = NULL;
    PyObject *gtrid = NULL;
    PyObject *bqual = NULL;
    XidObject *rv = NULL;

    /* check if the string is a possible XA triple with a regexp */
    if (!(regex = _xid_get_parse_regex())) { goto exit; }
    if (!(m = PyObject_CallMethod(regex, "match", "O", str))) { goto exit; }
    if (m == Py_None) {
        PyErr_SetString(PyExc_ValueError, "bad xid format");
        goto exit;
    }

    /* Extract the components from the regexp */
    if (!(group = PyObject_GetAttrString(m, "group"))) { goto exit; }
    if (!(item = PyObject_CallFunction(group, "i", 1))) { goto exit; }
    if (!(format_id = PyObject_CallFunctionObjArgs(
            (PyObject *)&PyInt_Type, item, NULL))) {
        goto exit;
    }
    if (!(egtrid = PyObject_CallFunction(group, "i", 2))) { goto exit; }
    if (!(gtrid = _xid_decode64(egtrid))) { goto exit; }

    if (!(ebqual = PyObject_CallFunction(group, "i", 3))) { goto exit; }
    if (!(bqual = _xid_decode64(ebqual))) { goto exit; }

    /* Try to build the xid with the parsed material */
    rv = (XidObject *)PyObject_CallFunctionObjArgs((PyObject *)&XidType,
        format_id, gtrid, bqual, NULL);

exit:
    Py_XDECREF(bqual);
    Py_XDECREF(ebqual);
    Py_XDECREF(gtrid);
    Py_XDECREF(egtrid);
    Py_XDECREF(format_id);
    Py_XDECREF(item);
    Py_XDECREF(group);
    Py_XDECREF(m);

    return rv;
}

/* Return a new Xid object representing a transaction ID not conform to
 * the XA specifications. */

static XidObject *
_xid_unparsed_from_string(PyObject *str) {
    XidObject *xid = NULL;
    XidObject *rv = NULL;
    PyObject *tmp;

    /* fake args to work around the checks performed by the xid init */
    if (!(xid = (XidObject *)PyObject_CallFunction((PyObject *)&XidType,
            "iss", 0, "", ""))) {
        goto exit;
    }

    /* set xid.gtrid = str */
    tmp = xid->gtrid;
    Py_INCREF(str);
    xid->gtrid = str;
    Py_DECREF(tmp);

    /* set xid.format_id = None */
    tmp = xid->format_id;
    Py_INCREF(Py_None);
    xid->format_id = Py_None;
    Py_DECREF(tmp);

    /* set xid.bqual = None */
    tmp = xid->bqual;
    Py_INCREF(Py_None);
    xid->bqual = Py_None;
    Py_DECREF(tmp);

    /* return the finished object */
    rv = xid;
    xid = NULL;

exit:
    Py_XDECREF(xid);

    return rv;
}

/* Build a Xid from a string representation.
 *
 * If the xid is in the format generated by Psycopg, unpack the tuple into
 * the struct members. Otherwise generate an "unparsed" xid.
 */
XidObject *
xid_from_string(PyObject *str) {
    XidObject *rv;

    /* Try to parse an XA triple from the string. This may fail for several
     * reasons, such as the rules stated in Xid.__init__. */
    rv = _xid_parse_string(str);
    if (!rv) {
        /* If parsing failed, treat the string as an unparsed id */
        PyErr_Clear();
        rv = _xid_unparsed_from_string(str);
    }

    return rv;
}


/* conn_tpc_recover -- return a list of pending TPC Xid */

PyObject *
xid_recover(PyObject *conn)
{
    PyObject *rv = NULL;
    PyObject *curs = NULL;
    PyObject *xids = NULL;
    XidObject *xid = NULL;
    PyObject *recs = NULL;
    PyObject *rec = NULL;
    PyObject *item = NULL;
    PyObject *tmp;
    Py_ssize_t len, i;

    /* curs = conn.cursor() */
    if (!(curs = PyObject_CallMethod(conn, "cursor", NULL))) { goto exit; }

    /* curs.execute(...) */
    if (!(tmp = PyObject_CallMethod(curs, "execute", "s",
        "SELECT gid, prepared, owner, database FROM pg_prepared_xacts;")))
    {
        goto exit;
    }
    Py_DECREF(tmp);

    /* recs = curs.fetchall() */
    if (!(recs = PyObject_CallMethod(curs, "fetchall", NULL))) { goto exit; }

    /* curs.close() */
    if (!(tmp = PyObject_CallMethod(curs, "close", NULL))) { goto exit; }
    Py_DECREF(tmp);

    /* Build the list with return values. */
    if (0 > (len = PySequence_Size(recs))) { goto exit; }
    if (!(xids = PyList_New(len))) { goto exit; }

    /* populate the xids list */
    for (i = 0; i < len; ++i) {
        if (!(rec = PySequence_GetItem(recs, i))) { goto exit; }

        /* Get the xid with the XA triple set */
        if (!(item = PySequence_GetItem(rec, 0))) { goto exit; }
        if (!(xid = xid_from_string(item))) { goto exit; }
        Py_DECREF(item); item = NULL;

        /* set xid.prepared */
        if (!(item = PySequence_GetItem(rec, 1))) { goto exit; }
        tmp = xid->prepared;
        xid->prepared = item;
        Py_DECREF(tmp);
        item = NULL;

        /* set xid.owner */
        if (!(item = PySequence_GetItem(rec, 2))) { goto exit; }
        tmp = xid->owner;
        xid->owner = item;
        Py_DECREF(tmp);
        item = NULL;

        /* set xid.database */
        if (!(item = PySequence_GetItem(rec, 3))) { goto exit; }
        tmp = xid->database;
        xid->database = item;
        Py_DECREF(tmp);
        item = NULL;

        /* xid finished: add it to the returned list */
        PyList_SET_ITEM(xids, i, (PyObject *)xid);
        xid = NULL;  /* ref stolen */

        Py_DECREF(rec); rec = NULL;
    }

    /* set the return value. */
    rv = xids;
    xids = NULL;

exit:
    Py_XDECREF(xids);
    Py_XDECREF(xid);
    Py_XDECREF(curs);
    Py_XDECREF(recs);
    Py_XDECREF(rec);
    Py_XDECREF(item);

    return rv;
}
