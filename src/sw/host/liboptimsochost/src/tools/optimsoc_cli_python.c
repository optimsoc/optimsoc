#include <Python.h>

#include <optimsochost/liboptimsochost.h>

// Forward declaration of the function executing an external script
void execscript(char* scriptname);

// Those are the functions as defined in optimsoc_cli.c
extern int mem_init(unsigned int* memory_ids, unsigned int memory_count,
                    const char* path);
extern int register_itm_trace(int core_id, char* trace_file_path,
                              int add_disassembly,
                              char* elf_file_path);
extern int log_stm_trace(char* filename);

// We also need to pass this pointer (global in optimsoc_cli.c)
extern struct optimsoc_ctx *ctx;

// Forward declaration of the python functions
/**
 * Defines version()
 */
static PyObject *python_version(PyObject *self, PyObject *args);

/**
 * Defines mem_init(memory_id, file)
 */
static PyObject *python_mem_init(PyObject *self, PyObject *args);

/**
 * Defines start()
 */
static PyObject *python_start(PyObject *self, PyObject *args);

static PyObject *python_log_raw_instruction_trace(PyObject *self, PyObject *args);
static PyObject *python_log_dis_instruction_trace(PyObject *self, PyObject *args);
static PyObject *python_log_stm_trace(PyObject *self, PyObject *args);

// Implement version()
static PyObject *python_version(PyObject *self, PyObject *args) {
    printf("liboptimsochost version %s\n", optimsoc_get_version_string());
    return Py_None;
}

// Implement mem_init(tile_id, file)
static PyObject *python_mem_init(PyObject *self, PyObject *args) {
    unsigned int memory_id;
    char *path;
    if (!args || !PyArg_ParseTuple(args, "Is", &memory_id, &path)) {
        printf("Invalid arguments when running mem_init\n");
        return Py_None;
    }

    mem_init(&memory_id, 1, path);

    return Py_None;
}

// Implement start()
// For the moment we just after starting
// TODO: Capture SIGABORT and continue script?
static PyObject *python_start(PyObject *self, PyObject *args) {
    printf("Start system.\n");
    optimsoc_cpu_start(ctx);
    while(1) {}
    return Py_None;
}

static PyObject *python_log_raw_instruction_trace(PyObject *self, PyObject *args) {
    int core_id;
    char* path;

    if (!args || !PyArg_ParseTuple(args, "is", &core_id, &path)) {
        printf("Invalid arguments when running log_raw_instruction_trace\n");
        return Py_None;
    }

    register_itm_trace(core_id, path, 0, NULL);
    return Py_None;
}

static PyObject *python_log_dis_instruction_trace(PyObject *self, PyObject *args) {
    int core_id;
    char* path;
    char* elf_path;

    if (!args || !PyArg_ParseTuple(args, "iss", &core_id, &path, &elf_path)) {
        printf("Invalid arguments when running log_dis_instruction_trace\n");
        return Py_None;
    }

    register_itm_trace(core_id, path, 1, elf_path);
    return Py_None;

}

static PyObject *python_log_stm_trace(PyObject *self, PyObject *args) {
    char* path;

    if (!args || !PyArg_ParseTuple(args, "s", &path)) {
        printf("Invalid arguments when running log_stm_trace\n");
        return Py_None;
    }

    log_stm_trace(path);
    return Py_None;

}


// Definition of the methods
static PyMethodDef pythonMethods[] = {
    {"version", python_version, METH_NOARGS, "Prints version."},
    {"mem_init", python_mem_init, METH_VARARGS, "Initialize Memory." },
    {"start", python_start, METH_NOARGS, "Prints version."},
    {"log_raw_instruction_trace", python_log_raw_instruction_trace,
            METH_VARARGS, "Logs raw instruction trace."},
    {"log_dis_instruction_trace", python_log_dis_instruction_trace,
            METH_VARARGS, "Logs disassembly instruction trace."},
    {"start", python_start, METH_VARARGS, "Prints version."},
    {"log_stm_trace", python_log_stm_trace,
            METH_VARARGS, "Logs STM trace."},
    {NULL, NULL, 0, NULL},
};

// Executed when script is called
void execscript(char* scriptname) {
    Py_SetProgramName("optimsoc_cli");

    Py_Initialize();

    printf("\nExecute script %s.\n\n", scriptname);

    Py_InitModule("optimsoc", pythonMethods);

    PyObject* file = PyFile_FromString(scriptname, "r");
    if (!file) {
        printf("Cannot find %s. Abort.\n",scriptname);
        return;
    }

    PyRun_SimpleString("from optimsoc import *");
    PyRun_SimpleFileEx(PyFile_AsFile(file), scriptname, 1);
}
