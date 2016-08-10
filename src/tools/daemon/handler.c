#include <daemon.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

void handle_ingress(struct osd_context *ctx,
                    void* arg, uint16_t* packet) {
    int socket = (intptr_t) arg;
    size_t size = (packet[0]+1);

    int rv = send(socket, packet, size*2, 0);
    assert(rv == (int) size*2);
}
