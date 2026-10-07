#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "project/sb3.h"
#include "runtime/runtime.h"

typedef struct {
    PyObject_HEAD

    SB3Project *project;
    SB3Runtime runtime;
} SkrashProject;


/* Free project */
static void
SkrashProject_dealloc(SkrashProject *self)
{
    sb3_runtime_free(&self->runtime);

    if (self->project != NULL) {
        sb3_project_free(self->project);
        self->project = NULL;
    }

    Py_TYPE(self)->tp_free((PyObject *)self);
}


/* Project type */
static PyTypeObject SkrashProjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)

    .tp_name = "skrash.Project",
    .tp_basicsize = sizeof(SkrashProject),
    .tp_dealloc = (destructor)SkrashProject_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Scratch 3 project",
};


/* Load project */
static PyObject *
skrash_load(PyObject *self, PyObject *args)
{
    const char *path;
    SB3Project *project;
    SkrashProject *object;

    (void)self;

    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;

    project = sb3_load(path);

    if (project == NULL) {
        PyErr_Format(
            PyExc_RuntimeError,
            "Failed to load SB3 project: %s",
            path
        );
        return NULL;
    }

    object = PyObject_New(
        SkrashProject,
        &SkrashProjectType
    );

    if (object == NULL) {
        sb3_project_free(project);
        return NULL;
    }

    object->project = project;

    if (!sb3_runtime_init(
            &object->runtime,
            object->project)) {

        sb3_project_free(object->project);
        object->project = NULL;

        Py_DECREF(object);

        PyErr_SetString(
            PyExc_RuntimeError,
            "Failed to initialize runtime."
        );

        return NULL;
    }

    return (PyObject *)object;
}


/* Start project */
static PyObject *
SkrashProject_start(
    SkrashProject *self,
    PyObject *Py_UNUSED(args)
)
{
    if (self->project == NULL) {
        PyErr_SetString(
            PyExc_RuntimeError,
            "Project is not loaded."
        );
        return NULL;
    }

    if (!sb3_runtime_start(&self->runtime)) {
        PyErr_SetString(
            PyExc_RuntimeError,
            "Failed to start project."
        );
        return NULL;
    }

    Py_RETURN_NONE;
}


/* Execute one runtime step */
static PyObject *
SkrashProject_step(
    SkrashProject *self,
    PyObject *Py_UNUSED(args)
)
{
    if (self->project == NULL) {
        PyErr_SetString(
            PyExc_RuntimeError,
            "Project is not loaded."
        );
        return NULL;
    }

    if (!self->runtime.running)
        Py_RETURN_FALSE;

    if (!sb3_runtime_step(&self->runtime)) {
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}


/* Project methods */
static PyMethodDef SkrashProject_methods[] = {
    {
        "start",
        (PyCFunction)SkrashProject_start,
        METH_NOARGS,
        "Start the Scratch project."
    },
    {
        "step",
        (PyCFunction)SkrashProject_step,
        METH_NOARGS,
        "Execute one runtime step."
    },
    {NULL, NULL, 0, NULL}
};


/* Module methods */
static PyMethodDef Skrash_methods[] = {
    {
        "load",
        skrash_load,
        METH_VARARGS,
        "Load a Scratch 3 project."
    },
    {NULL, NULL, 0, NULL}
};


/* Project type initialization */
static int
SkrashProject_init_type(void)
{
    SkrashProjectType.tp_methods = SkrashProject_methods;

    return PyType_Ready(&SkrashProjectType);
}


/* Skrash module */
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
    PyObject *module;

    if (SkrashProject_init_type() < 0)
        return NULL;

    module = PyModule_Create(&Skrash_module);

    if (module == NULL)
        return NULL;

    Py_INCREF(&SkrashProjectType);

    if (PyModule_AddObject(
            module,
            "Project",
            (PyObject *)&SkrashProjectType
        ) < 0) {

        Py_DECREF(&SkrashProjectType);
        Py_DECREF(module);

        return NULL;
    }

    return module;
}