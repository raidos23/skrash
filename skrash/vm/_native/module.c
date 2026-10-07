#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "project/sb3.h"

typedef struct {
    PyObject_HEAD
    SB3Project *project;
} SkrashProject;

// Free project 
static void
SkrashProject_dealloc(SkrashProject *self)
{
    if (self->project != NULL) {
        sb3_project_free(self->project);
        self->project = NULL;
    }

    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Project type 
static PyTypeObject SkrashProjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "skrash.Project",
    .tp_basicsize = sizeof(SkrashProject),
    .tp_dealloc = (destructor)SkrashProject_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Scratch 3 project",
};

// Load project 
static PyObject *
skrash_load(PyObject *self, PyObject *args)
{
    const char *path;

    (void)self;

    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;

    SB3Project *project = sb3_load(path);

    if (project == NULL) {
        PyErr_Format(
            PyExc_RuntimeError,
            "Failed to load SB3 project: %s",
            path
        );
        return NULL;
    }
    SkrashProject *object =
        PyObject_New(SkrashProject, &SkrashProjectType);

    if (object == NULL) {
        sb3_project_free(project);
        return NULL;
    }

    object->project = project;

    return (PyObject *)object;
}

// methods 
static PyMethodDef Skrash_methods[] = {
    {"load", skrash_load, METH_VARARGS, "Load a Scratch 3 project."}, {NULL, NULL, 0, NULL}
};

// Skrash module
static struct PyModuleDef Skrash_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_core",
    .m_doc = "Skrash native runtime.",
    .m_size = -1,
    .m_methods = Skrash_methods,
};

// Initialize module
PyMODINIT_FUNC
PyInit__core(void)
{
    if (PyType_Ready(&SkrashProjectType) < 0)
        return NULL;

    PyObject *module = PyModule_Create(&Skrash_module);

    if (module == NULL)
        return NULL;

    Py_INCREF(&SkrashProjectType);

    if (PyModule_AddObject(
            module,
            "Project",
            (PyObject *)&SkrashProjectType) < 0) {
        Py_DECREF(&SkrashProjectType);
        Py_DECREF(module);
        return NULL;
    }
    
    return module;
}