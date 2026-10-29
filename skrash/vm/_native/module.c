#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "project/sb3.h"
#include "runtime/runtime.h"
#include "../../renderer/renderer.h"

/* ------------------------------------------------------------------ */
/* Registre global des callbacks green flag                           */
/* ------------------------------------------------------------------ */

static PyObject *g_green_flag_callbacks = NULL;   /* liste Python ou NULL */

static void
skrash_ensure_green_flag_registry(void) {
    if (g_green_flag_callbacks == NULL) {
        g_green_flag_callbacks = PyList_New(0);
    }
}

static void
skrash_consume_green_flag_callbacks(void) {
    if (g_green_flag_callbacks == NULL)
        return;

    Py_ssize_t n = PyList_Size(g_green_flag_callbacks);
    if (n <= 0)
        return;

    for (Py_ssize_t i = 0; i < n; ++i) {
        PyObject *cb = PyList_GetItem(g_green_flag_callbacks, i);
        if (cb == NULL) continue;

        PyObject *result = PyObject_CallNoArgs(cb);
        if (result == NULL) {
            PyErr_Print();
            continue;
        }
        Py_DECREF(result);
    }

    if (PyList_SetSlice(g_green_flag_callbacks, 0, n, NULL) < 0)
        PyErr_Print();
}

/* ------------------------------------------------------------------ */
/* SkrashProject                                                      */
/* ------------------------------------------------------------------ */

typedef struct {
    PyObject_HEAD
    SB3Project *project;
    SB3Runtime runtime;
} SkrashProject;

/* forward declaration : utilisee dans skrash_load avant sa definition */
static PyTypeObject SkrashProjectType;

static void
SkrashProject_dealloc(SkrashProject *self) {
    sb3_runtime_free(&self->runtime);

    if (self->project != NULL) {
        sb3_project_free(self->project);
        self->project = NULL;
    }

    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* ------------------------------------------------------------------ */
/* SkrashSprite                                                       */
/* ------------------------------------------------------------------ */

typedef struct {
    PyObject_HEAD
    SB3Target *target;
} SkrashSprite;

static void
SkrashSprite_dealloc(SkrashSprite *self) {
    /* On ne libere PAS le target : il appartient au projet. */
    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* --- Proprietes --- */

static PyObject *
SkrashSprite_get_x(SkrashSprite *self, void *closure) {
    (void)closure;
    return PyFloat_FromDouble(self->target->x);
}

static int
SkrashSprite_set_x(SkrashSprite *self, PyObject *value, void *closure) {
    (void)closure;
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete x");
        return -1;
    }
    double v = PyFloat_AsDouble(value);
    if (PyErr_Occurred()) return -1;
    self->target->x = v;
    return 0;
}

static PyObject *
SkrashSprite_get_y(SkrashSprite *self, void *closure) {
    (void)closure;
    return PyFloat_FromDouble(self->target->y);
}

static int
SkrashSprite_set_y(SkrashSprite *self, PyObject *value, void *closure) {
    (void)closure;
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete y");
        return -1;
    }
    double v = PyFloat_AsDouble(value);
    if (PyErr_Occurred()) return -1;
    self->target->y = v;
    return 0;
}

static PyObject *
SkrashSprite_get_direction(SkrashSprite *self, void *closure) {
    (void)closure;
    return PyFloat_FromDouble(self->target->direction);
}

static int
SkrashSprite_set_direction(SkrashSprite *self, PyObject *value, void *closure) {
    (void)closure;
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete direction");
        return -1;
    }
    double v = PyFloat_AsDouble(value);
    if (PyErr_Occurred()) return -1;
    self->target->direction = v;
    return 0;
}

/* --- Methodes --- */

static PyObject *
SkrashSprite_move(SkrashSprite *self, PyObject *args) {
    double steps;
    if (!PyArg_ParseTuple(args, "d", &steps))
        return NULL;

    double rad = self->target->direction * 3.14159265358979323846 / 180.0;
    self->target->x += steps * sin(rad);
    self->target->y += steps * cos(rad);

    Py_RETURN_NONE;
}

static PyObject *
SkrashSprite_say(SkrashSprite *self, PyObject *args) {
    const char *msg;
    if (!PyArg_ParseTuple(args, "s", &msg))
        return NULL;

    printf("[%s] (say) %s\n",
           self->target->name ? self->target->name : "?", msg);
    Py_RETURN_NONE;
}

static PyObject *
SkrashSprite_show(SkrashSprite *self, PyObject *Py_UNUSED(args)) {
    self->target->visible = 1;
    Py_RETURN_NONE;
}

static PyObject *
SkrashSprite_hide(SkrashSprite *self, PyObject *Py_UNUSED(args)) {
    self->target->visible = 0;
    Py_RETURN_NONE;
}

/* --- Definitions Sprite --- */

static PyGetSetDef SkrashSprite_getset[] = {
    {"x",         (getter)SkrashSprite_get_x,         (setter)SkrashSprite_set_x,         "X position", NULL},
    {"y",         (getter)SkrashSprite_get_y,         (setter)SkrashSprite_set_y,         "Y position", NULL},
    {"direction", (getter)SkrashSprite_get_direction, (setter)SkrashSprite_set_direction, "Direction",  NULL},
    {NULL}
};

static PyMethodDef SkrashSprite_methods[] = {
    {"move", (PyCFunction)SkrashSprite_move, METH_VARARGS, "Move the sprite by N steps."},
    {"say",  (PyCFunction)SkrashSprite_say,  METH_VARARGS, "Say a message."},
    {"show", (PyCFunction)SkrashSprite_show, METH_NOARGS,  "Make the sprite visible."},
    {"hide", (PyCFunction)SkrashSprite_hide, METH_NOARGS,  "Hide the sprite."},
    {NULL}
};

static PyTypeObject SkrashSpriteType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "skrash.Sprite",
    .tp_basicsize = sizeof(SkrashSprite),
    .tp_dealloc   = (destructor)SkrashSprite_dealloc,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_doc       = "Scratch sprite",
    .tp_methods   = SkrashSprite_methods,
    .tp_getset    = SkrashSprite_getset,
};


/* ------------------------------------------------------------------ */
/* SkrashVariable                                                     */
/* ------------------------------------------------------------------ */

typedef struct {
    PyObject_HEAD
    SB3Variable *variable;
} SkrashVariable;

static void
SkrashVariable_dealloc(SkrashVariable *self) {
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
SkrashVariable_get_value(SkrashVariable *self, void *closure) {
    (void)closure;
    return PyFloat_FromDouble(self->variable->value);
}

static int
SkrashVariable_set_value(SkrashVariable *self, PyObject *value, void *closure) {
    (void)closure;
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete variable value");
        return -1;
    }
    double v = PyFloat_AsDouble(value);
    if (PyErr_Occurred()) return -1;
    self->variable->value = v;
    return 0;
}

static PyObject *
SkrashVariable_get_name(SkrashVariable *self, void *closure) {
    (void)closure;
    return PyUnicode_FromString(self->variable->name ? self->variable->name : "");
}

static PyGetSetDef SkrashVariable_getset[] = {
    {"value", (getter)SkrashVariable_get_value, (setter)SkrashVariable_set_value, "Variable value", NULL},
    {"name",  (getter)SkrashVariable_get_name,  NULL,                             "Variable name",  NULL},
    {NULL}
};

static PyTypeObject SkrashVariableType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "skrash.Variable",
    .tp_basicsize = sizeof(SkrashVariable),
    .tp_dealloc   = (destructor)SkrashVariable_dealloc,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_doc       = "Scratch variable",
    .tp_getset    = SkrashVariable_getset,
};


/* ------------------------------------------------------------------ */
/* Fonctions Project                                                  */
/* ------------------------------------------------------------------ */

static PyObject *
skrash_load(PyObject *self, PyObject *args) {
    const char *path;
    SB3Project *project;
    SkrashProject *object;

    (void)self;

    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;

    project = sb3_load(path);
    if (project == NULL) {
        PyErr_Format(PyExc_RuntimeError,
                     "Failed to load SB3 project: %s", path);
        return NULL;
    }

    object = PyObject_New(SkrashProject, &SkrashProjectType);
    if (object == NULL) {
        sb3_project_free(project);
        return NULL;
    }

    object->project = project;

    if (!sb3_runtime_init(&object->runtime, object->project)) {
        sb3_project_free(object->project);
        object->project = NULL;
        Py_DECREF(object);
        PyErr_SetString(PyExc_RuntimeError, "Failed to initialize runtime.");
        return NULL;
    }

    return (PyObject *)object;
}

static PyObject *
SkrashProject_start(SkrashProject *self, PyObject *Py_UNUSED(args)) {
    if (self->project == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Project is not loaded.");
        return NULL;
    }

    if (!sb3_runtime_start(&self->runtime)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to start project.");
        return NULL;
    }

    skrash_consume_green_flag_callbacks();

    Py_RETURN_NONE;
}

static PyObject *
SkrashProject_step(SkrashProject *self, PyObject *Py_UNUSED(args)) {
    if (self->project == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Project is not loaded.");
        return NULL;
    }

    if (sb3_runtime_step(&self->runtime))
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *
SkrashProject_run(SkrashProject *self, PyObject *Py_UNUSED(args)) {
    if (self->project == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Project is not loaded.");
        return NULL;
    }

    SB3Renderer renderer;
    if (!sb3_renderer_init(&renderer,
                           SB3_STAGE_WIDTH, SB3_STAGE_HEIGHT, "skrash")) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to initialize renderer.");
        return NULL;
    }

    renderer.runtime_ptr = &self->runtime;

    sb3_runtime_start(&self->runtime);
    skrash_consume_green_flag_callbacks();

    while (sb3_renderer_poll(&renderer)) {
        sb3_runtime_step(&self->runtime);

        sb3_renderer_clear(&renderer, 255, 255, 255);
        sb3_renderer_draw_stage(&renderer, self->project);
        sb3_renderer_draw_targets(&renderer, self->project);
        sb3_renderer_present(&renderer);

        SDL_Delay(16);
    }

    sb3_renderer_free(&renderer);
    Py_RETURN_NONE;
}

/* Propriete : dict de sprites (hors stage) par nom */
static PyObject *
SkrashProject_get_sprites(SkrashProject *self, void *closure) {
    (void)closure;
    if (self->project == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Project is not loaded.");
        return NULL;
    }

    PyObject *dict = PyDict_New();
    if (dict == NULL)
        return NULL;

    for (size_t i = 0; i < self->project->target_count; ++i) {
        SB3Target *t = &self->project->targets[i];
        if (t->is_stage || t->name == NULL)
            continue;

        SkrashSprite *spr = PyObject_New(SkrashSprite, &SkrashSpriteType);
        if (spr == NULL) {
            Py_DECREF(dict);
            return NULL;
        }
        spr->target = t;

        if (PyDict_SetItemString(dict, t->name, (PyObject *)spr) < 0) {
            Py_DECREF(spr);
            Py_DECREF(dict);
            return NULL;
        }
        Py_DECREF(spr);
    }

    return dict;
}

/* Propriete : dict de variables (toutes cibles) par nom */
static PyObject *
SkrashProject_get_variables(SkrashProject *self, void *closure) {
    (void)closure;
    if (self->project == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Project is not loaded.");
        return NULL;
    }

    PyObject *dict = PyDict_New();
    if (dict == NULL)
        return NULL;

    for (size_t i = 0; i < self->project->target_count; ++i) {
        SB3Target *t = &self->project->targets[i];
        for (size_t j = 0; j < t->variable_count; ++j) {
            SB3Variable *var = &t->variables[j];
            if (var->name == NULL)
                continue;

            SkrashVariable *pyvar = PyObject_New(SkrashVariable, &SkrashVariableType);
            if (pyvar == NULL) {
                Py_DECREF(dict);
                return NULL;
            }
            pyvar->variable = var;

            if (PyDict_SetItemString(dict, var->name, (PyObject *)pyvar) < 0) {
                Py_DECREF(pyvar);
                Py_DECREF(dict);
                return NULL;
            }
            Py_DECREF(pyvar);
        }
    }

    return dict;
}

/* ------------------------------------------------------------------ */
/* Decorateur module-level : when_green_flag_clicked                  */
/* ------------------------------------------------------------------ */

static PyObject *
skrash_when_green_flag_clicked(PyObject *self, PyObject *args) {
    PyObject *cb;
    (void)self;

    if (!PyArg_ParseTuple(args, "O", &cb))
        return NULL;

    if (!PyCallable_Check(cb)) {
        PyErr_SetString(PyExc_TypeError, "callback must be callable");
        return NULL;
    }

    skrash_ensure_green_flag_registry();
    if (g_green_flag_callbacks == NULL)
        return NULL;

    if (PyList_Append(g_green_flag_callbacks, cb) < 0)
        return NULL;

    Py_INCREF(cb);
    return cb;
}

/* ------------------------------------------------------------------ */
/* Definitions Project                                                */
/* ------------------------------------------------------------------ */

static PyMethodDef SkrashProject_methods[] = {
    {"start", (PyCFunction)SkrashProject_start, METH_NOARGS, "Start the Scratch project."},
    {"step",  (PyCFunction)SkrashProject_step,  METH_NOARGS, "Execute one runtime step."},
    {"run",   (PyCFunction)SkrashProject_run,   METH_NOARGS, "Run the project (opens SDL window)."},
    {NULL, NULL, 0, NULL}
};

static PyGetSetDef SkrashProject_getset[] = {
    {"sprites",   (getter)SkrashProject_get_sprites,   NULL, "Dict of sprites by name (stage excluded)", NULL},
    {"variables", (getter)SkrashProject_get_variables, NULL, "Dict of variables by name",                 NULL},
    {NULL}
};

static PyTypeObject SkrashProjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "skrash.Project",
    .tp_basicsize = sizeof(SkrashProject),
    .tp_dealloc   = (destructor)SkrashProject_dealloc,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_doc       = "Scratch 3 project",
    .tp_methods   = SkrashProject_methods,
    .tp_getset    = SkrashProject_getset,
};

/* ------------------------------------------------------------------ */
/* Module                                                             */
/* ------------------------------------------------------------------ */

static PyMethodDef Skrash_methods[] = {
    {"load",                    skrash_load,                    METH_VARARGS, "Load a Scratch 3 project."},
    {"when_green_flag_clicked", skrash_when_green_flag_clicked, METH_VARARGS, "Register a function to be called when the project starts."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef Skrash_module = {
    PyModuleDef_HEAD_INIT,
    .m_name    = "_core",
    .m_doc     = "Skrash native runtime.",
    .m_size    = -1,
    .m_methods = Skrash_methods,
};

PyMODINIT_FUNC
PyInit__core(void) {
    PyObject *module;

    if (PyType_Ready(&SkrashProjectType) < 0)
        return NULL;

    if (PyType_Ready(&SkrashSpriteType) < 0)
        return NULL;

    if (PyType_Ready(&SkrashVariableType) < 0)
        return NULL;

    module = PyModule_Create(&Skrash_module);
    if (module == NULL)
        return NULL;

    Py_INCREF(&SkrashProjectType);
    if (PyModule_AddObject(module, "Project",
                           (PyObject *)&SkrashProjectType) < 0) {
        Py_DECREF(&SkrashProjectType);
        Py_DECREF(module);
        return NULL;
    }

    Py_INCREF(&SkrashSpriteType);
    if (PyModule_AddObject(module, "Sprite",
                           (PyObject *)&SkrashSpriteType) < 0) {
        Py_DECREF(&SkrashSpriteType);
        Py_DECREF(module);
        return NULL;
    }

    Py_INCREF(&SkrashVariableType);
    if (PyModule_AddObject(module, "Variable",
                           (PyObject *)&SkrashVariableType) < 0) {
        Py_DECREF(&SkrashVariableType);
        Py_DECREF(module);
        return NULL;
    }

    return module;
}