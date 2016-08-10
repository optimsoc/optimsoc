#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <opensocdebug.h>

#include <stdio.h>
#include <err.h>

#include "daemon-packets.h"

#define INFO(format, ...) fprintf (stdout, format "\n", ##__VA_ARGS__)
#define WARN(format, ...) warnx(format, ##__VA_ARGS__)
#define ERR(format, ...) errx(1, format, ##__VA_ARGS__)

void handle_ingress(struct osd_context *ctx, void* arg,
                    uint16_t* packet);

#endif
