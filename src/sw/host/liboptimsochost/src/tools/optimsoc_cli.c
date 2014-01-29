/**
 * This file is part of liboptimsochost.
 *
 * liboptimsochost is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * liboptimsochost is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with liboptimsoc. If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * OpTiMSoC Debug Command Line Interface
 *
 * This example program uses liboptimsochost to create a simple command-line
 * interface for talking to the OpTiMSoC system.
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, philipp.wagner@tum.de
 *    Michael Tempelmeier, michael.tempelmeier@tum.de
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <optimsochost/liboptimsochost.h>

#include <config.h>

/*
 * OpenRISC objdump tool to create disassembly
 */
#define OR32_OBJDUMP "or32-elf-objdump"

int mem_init(unsigned int* memory_ids, unsigned int memory_count,
             const char* path);
int register_itm_trace(int core_id, char* trace_file_path,
                              int add_disassembly,
                              char* elf_file_path);
int log_stm_trace(char* filename);

struct optimsoc_ctx *ctx;
FILE *nrm_stat_file;
FILE *stm_trace_file;
char **stm_printf_buf;
unsigned int max_core_id;

int itm_callback_registered;

struct itm_sink {
    int do_trace;
    int add_disassembly;
    FILE *trace_file;
    unsigned int dis_instr_count;
    struct disassembled_instruction **dis;
};
struct itm_sink** itm_sinks;

struct disassembled_instruction {
    uint32_t instr;
    char disassembly[50];
    char funcname[50];
};

static int new_disassembly(char* elf_file_path,
                           struct disassembled_instruction** dis_instr[],
                           unsigned int* dis_instr_count)
{
    int rv;
    struct disassembled_instruction** instructions = NULL;

    /*
     * Allocate enough space for the array of disassembled_instruction.
     * For faster lookups the program counter (PC) represents also the
     * array index (index = PC >> 2).
     */
    struct stat bin_file_stat;
    rv = stat(elf_file_path, &bin_file_stat);
    if (rv != 0) {
        fprintf(stderr, "Unable read file size from file %s\n", elf_file_path);
        rv = -1;
        goto error_free_return;
    }
    int instr_count = bin_file_stat.st_size/4;
    instructions = calloc(instr_count,
                          sizeof(struct disassembled_instruction*));
    if (!instructions) {
        fprintf(stderr, "Unable to allocate space for disassembled instructions.");
        rv = -ENOMEM;
        goto error_free_return;
    }

    /*
     * Create disassembly by calling or32-elf-objdump
     */
    char objdump_cmd[255];
    snprintf(objdump_cmd, 255, "%s -d %s", OR32_OBJDUMP, elf_file_path);
    FILE* fp_objdump = popen(objdump_cmd, "r");
    if (fp_objdump == NULL) {
        fprintf(stderr, "Unable to execute %s.\n", OR32_OBJDUMP);
        rv = -1;
        goto error_free_return;
    }

    /*
     * read created disassembly into array
     */
    char line[255];
    uint32_t pc;
    uint8_t instr[4];
    char funcname[50];
    char funcname_tmp[50];
    while (fgets(line, 255, fp_objdump)) {
        struct disassembled_instruction* instruction = calloc(1, sizeof(struct disassembled_instruction));

        int matches = sscanf(line, "%*8x <%49[^>]>:\n", funcname_tmp);
        if (matches == 1) {
            strncpy(funcname, funcname_tmp, 50);
            funcname[49] = '\n'; /* ensure string termination */
            continue;
        }

        matches = sscanf(line, "%8x:\t%"SCNx8" %"SCNx8" %"SCNx8" %"SCNx8"\t%49[^\n]", &pc, &instr[3],
                         &instr[2], &instr[1], &instr[0], instruction->disassembly);
        instruction->instr = instr[3] << 24 | instr[2] << 16 | instr[1] << 8 | instr[0];
        instruction->disassembly[49] = '\0'; /* ensure string termination */
        strncpy(instruction->funcname, funcname, 50);

        if (matches != 6) {
            free(instruction);
            continue;
        }

        int idx = pc >> 2;
        assert(idx < instr_count);
        instructions[idx] = instruction;
    }

    *dis_instr = instructions;
    *dis_instr_count = instr_count;
    rv = 0;

    pclose(fp_objdump);
    return rv;

error_free_return:
    if (instructions) {
        free(instructions);
    }
    return rv;
}

static void free_disassembly(struct disassembled_instruction* dis_instr[],
                             int instr_count)
{
    if (!dis_instr) {
        return;
    }

    for (int i=0; i<instr_count; i++) {
        if (dis_instr[i] == NULL) {
            continue;
        }
        free(dis_instr[i]);
    }

    dis_instr = NULL;
}

static void parse_options(char* str, struct optimsoc_backend_option* options[],
                          int *num_options)
{
    char *opt;
    int count = 0;

    /* count the number of options */
    char *strcp = strdup(str);
    opt = strtok(strcp, ",");
    if (opt != 0) {
        count++;
        while (strtok(0, ",") != 0) {
            count++;
        }
    }
    free(strcp);

    *num_options = count;
    if (count <= 0) {
        return;
    }

    struct optimsoc_backend_option *optvec = calloc(count,
                                                    sizeof(struct optimsoc_backend_option));

    strcp = strdup(str);
    opt = strtok(str, ",");
    int i = 0;
    do {
        char *sep = index(opt, '=');
        if (sep) {
            optvec[i].name = strndup(opt, sep - opt);
            optvec[i].value = strndup(sep + 1, opt + strlen(opt) - sep);
        } else {
            optvec[i].name = strdup(opt);
            optvec[i].value = 0;
        }
        opt = strtok(0, ",");
        i++;
    } while (opt);

    free(strcp);
    *options = optvec;
}

#ifndef USE_PYTHON
static void execscript(char *scriptname) {
  printf("EXEC no python scripting compiled.\n");
}
#else
extern void execscript(char *scriptname);
#endif

static void connect(optimsoc_backend_id backend,
                    int num_options,
                    struct optimsoc_backend_option options[])
{
    int err;
    err = optimsoc_new(&ctx, backend, num_options, options);
    if (err < 0) {
        printf("Unable to create liboptimsochost context object.\n");
        exit(EXIT_FAILURE);
    }

    err = optimsoc_connect(ctx);
    if (err < 0) {
        printf("Unable to connect to target system.\n");
        exit(EXIT_FAILURE);
    }

    err = optimsoc_discover_system(ctx);
    if (err < 0) {
        printf("System discovery failed.\n");
        exit(EXIT_FAILURE);
    }

    max_core_id = 0;
    struct optimsoc_itm_config *itm_config;
    struct optimsoc_stm_config *stm_config;

    /* show system information and determine maximum core ID */
    printf("Connected to system.\n");
    printf("System ID: 0x%08x\n", optimsoc_get_sysid(ctx));
    printf("Module summary:\n");
    struct optimsoc_dbg_module *modules;
    int module_count;
    optimsoc_get_modules(ctx, &modules, &module_count);
    printf("addr.\ttype\tversion\tname\n");
    for (int i=0; i<module_count; i++) {
        printf("0x%02x\t0x%02x\t0x%02x\t%s\n", modules[i].dbgnoc_addr,
               modules[i].module_type, modules[i].module_version,
               optimsoc_get_module_name(modules[i].module_type));


        if (modules[i].module_type == OPTIMSOC_MODULE_TYPE_ITM) {
            optimsoc_itm_get_config(ctx, &modules[i], &itm_config);
            if (itm_config->core_id > max_core_id) {
                max_core_id = itm_config->core_id;
            }
        }

        if (modules[i].module_type == OPTIMSOC_MODULE_TYPE_STM) {
            optimsoc_stm_get_config(ctx, &modules[i], &stm_config);
            if (stm_config->core_id > max_core_id) {
                max_core_id = stm_config->core_id;
            }
        }
    }

    /* allocate memory for the ITM traces */
    itm_sinks = calloc(max_core_id + 1, sizeof(struct itm_sink*));
}

static int disconnect(void)
{
    int err;

    printf("Disconnecting from system...");
    fflush(stdout);

    err = optimsoc_disconnect(ctx);
    if (err < 0) {
        return err;
    }

    err = optimsoc_free(ctx);
    if (err < 0) {
        return err;
    }

    if (nrm_stat_file) {
        fclose(nrm_stat_file);
        nrm_stat_file = NULL;
    }
    if (stm_trace_file) {
        fclose(stm_trace_file);
        stm_trace_file = NULL;
    }
    if (itm_sinks) {
        for (unsigned int i = 0; i < max_core_id + 1; i++) {
            if (itm_sinks[i] != NULL) {
                if (itm_sinks[i]->trace_file) {
                    fclose(itm_sinks[i]->trace_file);
                    itm_sinks[i]->trace_file = NULL;
                }
                if (itm_sinks[i]->dis) {
                    free_disassembly(itm_sinks[i]->dis,
                                     itm_sinks[i]->dis_instr_count);
                }
                free(itm_sinks[i]);
            }
        }
        free(itm_sinks);
        itm_sinks = NULL;
    }
    if (stm_printf_buf) {
        for (unsigned int i = 0; i < max_core_id + 1; i++) {
            if (stm_printf_buf[i] != NULL) {
                free(stm_printf_buf[i]);
            }
        }
        free(stm_printf_buf);
        stm_printf_buf = NULL;
    }

    printf(" done\n");
    return 0;
}

static void sighandler(int sig)
{
    if (sig == SIGINT) {
        disconnect();
        exit(EXIT_SUCCESS);
    }
}

static int mem_write(unsigned int memory_id, const char* path,
                     unsigned int base_address)
{
    printf("Trying to write %s to memory %d starting at 0x%x.\n",
           path, memory_id, base_address);

    /* allocate memory for file */
    struct stat stat_buf;
    if (stat(path, &stat_buf) < 0) {
        printf("Unable to stat file %s\n", path);
        return -1;
    }
    uint8_t* data_buf;
    data_buf = calloc(stat_buf.st_size, sizeof(uint8_t));

    int pos = 0;
    FILE* fp = fopen(path, "r");
    if (!fp) {
        printf("Unable to open file %s\n", path);
        return -1;
    }

    while (!feof(fp)) {
        pos += fread(&data_buf[pos], 1, 512, fp);
    }
    fclose(fp);

    int rv = optimsoc_mem_write(ctx, memory_id, base_address, data_buf,
                                stat_buf.st_size);
    if (rv < 0) {
        printf("optimsoc_mem_write() not successful: %d\n", rv);
        return -1;
    }
    return 0;
}

int mem_init(unsigned int* memory_ids, unsigned int memory_count,
             const char* path)
{
    if (memory_count == 1) {
        printf("Initializing memory %d with %s\n", memory_ids[0], path);
    } else {
        printf("Initializing memories ");
        for (unsigned int i = 0; i < memory_count; i++) {
            printf("%d", memory_ids[i]);
            if (i != memory_count - 1) {
                printf(", ");
            }
        }
        printf(" with %s\n", path);
    }

    /* allocate memory for file */
    struct stat stat_buf;
    if (stat(path, &stat_buf) < 0) {
        printf("Unable to stat file %s\n", path);
        return -1;
    }
    uint8_t* data_buf;
    data_buf = calloc(stat_buf.st_size, sizeof(uint8_t));

    int pos = 0;
    FILE* fp = fopen(path, "r");
    if (!fp) {
        printf("Unable to open file %s\n", path);
        return -1;
    }

    while (!feof(fp)) {
        pos += fread(&data_buf[pos], 1, 512, fp);
    }
    fclose(fp);

    int rv = optimsoc_mem_init(ctx, memory_ids, memory_count, data_buf,
                               stat_buf.st_size);
    if (rv < 0) {
        printf("optimsoc_mem_init() not successful: %d\n", rv);
        return -1;
    }
    printf("Initialization successful!\n");
    return 0;
}

static void write_itm_trace_to_file(unsigned int core_id, uint32_t timestamp,
                                    uint32_t pc, int count)
{
    if (core_id >= max_core_id || itm_sinks[core_id] == NULL) {
        return;
    }

    struct itm_sink *sink = itm_sinks[core_id];
    if (sink->add_disassembly) {
        if ((pc >> 2) >= sink->dis_instr_count) {
            fprintf(stderr, "No disassembly for PC 0x%x available.\n", pc);
            return;
        }

        uint32_t pc_word = pc >> 2;
        for (int i=0; i<count; i++) {
            pc = pc_word << 2;
            fprintf(sink->trace_file, "0x%02x 0x%08x 0x%08x %04d %-50s %s\n",
                    core_id, timestamp, pc, count,
                    sink->dis[pc_word]->disassembly,
                    sink->dis[pc_word]->funcname);
            pc_word++;
        }
    } else {
        fprintf(sink->trace_file, "0x%02x 0x%08x 0x%08x %04d\n",
                core_id, timestamp, pc, count);
    }

    fflush(sink->trace_file);
}

static void write_stm_trace_to_file(uint32_t core_id, uint32_t timestamp,
                                    uint16_t id, uint32_t value)
{
    int do_print = 0;

    switch (id) {
    case 1:
        /* program terminated */
        fprintf(stm_trace_file, "[%d, %0d] [Program terminated.]\n",
                timestamp, core_id);
        break;
    case 4:
        /* simprint */
        if (value == '\n') {
            do_print = 1;
        } else {
            for (int i = 0; i < 49; i++) {
                if (stm_printf_buf[core_id][i] == '\0') {
                    if (i == 48) {
                        stm_printf_buf[core_id][46] = '.';
                        stm_printf_buf[core_id][47] = '.';
                        stm_printf_buf[core_id][48] = '.';
                        stm_printf_buf[core_id][49] = '\0';
                        do_print = 1;
                    } else {
                        stm_printf_buf[core_id][i] = value;
                        stm_printf_buf[core_id][i+1] = '\0';
                    }
                    break;
                }
            }
        }

        if (do_print) {
            fprintf(stm_trace_file, "[%d, %0d] %s\n", timestamp, core_id, stm_printf_buf[core_id]);
            stm_printf_buf[core_id][0] = '\0';
        }
        break;
    default:
        /* just a regular message */
        fprintf(stm_trace_file, "[%d, %0d] Event 0x%x: 0x%x\n",
                timestamp, core_id, id, value);
    }
    fflush(stm_trace_file);
}

static void write_noc_stats_to_file(int router_id, uint32_t timestamp,
                                    uint8_t *link_flit_count,
                                    int monitored_links)
{
    fprintf(nrm_stat_file, "0x%02x 0x%08x", router_id, timestamp);
    for (int i=0; i<monitored_links; i++) {
        fprintf(nrm_stat_file, " %03u", link_flit_count[i]);
    }
    fprintf(nrm_stat_file, "\n");
    fflush(nrm_stat_file);
}

int register_itm_trace(int core_id, char* trace_file_path,
                       int add_disassembly,
                       char* elf_file_path)
{
    int rv;

    struct itm_sink *sink = itm_sinks[core_id];
    if (!sink) {
        /* create new itm_sink */
        sink = calloc(1, sizeof(struct itm_sink));
        if (!sink) {
            fprintf(stderr, "Unable to allocate memory for itm_sink.\n");
            exit(ENOMEM);
        }
        itm_sinks[core_id] = sink;
    } else {
        /* reuse existing itm_sink; reset all data fields */
        if (sink->trace_file) {
            fclose(sink->trace_file);
            sink->trace_file = NULL;
        }

        if (sink->dis) {
            free_disassembly(sink->dis, sink->dis_instr_count);
        }
        sink->dis_instr_count = 0;
    }

    /* trace file */
    FILE *trace_file = fopen(trace_file_path, "w");
    if (!trace_file) {
        fprintf(stderr, "Opening instruction trace file for core %d failed: %s (%d)\n",
                core_id, strerror(errno), errno);
        return -1;
    }
    sink->trace_file = trace_file;
    sink->do_trace = 1;

    /* disassembly */
    if (add_disassembly && elf_file_path) {
        rv = new_disassembly(elf_file_path, &sink->dis, &sink->dis_instr_count);
        if (rv < 0) {
            sink->add_disassembly = 0;
            fprintf(stderr, "Unable to create disassembly. Creating raw "
                            "instruction trace instead.\n");
        } else {
            sink->add_disassembly = 1;
        }
    } else {
        sink->add_disassembly = 0;
        sink->dis = NULL;
        sink->dis_instr_count = 0;
    }

    if (!itm_callback_registered) {
        optimsoc_itm_register_callback(ctx, &write_itm_trace_to_file);
        itm_callback_registered = 1;
    }
    printf("Starting to log instruction traces for core %d to %s\n", core_id,
           trace_file_path);
    return 0;
}

int log_stm_trace(char* filename)
{
    if (stm_trace_file) {
        fclose(stm_trace_file);
    }

    stm_trace_file = fopen(filename, "w");
    if (!stm_trace_file) {
        printf("Opening STM trace file failed: %s (%d)\n",
               strerror(errno), errno);
        return -1;
    }

    stm_printf_buf = calloc(max_core_id + 1, sizeof(char*));
    for (unsigned int i = 0; i < max_core_id + 1; i++) {
        stm_printf_buf[i] = calloc(50, sizeof(char));
    }

    optimsoc_stm_register_callback(ctx, &write_stm_trace_to_file);
    printf("Starting to log STM trace to %s\n", filename);
    return 0;
}


static int log_noc_stats(char* filename)
{
    if (nrm_stat_file) {
        fclose(nrm_stat_file);
    }

    nrm_stat_file = fopen(filename, "w");
    if (!nrm_stat_file) {
        printf("Opening NRM statistics file failed: %s (%d)\n",
               strerror(errno), errno);
        return -1;
    }

    optimsoc_nrm_register_callback(ctx, &write_noc_stats_to_file);
    printf("Starting to log NoC statistics to %s\n", filename);
    return 0;
}

static void display_clkstat(void)
{
    uint32_t sys_clk;
    uint32_t sys_clk_halted;

    /*
     * Currently we don't have a separate virtual channel for control messages.
     * If the Debug NoC has high load, e.g. when instruction traces are running,
     * the register writes take a very long time to get through.
     * As a temporary workaround, we stall the CPUs, causing all instruction
     * traces to stop, wait a bit and restart the CPUs.
     */
    optimsoc_cpu_stall(ctx, 1);
    sleep(1);

    int rv = optimsoc_read_clkstats(ctx, &sys_clk, &sys_clk_halted);
    if (rv < 0) {
        fprintf(stderr, "Unable to read clock statistics from system\n");
        return;
    }

    optimsoc_cpu_stall(ctx, 0);

    double clk_is_halted_ratio = ((double)sys_clk_halted / (double)sys_clk) *
                                 100.0;

    printf("Clock was halted %u of %u cycles (%.04lf %%)\n",
           sys_clk_halted, sys_clk, clk_is_halted_ratio);
}

static void display_help(void)
{
    printf("Usage: optimsoc_cli [OPTIONS]\n"
           "Command-line client to control an OpTiMSoC system.\n"
           "\n"
           "-i, --interactive  interactive mode\n"
           "-b, --backend      select backend (see below)\n"
           "-o, --options      specify backend options (see below)\n"
           "                   Each option is a key-value pair in the form of\n"
           "                   key=value;key2=value2;...\n"
           "-h, --help         display this help and exit\n"
           "-v, --version      output version information and exit\n"
           "\n"
           "Available backends: dbgnoc, simtcp\n"
           "\n"
           "Backend options:\n"
           "dbgnoc: Connect over the Debug NoC infrastructure\n"
           " conn  Connection Type (usb or tcp)\n"
           " host  Hostname (only for conn=tcp, default: localhost)\n"
           " port  Port (only for conn=tcp, default: 22000)\n"
           "\n"
           "simtcp: Connect to a simulation using TCP (e.g. SystemC without debug system)\n"
           " host  Hostname (default: localhost)\n"
           " port  Port (default: 23000)\n"
           "\n"
           "Example:\n"
           "Connect over USB to a FPGA-board running OpTiMSoC:\n"
           "  optimsoc_cli -i -bdbgnoc -oconn=usb\n"
           "Connect over TCP to a system simulation with debug emulator:\n"
           "  optimsoc_cli -i -bdbgnoc -oconn=tcp,host=localhost,port=23000\n");
}

static void display_interactive_help(void)
{
    printf("Available commands:\n"
            "start\n"
            "   Start system\n"
            "cpureset\n"
            "   Reset all CPUs (use start to re-enable CPUs)\n"
            "reset\n"
            "   Reset the whole system, including the debug system\n"
            "clkstat\n"
            "   Read clock statistics\n"
            "mem_write FILE MEMORY_ID [BASE_ADDR]\n"
            "   write a memory dump from FILE to memory MEMORY_ID, \n"
            "   starting at address 0xBASE_ADDR\n"
            "mem_init FILE MEMORY_ID\n"
            "   initialize one or many memories with FILE\n"
            "   MEMORY_ID can be a single memory, e.g. 0 or a range of \n"
            "      memories, e.g. 0-3.\n"
            "log_raw_instruction_trace CORE_ID OUT_FILE\n"
            "   write an instruction trace for CPU CORE_ID to OUT_FILE\n"
            "log_dis_instruction_trace CORE_ID ELF_FILE OUT_FILE\n"
            "   write an instruction trace including disassembly for the "
                "program ELF_FILE \n"
            "   to OUT_FILE\n"
            "log_stm_trace FILE\n"
            "   write an STM trace for all CPUs to FILE\n"
            "log_noc_stats FILE\n"
            "   write NoC link statistics to FILE\n"
            "nrm_set_sample_interval SAMPLE_INTERVAL\n"
            "   set the NRM sample interval to SAMPLE_INTERVAL clock cycles\n"
            "quit\n"
            "   Leave\n");
}

int main(int argc, char *argv[])
{
    int c;
    char *optionstring;
    struct optimsoc_backend_option *options = 0;
    int num_options = 0;
    int interactive_mode = 0;
    char *script = NULL;
    optimsoc_backend_id backend = OPTIMSOC_BACKEND_DBGNOC;

    itm_callback_registered = 0;

    /* setup SIGINT (CTRL-C) signal handler */
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &sighandler;
    sigaction(SIGINT, &act, NULL);

    while (1) {
        static struct option long_options[] = {
            {"interactive", no_argument,       0, 'i'},
            {"script",      required_argument, 0, 's'},
            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'v'},
            {"backend",     required_argument, 0, 'b'},
            {"options",     required_argument, 0, 'o'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long(argc, argv, "is:vhb:o:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            /* If this option set a flag, do nothing else now.   */
            if (long_options[option_index].flag != 0) {
                break;
            }
            break;
        case 'b':
            if (strcmp(optarg, "dbgnoc") == 0) {
                backend = OPTIMSOC_BACKEND_DBGNOC;
            } else if (strcmp(optarg, "simtcp") == 0) {
                backend = OPTIMSOC_BACKEND_SIMTCP;
            }
            break;
        case 'o':
            optionstring = optarg;
            parse_options(optionstring, &options, &num_options);
            break;
        case 'i':
            interactive_mode = 1;
            break;
        case 's':
            script = optarg;
            break;
        case 'v':
            printf("liboptimsochost version %s\n", optimsoc_get_version_string());
            return EXIT_SUCCESS;
        case 'h':
            display_help();
            return EXIT_SUCCESS;
        default:
            display_help();
            return EXIT_FAILURE;
        }
    }

    if (script && (interactive_mode > 0)) {
        printf("Cannot run interactive and script at same time.\n");
    }

    /* connect to target system */
    connect(backend, num_options, options);

    /* run in interactive mode */
    if (interactive_mode) {
        char *line = 0;
        while (1) {
            if (line) {
                free(line);
            }
            line = readline("OpTiMSoC> ");
            add_history(line);

            char *cmd = strtok(line, " ");

            if (!cmd) {
                continue;
            }

            if (!strcmp(cmd, "quit")) {
                disconnect();
                break;
            } else if (!strcmp(cmd, "start")) {
                optimsoc_cpu_start(ctx);
            } else if (!strcmp(cmd, "cpureset")) {
                optimsoc_cpu_reset(ctx);
            } else if (!strcmp(cmd, "reset")) {
                optimsoc_reset(ctx);
            } else if (!strcmp(cmd, "clkstat")) {
                display_clkstat();
            } else if (!strcmp(cmd, "log_raw_instruction_trace")) {
                char* tmp;
                char* endptr;

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("CORE_ID argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                errno = 0;
                int core_id = strtol(tmp, &endptr, 10);
                if (endptr == tmp || errno != 0) {
                    printf("CORE_ID argument invalid.\n");
                    display_interactive_help();
                    continue;
                }

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("OUT_FILE argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                register_itm_trace(core_id, tmp, 0, NULL);

            } else if (!strcmp(cmd, "log_dis_instruction_trace")) {
                char* tmp;
                char* endptr;

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("CORE_ID argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                errno = 0;
                int core_id = strtol(tmp, &endptr, 10);
                if (endptr == tmp || errno != 0) {
                    printf("CORE_ID argument invalid.\n");
                    display_interactive_help();
                    continue;
                }

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("ELF_FILE argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                char* elf_file_path = strdup(tmp);

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("OUT_FILE argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                char* trace_file_path = strdup(tmp);
                register_itm_trace(core_id, trace_file_path, 1, elf_file_path);

                free(trace_file_path);
                free(elf_file_path);

            } else if (!strcmp(cmd, "log_stm_trace")) {
                char* tmp;

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("FILE argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                log_stm_trace(tmp);

            } else if (!strcmp(cmd, "log_noc_stats")) {
                char* tmp;

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("FILE argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                log_noc_stats(tmp);

            } else if (!strcmp(cmd, "mem_write")) {
                char* endptr;
                char* tmp;

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("FILE argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                char* file = strdup(tmp);

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("MEMORY_ID argument missing.\n");
                    display_interactive_help();
                    free(file);
                    continue;
                }
                errno = 0;
                int memory_id = strtol(tmp, &endptr, 10);
                if (endptr == tmp || errno != 0) {
                    printf("MEMORY_ID argument invalid.\n");
                    display_interactive_help();
                    free(file);
                    continue;
                }

                tmp = strtok(NULL, " ");
                int base_addr = 0;
                if (tmp) {
                    errno = 0;
                    base_addr = strtol(tmp, &endptr, 16);
                    if (endptr == tmp || errno != 0) {
                        printf("BASE_ADDR argument invalid.\n");
                        display_interactive_help();
                        free(file);
                        continue;
                    }
                }

                mem_write(memory_id, file, base_addr);
                free(file);

            } else if (!strcmp(cmd, "mem_init")) {
                char* endptr;
                char* tmp;

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("FILE argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                char* file = strdup(tmp);

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("MEMORY_ID argument missing.\n");
                    display_interactive_help();
                    goto mem_init_free;
                }

                int is_range = 0;
                int range_delim_pos = 0;
                for (unsigned int cpos = 0; cpos < strlen(tmp); cpos++) {
                    if (tmp[cpos] == '-') {
                        is_range = 1;
                        range_delim_pos = cpos;
                        tmp[cpos] = '\0';
                    }

                    if (!isdigit(tmp[cpos]) && tmp[cpos] != '-' && tmp[cpos] != '\0') {
                        printf("Unable to parse MEMORY_ID argument. Invalid character '%c' found.", tmp[cpos]);
                        display_interactive_help();
                        goto mem_init_free;
                    }
                }

                if (!is_range) {
                    errno = 0;
                    unsigned int memory_id = strtol(tmp, &endptr, 10);
                    if (endptr == tmp || errno != 0) {
                        printf("Unable to parse MEMORY_ID argument. Not a number.\n");
                        display_interactive_help();
                        goto mem_init_free;
                    }

                    mem_init(&memory_id, 1, file);
                } else {
                    int range_start = strtol(tmp, &endptr, 10);
                    if (endptr == tmp || errno != 0) {
                        printf("Unable to parse MEMORY_ID argument. Start of range is not a number.\n");
                        display_interactive_help();
                        goto mem_init_free;
                    }
                    int range_end = strtol(&tmp[range_delim_pos+1], &endptr, 10);
                    if (endptr == tmp || errno != 0) {
                        printf("Unable to parse MEMORY_ID argument. End of range is not a number.\n");
                        display_interactive_help();
                        goto mem_init_free;
                    }

                    if (range_end < range_start) {
                        printf("Unable to parse MEMORY_ID argument. Range end smaller than range start!.\n");
                        display_interactive_help();
                        goto mem_init_free;
                    }

                    unsigned int* memory_ids = calloc(range_end - range_start + 1, sizeof(unsigned int));
                    int i = 0;
                    for (int memory_id = range_start; memory_id <= range_end; memory_id++) {
                        memory_ids[i++] = memory_id;
                    }
                    mem_init(memory_ids, range_end - range_start + 1, file);
                }

mem_init_free:
                free(file);
            } else if (!strcmp(cmd, "nrm_set_sample_interval")) {
                char* endptr;
                char* tmp;
                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("SAMPLE_INTERVAL argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                errno = 0;
                int sample_interval = strtol(tmp, &endptr, 10);
                if (endptr == tmp || errno != 0) {
                    printf("SAMPLE_INTERVAL argument invalid.\n");
                    display_interactive_help();
                    continue;
                }

                optimsoc_nrm_set_sample_interval(ctx, sample_interval);

            } else if (!strcmp(cmd, "exec")) {
                char* escript;
                escript = strtok(NULL, " ");
                if (!script) {
                    printf("EXEC script missing.\n");
                    display_interactive_help();
                    continue;
                }
              execscript(escript);
          } else {
                printf("Unknown command \"%s\".\n", cmd);
                display_interactive_help();
            }
        }
        if (line) {
            free(line);
        }
    } else if (script) {
        execscript(script);
    }



    return EXIT_SUCCESS;
}
