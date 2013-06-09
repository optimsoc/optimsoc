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

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <optimsochost/liboptimsochost.h>

struct optimsoc_ctx *ctx;
FILE *trace_file;
FILE *nrm_stat_file;
FILE *stm_trace_file;

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

    /* show system information */
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
    }
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

    if (trace_file) {
        fclose(trace_file);
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

static int mem_write(int mem_tile_id, const char* path, int base_address)
{
    printf("Trying to write %s to memory tile %d starting at 0x%x.\n",
           path, mem_tile_id, base_address);

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

    int rv = optimsoc_mem_write(ctx, mem_tile_id, base_address, data_buf,
                                stat_buf.st_size);
    if (rv < 0) {
        printf("optimsoc_mem_write() not successful: %d\n", rv);
        return -1;
    }
    return 0;
}

static int mem_init(int mem_tile_id, const char* path)
{
    printf("Trying to initialize memory tile %d with %s\n",
           mem_tile_id, path);

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

    int rv = optimsoc_mem_init(ctx, mem_tile_id, data_buf,
                               stat_buf.st_size);
    if (rv < 0) {
        printf("optimsoc_mem_init() not successful: %d\n", rv);
        return -1;
    }
    printf("Memory tile successfully initialized.\n");
    return 0;
}

static void write_trace_to_file(int core_id, uint32_t timestamp,
                                uint32_t pc, int count)
{
    fprintf(trace_file, "0x%02x 0x%08x 0x%08x %04d\n",
            core_id, timestamp, pc, count);
    fflush(trace_file);
}

static void write_stm_trace_to_file(uint32_t core_id, uint32_t timestamp,
                                    uint16_t id, uint32_t value)
{
    static int nl_printed = 0;
    switch (id) {
    case 1:
        /* program terminated */
        fprintf(stm_trace_file, "[%d, %0d] [Program terminated.]",
                timestamp, core_id);
        break;
    case 4:
        /* simprint */
        if (!nl_printed) {
            fprintf(stm_trace_file, "[%d, %0d] ", timestamp, core_id);
            nl_printed = 1;
        }
        fprintf(stm_trace_file, "%c", value);
        if (value == '\n') {
            nl_printed = 0;
        }
        break;
    default:
        /* just a regular message */
        fprintf(stm_trace_file, "[%d, %0d] Event 0x%x: 0x%x",
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

static int log_instruction_trace(char* filename)
{
    if (trace_file) {
        fclose(trace_file);
    }

    trace_file = fopen(filename, "w");
    if (!trace_file) {
        printf("Opening instruction trace file failed: %s (%d)\n",
               strerror(errno), errno);
        return -1;
    }

    optimsoc_itm_register_callback(ctx, &write_trace_to_file);
    printf("Starting to log instruction traces to %s\n", filename);
    return 0;
}

static int log_stm_trace(char* filename)
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
           "Connect over USB to a FPGA-board running OpTiMSoC:\n"
           "  optimsoc_cli -i -bdbgnoc -oconn=tcp,host=localhost,port=22000\n");
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
            "mem_write FILE MEM_TILE_ID [BASE_ADDR]\n"
            "   write a memory dump from FILE to memory tile MEM_TILE_ID, \n"
            "   starting at address 0xBASE_ADDR\n"
            "mem_init FILE MEM_TILE_ID\n"
            "   initialize a memory tile MEM_TILE_ID with FILE\n"
            "log_instruction_trace FILE\n"
            "   write an instruction trace for all CPUs to FILE\n"
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
    optimsoc_backend_id backend = OPTIMSOC_BACKEND_DBGNOC;

    trace_file = 0;

    /* setup SIGINT (CTRL-C) signal handler */
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &sighandler;
    sigaction(SIGINT, &act, NULL);

    while (1) {
        static struct option long_options[] = {
            {"interactive", no_argument,       0, 'i'},
            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'v'},
            {"backend",     required_argument, 0, 'b'},
            {"options",     required_argument, 0, 'o'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long(argc, argv, "ivhb:o:", long_options, &option_index);
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
            } else if (!strcmp(cmd, "log_instruction_trace")) {
                char* tmp;

                tmp = strtok(NULL, " ");
                if (!tmp) {
                    printf("FILE argument missing.\n");
                    display_interactive_help();
                    continue;
                }
                log_instruction_trace(tmp);

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
                    printf("MEM_TILE_ID argument missing.\n");
                    display_interactive_help();
                    free(file);
                    continue;
                }
                errno = 0;
                int mem_tile_id = strtol(tmp, &endptr, 10);
                if (endptr == tmp || errno != 0) {
                    printf("MEM_TILE_ID argument invalid.\n");
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

                mem_write(mem_tile_id, file, base_addr);
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
                    printf("MEM_TILE_ID argument missing.\n");
                    display_interactive_help();
                    free(file);
                    continue;
                }
                errno = 0;
                int mem_tile_id = strtol(tmp, &endptr, 10);
                if (endptr == tmp || errno != 0) {
                    printf("MEM_TILE_ID argument invalid.\n");
                    display_interactive_help();
                    free(file);
                    continue;
                }

                mem_init(mem_tile_id, file);
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

            } else {
                printf("Unknown command \"%s\".\n", cmd);
                display_interactive_help();
            }
        }
        if (line) {
            free(line);
        }
    }

    return EXIT_SUCCESS;
}
