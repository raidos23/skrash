#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "project/sb3.h"
#include "runtime/runtime.h"
#include "../../renderer/renderer.h"

/* ------------------------------------------------------------------ */
/* SkrashProject                                                      */
/* ------------------------------------------------------------------ */

typedef struct {
    PyObject_HEAD
    SB3Project *project;
    SB3Runtime runtime;
} SkrashProject;

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

/* ------------------------------------------------------------------ */
/* SkrashSprite                                                       */
/* ------------------------------------------------------------------ */

typedef struct {
    PyObject_HEAD
    SB3Target *target;
} SkrashSprite;

static void
SkrashSprite_dealloc(SkrashSprite *self)
{
    /* On ne libere PAS le target : il appartient au projet. */
    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* --- Proprietes --- */

static PyObject *
SkrashSprite_get_x(SkrashSprite *self, void *closure)
{
    (void)closure;
    return PyFloat_FromDouble(self->target->x);
}

static int
SkrashSprite_set_x(SkrashSprite *self, PyObject *value, void *closure)
{
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
SkrashSprite_get_y(SkrashSprite *self, void *closure)
{
    (void)closure;
    return PyFloat_FromDouble(self->target->y);
}

static int
SkrashSprite_set_y(SkrashSprite *self, PyObject *value, void *closure)
{
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
SkrashSprite_get_direction(SkrashSprite *self, void *closure)
{
    (void)closure;
    return PyFloat_FromDouble(self->target->direction);
}

static int
SkrashSprite_set_direction(SkrashSprite *self, PyObject *value, void *closure)
{
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
SkrashSprite_move(SkrashSprite *self, PyObject *args)
{
    double steps;
    if (!PyArg_ParseTuple(args, "d", &steps))
        return NULL;

    double rad = self->target->direction * 3.14159265358979323846 / 180.0;
    self->target->x += steps * sin(rad);
    self->target->y += steps * cos(rad);

    Py_RETURN_NONE;
}

static PyObject *
SkrashSprite_say(SkrashSprite *self, PyObject *args)
{
    const char *msg;
    if (!PyArg_ParseTuple(args, "s", &msg))
        return NULL;

    printf("[%s] (say) %s\n",
           self->target->name ? self->target->name : "?", msg);
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
SkrashVariable_dealloc(SkrashVariable *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
SkrashVariable_get_value(SkrashVariable *self, void *closure)
{
    (void)closure;
    return PyFloat_FromDouble(self->variable->value);
}

static int
SkrashVariable_set_value(SkrashVariable *self, PyObject *value, void *closure)
{
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
SkrashVariable_get_name(SkrashVariable *self, void *closure)
{
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

/* forward declaration pour le dispatch des methodes */
static PyTypeObject SkrashProjectType;

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
SkrashProject_start(SkrashProject *self, PyObject *Py_UNUSED(args))
{
    if (self->project == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Project is not loaded.");
        return NULL;
    }

    if (!sb3_runtime_start(&self->runtime)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to start project.");
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
SkrashProject_step(SkrashProject *self, PyObject *Py_UNUSED(args))
{
    if (self->project == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Project is not loaded.");
        return NULL;
    }

    if (sb3_runtime_step(&self->runtime))
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *
SkrashProject_run(SkrashProject *self, PyObject *Py_UNUSED(args))
{
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

static PyObject *
SkrashProject_get_sprite(SkrashProject *self, PyObject *args)
{
    const char *name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    for (size_t i = 0; i < self->project->target_count; ++i) {
        SB3Target *t = &self->project->targets[i];
        if (t->name && strcmp(t->name, name) == 0) {
            SkrashSprite *spr = PyObject_New(SkrashSprite, &SkrashSpriteType);
            if (!spr) return NULL;
            spr->target = t;
            return (PyObject *)spr;
        }
    }

    PyErr_Format(PyExc_KeyError, "Sprite not found: %s", name);
    return NULL;
}

static PyObject *
SkrashProject_get_variable(SkrashProject *self, PyObject *args)
{
    const char *name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    /* On cherche dans le stage (variables globales) 
       puis dans tous les sprites (variables locales) */
    for (size_t i = 0; i < self->project->target_count; ++i) {
        SB3Target *t = &self->project->targets[i];
        for (size_t j = 0; j < t->variable_count; ++j) {
            SB3Variable *var = &t->variables[j];
            if (var->name && strcmp(var->name, name) == 0) {
                SkrashVariable *pyvar = PyObject_New(SkrashVariable, &SkrashVariableType);
                if (!pyvar) return NULL;
                pyvar->variable = var;
                return (PyObject *)pyvar;
            }
        }
    }

    PyErr_Format(PyExc_KeyError, "Variable not found: %s", name);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Fonctions module-level                                             */
/* ------------------------------------------------------------------ */

static PyObject *
skrash_get_sprite(PyObject *self, PyObject *args)
{
    PyObject *project_obj;
    const char *name;

    (void)self;

    if (!PyArg_ParseTuple(args, "Os", &project_obj, &name))
        return NULL;

    if (!PyObject_TypeCheck(project_obj, &SkrashProjectType)) {
        PyErr_SetString(PyExc_TypeError,
                        "get_sprite() first argument must be a Project");
        return NULL;
    }

    return SkrashProject_get_sprite((SkrashProject *)project_obj, args + 0 * 0 == NULL ? NULL : NULL);
}

static PyObject *
skrash_get_variable(PyObject *self, PyObject *args)
{
    PyObject *project_obj;
    const char *name;

    (void)self;

    if (!PyArg_ParseTuple(args, "Os", &project_obj, &name))
        return NULL;

    if (!PyObject_TypeCheck(project_obj, &SkrashProjectType)) {
        PyErr_SetString(PyExc_TypeError,
                        "get_variable() first argument must be a Project");
        return NULL;
    }

    SkrashProject *proj = (SkrashProject *)project_obj;

    for (size_t i = 0; i < proj->project->target_count; ++i) {
        SB3Target *t = &proj->project->targets[i];
        for (size_t j = 0; j < t->variable_count; ++j) {
            SB3Variable *var = &t->variables[j];
            if (var->name && strcmp(var->name, name) == 0) {
                SkrashVariable *pyvar = PyObject_New(SkrashVariable, &SkrashVariableType);
                if (!pyvar) return NULL;
                pyvar->variable = var;
                return (PyObject *)pyvar;
            }
        }
    }

    PyErr_Format(PyExc_KeyError, "Variable not found: %s", name);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Definitions Project                                                */
/* ------------------------------------------------------------------ */

static PyMethodDef SkrashProject_methods[] = {
    {"start",      (PyCFunction)SkrashProject_start,      METH_NOARGS,  "Start the Scratch project."},
    {"step",       (PyCFunction)SkrashProject_step,       METH_NOARGS,  "Execute one runtime step."},
    {"run",        (PyCFunction)SkrashProject_run,        METH_NOARGS,  "Run the project (opens SDL window)."},
    {"get_sprite", (PyCFunction)SkrashProject_get_sprite, METH_VARARGS, "Get a sprite by name."},
    {"get_variable", (PyCFunction)SkrashProject_get_variable, METH_VARARGS, "Get a variable by name."},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject SkrashProjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "skrash.Project",
    .tp_basicsize = sizeof(SkrashProject),
    .tp_dealloc   = (destructor)SkrashProject_dealloc,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_doc       = "Scratch 3 project",
    .tp_methods   = SkrashProject_methods,
};

/* ------------------------------------------------------------------ */
/* Module                                                             */
/* ------------------------------------------------------------------ */

static PyMethodDef Skrash_methods[] = {
    {"load",         skrash_load,         METH_VARARGS, "Load a Scratch 3 project."},
    {"get_sprite",   skrash_get_sprite,   METH_VARARGS, "Get a sprite by name from a Project."},
    {"get_variable", skrash_get_variable, METH_VARARGS, "Get a variable by name from a Project."},
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
PyInit__core(void)
{
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