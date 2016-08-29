#include "daemon.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>

int main(int argc, char* argv[]) {
    INFO("Open SoC Debug Daemon");

    struct osd_context *ctx;

    if (argc < 2) {
        ERR("Usage: opensocdebugd <backend> [<backend_option> ..]");
    }

    INFO("Backend: %s", argv[1]);

    struct osd_mode_option *options = calloc(argc-1, sizeof(struct osd_mode_option));
    options[0].name = "backend";
    options[0].value = argv[1];

    for (int i = 2; i < argc; i++) {
        options[i-1].name = "backend_option";
        options[i-1].value = argv[i];
    }

    osd_new(&ctx, OSD_MODE_STANDALONE, argc-1, options);

    int rv = osd_connect(ctx);
    if (rv != OSD_SUCCESS) {
      ERR("Cannot connect");
    }

    uint16_t systemid, num_modules;
    osd_get_system_identifier(ctx, &systemid);
    INFO("System ID: %04x", systemid);

    osd_get_num_modules(ctx, &num_modules);
    INFO("%d debug modules found:", num_modules);

    for (int i = 0; i < num_modules; i++) {
        char *name;

        osd_get_module_name(ctx, i, &name);

        INFO(" [%d]: %s", i, name);
        osd_print_module_info(ctx, i, stdout, 3);

        free(name);
    }

    struct sockaddr_in serveraddr;
    int connsocket;

    connsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (connsocket < 0) {
        return -1;
    }

    int reuse = 1;
    setsockopt(connsocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;

    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(7450);

    if(bind(connsocket, (const struct sockaddr*) &serveraddr,
            sizeof(serveraddr)) < 0) {
        ERR("Cannot bind to socket");
    }

    if (listen(connsocket, 1) < 0) {
        ERR("ERROR on listen");
    }

    while(1) {
        int clientsocket;
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen;

        INFO("Wait for connection");

        clientsocket = accept(connsocket,
                              (struct sockaddr *) &clientaddr,
                              &clientaddrlen);
        if (clientsocket < 0) {
            ERR("Error in connection");
        }

        INFO("Connected");

        while (1) {
            uint16_t *packet = malloc(64);

            int rv = recv(clientsocket, packet, 2, 0);
            if (rv == 0) {
                break;
            }
            assert(rv == 2);

            size_t size = packet[0];

            rv = recv(clientsocket, &packet[1], size*2, 0);
            if (rv == 0) {
                break;
            }
            assert(rv == (int) size*2);

            if (packet[1] == 0xffff) {
                // This is a daemon packet
                switch (packet[2]) {
                    case OSD_DP_CLAIM:
                        osd_module_claim(ctx, packet[3]);
                        osd_module_register_handler(ctx, packet[3], OSD_EVENT_PACKET,
                                                    (void*) (intptr_t) clientsocket, handle_ingress);
                        osd_module_register_handler(ctx, packet[3], OSD_EVENT_TRACE,
                                                    (void*) (intptr_t) clientsocket, handle_ingress);
                        break;
                    default:
                        break;
                }
            } else if ((packet[2] & 0xc000) == 0) {
                // This is a register access. We cannot plainly access
                // the interface, but need to call the actual functions
                osd_reg_access(ctx, packet);

                rv = send(clientsocket, packet, (packet[0]+1)*2, 0);
            } else if ((packet[2] & 0xc000) == 0x4000) {
                osd_send_packet(ctx, packet);
            } else {
                WARN("Dropped invalid egress packet\n");
            }
        }

        INFO("Disconnect");
    }
}
