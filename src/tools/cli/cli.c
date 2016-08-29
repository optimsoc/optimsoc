/* Copyright (c) 2016 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ============================================================================
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

#include <opensocdebug.h>

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "cli.h"

#include "terminal.h"

#include "help_strings.h"

#define PRINT_HELP(name) \
    fputs(help_ ## name, stderr);

#define CHECK_MATCH(input, string) \
        (input && !strncmp(input, string, strlen(string)+1))

static int interpret(struct osd_context *ctx, char *line) {
    char *cmd = strtok(line, " ");

    if (!cmd) {
        return 0;
    }

    if (CHECK_MATCH(cmd, "help") ||
            CHECK_MATCH(cmd, "h")) {
        PRINT_HELP(cmd);
    } else if (CHECK_MATCH(cmd, "quit") ||
            CHECK_MATCH(cmd, "q") ||
            CHECK_MATCH(cmd, "exit")) {
        return 1;
    } else if (CHECK_MATCH(cmd, "reset")) {
        int haltcpus = 0;
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            PRINT_HELP(reset);
        } else if (CHECK_MATCH(subcmd, "-halt")) {
            haltcpus = 1;
        }

        osd_reset_system(ctx, haltcpus);
    } else if (CHECK_MATCH(cmd, "start")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            PRINT_HELP(start);
        } else if (subcmd) {
            fprintf(stderr, "No parameters accepted or unknown subcommand: %s", subcmd);
            PRINT_HELP(start);
        } else {
            osd_start_cores(ctx);
        }
    } else if (CHECK_MATCH(cmd, "mem")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            PRINT_HELP(mem);
        } else if (CHECK_MATCH(subcmd, "test")) {
            memory_tests(ctx);
        } else if (CHECK_MATCH(subcmd, "loadelf")) {
            subcmd = strtok(NULL, " ");

            if (CHECK_MATCH(subcmd, "help")) {
                PRINT_HELP(mem_loadelf);
                return 0;
            } else if (!subcmd){
                fprintf(stderr, "Missing filename\n");
                PRINT_HELP(mem_loadelf);
                return 0;
            }
            char *file = subcmd;
            char *smem = strtok(NULL, " ");

            if (!smem) {
                fprintf(stderr, "Missing memory id\n");
                PRINT_HELP(mem_loadelf);
                return 0;
            }

            errno = 0;
            unsigned int mem = strtol(smem, 0, 0);
            if (errno != 0) {
                fprintf(stderr, "Invalid memory id: %s\n", smem);
                PRINT_HELP(mem_loadelf);
                return 0;
            }

            int verify = 0;
            char *option = strtok(NULL, " ");
            if (CHECK_MATCH(option, "-verify")) {
                verify = 1;
            }

            printf("Verify: %d\n", verify);

            osd_memory_loadelf(ctx, mem, file, verify);
        }
    } else if (CHECK_MATCH(cmd, "stm")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            PRINT_HELP(stm);
        } else if (CHECK_MATCH(subcmd, "log")) {
            subcmd = strtok(NULL, " ");

            if (CHECK_MATCH(subcmd, "help")) {
                PRINT_HELP(stm_log);
                return 0;
            } else if (!subcmd){
                fprintf(stderr, "Missing filename\n");
                PRINT_HELP(stm_log);
                return 0;
            }
            char *file = subcmd;
            char *sstm = strtok(NULL, " ");

            if (!sstm) {
                fprintf(stderr, "Missing STM id\n");
                PRINT_HELP(stm_log);
                return 0;
            }

            errno = 0;
            unsigned int stm = strtol(sstm, 0, 0);
            if (errno != 0) {
                fprintf(stderr, "Invalid STM id: %s\n", sstm);
                PRINT_HELP(stm_log);
                return 0;
            }
            osd_stm_log(ctx, stm, file);
        } else {
            PRINT_HELP(stm);
        }
    } else if (CHECK_MATCH(cmd, "ctm")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            PRINT_HELP(ctm);
        } else if (CHECK_MATCH(subcmd, "log")) {
            subcmd = strtok(NULL, " ");

            if (CHECK_MATCH(subcmd, "help")) {
                PRINT_HELP(ctm_log);
                return 0;
            } else if (!subcmd){
                fprintf(stderr, "Missing filename\n");
                PRINT_HELP(ctm_log);
                return 0;
            }
            char *file = subcmd;
            char *sctm = strtok(NULL, " ");

            if (!sctm) {
                fprintf(stderr, "Missing CTM id\n");
                PRINT_HELP(ctm_log);
                return 0;
            }

            errno = 0;
            unsigned int ctm = strtol(sctm, 0, 0);
            if (errno != 0) {
                fprintf(stderr, "Invalid CTM id: %s\n", sctm);
                PRINT_HELP(ctm_log);
                return 0;
            }

            char *elffile = strtok(NULL, " ");
            if (elffile) {
                osd_ctm_log(ctx, ctm, file, elffile);
            } else {
                osd_ctm_log(ctx, ctm, file, 0);
            }
        } else {
            PRINT_HELP(ctm);
        }
    } else if (CHECK_MATCH(cmd, "terminal")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            PRINT_HELP(terminal);
            return 0;
        }

        if (!subcmd) {
            fprintf(stderr, "Missing id\n");
            PRINT_HELP(terminal);
            return 0;
        }

        errno = 0;
        unsigned int id = strtol(subcmd, 0, 0);
        if (errno != 0) {
            fprintf(stderr, "Invalid id: %s\n", subcmd);
            PRINT_HELP(terminal);
            return 0;
        }

        if (!osd_module_is_terminal(ctx, id)) {
            fprintf(stderr, "No terminal at this id: %d\n", id);
            PRINT_HELP(terminal);
            return 0;
        }

        struct terminal *term_arg;
        terminal_open(ctx, id, &term_arg);

        osd_module_claim(ctx, id);
        osd_module_register_handler(ctx, id, OSD_EVENT_PACKET,
                                    term_arg, terminal_ingress);

        osd_module_unstall(ctx, id);
    } else if (CHECK_MATCH(cmd, "wait")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            PRINT_HELP(wait);
            return 0;
        }

        if (subcmd) {
            errno = 0;
            unsigned int sec = strtol(subcmd, 0, 0);
            if (errno != 0) {
                fprintf(stderr, "No valid seconds given: %s\n", subcmd);
                return 0;
            }

            sleep(sec);
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        PRINT_HELP(cmd);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    struct osd_context *ctx;

    int c;
    char *source = NULL;
    int batch = 0;

    while (1) {
        static struct option long_options[] = {
            {"help",        no_argument,       0, 'h'},
            {"source",      required_argument, 0, 's'},
            {"batch",       required_argument, 0, 'b'},
            {"python",      no_argument,       0, '0'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long(argc, argv, "hs:b:0", long_options, &option_index);
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
        case 's':
            source = optarg;
            break;
        case 'b':
            source = optarg;
            batch = 1;
            break;
        case '0':
            fprintf(stderr, "Python not supported\n");
            break;
        case 'h':
            PRINT_HELP(param);
            return 0;
        default:
            PRINT_HELP(param);
            return -1;
        }
    }

    osd_new(&ctx, OSD_MODE_DAEMON, 0, 0);

    if (osd_connect(ctx) != OSD_SUCCESS) {
        fprintf(stderr, "Cannot connect to Open SoC Debug daemon\n");
        exit(1);
    }

    char *line = 0;

    if (source != NULL) {
        FILE* fp = fopen(source, "r");
        if (fp == NULL) {
            fprintf(stderr, "cannot open file '%s'\n", source);
            return 1;
        }

        size_t n = 64;
        line = malloc(64);
        ssize_t len;
        while ((len = getline(&line, &n, fp)) > 0) {
            if (line[len-1] == '\n') {
                line[len-1] = 0;
            }
            printf("execute: %s\n", line);
            int rv = interpret(ctx, line);
            if (rv != 0) {
                return rv;
            }
        }

        free(line);
        line = 0;

        if (batch != 0) {
            return 0;
        }
    }

    while (1) {
        if (line) {
            free(line);
        }
        line = readline("osd> ");
        add_history(line);

        int rv = interpret(ctx, line);
        if (rv != 0) {
            return rv;
        }
    }
}
